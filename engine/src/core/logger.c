#include "logger.h"

// TODO: Temporary.
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

b8 initialize_logging()
{
    // TODO: Create a log file.
    return TRUE;
}

void shutdown_logging()
{
    // TODO: Cleanup logging/write queued entries.
}

void log_output(log_level level, const char* message, ...)
{
    const char* level_strings[6] = {"[FATAL]: ", "[ERROR]: ", " [WARN]: ", " [INFO]: ", "[DEBUG]: ", "[TRACE]: "};
    b8 is_error = level < 2;

    // Technically imposes a 32k character limit on a single log entry, but...
    // DON'T DO THAT!
    char out_message[32000];
    memset(out_message, 0, sizeof(out_message));

    // Format original message
    // NOTE: Oddly enough, MS's headers override the GCC/Clang va_list type with a typedef char* va_list in some
    // cases, and as a result throws a strange error here. The workaround for now is to just user __builtin_va_list,
    // which is the GCC/Clang's va_start expects.
    //__builtin_va_list arg_ptr;
    va_list arg_ptr;
    va_start(arg_ptr, message);
    vsnprintf(out_message, 32000, message, arg_ptr);
    va_end(arg_ptr);

    // TODO: platform-specific output.
    printf("%s%s\n", level_strings[level], out_message);
}
