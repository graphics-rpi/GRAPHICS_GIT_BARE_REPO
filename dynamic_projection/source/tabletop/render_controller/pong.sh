#!/bin/bash
rm /ramdisk/images/*.ppm


ppmmake red   256 256 > /ramdisk/images/canvas_wall_0_texture.ppm
ppmmake green 256 256 > /ramdisk/images/luan_wall_0_texture.ppm
./render_controller   -pong
