#This python applications sends controls through serial to the LandTiger 2.0 board
#By Kassie Povinelli
#serial code based on https://www.xanthium.in/Cross-Platform-serial-communication-using-Python-and-PySerial
import serial
import time
import keyboard

com_name = 'COM4'
serial_object = serial.Serial('COM4')

serial_object.baudrate = 9600  # set Baud rate to 9600
serial_object.bytesize = 8     # Number of data bits = 8
serial_object.parity   ='N'    # No parity
serial_object.stopbits = 1     # Number of Stop bits = 1

time.sleep(3)
#send game start signal
print("sending game start signal")
serial_object.write(b"s")
#poll the keys to send through serial
while True:
    input_key = keyboard.read_key()
    if input_key == "right":
        serial_object.write(b"d")
    elif input_key == "left":
        serial_object.write(b"a")
    elif input_key == "up":
        serial_object.write(b"w")
    elif input_key == "down":
        serial_object.write(b"s")

