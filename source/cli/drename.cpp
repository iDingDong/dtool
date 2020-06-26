#include <dtool/renamer.hpp>
#include <dtool/common.hpp>

#if defined(_WIN32)
#	include <windows.h>
#endif

#include <string>
#include <iostream>
#include <sstream>
#include <filesystem>
#include <stdexcept>
#include <queue>
#include <deque>

namespace {
	struct ConsoleGuard {
#if defined(_WIN32)
		private: HANDLE m_reservedStdoutHandle;
		private: DWORD m_reservedOutMode;
		public: ConsoleGuard() noexcept {
			this->m_reservedStdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
			if (this->m_reservedStdoutHandle == INVALID_HANDLE_VALUE) {
				exit(GetLastError());
			}
			if (!GetConsoleMode(this->m_reservedStdoutHandle, &(this->m_reservedOutMode))) {
				exit(GetLastError());
			}
			if (!SetConsoleMode(this->m_reservedStdoutHandle, this->m_reservedOutMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
				exit(GetLastError());
			}
		}

		public: ~ConsoleGuard() noexcept {
			if(!SetConsoleMode(this->m_reservedStdoutHandle, this->m_reservedOutMode)) {
				exit(GetLastError());
			}
		}
#endif
	};

	bool g_quiet = false;

	template <typename... T> auto standardOutput(T&&... toOutput) -> void {
		if (!g_quiet) {
			((std::cout << std::forward<T>(toOutput)), ...) << std::flush;
		}
	}

	template <typename... T> auto standardOutputHighlited(T&&... toOutput) -> void {
		standardOutput("\x1b[37;42m", std::forward<T>(toOutput)..., "\x1b[0m");
	}

	template <typename... T> auto standardOutputWarning(T&&... toOutput) -> void {
		((std::cerr << "\x1b[33mWarning\x1b[0m: " << std::forward<T>(toOutput)), ...) << std::flush;
	}

	template <typename... T> auto standardOutputError(T&&... toOutput) -> void {
		((std::cerr << "\x1b[31mError\x1b[0m: " << std::forward<T>(toOutput)), ...) << std::flush;
	}

	auto displayPreviews(dtool::renamer::Pattern const& pattern, dtool::renamer::Core::Previews const& previews) -> void {
		standardOutput(
			"---\n"
			"Current pattern: ["
		);
		standardOutputHighlited(pattern.raw());
		standardOutput("]\n");
		if (previews.empty()) {
			standardOutput("No file selected.\n");
		} else {
			std::filesystem::path lastPath;
			for (dtool::renamer::Core::Previews::size_type i = 0; i < previews.size(); ++i) {
				auto folder = previews[i].origin->parent_path();
				if (lastPath != folder) {
					standardOutput(folder.generic_string(), "\n");
					lastPath = folder;
				}
				standardOutput(
					"  (",
					dtool::renamer::ItemIndex::fromUnderlyingIndex(i),
					")  ",
					previews[i].origin->filename().string(),
					" -> ",
					previews[i].newName,
					"\n"
				);
			}
		}
		standardOutput("---\n");
	}

	template <typename GetterT> auto patternHandler(GetterT& getter) -> dtool::renamer::Core::Action {
		standardOutput("Input your pattern: ");
		std::string rawPattern;
		getter(rawPattern);
		try {
			return dtool::renamer::Pattern(rawPattern);
		} catch (dtool::renamer::BadPattern exception) {
			standardOutputError(exception.what(), "\n");
		}
		return dtool::renamer::Core::NO_OP;
	}

	template <typename GetterT> auto reorderHandler(GetterT& getter) -> dtool::renamer::Core::Action {
		standardOutput(
			"Available sort methods:\n"
			"  (1)  Sort by name\n"
			"  (2)  Sort by last modifed time\n"
			"  (3)  Reverse\n"
			"Select a reorder method: "
		);
		int choice;
		getter(choice);
		switch (choice) {
			case 1: {
				return dtool::renamer::Core::ReorderMethod::SORT_BY_NAME;
			}
			case 2: {
				return dtool::renamer::Core::ReorderMethod::SORT_BY_MODIFIED_TIME;
			}
			case 3: {
				return dtool::renamer::Core::ReorderMethod::REVERSE;
			}
			default: {
				break;
			}
		}
		return dtool::renamer::Core::NO_OP;
	}

	template <typename GetterT> auto swapHandler(
		dtool::renamer::Core::Previews::size_type previewCount, GetterT& getter
	) -> dtool::renamer::Core::Action {
		dtool::renamer::Core::SwapInfo swapInfo;
		try {
			standardOutput("Input first to swap: ");
			getter(swapInfo.left);
			standardOutput("Input second to swap: ");
			getter(swapInfo.right);
		} catch (dtool::renamer::BadItemIndex const& e) {
			standardOutputError("Index out of range.\n");
			return dtool::renamer::Core::NO_OP;
		}
		if (swapInfo.left.underlyingIndex() >= previewCount || swapInfo.right.underlyingIndex() >= previewCount) {
			standardOutputError("Index out of range.\n");
			return dtool::renamer::Core::NO_OP;
		}
		return swapInfo;
	}

