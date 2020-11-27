#!/bin/bash
clear
make clean
make
sudo setcap 38+ep usermode
