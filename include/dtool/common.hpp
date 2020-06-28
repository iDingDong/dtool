#ifndef DTOOL_COMMON_HPP_INCLUDED
#	define DTOOL_COMMON_HPP_INCLUDED 1

#	include <dtool/common.h>

#	include <cstdint>

namespace dtool {
	std::uint16_t constexpr VERSION_MAJOR = DTOOL_VERSION_MAJOR;
	std::uint16_t constexpr VERSION_MINOR = DTOOL_VERSION_MINOR;
	std::uint16_t constexpr VERSION_PATCH = DTOOL_VERSION_PATCH;
	char constexpr VERSION[] = DTOOL_VERSION;
}

#endif
