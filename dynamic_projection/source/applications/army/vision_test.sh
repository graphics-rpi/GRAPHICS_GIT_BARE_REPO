#!/bin/bash

VISION_LOG=vision_log.txt

./grab_frame out.ppm >> $VISION_LOG 2>&1
./find_terrain out.ppm terrain.army >> $VISION_LOG 2>&1
./find_soldiers out.ppm soldiers.army >> $VISION_LOG 2>&1