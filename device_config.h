/*
 * Copyright 2015, Keshav Varma
 * All Rights Reserved.
 */

#ifndef DEVICE_CONFIG
#define DEVICE_CONFIG

#ifdef __cplusplus
extern "C" {
#endif

#include "protocol/messages.pb.h"

/* Setup device configuration */
// extern const DeviceInfo DEVICE_INFO = {
// 	{
// 	{Sensor_init_zero}
// 	{SensorConfiguration_init_zero}
// 	}
// };

bool _encode_sensors(pb_ostream_t *stream, const pb_field_t *field, void * const *arg);
DeviceProfile* encode_deviceprofile();

typedef enum {IDLE, POLL} device_state;

typedef struct {
	uint32_t ts;
	int d0;
	uint16_t a0;
} datum;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
