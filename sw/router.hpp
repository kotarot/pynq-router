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

//#define DEBUG_PRINT  // いろいろ表示する
#define USE_ASTAR    // A* 探索を使う
#define USE_MOD_CALC // ターゲットラインの選択に剰余演算を用いる

using namespace std;


// 各種設定値
#define MAX_WIDTH   72      // X, Y の最大値 (7ビットで収まる)
#define BITWIDTH_XY 13
#define BITMASK_XY  65528   // 1111 1111 1111 1000
#define MAX_LAYER   8       // Z の最大値 (3ビット)
#define BITWIDTH_Z  3
#define BITMASK_Z   7       // 0000 0000 0000 0111

#define MAX_CELLS  41472    // セルの総数 =72*72*8 (16ビットで収まる)
#define MAX_LINES  128      // ライン数の最大値 (7ビット)
#define MAX_PATH   256      // 1つのラインが対応するセル数の最大値 (8ビット)
#define MAX_PQ     4096     // 探索時のプライオリティ・キューの最大サイズ (12ビット) 足りないかも？

#define PQ_PRIORITY_WIDTH 16
#define PQ_PRIORITY_MASK  65535       // 0000 0000 0000 0000 1111 1111 1111 1111
#define PQ_DATA_WIDTH     16
#define PQ_DATA_MASK      4294901760  // 1111 1111 1111 1111 0000 0000 0000 0000

#define MAX_WEIGHT 255      // 重みの最大値 (8ビットで収まる)
#define BOARDSTR_SIZE 41472 // ボードストリングの最大文字数 (セル数 72*72*8 あれば良い)


void lfsr_random_init(ap_uint<32> seed);
ap_uint<32> lfsr_random();
//ap_uint<32> lfsr_random_uint32(ap_uint<32> a, ap_uint<32> b);
//ap_uint<32> lfsr_random_uint32_0(ap_uint<32> b);

ap_uint<8> new_weight(ap_uint<16> x);
bool pynqrouter(char boardstr[BOARDSTR_SIZE], ap_uint<32> seed, ap_int<32> *status);

#ifdef USE_ASTAR
ap_uint<7> abs_uint7(ap_uint<7> a, ap_uint<7> b);
ap_uint<3> abs_uint3(ap_uint<3> a, ap_uint<3> b);
#endif

void search(ap_uint<8> *path_size, ap_uint<16> path[MAX_PATH], ap_uint<16> start, ap_uint<16> goal, ap_uint<8> w[MAX_WEIGHT]);
void pq_push(ap_uint<16> priority, ap_uint<16> data, ap_uint<12> *pq_len, ap_uint<32> pq_nodes[MAX_PQ]);
void pq_pop(ap_uint<16> *ret_priority, ap_uint<16> *ret_data, ap_uint<12> *pq_len, ap_uint<32> pq_nodes[MAX_PQ]);

#endif /* __ROUTER_HPP__ */
