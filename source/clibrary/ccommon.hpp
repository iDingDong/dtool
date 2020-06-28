#ifndef DTOOL_CCOMMON_HPP_INCLUDED
#	define DTOOL_CCOMMON_HPP_INCLUDED 1

#	include <dtool/common.h>

#	include <variant>
#	include <new>

namespace dtool {
	namespace {
		template <typename HandleT> using CHandleCreationResult = std::variant<HandleT, DTOOL_Result>;

		template <
			typename HandleT, typename ValueT, typename... ParameterTs
		> auto createHandle(ParameterTs&&... parameters) -> CHandleCreationResult<HandleT> {
			HandleT result;
			result.m_holder = static_cast<void*>(new (std::nothrow) ValueT(std::forward<ParameterTs>(parameters)...));
			if (result.m_holder == nullptr) {
				return DTOOL_RESULT_OUT_OF_MEMORY;
			}
			return result;
		}
	}
}

#endif
