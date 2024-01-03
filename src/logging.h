#ifndef FMOD_LOGGING
#define FMOD_LOGGING

#include "core/error_macros.h"

#define GODOT_ERROR(m_error)                        \
    _err_print_error(FUNCTION_STR, __FILE__, __LINE__, m_error);

#define GODOT_ERROR_MSG(m_error, m_msg)             \
    _err_print_error(FUNCTION_STR, __FILE__, __LINE__, m_error, m_msg);

#define FMOD_ERR_COND_FAIL(m_result) FMOD_ERR_COND_FAIL_V(m_result, void())

#define FMOD_ERR_COND_FAIL_MSG(m_result, m_msg) 	\
    if (true)										\
    {												\
        FMOD_RESULT result = m_result;				\
        if (unlikely(result != FMOD_OK)){ 			\
            GODOT_ERROR_MSG(FMOD_ErrorString(result), m_msg); \
            return;									\
        }											\
    } else                                          \
        ((void)0)

#define FMOD_ERR_COND_FAIL_V(m_result, m_default) 	\
    if (true)										\
    {												\
        FMOD_RESULT result = m_result;				\
        if (unlikely(result != FMOD_OK)){			\
            GODOT_ERROR(FMOD_ErrorString(result));  \
            return m_default;						\
        }											\
    } else                                          \
        ((void)0)

#define FMOD_ERR_COND_PRINT(m_result)				\
    if (true)										\
    {												\
        FMOD_RESULT result = m_result;				\
        if (unlikely(result != FMOD_OK)){			\
            GODOT_ERROR(FMOD_ErrorString(result));	\
        }											\
    } else                                          \
        ((void)0)

#define FMOD_INIT_CHECK() FMOD_INIT_CHECK_V(void())

#define FMOD_INIT_CHECK_V(m_return)                 \
    if (unlikely(!FmodInterface::get_singleton()->is_initialized()))\
    {												\
        GODOT_ERROR("Fmod is not initialized!")		\
        return m_return;							\
    } else                                          \
        ((void)0)

#define FMOD_INIT_CHECK_ONCE()                      \
    if (unlikely(!FmodInterface::get_singleton()->is_initialized()))\
    {												\
		static bool first_print = true;             \
		if (first_print) {                          \
			first_print = false;                    \
            GODOT_ERROR("Fmod is not initialized!") \
		}                                           \
        return;             						\
    } else                                          \
        ((void)0)

#define LOG_VERBOSE(m_msg) print_verbose(String("FmodInterface: ") + m_msg);

#endif