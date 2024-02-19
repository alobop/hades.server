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

    /* Initialise the va_list variable with the ... after fmt */

    va_start(args, fmt);

    /* Forward the '...' to vprintf */
    hades->logger.log_message(hades->logger.logger, fmt, args);

    /* Clean up the va_list */
    va_end(args);
}