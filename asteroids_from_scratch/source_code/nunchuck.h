// cristinel ababei; sep. 2014 - 2020;
// this is an example, built on top of the "I2C" example that comes with the CD of the
// LandTiger 2.0 board; I use a NunChuck to move around a circle on the LCD display;
// the board is connected to the NunChuck via I2C2, which uses pins:
//
// P0.10 --> Data of Chuck
// P0.11 --> Clock of Chuck
// 3.3V of LandTiger --> VDD of Chuck
// GND --> GND
//
// Note: I used I2C0 in the past; but, it has issues with boards purchased more recently;
// it's very weird...
#include "i2c.h"
// #include "GLCD.h"

// Note: do not use I2C1 because P0.19 and P0.20 are used by GLCD!
#define PORT_USED 2 // <--- Use I2C2, not I2C0 (used to work on older boards)!

#define NUNCHUK_ADDRESS_SLAVE1 0xA4
#define NUNCHUK_ADDRESS_SLAVE2 0xA5

typedef struct
{
	int joy_x_axis;
	int joy_y_axis;
	int accel_x_axis;
	int accel_y_axis;
	int accel_z_axis;
	int z_button;
	int c_button;
} NunchuckData;

// function declarations;
void NunChuck_init(void);
NunchuckData NunChuck_read(void);