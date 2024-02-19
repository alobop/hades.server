/**
 * @file logger.c      Manages internal logging
 **/

// ---------------- Includes ---------------- //

#include "logger.h"

#include <stdarg.h>

// ----------------- Defines ---------------- //

// ---------------- Typedefs ---------------- //

// --------------- Prototypes --------------- //

// ----------- Statics & Globals ------------ //

// --------------- Functions ---------------- //

void hades_log(hades_t* hades, const char* fmt, ...)
{
    if (NULL == hades || NULL == hades->logger.log_message)
    {
        return;
    }

    va_list args;
    va_start(args, fmt);
    hades->logger.log_message(hades->logger.logger, fmt, args);
    va_end(args);
}
