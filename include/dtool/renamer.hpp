#ifndef DTOOL_RENAMER_HPP_INCLUDED
#	define DTOOL_RENAMER_HPP_INCLUDED 1

#	include <cstdlib>
#	include <cstddef>
#	include <string>
#	include <string_view>
#	include <vector>
#	include <set>
#	include <functional>
#	include <filesystem>
#	include <variant>
#	include <memory>
#	include <stdexcept>
#	include <iostream>
#	include <numeric>

namespace dtool::renamer {
	// using ItemIndex = std::size_t;

	class BadItemIndex: public std::runtime_error {
		public: BadItemIndex(): std::runtime_error("Not a valid item index") {
		}
		public: BadItemIndex(char const* message): std::runtime_error(message) {
		}
	};

	struct ItemIndex {
		public: using Self = ItemIndex;
		private: std::size_t m_index;
		public: constexpr explicit ItemIndex(std::size_t index = std::numeric_limits<std::ptrdiff_t>::max()): m_index(index) {
			if (index <= 0 || index > std::numeric_limits<std::ptrdiff_t>::max()) {
				throw BadItemIndex();
			}
		}
		public: constexpr auto underlyingIndex() const noexcept -> std::size_t {
			return this->m_index - 1;
		}
		public: static constexpr auto fromUnderlyingIndex(std::size_t index) noexcept -> Self {
			return Self(index + 1);
		}
		public: constexpr auto operator ++() -> Self& {
			if (this->m_index >= std::numeric_limits<std::ptrdiff_t>::max()) {
				throw BadItemIndex();
			}
			++this->m_index;
			return *this;
		}
		public: constexpr auto operator ++(int) -> Self {
			if (this->m_index >= std::numeric_limits<std::ptrdiff_t>::max()) {
				throw BadItemIndex();
			}
			Self result = *this;
			++*this;
			return result;
		}
		public: constexpr auto operator --() -> Self& {
			if (this->m_index <= 1) {
				throw BadItemIndex();
			}
			--this->m_index;
			return *this;
		}
		public: constexpr auto operator --(int) -> Self {
			if (this->m_index <= 1) {
				throw BadItemIndex();
			}
			Self result = *this;
			--*this;
			return result;
		}
		public: constexpr auto operator +=(std::ptrdiff_t offset) -> Self& {
			if (offset < 0 && -offset >= this->m_index) {
				throw BadItemIndex();
			}
			if (std::numeric_limits<std::ptrdiff_t>::max() - offset > this->m_index) {
				throw BadItemIndex();
			}
			this->m_index += offset;
			return *this;
		}
		public: constexpr auto operator -=(std::ptrdiff_t offset) -> Self& {
			if (offset == std::numeric_limits<std::ptrdiff_t>::min()) {
				throw BadItemIndex();
			}
			return (*this) += -offset;
		}
		public: auto toString() const -> std::string {
			return std::to_string(this->m_index);
		}
	};

	constexpr inline auto operator ""_ItemIndex(unsigned long long value) -> ItemIndex {
		return ItemIndex(value);
	}

	constexpr inline auto operator +(ItemIndex left, std::ptrdiff_t right) -> ItemIndex {
		return left += right;
	}

	constexpr inline auto operator +(std::ptrdiff_t left, ItemIndex right) -> ItemIndex {
		return right + left;
	}

	constexpr inline auto operator -(ItemIndex left, std::ptrdiff_t right) -> ItemIndex {
		return left -= right;
	}

	constexpr inline auto operator -(ItemIndex left, ItemIndex right) noexcept -> std::ptrdiff_t {
		return static_cast<std::ptrdiff_t>(left.underlyingIndex()) - static_cast<std::ptrdiff_t>(right.underlyingIndex());
	}

	constexpr inline auto operator ==(ItemIndex left, ItemIndex right) noexcept -> bool {
		return left.underlyingIndex() == right.underlyingIndex();
	}

	constexpr inline auto operator !=(ItemIndex left, ItemIndex right) noexcept -> bool {
		return !(left == right);
	}

	constexpr inline auto operator <(ItemIndex left, ItemIndex right) noexcept -> bool {
		return left.underlyingIndex() < right.underlyingIndex();
	}

	constexpr inline auto operator >(ItemIndex left, ItemIndex right) noexcept -> bool {
		return right < left;
	}

	constexpr inline auto operator <=(ItemIndex left, ItemIndex right) noexcept -> bool {
		return !(left > right);
	}

	constexpr inline auto operator >=(ItemIndex left, ItemIndex right) noexcept -> bool {
		return !(left < right);
	}

	inline auto operator >>(std::istream& in, ItemIndex& itemIndex) -> std::istream& {
		std::size_t index;
		in >> index;
		itemIndex = ItemIndex(index);
		return in;
	}

	inline auto operator <<(std::ostream& out, ItemIndex itemIndex) -> std::ostream& {
		return out << itemIndex.toString();
	}

