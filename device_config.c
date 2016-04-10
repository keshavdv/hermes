#include "wiced.h"
#include "device_config.h"
#include "pb_common.h"
#include "pb_encode.h"
#include "pb_decode.h"

int num_sensors = 2;
Sensor sensors[] = {
	{1, Sensor_SensorType_DIGITAL, "d0"},
	{2, Sensor_SensorType_DIGITAL, "d1"},
};

bool _encode_sensors(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{
    Sensor sensor = Sensor_init_zero;
    sensor.id = 1;
    sensor.type = Sensor_SensorType_DIGITAL;
    for(int i = 0; i < num_sensors; i++){
    	if (!pb_encode_tag_for_field(stream, field))
	        return false;

	    if (!pb_encode_submessage(stream, Sensor_fields, &sensors[i])) {
    		WPRINT_APP_INFO(("no\n"));
	        return false;
	    }
	}
 
    return true;
}

DeviceProfile* encode_deviceprofile()
{
	DeviceProfile *device_info = malloc(sizeof(DeviceProfile));
	strcpy(device_info->model, "Hermes v1.0");
	device_info->sensors.funcs.encode = &_encode_sensors;
    device_info->sensors.arg = NULL;
    return device_info;
}



