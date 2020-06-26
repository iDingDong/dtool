# DTool documents

## Reminder

- If not otherwise specified, any **interactive** CLI interfaces are considered unstable and are not planned to be in the future. Use the stable non-interactive commands if you are writing a script.
- Undocumented features and features marked as 'unstable' are discouraged to be relied on.

## Information macros and constants

Defined in header `dtool/common.hpp`

```cpp
#	define DTOOL_VERSION_MAJOR 1
#	define DTOOL_VERSION_MINOR 0
#	define DTOOL_VERSION_PATCH 0
#	define DTOOL_VERSION "1.0.0"

std::uint16_t constexpr dtool::VERSION_MAJOR = DTOOL_VERSION_MAJOR;
std::uint16_t constexpr dtool::VERSION_MINOR = DTOOL_VERSION_MINOR;
std::uint16_t constexpr dtool::VERSION_PATCH = DTOOL_VERSION_PATCH;
char constexpr dtool::VERSION[] = DTOOL_VERSION;
```

Indicate the version of DTool.
