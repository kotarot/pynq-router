#!/bin/sh

# 置換すること
# (1)
# MAX_LINES を 65535 に置換
# (2)
# 1 から 16 までは int / unsigned int に置換
# Note: 正規表現で書く場合は -e "s/ap_int<\([0-9]\+\)>/int/g"

sed -e "s/ap_int<1>/int/g" \
    -e "s/ap_int<2>/int/g" \
    -e "s/ap_int<3>/int/g" \
    -e "s/ap_int<4>/int/g" \
    -e "s/ap_int<5>/int/g" \
    -e "s/ap_int<6>/int/g" \
    -e "s/ap_int<7>/int/g" \
    -e "s/ap_int<8>/int/g" \
    -e "s/ap_int<9>/int/g" \
    -e "s/ap_int<10>/int/g" \
    -e "s/ap_int<11>/int/g" \
    -e "s/ap_int<12>/int/g" \
    -e "s/ap_int<13>/int/g" \
    -e "s/ap_int<14>/int/g" \
    -e "s/ap_int<15>/int/g" \
    -e "s/ap_int<16>/int/g" \
    -e "s/ap_uint<1>/unsigned int/g" \
    -e "s/ap_uint<2>/unsigned int/g" \
    -e "s/ap_uint<3>/unsigned int/g" \
    -e "s/ap_uint<4>/unsigned int/g" \
    -e "s/ap_uint<5>/unsigned int/g" \
    -e "s/ap_uint<6>/unsigned int/g" \
    -e "s/ap_uint<7>/unsigned int/g" \
    -e "s/ap_uint<8>/unsigned int/g" \
    -e "s/ap_uint<9>/unsigned int/g" \
    -e "s/ap_uint<10>/unsigned int/g" \
    -e "s/ap_uint<11>/unsigned int/g" \
    -e "s/ap_uint<12>/unsigned int/g" \
    -e "s/ap_uint<13>/unsigned int/g" \
    -e "s/ap_uint<14>/unsigned int/g" \
    -e "s/ap_uint<15>/unsigned int/g" \
    -e "s/ap_uint<16>/unsigned int/g" \
    ../sw/router.cpp > router.cpp

sed -e "s/ap_int<1>/int/g" \
    -e "s/ap_int<2>/int/g" \
    -e "s/ap_int<3>/int/g" \
    -e "s/ap_int<4>/int/g" \
    -e "s/ap_int<5>/int/g" \
    -e "s/ap_int<6>/int/g" \
    -e "s/ap_int<7>/int/g" \
    -e "s/ap_int<8>/int/g" \
    -e "s/ap_int<9>/int/g" \
    -e "s/ap_int<10>/int/g" \
    -e "s/ap_int<11>/int/g" \
    -e "s/ap_int<12>/int/g" \
    -e "s/ap_int<13>/int/g" \
    -e "s/ap_int<14>/int/g" \
    -e "s/ap_int<15>/int/g" \
    -e "s/ap_int<16>/int/g" \
    -e "s/ap_uint<1>/unsigned int/g" \
    -e "s/ap_uint<2>/unsigned int/g" \
    -e "s/ap_uint<3>/unsigned int/g" \
    -e "s/ap_uint<4>/unsigned int/g" \
    -e "s/ap_uint<5>/unsigned int/g" \
    -e "s/ap_uint<6>/unsigned int/g" \
    -e "s/ap_uint<7>/unsigned int/g" \
    -e "s/ap_uint<8>/unsigned int/g" \
    -e "s/ap_uint<9>/unsigned int/g" \
    -e "s/ap_uint<10>/unsigned int/g" \
    -e "s/ap_uint<11>/unsigned int/g" \
    -e "s/ap_uint<12>/unsigned int/g" \
    -e "s/ap_uint<13>/unsigned int/g" \
    -e "s/ap_uint<14>/unsigned int/g" \
    -e "s/ap_uint<15>/unsigned int/g" \
    -e "s/ap_uint<16>/unsigned int/g" \
    -e "s/MAX_LINES  128/MAX_LINES  65535/g" \
    ../sw/router.hpp > router.hpp

cp ../sw/main.cpp ./main.cpp
