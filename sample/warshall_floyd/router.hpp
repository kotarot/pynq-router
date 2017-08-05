/**
 * router.hpp
 *
 * for Vivado HLS
 */

#ifndef __ROUTER_HPP__
#define __ROUTER_HPP__

#ifdef SOFTWARE
#include "ap_int.h"
#else
#include <ap_int.h>
#endif

#ifdef SOFTWARE
//#define DEBUG_PRINT
#endif


using namespace std;


// 各種設定値
#define MAX_CELLS 131072    // セルの総数 =128*128*8 (17ビットで収まる)
#define MAX_DIST 65535      // 2^16 - 1


ap_uint<16> pynqrouter(ap_uint<8> weights[MAX_CELLS]);

#endif /* __ROUTER_HPP__ */
