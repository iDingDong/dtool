#ifndef DTOOL_RENAMER_H_INCLUDED
#	define DTOOL_RENAMER_H_INCLUDED 1

#	include <dtool/common.h>

DTOOL_HANDLE_TYPE(DTOOL_RENAMER_Pattern)

DTOOL_EXTERN_C DTOOL_Result DTOOL_RENAMER_createPattern(char const* rawPattern, DTOOL_RENAMER_Pattern* outputPattern);
DTOOL_EXTERN_C void DTOOL_RENAMER_destroyPattern(DTOOL_RENAMER_Pattern* outputPattern);

#endif
