#!/bin/bash
./ccal projector_A_tsai.dat       projector_A_intrinsics.dat \
       projector_A_extrinsics.dat projector_A_calibration.dat 0.1 0.1 0.0

./ccal projector_B_tsai.dat       projector_B_intrinsics.dat \
       projector_B_extrinsics.dat projector_B_calibration.dat 0.0 0.0 0.0

./ccal projector_C_tsai.dat       projector_C_intrinsics.dat \
       projector_D_extrinsics.dat projector_C_calibration.dat -0.1 -0.1 0.0

./ccal projector_D_tsai.dat       projector_D_intrinsics.dat \
       projector_D_extrinsics.dat projector_D_calibration.dat 0.0 0.0 0.0

