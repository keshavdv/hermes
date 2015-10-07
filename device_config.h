/*
 * Copyright 2015, Keshav Varma
 * All Rights Reserved.
 */
h
#ifndef DEVICE_CONFIG
#define DEVICE_CONFIG

#ifdef __cplusplus
extern "C" {
#endif

#include "messages.pb.h"

/* Setup device configuration */
extern const DeviceInfo DEVICE_INFO = {
	{
	{Sensor_init_zero}
	{SensorConfiguration_init_zero}
	}
};

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
