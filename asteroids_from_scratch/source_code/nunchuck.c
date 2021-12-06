
#include "nunchuck.h"
#include "i2c.h"

// function declarations;
void NunChuck_phase1_init(void);
void NunChuck_phase2_read(void);
void NunChuck_translate_data(void);
void search_for_i2c_devices(void);

// variable declarations;
volatile int joy_x_axis;
volatile int joy_y_axis;
volatile int accel_x_axis;
volatile int accel_y_axis;
volatile int accel_z_axis;
volatile int z_button = 0;
volatile int c_button = 0;

extern volatile uint8_t I2CMasterBuffer[I2C_PORT_NUM][BUFSIZE]; // BUFSIZE=64
extern volatile uint8_t I2CSlaveBuffer[I2C_PORT_NUM][BUFSIZE];
extern volatile uint32_t I2CReadLength[I2C_PORT_NUM];
extern volatile uint32_t I2CWriteLength[I2C_PORT_NUM];

volatile uint8_t ack_received, ack_sent; // debugging only;

void delay_dirty(int n)
{
	volatile int d;
	for (d = 0; d < n * 3000; d++)
	{
	}
}

void NunChuck_init(void)
{
	//init I2C device
	//if (PORT_USED == 0)
	//{
	//	I2C0Init();
	//}
	//else if (PORT_USED == 1)
	//{
	//	I2C1Init(); // Note: do not use I2C1 because P0.19 and P0.20 are used by GLCD!
	//}
	//else if (PORT_USED == 2)
	//{
		I2C2Init();
	//}

	NunChuck_phase1_init();
}

NunchuckData NunChuck_read(void)
{
	// (a) reset stuff
	for (int i = 0; i < BUFSIZE; i++)
	{
		I2CSlaveBuffer[PORT_USED][i] = 0x00;
	}

	// (b) NunChuck phase 2
	NunChuck_phase2_read();
	NunChuck_translate_data();

	NunchuckData output;
	output.joy_x_axis = joy_x_axis;
	output.joy_y_axis = joy_y_axis;
	output.accel_x_axis = accel_x_axis;
	output.accel_y_axis = accel_y_axis;
	output.accel_z_axis = accel_z_axis;
	output.z_button = z_button;
	output.c_button = c_button;
	return output;
}

void NunChuck_phase1_init(void)
{
	// this function should be called once only;

	I2CWriteLength[PORT_USED] = 3; // write 3 bytes
	I2CReadLength[PORT_USED] = 0;  // read 0 bytes
	I2CMasterBuffer[PORT_USED][0] = NUNCHUK_ADDRESS_SLAVE1;
	I2CMasterBuffer[PORT_USED][1] = 0xF0; // at adress 0xF0 of NunChuck write:
	I2CMasterBuffer[PORT_USED][2] = 0x55; // data 0x55
	I2CEngine(PORT_USED);
	delay_dirty(10);

	I2CWriteLength[PORT_USED] = 3; // write 3 bytes
	I2CReadLength[PORT_USED] = 0;  // read 0 bytes
	I2CMasterBuffer[PORT_USED][0] = NUNCHUK_ADDRESS_SLAVE1;
	I2CMasterBuffer[PORT_USED][1] = 0xFB; // at adress 0xFB of NunChuck write:
	I2CMasterBuffer[PORT_USED][2] = 0x00; // data 0x00
	I2CEngine(PORT_USED);
	delay_dirty(10);
}

void NunChuck_phase2_read(void)
{
	// this is called repeatedly to realize continued polling of NunChuck

	I2CWriteLength[PORT_USED] = 2; // write 2 bytes
	I2CReadLength[PORT_USED] = 0;  // read 0 bytes;
	I2CMasterBuffer[PORT_USED][0] = NUNCHUK_ADDRESS_SLAVE1;
	I2CMasterBuffer[PORT_USED][1] = 0x00; // value;
	I2CEngine(PORT_USED);
	delay_dirty(10);

	I2CWriteLength[PORT_USED] = 1; // write 1 byte
	I2CReadLength[PORT_USED] = 6;  // read 6 bytes;
	I2CMasterBuffer[PORT_USED][0] = NUNCHUK_ADDRESS_SLAVE2;
	I2CEngine(PORT_USED);
	// when I2CEngine() is executed, 6 bytes will be read and placed
	// into I2CSlaveBuffer[][]
	delay_dirty(10);
}

void NunChuck_translate_data(void)
{
	int byte5 = I2CSlaveBuffer[PORT_USED][5];
	joy_x_axis = I2CSlaveBuffer[PORT_USED][0];
	joy_y_axis = I2CSlaveBuffer[PORT_USED][1];
	accel_x_axis = (I2CSlaveBuffer[PORT_USED][2] << 2);
	accel_y_axis = (I2CSlaveBuffer[PORT_USED][3] << 2);
	accel_z_axis = (I2CSlaveBuffer[PORT_USED][4] << 2);
	z_button = 0;
	c_button = 0;

	// byte I2CSlaveBuffer[PORT_USED][5] contains bits for z and c buttons
	// it also contains the least significant bits for the accelerometer data
	if ((byte5 >> 0) & 1)
		z_button = 1;
	if ((byte5 >> 1) & 1)
		c_button = 1;
	accel_x_axis += (byte5 >> 2) & 0x03;
	accel_y_axis += (byte5 >> 4) & 0x03;
	accel_z_axis += (byte5 >> 6) & 0x03;
}

void search_for_i2c_devices(void)
{
	// this is a debug function; not used normally;
	// I used it for debugging only to find the address of slaves
	// connected to the I2C bus;
	uint32_t i;
	int count = 0;
	int address = 0x01;

	for (address = 0x50; address < 0xAF; address++)
	{
		//I2CSlaveBuffer[PORT_USED][0] = 0x33; // reset
		I2CWriteLength[PORT_USED] = 1; // write 1 byte;
		I2CReadLength[PORT_USED] = 0;  // read 0 bytes;
		I2CMasterBuffer[PORT_USED][0] = address;
		I2CEngine(PORT_USED);
		delay_dirty(40);

		// sprintf(text_buffer, "0x%02X", address);
		// LCD_PutText(32, 32, (uint8_t *)text_buffer, Yellow, Black);
		// if (ack_received > 0)
		// {
		// 	ack_received = 0; // reset;
		// 	sprintf(text_buffer, "0x%02X", address);
		// 	LCD_PutText(32, 48, (uint8_t *)text_buffer, Yellow, Black);
		// 	sprintf(text_buffer, "0x%02X", I2CSlaveBuffer[PORT_USED][0]);
		// 	LCD_PutText(32, 64, (uint8_t *)text_buffer, Yellow, Black);
		// 	count++;
		// }
	}
	// sprintf(text_buffer, "found: %02d", count);
	// LCD_PutText(32, 80, (uint8_t *)text_buffer, Yellow, Black);
}