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
#define NOT_USE -1

#define MAX_BOXES 72        // X, Y の最大値 (7ビットで収まる)
#define BITWIDTH_X 7
#define BITWIDTH_Y 7
#define BITMASK_X  127
#define BITMASK_Y  127
#define MAX_LAYER 8         // Z の最大値 (3ビットで収まる)
#define BITWIDTH_Z 3
#define BITMASK_Z  7

#define MAX_CELLS 131072    // セルの総数 =128*128*8 (17ビットで収まる)
#define MAX_LINES 128       // ライン数の最大値 (7ビットで収まる)
#define MAX_PATH 256        // 1つのラインが対応するセル数の最大値 (8ビットで収まる)

#define BOARDSTR_SIZE 32768 // ボードストリングの最大文字数 (15ビットで収まる : 値はとりあえず)

#define MAX_PQ 512          // 探索時のプライオリティ・キューの最大サイズ

#define MAX_WEIGHT  255      // 重みの最大値 (8ビットで収まる)
#define COST_WEIGHT 4        // 重みの更新値

void lfsr_random_init(ap_uint<32> seed);
ap_uint<32> lfsr_random();
//ap_uint<32> lfsr_random_uint32(ap_uint<32> a, ap_uint<32> b);
//ap_uint<32> lfsr_random_uint32_0(ap_uint<32> b);

ap_uint<8> new_weight(ap_uint<8> x);
bool pynqrouter(char boardstr[BOARDSTR_SIZE], ap_uint<32> seed, ap_int<8> *status);

void search(ap_uint<8> *path_size, ap_uint<17> path[MAX_PATH], ap_uint<17> start, ap_uint<17> goal, ap_uint<8> w[MAX_WEIGHT]);
void pq_push(ap_uint<8> priority, ap_uint<17> data, ap_uint<16> *pq_len, ap_uint<8> pq_nodes_priority[MAX_PQ], ap_uint<17> pq_nodes_data[MAX_PQ]);
void pq_pop(ap_uint<8> *ret_priority, ap_uint<17> *ret_data, ap_uint<16> *pq_len, ap_uint<8> pq_nodes_priority[MAX_PQ], ap_uint<17> pq_nodes_data[MAX_PQ]);

#ifdef SOFTWARE
void show_board(ap_uint<7> line_num, ap_uint<8> paths_size[MAX_LINES], ap_uint<17> paths[MAX_LINES][MAX_PATH], ap_uint<17> starts[MAX_LINES], ap_uint<17> goals[MAX_LINES]);
#endif

#endif /* __ROUTER_HPP__ */
