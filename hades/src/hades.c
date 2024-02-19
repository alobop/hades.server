/**
 * @file hades.c      RPC based on nanopb and google proto files
 **/

// ---------------- Includes ---------------- //

#include "hades.h"

#include "request.h"
#include "commands.h"

#include "protocol.h"

#include <pb_decode.h>
#include <pb_encode.h>

#include <stdint.h>

// ----------------- Defines ---------------- //

// ---------------- Typedefs ---------------- //

// --------------- Prototypes --------------- //

// ----------- Statics & Globals ------------ //

// --------------- Functions ---------------- //

hades_status_t hades_init(hades_t *hades, hades_config_t *config)
{
    if (NULL == hades 
        || NULL == config 
        || NULL == config->memory_pool.pool 
        || NULL == config->mutex.lock 
        || NULL == config->mutex.unlock 
        || NULL == config->transport.read_message
        || NULL == config->transport.write_message
    )
    {
        return HADES_E_INVALID_ARG;
    }

    if (config->memory_pool.size < sizeof(hades_request_t) * config->request_count)
    {
        return HADES_E_INSUFFICIENT_MEMORY;
    }

    *hades = (hades_t){};

    hades->mutex = config->mutex;
    hades->transport = config->transport;
    hades->logger = config->logger;

    size_t chunk_size = config->memory_pool.size / config->request_count;
    chunk_size = chunk_size - (chunk_size % sizeof(uintptr_t)); // align to ptr size
    hades->command_size = (chunk_size - sizeof(hades_request_t)) / 2;
    hades->command_size = hades->command_size - (hades->command_size % (sizeof(uintptr_t))); // align to ptr size

    hades_rpc_t **current_command = &hades->commands;

    for (size_t i = 0; i < config->request_count; i++)
    {
        hades_rpc_t *command = (hades_rpc_t *)(((uintptr_t)config->memory_pool.pool) + (chunk_size * i));
        command->next = NULL;
        command->hades = hades;
        command->associated_request = (hades_protocol_request_t *)(((uintptr_t)command) + sizeof(hades_request_t));
        command->request_size = hades->command_size;
        command->associated_response = (hades_protocol_response_t *)(((uintptr_t)command->associated_request) + hades->command_size);
        command->response_size = hades->command_size;

        *current_command = command;
        current_command = &command->next;
    }

    return HADES_SUCCESS;
}

hades_status_t hades_start(hades_t* hades)
{
    if (NULL == hades)
    {
        return HADES_E_INVALID_ARG;
    }

    if (NULL == hades->mutex.lock
        || NULL == hades->mutex.unlock
        || NULL == hades->transport.read_message
        || NULL == hades->commands
    )
    {
        return HADES_E_UNEXPECTED;
    }

    hades_rpc_t **current_command = &hades->commands;

    while (NULL != *current_command)
    {
        hades_request_receive(hades, *current_command);
        current_command = &(*current_command)->next;
    }

    return HADES_SUCCESS;
}

hades_status_t hades_register_service(hades_t *hades, hades_service_entry_t *registration_entry, void *context)
{
    if (NULL == hades || NULL == registration_entry)
    {
        return HADES_E_INVALID_ARG;
    }

    if (NULL == hades->mutex.unlock
        || NULL == hades->mutex.lock
    )
    {
        return HADES_E_UNEXPECTED;
    }

    hades->mutex.lock(hades->mutex.mutex);

    registration_entry->next = NULL;
    registration_entry->service_context = context;

    hades_service_entry_t **current = &hades->services;

    while (NULL != *current)
    {
        current = &(*current)->next;
    }

    *current = registration_entry;

    hades->mutex.unlock(hades->mutex.mutex);

    return HADES_SUCCESS;
}

hades_status_t hades_rpc_get_input(hades_rpc_t *rpc, void *input_msg)
{
    if (NULL == rpc || NULL == input_msg)
    {
        return HADES_E_INVALID_ARG;
    }

    if (NULL == rpc->associated_request || NULL == rpc->command_data.rpc.descriptor)
    {
        return HADES_E_UNEXPECTED;
    }

    hades_cmd_req_rpc_t *request = (hades_cmd_req_rpc_t *)rpc->associated_request;
    struct pb_istream_s stream = pb_istream_from_buffer(request->payload, request->size);

    if (!pb_decode(&stream, rpc->command_data.rpc.descriptor->input_msg_descriptor, input_msg))
    {
        return HADES_E_FAIL;
    }

    return HADES_SUCCESS;
}

hades_status_t hades_rpc_complete(hades_rpc_t *rpc, hades_status_t completion_status, void *output_msg)
{
    if (NULL == rpc || NULL == output_msg)
    {
        return HADES_E_INVALID_ARG;
    }

    if (NULL == rpc->associated_response || NULL == rpc->command_data.rpc.descriptor)
    {
        return HADES_E_UNEXPECTED;
    }

    if (HADES_SUCCESS != completion_status)
    {
        hades_request_send(rpc->hades, rpc, completion_status, 0);
        return HADES_SUCCESS;
    }

    *rpc->associated_response = (hades_protocol_response_t){
        .request = 0,
        .id = rpc->associated_request->id,
        .status = completion_status};

    hades_cmd_resp_rpc_t *response = (hades_cmd_resp_rpc_t *)rpc->associated_response;

    struct pb_ostream_s stream = pb_ostream_from_buffer(response->payload, response->size);

    if (!pb_encode(&stream, rpc->command_data.rpc.descriptor->output_msg_descriptor, output_msg))
    {
        return HADES_E_FAIL;
    }

    response->size = stream.bytes_written;

    hades_request_send(rpc->hades, rpc, completion_status, sizeof(hades_cmd_resp_rpc_t) + response->size);

    return HADES_SUCCESS;
}
