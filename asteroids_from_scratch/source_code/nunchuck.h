// COEN 4720
// Project
// Gravitoids: Asteroids with Extra Physics and Multiplayer
// Brendan Nenninger, Kassie Povinelli, Carl Sustar
//
// nunchuck.h
// handles nunchuck input
// based on code by Dr.Cris Ababei,
// modified to be a separate file, rather than main, and to return a struct of the nunchuck inputs

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