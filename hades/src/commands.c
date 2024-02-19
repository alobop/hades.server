/**
 * @file commands.c      Protocol commandset
 **/

// ---------------- Includes ---------------- //

#include "commands.h"

#include "protocol.h"
#include "request.h"

#include <assert.h>
#include <stdbool.h>

// ----------------- Defines ---------------- //

// ---------------- Typedefs ---------------- //

// --------------- Prototypes --------------- //

// ----------- Statics & Globals ------------ //

static bool hades_id_equal(const hades_uuid_t* right, const hades_uuid_t* left);

// --------------- Functions ---------------- //

void hades_cmd_get_version(
    hades_t* hades,
    hades_request_t* command
)
{
    assert(NULL != hades);
    assert(NULL != command);
    assert(NULL != command->associated_request);
    assert(NULL != command->associated_response);

    if (sizeof(hades_cmd_req_get_version_t) != command->request_size)
    {
        hades_request_send(hades, command, HADES_E_INVALID_ARG, 0);
        return;
    }

    if (sizeof(hades_cmd_resp_get_version_t) > command->response_size)
    {
        hades_request_send(hades, command, HADES_E_INSUFFICIENT_MEMORY, 0);
        return;
    }

    hades_cmd_resp_get_version_t* response = (hades_cmd_resp_get_version_t*)command->associated_response;

    response->major = HADES_PROTOCOL_VERSION_MAJOR;
    response->minor = HADES_PROTOCOL_VERSION_MINOR;
    response->revision = HADES_PROTOCOL_VERSION_REVISION;

    hades_request_send(hades, command, HADES_SUCCESS, sizeof(hades_cmd_resp_get_version_t));
}

void hades_cmd_negotiate_size(
    hades_t* hades,
    hades_request_t* command
)
{
    assert(NULL != hades);
    assert(NULL != command);
    assert(NULL != command->associated_request);
    assert(NULL != command->associated_response);

    if (sizeof(hades_cmd_req_negotiate_size_t) != command->request_size)
    {
        hades_request_send(hades, command, HADES_E_INVALID_ARG, 0);
        return;
    }

    if (sizeof(hades_cmd_req_negotiate_size_t) > command->response_size)
    {
        hades_request_send(hades, command, HADES_E_INSUFFICIENT_MEMORY, 0);
        return;
    }

    const hades_cmd_req_negotiate_size_t* request = (const hades_cmd_req_negotiate_size_t*)command->associated_request;
    hades_cmd_resp_negotiate_size_t* response = (hades_cmd_resp_negotiate_size_t*)command->associated_response;

    response->negotiated_size = request->proposed_size > hades->command_size ? hades->command_size : request->proposed_size;

    hades_request_send(hades, command, HADES_SUCCESS, sizeof(hades_cmd_resp_negotiate_size_t));
}

void hades_cmd_rpc(
    hades_t* hades,
    hades_request_t* command
)
{
    assert(NULL != hades);
    assert(NULL != command);
    assert(NULL != command->associated_request);
    assert(NULL != command->associated_response);
    assert(NULL != hades->mutex.lock);
    assert(NULL != hades->mutex.unlock);

    if (sizeof(hades_cmd_req_rpc_t) > command->request_size )
    {
        hades_request_send(hades, command, HADES_E_INVALID_ARG, 0);
        return;
    }

    if (sizeof(hades_cmd_resp_rpc_t) > command->response_size)
    {
        hades_request_send(hades, command, HADES_E_INSUFFICIENT_MEMORY, 0);
        return;
    }

    const hades_cmd_req_rpc_t* request = (const hades_cmd_req_rpc_t*)command->associated_request;
    hades_cmd_resp_rpc_t* response = (hades_cmd_resp_rpc_t*)command->associated_response;

    if (request->size + sizeof(hades_cmd_req_rpc_t) > command->request_size)
    {
        hades_request_send(hades, command, HADES_E_FAIL, 0);
        return;
    }

    hades->mutex.lock(hades->mutex.mutex);

    hades_service_entry_t **current = &hades->services;
    hades_rpc_descriptor_t* const* current_rpc = NULL;

    while (NULL != *current)
    {
        current_rpc = (*current)->descriptor->rpcs;

        while (NULL != *current_rpc)
        {
            if (hades_id_equal(&(*current_rpc)->id, &request->target))
            {
                break;
            }
            current_rpc++;
        }

        if (NULL != *current_rpc)
        {
            break;
        }
        current = &(*current)->next;
    }

    hades->mutex.unlock(hades->mutex.mutex);

    if (NULL != *current)
    {
        command->command_data.rpc.descriptor = *current_rpc;
        response->size = command->response_size - sizeof(hades_cmd_resp_rpc_t);

        (*current_rpc)->handler(command, (*current)->service_context);
    }
    else
    {
        hades_request_send(hades, command, HADES_E_NOT_FOUND, 0);
    }
}

static bool hades_id_equal(const hades_uuid_t* right, const hades_uuid_t* left)
{
    if (NULL == right || NULL == left)
    {
        return false;
    }
    for (size_t i = 0; i < sizeof(right->data)/sizeof(right->data[0]); i++)
    {
        if (right->data[i] != left->data[i])
        {
            return false;
        }
    }
    return true;
}
