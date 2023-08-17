#pragma once

#include "defines.h"

// Disable assertions by commenting out the line below.
#define AASSERTIONS_ENABLED

#ifdef AASSERTIONS_ENABLED
#if _MSC_VER
#include <intrin.h>
#define debugBreak() __debugbreak();
#else
#define debugBreak() __builtin_trap();
#endif

AAPI void report_assertion_failure(const char* expr, const char* message, const char* file, i32 line);

#define AASSERT(expr)                                                                               \
    {                                                                                               \
        if(expr)                                                                                    \
        {                                                                                           \
        }                                                                                           \
        else                                                                                        \
        {                                                                                           \
            report_assertion_failure(#expr, "", __FILE__, __LINE__);                                \
            debugBreak();                                                                           \
        }                                                                                           \
    }

#define AASSERT_MSG(expr, message)                                                                  \
    {                                                                                               \
        if(expr)                                                                                    \
        {                                                                                           \
        }                                                                                           \
        else                                                                                        \
        {                                                                                           \
            report_assertion_failure(#expr, #message, __FILE__, __LINE__);                          \
            debugBreak();                                                                           \
        }                                                                                           \
    }

#ifdef _DEBUG
#define AASSERT_DEBUG(expr)                                                                         \
    {                                                                                               \
        if(expr)                                                                                    \
        {                                                                                           \
        }                                                                                           \
        else                                                                                        \
        {                                                                                           \
            report_assertion_failure(#expr, "", __FILE__, __LINE__);                                \
            debugBreak();                                                                           \
        }                                                                                           \
    }
#else
#define AASSERT_DEBUG(expr)
#endif
#else
#define AASSERT(expr)
#define AASSERT_MSG(expr, message)
#define AASSERT_DEBUG(expr)
#endif
