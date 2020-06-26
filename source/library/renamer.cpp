#include <dtool/renamer.hpp>

#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <filesystem>
#include <algorithm>

namespace {
	template<class... T> struct OverloadHelper : T... { using T::operator()...; };
	template<class... T> OverloadHelper(T...) -> OverloadHelper<T...>;
}

namespace dtool::renamer {
	namespace {
		auto getPureName(std::string_view name) -> std::string {
			auto separatorPosition = name.find_last_of('.');
			if (separatorPosition == std::string_view::npos) {
				return std::string(name);
			}
			return std::string(name.begin(), name.begin() + separatorPosition);
		}

		auto getExtension(std::string_view name) -> std::string {
			auto separatorPosition = name.find_last_of('.');
			if (separatorPosition == std::string_view::npos) {
				using namespace std::literals::string_literals;
				return ""s;
			}
			return std::string(name.begin() + separatorPosition + 1, name.end());
		}

		auto bracedSpecialPattern(std::string_view rawSpecialPattern) -> std::string {
			return "{" + std::string(rawSpecialPattern) + "}";
		}
	} // namespace

	namespace pattern::element {
		namespace {
			class Static: public CloneHelper<Static> {
				public: using Self = Static;
				private: std::string m_raw;
				public: Static(std::string raw): m_raw(std::move(raw)) {
				}
				public: auto generate(std::string_view originalName, ItemIndex index) const -> std::string override {
					return this->m_raw;
				}
				public: auto raw() const -> std::string override {
					std::string result;
					for (std::string::size_type current = 0; ; ) {
						auto next = this->m_raw.find('{', current);
						if (next == std::string::npos) {
							result += std::string(this->m_raw.begin() + current, this->m_raw.end());
							break;
						}
						result += std::string(this->m_raw.begin() + current, this->m_raw.begin() + next) + "{{";
						current = next + 1;
					};
					return result;
				}
			};
		}
	}

	Pattern::Pattern(std::string_view rawPattern) {
		auto begin = rawPattern.begin();
		bool braceEncountered = false;
		std::string cache;
		for (auto current = begin; current != rawPattern.end(); ++current) {
			if (braceEncountered) {
				braceEncountered = false;
				if (*current != '{') {
					if (!cache.empty()) {
						this->append(pattern::element::Static(cache));
						cache.clear();
					}
					auto currentOffset = current - begin;
					auto rightBraceOffset = rawPattern.find('}', currentOffset);
					if (rightBraceOffset == std::string_view::npos) {
						throw BadPattern("Invalid pattern: brace not closed.");
					}
					this->parseSpecialPattern(std::string_view(current, rightBraceOffset - currentOffset));
					current = begin + rightBraceOffset;
					continue;
				}
			} else if (*current == '{') {
				braceEncountered = true;
				continue;
			}
			cache.push_back(*current);
		}
		if (braceEncountered) {
			throw BadPattern("Invalid pattern: brace not closed.");
		}
		if (!cache.empty()) {
			this->append(pattern::element::Static(cache));
		}
	}

	auto Pattern::parseSpecialPattern(std::string_view rawSpecialPattern) -> void {
		switch (rawSpecialPattern[0]) {
			case 'i': {
				this->append(pattern::element::Quick([](std::string_view originalName, ItemIndex index) -> std::string {
					return index.toString();
				}, bracedSpecialPattern(rawSpecialPattern)));
				break;
			}
			case 'o': {
				this->append(pattern::element::Quick([](std::string_view originalName, ItemIndex index) -> std::string {
					return std::string(originalName);
				}, bracedSpecialPattern(rawSpecialPattern)));
				break;
			}
			case 'p': {
				this->append(pattern::element::Quick([](std::string_view originalName, ItemIndex index) -> std::string {
					return getPureName(originalName);
				}, bracedSpecialPattern(rawSpecialPattern)));
				break;
			}
			case 'e': {
				this->append(pattern::element::Quick([](std::string_view originalName, ItemIndex index) -> std::string {
					return getExtension(originalName);
				}, bracedSpecialPattern(rawSpecialPattern)));
				break;
			}
			case 'c': {
				if (rawSpecialPattern.size() <= 1) {
					break;
				}
				this->append(pattern::element::Quick([
					set = std::string(rawSpecialPattern.begin() + 1, rawSpecialPattern.end())
				](std::string_view originalName, ItemIndex index) -> std::string {
					return std::string(1, set[index.underlyingIndex() % set.size()]);
				}, bracedSpecialPattern(rawSpecialPattern)));
				break;
			}
		}
	}

