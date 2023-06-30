#! /usr/bin/python3

import os
from socket import timeout
import time
import sys
import serial
from time import sleep

BAUD_RATE = 115200

def send_img(ser,kernel):
    print("Please sent the kernel image size:")
    kernel_size=os.stat(kernel).st_size
    ser.write((str(kernel_size)+"\n").encode())
    print(ser.read_until(b"Start to load the kernel image... \r\n").decode(), end="")

    with open(kernel, "rb") as image:
        while kernel_size > 0:
            kernel_size -= ser.write(image.read(1))
            ser.read_until(b".")
    print(ser.read_until(b"$ ").decode(), end="")
    return

if __name__ == "__main__":
    ser = serial.Serial("/dev/ttyUSB0", BAUD_RATE, timeout=5)
    send_img(ser,"../build/kernel8.img")

    

