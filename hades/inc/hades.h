/**
 * @file hades.h      RPC based on nanopb and google proto files
 **/

#pragma once

// ---------------- Includes ---------------- //

#include <stddef.h>
#include <stdint.h>

// ----------------- Defines ---------------- //

// ---------------- Typedefs ---------------- //

typedef enum
{
    HADES_SUCCESS,
    HADES_E_INVALID_ARG = -1,
    HADES_E_NOT_FOUND = -2,
    HADES_E_INSUFFICIENT_MEMORY = -3,
    HADES_E_INVALID_COMMAND = -4,
    HADES_E_UNEXPECTED = -5,
    HADES_E_FAIL = -6
} hades_status_t;

typedef struct hades_rpc_t hades_rpc_t;

typedef void (*hades_rpc_handler_t)(hades_rpc_t*, void*);

typedef struct hades_uuid_t
{
    uint8_t data[20];
} hades_uuid_t;

typedef struct hades_rpc_descriptor_t
{
    hades_uuid_t id;
    const void* input_msg_descriptor;
    const void* output_msg_descriptor;
    hades_rpc_handler_t handler;
} hades_rpc_descriptor_t;

typedef struct hades_service_descriptor_t
{
    hades_uuid_t id;
    hades_rpc_descriptor_t* rpcs[];
} hades_service_descriptor_t;

typedef struct hades_service_entry_t
{
    struct hades_service_entry_t* next;
    const hades_service_descriptor_t* descriptor;
    void* service_context;
} hades_service_entry_t;

typedef struct hades_mutex_t
{
    void* mutex;
    void (*lock)(void* mutex);
    void (*unlock)(void* mutex);
} hades_mutex_t;

typedef void (*hades_transport_completion_t)(uint32_t status, size_t bytes_transacted, void* context);

typedef struct hades_transport_t
{
    void* transport;
    void (
        *read_message)(void* transport, void* buffer, size_t max_bytes, hades_transport_completion_t cb, void* context);
    void (*write_message)(
        void* transport,
        void* buffer,
        size_t byte_count,
        hades_transport_completion_t cb,
        void* context);
} hades_transport_t;

typedef struct hades_logger_t
{
    void* logger;
    void (*log_message)(void* logger, const char* fmt, ...);
} hades_logger_t;

typedef struct hades_t
{
    hades_service_entry_t* services;
    hades_rpc_t* commands;
    size_t command_size;
    hades_mutex_t mutex;
    hades_transport_t transport;
    hades_logger_t logger;
} hades_t;

typedef struct hades_config_t
{
    struct
    {
        void* pool;
        size_t size;
    } memory_pool;
    size_t request_count;
    hades_mutex_t mutex;
    hades_transport_t transport;
    hades_logger_t logger;
} hades_config_t;

// --------------- Prototypes --------------- //

hades_status_t hades_init(hades_t* hades, hades_config_t* config);

hades_status_t hades_start(hades_t* hades);

hades_status_t hades_register_service(hades_t* hades, hades_service_entry_t* registration_entry, void* context);

hades_status_t hades_rpc_get_input(hades_rpc_t* rpc, void* input_msg);

hades_status_t hades_rpc_complete(hades_rpc_t* rpc, hades_status_t completion_status, void* output_msg);
