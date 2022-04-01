COEN 4720
Project
Gravitoids: Asteroids with Extra Physics and Multiplayer
Brendan Nenninger, Kassie Povinelli, Carl Sustar

This project is largely as Asteroids game clone, but we have added a scrolling, wraparound play space and gravity for the asteroids. It is also possible for an outside player to connect to the game via Bluetooth to see the score and number of lives the player has left, in addition to being able to spawn in black holes.

The game is played on a LandTiger board with an LPC1768 microcontroller.

Controls:
The game is controlled via a Wii Nunchuck. Left and right presses on the joystick are used to rotate the spaceship. The 'z' button is used to fire the spaceship's engine, and the 'c' button fires bullets to destroy asteroids.
A person connected via a Bluetooth terminal can spawn asteroids by entering "black hole"

Connections:

Nunchuck:
	Data: P0.10
	Clock: P0.11
	VDD: 3.3 V of LandTiger
	GND: GND of LandTiger

Bluetooth module:
	RXD: P2.8
	TXD: P2.9
	VCC: 3.3 V of LandTiger
	GND: GND of LandTiger

Compilation:

The executable size for this project is around 47 kB, considerably beyond the free limit for the Microvision compiler. Therefore, either a payed or evaluation license must be used to compile the software. However, this package includes precompiled code, so it should be possible to load onto a LandTiger board without any additional licensing needs.

Source Code:
All custom source code is stored in the source_code folder, and the Microvision project is stored in the project_code folder. This project references the code in the source code folder.