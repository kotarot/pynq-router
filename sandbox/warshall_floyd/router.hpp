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


// �e��ݒ�l
#define MAX_CELLS 131072    // �Z���̑��� =128*128*8 (17�r�b�g�Ŏ��܂�)
#define MAX_DIST 65535      // 2^16 - 1


ap_uint<16> pynqrouter(ap_uint<8> weights[MAX_CELLS]);

#endif /* __ROUTER_HPP__ */
