/**
 * @file protocol.h      Structures used for communication
 **/

#pragma once

// ---------------- Includes ---------------- //

#include <hades.h>

#include <stddef.h>
#include <stdint.h>

// ----------------- Defines ---------------- //

#define HADES_PROTOCOL_VERSION_MAJOR (0U)
#define HADES_PROTOCOL_VERSION_MINOR (1U)
#define HADES_PROTOCOL_VERSION_REVISION (5U)

// ---------------- Typedefs ---------------- //

enum
{
    HADES_COMMAND_GET_VERSION,
    HADES_COMMAND_NEGOTIATE_PAYLOAD_SIZE,
    HADES_COMMAND_RPC,

    HADES_COMMAND_MAX
};

#pragma pack(push, 1)

typedef struct hades_protocol_request_t
{
    uint16_t request : 1;
    uint16_t id : 15;
    uint16_t command;
} hades_protocol_request_t;

typedef struct hades_protocol_response_t
{
    uint16_t request : 1;
    uint16_t id : 15;
    uint16_t status;
} hades_protocol_response_t;

typedef struct hades_cmd_req_get_version_t
{
    hades_protocol_request_t header;
} hades_cmd_req_get_version_t;

typedef struct hades_cmd_resp_get_version_t
{
    hades_protocol_response_t header;
    uint32_t major : 10;
    uint32_t minor : 11;
    uint32_t revision : 11;
} hades_cmd_resp_get_version_t;

typedef struct hades_cmd_req_negotiate_size_t
{
    hades_protocol_request_t header;
    uint32_t proposed_size;
} hades_cmd_req_negotiate_size_t;

typedef struct hades_cmd_resp_negotiate_size_t
{
    hades_protocol_response_t header;
    uint32_t negotiated_size;
} hades_cmd_resp_negotiate_size_t;

typedef struct hades_cmd_req_rpc_t
{
    hades_protocol_request_t header;
    hades_uuid_t target;
    uint32_t size;
    uint8_t payload[];
} hades_cmd_req_rpc_t;

typedef struct hades_cmd_resp_rpc_t
{
    hades_protocol_response_t header;
    uint32_t size;
    uint8_t payload[];
} hades_cmd_resp_rpc_t;

#pragma pack(pop)

// --------------- Prototypes --------------- //
