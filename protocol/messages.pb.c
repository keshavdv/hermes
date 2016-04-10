/* Automatically generated nanopb constant definitions */
/* Generated by nanopb-0.3.3 at Sat Apr  9 21:50:09 2016. */

#include "messages.pb.h"

#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif



const pb_field_t Payload_fields[6] = {
    PB_FIELD(  1, ENUM    , REQUIRED, STATIC  , FIRST, Payload, type, type, 0),
    PB_FIELD(  2, MESSAGE , OPTIONAL, STATIC  , OTHER, Payload, profile, type, &DeviceProfile_fields),
    PB_FIELD(  3, MESSAGE , OPTIONAL, STATIC  , OTHER, Payload, request, profile, &DeviceControlRequest_fields),
    PB_FIELD(  4, MESSAGE , OPTIONAL, STATIC  , OTHER, Payload, response, request, &DeviceControlResponse_fields),
    PB_FIELD(  5, MESSAGE , OPTIONAL, STATIC  , OTHER, Payload, state, response, &SensorState_fields),
    PB_LAST_FIELD
};

const pb_field_t Sensor_fields[4] = {
    PB_FIELD(  1, INT32   , REQUIRED, STATIC  , FIRST, Sensor, id, id, 0),
    PB_FIELD(  2, ENUM    , REQUIRED, STATIC  , OTHER, Sensor, type, id, 0),
    PB_FIELD(  3, STRING  , REQUIRED, CALLBACK, OTHER, Sensor, name, type, 0),
    PB_LAST_FIELD
};

const pb_field_t DeviceProfile_fields[3] = {
    PB_FIELD(  1, STRING  , REQUIRED, CALLBACK, FIRST, DeviceProfile, model, model, 0),
    PB_FIELD(  2, MESSAGE , REPEATED, CALLBACK, OTHER, DeviceProfile, sensors, model, &Sensor_fields),
    PB_LAST_FIELD
};

const pb_field_t SensorConfiguration_fields[4] = {
    PB_FIELD(  1, INT32   , REQUIRED, STATIC  , FIRST, SensorConfiguration, id, id, 0),
    PB_FIELD(  2, BOOL    , REQUIRED, STATIC  , OTHER, SensorConfiguration, enabled, id, 0),
    PB_FIELD(  3, INT32   , REQUIRED, STATIC  , OTHER, SensorConfiguration, frequency, enabled, 0),
    PB_LAST_FIELD
};

const pb_field_t DeviceConfiguration_fields[2] = {
    PB_FIELD(  1, MESSAGE , REPEATED, CALLBACK, FIRST, DeviceConfiguration, configuration, configuration, &SensorConfiguration_fields),
    PB_LAST_FIELD
};

const pb_field_t DeviceControlRequest_fields[4] = {
    PB_FIELD(  1, INT32   , REQUIRED, STATIC  , FIRST, DeviceControlRequest, id, id, 0),
    PB_FIELD(  2, ENUM    , REQUIRED, STATIC  , OTHER, DeviceControlRequest, action, id, 0),
    PB_FIELD(  3, MESSAGE , OPTIONAL, STATIC  , OTHER, DeviceControlRequest, config, action, &DeviceConfiguration_fields),
    PB_LAST_FIELD
};

const pb_field_t DeviceControlResponse_fields[4] = {
    PB_FIELD(  1, INT32   , REQUIRED, STATIC  , FIRST, DeviceControlResponse, requestId, requestId, 0),
    PB_FIELD(  2, ENUM    , REQUIRED, STATIC  , OTHER, DeviceControlResponse, status, requestId, 0),
    PB_FIELD(  3, MESSAGE , OPTIONAL, STATIC  , OTHER, DeviceControlResponse, config, status, &DeviceConfiguration_fields),
    PB_LAST_FIELD
};

const pb_field_t SensorValue_fields[3] = {
    PB_FIELD(  1, UINT32  , REQUIRED, STATIC  , FIRST, SensorValue, id, id, 0),
    PB_FIELD(  2, UINT32  , REQUIRED, STATIC  , OTHER, SensorValue, value, id, 0),
    PB_LAST_FIELD
};

const pb_field_t SensorState_fields[3] = {
    PB_FIELD(  1, UINT32  , REQUIRED, STATIC  , FIRST, SensorState, timestamp, timestamp, 0),
    PB_FIELD(  2, MESSAGE , REPEATED, CALLBACK, OTHER, SensorState, messages, timestamp, &SensorValue_fields),
    PB_LAST_FIELD
};


/* Check that field information fits in pb_field_t */
#if !defined(PB_FIELD_32BIT)
/* If you get an error here, it means that you need to define PB_FIELD_32BIT
 * compile-time option. You can do that in pb.h or on compiler command line.
 * 
 * The reason you need to do this is that some of your messages contain tag
 * numbers or field sizes that are larger than what can fit in 8 or 16 bit
 * field descriptors.
 */
PB_STATIC_ASSERT((pb_membersize(Payload, profile) < 65536 && pb_membersize(Payload, request) < 65536 && pb_membersize(Payload, response) < 65536 && pb_membersize(Payload, state) < 65536 && pb_membersize(DeviceProfile, sensors) < 65536 && pb_membersize(DeviceConfiguration, configuration) < 65536 && pb_membersize(DeviceControlRequest, config) < 65536 && pb_membersize(DeviceControlResponse, config) < 65536 && pb_membersize(SensorState, messages) < 65536), YOU_MUST_DEFINE_PB_FIELD_32BIT_FOR_MESSAGES_Payload_Sensor_DeviceProfile_SensorConfiguration_DeviceConfiguration_DeviceControlRequest_DeviceControlResponse_SensorValue_SensorState)
#endif

#if !defined(PB_FIELD_16BIT) && !defined(PB_FIELD_32BIT)
/* If you get an error here, it means that you need to define PB_FIELD_16BIT
 * compile-time option. You can do that in pb.h or on compiler command line.
 * 
 * The reason you need to do this is that some of your messages contain tag
 * numbers or field sizes that are larger than what can fit in the default
 * 8 bit descriptors.
 */
PB_STATIC_ASSERT((pb_membersize(Payload, profile) < 256 && pb_membersize(Payload, request) < 256 && pb_membersize(Payload, response) < 256 && pb_membersize(Payload, state) < 256 && pb_membersize(DeviceProfile, sensors) < 256 && pb_membersize(DeviceConfiguration, configuration) < 256 && pb_membersize(DeviceControlRequest, config) < 256 && pb_membersize(DeviceControlResponse, config) < 256 && pb_membersize(SensorState, messages) < 256), YOU_MUST_DEFINE_PB_FIELD_16BIT_FOR_MESSAGES_Payload_Sensor_DeviceProfile_SensorConfiguration_DeviceConfiguration_DeviceControlRequest_DeviceControlResponse_SensorValue_SensorState)
#endif


