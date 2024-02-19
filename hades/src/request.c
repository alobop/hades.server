/**
 * @file request.c      Manages incoming/outgoing messages
 **/

// ---------------- Includes ---------------- //

#include "request.h"

#include "commands.h"
#include "logger.h"
#include "protocol.h"

#include <assert.h>
#include <stdbool.h>

// ----------------- Defines ---------------- //

// ---------------- Typedefs ---------------- //

// --------------- Prototypes --------------- //

static void hades_request_dispatch(hades_t* hades, hades_request_t* command);
static void hades_request_receive_complete(uint32_t status, size_t bytes_transacted, void* context);
static void hades_request_send_complete(uint32_t status, size_t bytes_transacted, void* context);

// ----------- Statics & Globals ------------ //

static const hades_command_handler_t COMMAND_HANDLERS[] = {
    [HADES_COMMAND_GET_VERSION] = hades_cmd_get_version,
    [HADES_COMMAND_NEGOTIATE_PAYLOAD_SIZE] = hades_cmd_negotiate_size,
    [HADES_COMMAND_RPC] = hades_cmd_rpc
};

// --------------- Functions ---------------- //

void hades_request_receive(hades_t* hades, hades_request_t* command)
{
    assert(NULL != hades);
    assert(NULL != command);
    assert(NULL != command->associated_request);
    assert(NULL != command->associated_response);
    assert(NULL != hades->mutex.lock);
    assert(NULL != hades->mutex.unlock);
    assert(NULL != hades->transport.read_message);

    hades->mutex.lock(hades->mutex.mutex);
    hades->transport.read_message(hades->transport.transport, command->associated_request, hades->command_size, hades_request_receive_complete, command);
    hades->mutex.unlock(hades->mutex.mutex);
}

void hades_request_send(hades_t* hades, hades_request_t* command, hades_status_t completion_status, size_t response_size)
{
    assert(NULL != hades);
    assert(NULL != command);
    assert(NULL != command->associated_request);
    assert(NULL != command->associated_response);
    assert(NULL != hades->mutex.lock);
    assert(NULL != hades->mutex.unlock);
    assert(NULL != hades->transport.write_message);

    *command->associated_response = (hades_protocol_response_t){
        .request = 0,
        .id = command->associated_request->id,
        .status = completion_status
    };

    if (HADES_SUCCESS != completion_status)
    {
        response_size = sizeof(hades_protocol_response_t);
    }

    hades_log(
        hades,
        "Response: req=%u id=%u status=%u size=%u\n\r",
        command->associated_response->request,
        command->associated_response->id, 
        command->associated_response->status, 
        response_size
    );

    hades->mutex.lock(hades->mutex.mutex);
    hades->transport.write_message(hades->transport.transport, command->associated_response, response_size, hades_request_send_complete, command);
    hades->mutex.unlock(hades->mutex.mutex);
}

static void hades_request_dispatch(hades_t* hades, hades_request_t* command)
{
    assert(NULL != hades);
    assert(NULL != command);
    assert(NULL != command->associated_request);

    hades_protocol_request_t *request = command->associated_request;

    if (command->request_size < sizeof(hades_protocol_request_t))
    {
        hades_log(hades, "Bad request\n\r");
        return;
    }

    hades_log(
        hades,
        "Request: req=%u id=%u cmd=%u size=%u\n\r", 
        request->request, 
        request->id, 
        request->command, 
        command->request_size
    );

    if (request->command < HADES_COMMAND_MAX)
    {
        COMMAND_HANDLERS[request->command](hades, command);
    }
    else
    {
        hades_request_send(hades, command, HADES_E_INVALID_COMMAND, 0);
    }
}

static void hades_request_receive_complete(uint32_t status, size_t bytes_transacted, void* context)
{
    hades_request_t* request = context;

    hades_log(
        request->hades,
        "InboundMessage: status=%x size=%u associated_context=%p\n\r", 
        status, 
        bytes_transacted, 
        context
    );

    request->request_size = bytes_transacted;
    request->response_size = request->hades->command_size;
    hades_request_dispatch(request->hades, request);
}

static void hades_request_send_complete(uint32_t status, size_t bytes_transacted, void* context)
{
    hades_request_t* request = context;

    hades_log(
        request->hades,
        "OutboundMessage: status=%x size=%u associated_context=%p\n\r", 
        status, 
        bytes_transacted, 
        context
    );

    hades_request_receive(request->hades, request);
}