	template <typename GetterT> auto removeHandler(
		dtool::renamer::Core::Previews::size_type previewCount, GetterT& getter
	) -> dtool::renamer::Core::Action {
		dtool::renamer::Core::RemoveInfo result;
		try {
			standardOutput("Input a index: ");
			getter(result.index);
		} catch (dtool::renamer::BadItemIndex const& e) {
			standardOutputError("Index out of range.\n");
			return dtool::renamer::Core::NO_OP;
		}
		if (result.index.underlyingIndex() >= previewCount) {
			standardOutputError("Index out of range.\n");
			return dtool::renamer::Core::NO_OP;
		}
		return result;
	}

	template <typename GetterT> auto actionHandler(
		dtool::renamer::Pattern const& pattern, dtool::renamer::Core::Previews const& previews, GetterT getter
	) -> dtool::renamer::Core::Action {
		displayPreviews(pattern, previews);
		standardOutput("Choose an action <pattern(p)/insert(i)/exclude(e)/reorder(r)/swap(s)/confirm(c)/abort(a)>: ");
		std::string input;
		getter(input);
		if (input == "p" || input == "pattern") {
			return patternHandler(getter);
		} else if (input == "r" || input == "reorder") {
			return reorderHandler(getter);
		} else if (input == "i" || input == "insert") {
			standardOutput("Input a path: ");
			std::string rawPath;
			getter(rawPath);
			return dtool::renamer::Core::AddInfo{ std::filesystem::path(rawPath) };
		} else if (input == "e" || input == "exclude") {
			return removeHandler(previews.size(), getter);
		} else if (input == "s" || input == "swap") {
			return swapHandler(previews.size(), getter);
		} else if (input == "c" || input == "confirm") {
			return dtool::renamer::Core::DoneChoice::CONFIRM;
		} else if (input == "a" || input == "abort") {
			return dtool::renamer::Core::DoneChoice::ABORT;
		}
		return dtool::renamer::Core::NO_OP;
	}

	auto goQuiet(dtool::renamer::Core::Paths const& paths, char* const* arguments, size_t leftOver) {
		if (leftOver <= 0) {
			standardOutputWarning("No command received.");
		}
		g_quiet = true;
		dtool::renamer::Core renamer([input = std::deque<char const*>(arguments, arguments + leftOver)](
			dtool::renamer::Pattern const& pattern, dtool::renamer::Core::Previews const& previews
		) mutable -> dtool::renamer::Core::Action {
			if (input.empty()) {
				return dtool::renamer::Core::DoneChoice::CONFIRM;
			}
			return actionHandler(pattern, previews, [&input](auto& output) -> void {
				std::istringstream(std::string(input.front())) >> output;
				input.pop_front();
			});
		});
		renamer.interact(paths);
	}
} // namespace

int main(int argc, char** argv) {
	ConsoleGuard guard;
	dtool::renamer::Core::Paths paths;
	for (int i = 1; i < argc; ++i) {
		using namespace std::literals::string_literals;
		if (argv[i] == "-v"s || argv[i] == "--version"s) {
			standardOutput("drename ", dtool::VERSION, "\n");
			return 0;
		}
		if (argv[i] == "-h"s || argv[i] == "--help"s) {
			standardOutput(
				"Batch rename files with a customizable pattern.\n"
				"\n"
				"Usage: drename [<file>...] [<option>...]\n"
				"\n"
				"Options:\n"
				"  -h|--help\n"
				"    Display this information.\n"
				"\n"
				"  -c|--commands <command>...\n"
				"    Play the following sequence of commands instead of going to\n"
				"    interactive mode. A 'confirm' command will be automatically\n"
				"    appended. Use interactive mode to see available commands.\n"
				"\n"
				"Pattern is a customizable string which may contain format\n"
				"operators surrounded with braces('{}'). Available operators are:\n"
				"  o\n"
				"    Writes the old file name.\n"
				"\n"
				"  p\n"
				"    Writes the old file name without its extension.\n"
				"\n"
				"  e\n"
				"    Writes the old file name's extension.\n"
				"\n"
				"  i\n"
				"    Writes the file index.\n"
				"\n"
				"  c<character>...\n"
				"    Writes the nth character in the character sequence, where n\n"
				"    is the remainder of the file index divided by the length of\n"
				"    the character sequence.\n"
			);
			return 0;
		}
		if (argv[i] == "-c"s || argv[i] == "--commands"s) {
			goQuiet(paths, argv + i + 1, argc - i - 1);
			return 0;
		}
		paths.insert(std::filesystem::path(argv[i]));
	}
	dtool::renamer::Core renamer([](
		dtool::renamer::Pattern const& pattern, dtool::renamer::Core::Previews const& previews
	) -> dtool::renamer::Core::Action {
		return actionHandler(pattern, previews, [](auto& output) -> void {
			std::cin >> output;
		});
	});
	standardOutputWarning("CLI of drename is not yet stable.\n");
	renamer.interact(paths);
	return 0;
}
