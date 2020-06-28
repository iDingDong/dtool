#include <dtool/renamer.h>

#include "ccommon.hpp"

#include <dtool/common.h>
#include <dtool/renamer.hpp>

#include <variant>

extern "C" DTOOL_Result DTOOL_RENAMER_createPattern(char const* rawPattern, DTOOL_RENAMER_Pattern* outputPattern) {
	try {
		auto result = (dtool::createHandle<DTOOL_RENAMER_Pattern, dtool::renamer::Pattern>(rawPattern));
		DTOOL_Result* errorCode = std::get_if<DTOOL_Result>(&result);
		if (errorCode != nullptr) {
			return *errorCode;
		}
		*outputPattern = std::get<DTOOL_RENAMER_Pattern>(result);
	} catch (dtool::renamer::BadPattern const& e) {
		return DTOOL_RESULT_INVALID_INPUT;
	} catch (...) {
		return DTOOL_RESULT_UNKNOWN;
	}
	return DTOOL_RESULT_OK;
}

extern "C" void DTOOL_RENAMER_destroyPattern(DTOOL_RENAMER_Pattern* outputPattern) {
	delete static_cast<dtool::renamer::Pattern*>(outputPattern->m_holder);
}
