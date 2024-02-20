/**
 * @file request.h      Manages incoming/outgoing messages
 **/

#pragma once

// ---------------- Includes ---------------- //

#include "hades.h"
#include "protocol.h"

#include <stddef.h>

// ----------------- Defines ---------------- //

// ---------------- Typedefs ---------------- //

typedef struct hades_rpc_t
{
    struct hades_rpc_t* next;
    hades_t* hades;
    hades_protocol_request_t* associated_request;
    size_t request_size;
    hades_protocol_response_t* associated_response;
    size_t response_size;

    union
    {
        struct
        {
            const hades_rpc_descriptor_t* descriptor;
        } rpc;
    } command_data;
} hades_rpc_t;

typedef hades_rpc_t hades_request_t;

typedef void (*hades_command_handler_t)(hades_t* hades, hades_request_t* handle);

// --------------- Prototypes --------------- //

void hades_request_receive(hades_t* hades, hades_request_t* command);

void hades_request_send(
    hades_t* hades,
    hades_request_t* command,
    hades_status_t completion_status,
    size_t response_size);
