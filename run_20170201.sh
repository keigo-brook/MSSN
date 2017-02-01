#!/bin/bash

set -eu
./RaspberryPi/get_sensor_from_serial /dev/ttyACM0 > ids_${1}.txt &
./Sensors/LaserScanner/simple_scan > ls_${1}.txt &
echo $$

