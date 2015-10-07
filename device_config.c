#include "wiced.h"
#include "device_config.h"
#include "pb_common.h"
#include "pb_encode.h"
#include "pb_decode.h"

extern DEVICE_INFO = DeviceInfo_init_zero;

int init_device_info(){
	DEVICE_INFO.model = "Hermes v1.0";
	DEVICE_INFO.sensors = 
}
