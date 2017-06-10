/**
 * main.hpp
 *
 * for Vivado HLS
 */

#ifndef __MAIN_HPP__
#define __MAIN_HPP__

#ifdef SOFTWARE
#include <bitset>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>

#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#endif

#ifdef SOFTWARE
#include "ap_int.h"
#else
#include <ap_int.h>
#endif


using namespace std;


// メルセンヌ・ツイスタを使う場合
//#define USE_MT 1


// 各種設定値
#define NOT_USE -1

#define MAX_BOXES 72     // X, Y の最大値 (7ビットで収まる)
#define BITWIDTH_X 7
#define BITWIDTH_Y 7
#define MAX_LAYER 8      // Z の最大値 (3ビットで収まる)
#define BITWIDTH_Z 3
#define MAX_CELLS 131072 // セルの総数 =128*128*8 (17ビットで収まる)
#define MAX_LINES 128    // ライン数の最大値 (7ビットで収まる)
#define MAX_PATH 256     // 1つのラインが対応するセル数の最大値 (8ビットで収まる)

#define BOARDSTR_SIZE 32768 // ボードストリングの最大文字数 (15ビットで収まる : 値はとりあえず)


#ifdef USE_MT
// メルセンヌ・ツイスタ
void mt_init_genrand(unsigned long s);
unsigned long mt_genrand_int32(int a, int b);
#else
void lfsr_random_init(ap_uint<32> seed);
ap_uint<32> lfsr_random();
ap_uint<32> lfsr_random_uint32(ap_uint<32> a, ap_uint<32> b);
#endif

bool pynqrouter(char boardstr[BOARDSTR_SIZE], ap_int<8> *status);
//void initialize(char boardstr[BOARDSTR_SIZE], Board *board);
//bool isFinished(Board *board);
//void generateSolution(char boardstr[BOARDSTR_SIZE], Board *board);

void search(ap_uint<7> size_x, ap_uint<7> size_y, ap_uint<3> size_z, ap_uint<17> start, ap_uint<17> goal, ap_uint<8> w[]);


#endif /* __MAIN_HPP__ */