	class BadPattern: public std::runtime_error {
		public: BadPattern(): BadPattern("Not a valid pattern") {
		}
		public: BadPattern(char const* message): std::runtime_error(message) {
		}
	};

	namespace pattern {
		class Element {
			public: using Self = Element;
			public: virtual auto generate(std::string_view, ItemIndex) const -> std::string = 0;
			public: virtual auto raw() const -> std::string = 0;
			public: virtual auto clone() const -> std::unique_ptr<Element> = 0;
		};

		namespace element {
			struct Holder {
				public: using Self = Holder;
				private: std::unique_ptr<pattern::Element> m_element;
				public: template <typename T> Holder(T element): m_element(std::make_unique<T>(std::move(element))) {
				}
				public: Holder(Self const& other): m_element(other.m_element->clone()) {
				}
				public: Holder(Self&& other) = default;
				public: Self& operator =(Self const& other) {
					m_element = other.m_element->clone();
					return *this;
				}
				public: Self& operator =(Self&& other) = default;
				public: auto generate(std::string_view originalName, ItemIndex index) const -> std::string {
					return this->m_element->generate(originalName, index);
				}
				public: auto raw() const -> std::string {
					return this->m_element->raw();
				}
			};

			template <typename T> class CloneHelper: public Element {
				public: using Self = CloneHelper<T>;
				public: using SpecificElement = T;
				public: auto clone() const -> std::unique_ptr<Element> override {
					return std::make_unique<SpecificElement>(static_cast<SpecificElement const&>(*this));
				}
			};

			class Quick: public CloneHelper<Quick> {
				public: using Self = Quick;
				public: using Generator = std::function<auto (std::string_view, ItemIndex) -> std::string>;
				public: using RawGetter = std::function<auto () -> std::string>;
				private: Generator m_generator;
				private: RawGetter m_rawGetter;
				public: Quick(
					Generator generator, RawGetter rawGetter
				): m_generator(std::move(generator)), m_rawGetter(std::move(rawGetter)) {
				}
				public: Quick(
					Generator generator, std::string raw
				): Quick(std::move(generator), [raw = std::move(raw)]() -> std::string {
					return raw;
				}) {
				}
				public: Quick(
					Generator generator, std::string_view raw
				): m_generator(std::move(generator)), m_rawGetter([raw = std::string(raw)]() -> std::string {
					return raw;
				}) {
				}
				public: auto generate(std::string_view originalName, ItemIndex index) const -> std::string override {
					return this->m_generator(originalName, index);
				}
				public: auto raw() const -> std::string override {
					return this->m_rawGetter();
				}
			};
		} // namespace element
	} // namespace pattern

	class Pattern {
		public: using Self = Pattern;
		private: using Elements = std::vector<pattern::element::Holder>;
		private: Elements m_elements;
		public: explicit Pattern(std::string_view rawPattern);
		public: auto generate(std::string_view originalName, ItemIndex index) const -> std::string {
			std::string result;
			for (auto const& element: this->m_elements) {
				result += element.generate(originalName, index);
			}
			return result;
		}
		public: auto raw() const -> std::string {
			std::string result;
			for (auto const& element: this->m_elements) {
				result += element.raw();
			}
			return result;
		}
		public: auto append(pattern::element::Holder element) -> void {
			this->m_elements.push_back(std::move(element));
		}
		public: template <typename T> auto append(T element) -> void {
			this->append(pattern::element::Holder(std::move(element)));
		}
		private: auto parseSpecialPattern(std::string_view rawSpecialPattern) -> void;
	};

	class Core {
		public: using Self = Core;
		public: using Paths = std::set<std::filesystem::path>;
		public: struct Preview {
			Core::Paths::iterator origin;
			std::string newName;
		};
		public: using Previews = std::vector<Preview>;
		enum class DoneChoice {
			CONFIRM,
			ABORT
		};
		public: static struct {
		} constexpr NO_OP = {};
		public: enum class ReorderMethod {
			SORT_BY_NAME,
			SORT_BY_MODIFIED_TIME,
			REVERSE
		};
		public: struct SwapInfo {
			ItemIndex left;
			ItemIndex right;
		};
		public: struct AddInfo {
			std::filesystem::path path;
		};
		public: struct RemoveInfo {
			ItemIndex index;
		};
		public: using Action = std::variant<decltype(NO_OP), DoneChoice, Pattern, SwapInfo, ReorderMethod, AddInfo, RemoveInfo>;
		public: using ActionHandler = std::function<auto (Pattern const&, Previews const&) -> Action>;
		private: ActionHandler m_handler;
		public: Core(ActionHandler handler): m_handler(handler) {
		}
		public: auto interact(Self::Paths const& inputPaths) -> void {
			this->interact(Pattern("{o}"), inputPaths);
		}
		public: auto interact(Pattern pattern = Pattern("{o}"), Self::Paths const& inputPaths = Self::Paths()) -> void;
	};
} // namespace dtool::renamer

#endif // ifndef DTOOL_DRENAME_HPP_INCLUDED
