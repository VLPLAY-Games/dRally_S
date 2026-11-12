#include "switch_gyro.h"
#include "switch_steer.h"

#include <switch.h>

/* The tilt of the console itself (handheld mode): the six-axis sensor is read from HID and its
   accelerometer, which shows where gravity points, gives the angle the console is turned about
   the axis out of the screen, as a steering wheel is. Only read while it is switched on. */

static HidSixAxisSensorHandle handle;
static int started = 0, failed = 0;

void SwitchGyro_poll(void){

    HidSixAxisSensorState state;

    if(!SwitchSteer_gyroEnabled() || failed) return;

    if(!started){

        if(R_FAILED(hidGetSixAxisSensorHandles(&handle, 1, HidNpadIdType_Handheld, HidNpadStyleTag_NpadHandheld))
            || R_FAILED(hidStartSixAxisSensor(handle))){

            failed = 1;
            return;
        }
        started = 1;
    }

    if(hidGetSixAxisSensorStates(handle, &state, 1) < 1) return;
    if(!(state.attributes&HidSixAxisSensorAttribute_IsConnected)) return;

    SwitchSteer_feedAccel(state.acceleration.x, state.acceleration.y, state.acceleration.z);
}
