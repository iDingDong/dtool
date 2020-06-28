#ifndef DTOOL_COMMON_H_INCLUDED
#	define DTOOL_COMMON_H_INCLUDED 1

#	define DTOOL_STR_HELPER(parameter)#parameter
#	define DTOOL_STR(parameter) DTOOL_STR_HELPER(parameter)
#	define DTOOL_VERSION_MAJOR 1
#	define DTOOL_VERSION_MINOR 0
#	define DTOOL_VERSION_PATCH 0
#	define DTOOL_VERSION DTOOL_STR(DTOOL_VERSION_MAJOR) "." DTOOL_STR(DTOOL_VERSION_MINOR) "." DTOOL_STR(DTOOL_VERSION_PATCH)
#	if defined(__cplusplus)
#		define DTOOL_EXTERN_C extern "C"
#		define DTOOL_REFINE_ENUM_NAME(Name)
#		define DTOOL_REFINE_STRUCT_NAME(Name)
#	else
#		define DTOOL_EXTERN_C
#		define DTOOL_REFINE_ENUM_NAME(Name) typedef enum Name Name;
#		define DTOOL_REFINE_STRUCT_NAME(Name) typedef struct Name Name;
#	endif
#	define DTOOL_HANDLE_TYPE(Name) struct Name { \
		void* m_holder; \
	}; \
	\
	DTOOL_REFINE_STRUCT_NAME(Name)

DTOOL_HANDLE_TYPE(DTOOL_String)

enum DTOOL_Result {
	DTOOL_RESULT_OK,
	DTOOL_RESULT_UNKNOWN,
	DTOOL_RESULT_INVALID_INPUT,
	DTOOL_RESULT_OUT_OF_MEMORY
};

DTOOL_REFINE_ENUM_NAME(DTOOL_Result)

#endif
