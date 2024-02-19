/**
 * @file commands.h      Protocol commandset
 **/

#pragma once

// ---------------- Includes ---------------- //

#include "request.h"

#include <stddef.h>
#include <stdint.h>

// ----------------- Defines ---------------- //

// ---------------- Typedefs ---------------- //

// --------------- Prototypes --------------- //

void hades_cmd_get_version(hades_t* hades, hades_request_t* handle);

void hades_cmd_negotiate_size(hades_t* hades, hades_request_t* handle);

void hades_cmd_rpc(hades_t* hades, hades_request_t* handle);