	namespace {
		auto regeneratePreviews(Pattern const& pattern, Core::Previews& previews) {
			for (auto current = previews.begin(); current < previews.end(); ++current) {
				current->newName = pattern.generate(
					current->origin->filename().string(), ItemIndex::fromUnderlyingIndex(current - previews.begin())
				);
			}
		}

		auto insertOnePath(Core::Paths& paths, std::filesystem::path const& newPath) -> Core::Paths::iterator {
			std::filesystem::path uniformedPath;
			try {
				uniformedPath = std::filesystem::canonical(newPath);
			} catch (std::filesystem::filesystem_error) {
				return paths.end();
			}
			auto insertResult = paths.insert(std::move(uniformedPath));
			if (!insertResult.second) {
				return paths.end();
			}
			return insertResult.first;
		}
	} // namespace

	auto Core::interact(Pattern pattern, Self::Paths const& inputPaths) -> void {
		Self::Previews previews;
		Self::Paths uniformedPaths;
		std::vector<std::string> newNames;
		for (auto const& inputPath: inputPaths) {
			if (auto inserted = insertOnePath(uniformedPaths, inputPath); inserted != uniformedPaths.end()) {
				previews.push_back(Preview { inserted });
			}
		}
		regeneratePreviews(pattern, previews);
		for (; ; ) {
			Action action = this->m_handler(pattern, previews);
			if (std::visit(OverloadHelper {
				[](decltype(Core::NO_OP)) -> bool {
					return false;
				},
				[&previews = std::as_const(previews)](DoneChoice doneChoice) -> bool {
					if (doneChoice == DoneChoice::CONFIRM) {
						for (auto const& preview: previews) {
							std::filesystem::rename(*(preview.origin), preview.origin->parent_path() / preview.newName);
						}
					}
					return true;
				},
				[&pattern, &previews](Pattern const& newPattern) -> bool {
					pattern = newPattern;
					regeneratePreviews(pattern, previews);
					return false;
				},
				[&pattern = std::as_const(pattern), &previews](SwapInfo const& swapInfo) -> bool {
					std::swap(previews.at(swapInfo.left.underlyingIndex()), previews.at(swapInfo.right.underlyingIndex()));
					regeneratePreviews(pattern, previews);
					return false;
				},
				[&pattern = std::as_const(pattern), &previews](ReorderMethod reorderMethod) -> bool {
					switch(reorderMethod) {
						case Self::ReorderMethod::SORT_BY_MODIFIED_TIME: {
							std::sort(previews.begin(), previews.end(), [](Self::Preview const& left, Self::Preview const& right) -> bool {
								return std::filesystem::last_write_time(*(left.origin)) < std::filesystem::last_write_time(*(right.origin));
							});
							break;
						}
						case Self::ReorderMethod::REVERSE: {
							std::reverse(previews.begin(), previews.end());
							break;
						}
						case Self::ReorderMethod::SORT_BY_NAME:
						default: {
							std::sort(previews.begin(), previews.end(), [](Self::Preview const& left, Self::Preview const& right) -> bool {
								return *(left.origin) < *(right.origin);
							});
							break;
						}
					}
					regeneratePreviews(pattern, previews);
					return false;
				},
				[&pattern = std::as_const(pattern), &previews, &uniformedPaths](AddInfo const& addInfo) -> bool {
					if (auto inserted = insertOnePath(uniformedPaths, addInfo.path); inserted != uniformedPaths.end()) {
						auto id = static_cast<ItemIndex>(previews.size());
						previews.push_back(Preview { inserted, pattern.generate(inserted->filename().string(), id) });
					}
					return false;
				},
				[&pattern = std::as_const(pattern), &previews, &uniformedPaths](RemoveInfo const& removeInfo) -> bool {
					Self::Previews::size_type underlyingIndex = removeInfo.index.underlyingIndex();
					if (underlyingIndex < previews.size()) {
						uniformedPaths.erase(previews[underlyingIndex].origin);
						previews.erase(previews.begin() + underlyingIndex);
						for (auto current = underlyingIndex; current < previews.size(); ++current) {
							previews[current].newName = pattern.generate(
								previews[current].origin->filename().string(), ItemIndex::fromUnderlyingIndex(current)
							);
						}
					}
					return false;
				}
			}, action)) {
				return;
			}
		}
	}
} // namespace dtool::renamer
