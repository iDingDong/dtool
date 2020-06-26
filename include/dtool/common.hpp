#ifndef DTOOL_DTOOL_HPP_INCLUDED
#	define DTOOL_DTOOL_HPP_INCLUDED 1

#	include <cstdint>

#	define DTOOL_STR_HELPER(parameter)#parameter
#	define DTOOL_STR(parameter) DTOOL_STR_HELPER(parameter)
#	define DTOOL_VERSION_MAJOR 1
#	define DTOOL_VERSION_MINOR 0
#	define DTOOL_VERSION_PATCH 0
#	define DTOOL_VERSION DTOOL_STR(DTOOL_VERSION_MAJOR) "." DTOOL_STR(DTOOL_VERSION_MINOR) "." DTOOL_STR(DTOOL_VERSION_PATCH)

namespace dtool {
	std::uint16_t constexpr VERSION_MAJOR = DTOOL_VERSION_MAJOR;
	std::uint16_t constexpr VERSION_MINOR = DTOOL_VERSION_MINOR;
	std::uint16_t constexpr VERSION_PATCH = DTOOL_VERSION_PATCH;
	char constexpr VERSION[] = DTOOL_VERSION;
}

#endif
