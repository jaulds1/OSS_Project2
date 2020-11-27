#!/bin/bash
clear
rmmod char.ko
make clean
make
insmod char.ko
sleep 2
chmod 777 /dev/secret612a
chmod 777 /dev/secret612b
