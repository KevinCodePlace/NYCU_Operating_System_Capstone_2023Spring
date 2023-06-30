#!/bin/bash

sudo chmod 666 /dev/ttyUSB0
source ../.env/bin/activate
python3 ./loadImg.py