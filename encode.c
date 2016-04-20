#include <stdio.h>
#include "pb_common.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "protocol/messages.pb.h"
#include <inttypes.h>
#include "device_config.h"


bool encode_state(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{
    SensorValue sv = SensorValue_init_zero;
    datum *data = (datum*) *arg;

    sv.id = 1;
    sv.value = data->d0;
    if (!pb_encode_tag_for_field(stream, field))
        return false;

    if (!pb_encode_submessage(stream, SensorValue_fields, &sv))
        return false;

    sv.id = 2;
    sv.value = data->a0;
    if (!pb_encode_tag_for_field(stream, field))
        return false;

    if (!pb_encode_submessage(stream, SensorValue_fields, &sv))
        return false;

    return true;
}

bool encode_payload(pb_ostream_t *stream, const pb_field_t messagetype[], const void *message)
{
    const pb_field_t *field;
    for (field = Payload_fields; field->tag != 0; field++)
    {
        if (field->ptr == messagetype)
        {
            /* This is our field, encode the message using it. */
            if (!pb_encode_tag_for_field(stream, field))
                return false;

            return pb_encode_submessage(stream, messagetype, message);
        }
    }

    /* Didn't find the field for messagetype */
    return false;
}