/**
 * main.cpp
 *
 * for Vivado HLS
 */

#ifndef SOFTWARE
#include <stdio.h>
#include <string.h>
//#include <ap_int.h>
#endif

#ifdef SOFTWARE
#include "ap_int.h"
#else
#include <ap_int.h>
#endif

#include "./main.hpp"
//#include "./route.hpp"


// ================================ //
// メルセンヌ・ツイスタ
// ================================ //
#include "mt19937ar.hpp"

void mt_init_genrand(unsigned long s) {
////#pragma HLS INLINE
    init_genrand(s);
}

// AからBの範囲の整数の乱数が欲しいとき
// 参考 http://www.sat.t.u-tokyo.ac.jp/~omi/random_variables_generation.html
unsigned long mt_genrand_int32(int a, int b) {
////#pragma HLS INLINE
    return genrand_int32() % (b - a + 1) + a;
}

// ================================ //
// 探索
// ================================ //

void search(ap_uint<7> size_x, ap_uint<7> size_y, ap_uint<3> size_z,
    ap_uint<17> start, ap_uint<17> goal, ap_uint<8> *w) {

    cout << size_x << " " << size_y << " " << size_z << endl;
    cout << start << " -> " << goal << endl;

}


// ================================ //
// メインモジュール
// ================================ //

bool pynqrouter(char boardstr[BOARDSTR_SIZE], ap_int<8> *status) {
#pragma HLS INTERFACE s_axilite port=boardstr bundle=AXI4LS
#pragma HLS INTERFACE s_axilite port=status bundle=AXI4LS
#pragma HLS INTERFACE s_axilite port=return bundle=AXI4LS

    *status = -127;

    // ボードに関する変数
    ap_uint<7> size_x; // ボードの X サイズ
    ap_uint<7> size_y; // ボードの Y サイズ
    ap_uint<3> size_z; // ボードの Z サイズ

    ap_uint<8> line_size = 0; // ラインの総数

    ap_uint<17> starts[MAX_LINES]; // ラインのスタートリスト
    ap_uint<17> goals[MAX_LINES];  // ラインのゴールリスト

    ap_uint<8> weights[MAX_CELLS];          // セルの重み
    ap_uint<8> paths_size[MAX_LINES];       // ラインが対応するセルIDのサイズ
    ap_uint<17> paths[MAX_LINES][MAX_PATH]; // ラインが対応するセルIDの集合 (スタートとゴールは除く)
    bool adjacents[MAX_LINES];              // スタートとゴールが隣接しているライン
//#pragma HLS ARRAY_PARTITION variable=adjacents complete dim=0

    // ================================
    // 初期化 BEGIN
    // ================================

    // ループカウンタは1ビット余分に用意しないと終了判定できない
    for (ap_uint<8> i = 0; i < (ap_uint<8>)(MAX_LINES); i++) {
//#pragma HLS PIPELINE
        adjacents[i] = false;
    }

    // ボードストリングの解釈
    // unsigned 15bit は 32768 に収まるため
    for (ap_uint<15> idx = 0; ; ) {
#pragma HLS LOOP_TRIPCOUNT min=100 max=32768 avg=1000

        if (boardstr[idx] == 'X') {
            size_x = (boardstr[idx+1] - '0') * 10 + (boardstr[idx+2] - '0');
            idx += 3;
        }
        else if (boardstr[idx] == 'Y') {
            size_y = (boardstr[idx+1] - '0') * 10 + (boardstr[idx+2] - '0');
            idx += 3;
        }
        else if (boardstr[idx] == 'Z') {
            size_z = boardstr[idx+1] - '0';
            idx += 2;
        }
        else if (boardstr[idx] == 'L') {
            //ap_uint<8> i = (boardstr[idx+1] - '0') * 10 + (boardstr[idx+2] - '0');
            ap_uint<7> s_x = (boardstr[idx+1] - '0') * 10 + (boardstr[idx+2] - '0');
            ap_uint<7> s_y = (boardstr[idx+3] - '0') * 10 + (boardstr[idx+4] - '0');
            ap_uint<3> s_z = (boardstr[idx+5] - '0') - 1;
            ap_uint<7> g_x = (boardstr[idx+6] - '0') * 10 + (boardstr[idx+7] - '0');
            ap_uint<7> g_y = (boardstr[idx+8] - '0') * 10 + (boardstr[idx+9] - '0');
            ap_uint<3> g_z = (boardstr[idx+10] - '0') - 1;
            //cout << "L " << line_size << ": " << s_x << " " << s_y << " " << s_z << " "
            //                                  << g_x << " " << g_y << " " << g_z << endl;
            idx += 11;

            // スタートとゴール
            starts[line_size] = (s_z << (BITWIDTH_X + BITWIDTH_Y)) & (s_y << BITWIDTH_X) & s_x;
            goals[line_size]  = (g_z << (BITWIDTH_X + BITWIDTH_Y)) & (g_y << BITWIDTH_X) & g_x;

            // 初期状態で数字が隣接しているか判断
            ap_int<8> dx = g_x - s_x;
            ap_int<8> dy = g_y - s_y;
            ap_int<4> dz = g_z - s_z;
            if ((dx == 0 && dy == 0 && (dz == 1 || dz == -1)) || (dx == 0 && (dy == 1 || dy == -1) && dz == 0) ||
                ((dx == 1 || dx == -1) && dy == 0 && dz == 0)) {
                adjacents[line_size] = true;
            } else {
                adjacents[line_size] = false;
            }

            line_size++;
        }

        // 終端 (null) 文字
        if (boardstr[idx] == 0) {
            break;
        }
    }
    //cout << size_x << " " << size_y << " " << size_z << endl;

    // ループカウンタは1ビット余分に用意しないと終了判定できない
    for (ap_uint<18> i = 0; i < (ap_uint<18>)(MAX_CELLS); i++) {
        weights[i] = 1;
    }

    // ループカウンタは1ビット余分に用意しないと終了判定できない
    for (ap_uint<8> i = 0; i < (ap_uint<18>)(MAX_LINES); i++) {
        paths_size[i] = 0;
    }

    // 乱数の初期化
    mt_init_genrand(12345);

    // ================================
    // 初期化 END
    // ================================

    //printBoard();

    ap_int<8> output;

    // 初期ルーティング
    for (ap_uint<8> i = 0; i < (ap_uint<8>)(line_size); i++) {
#pragma HLS LOOP_TRIPCOUNT min=2 max=128 avg=50

        // 数字が隣接する場合スキップ
        if (adjacents[i] == false) {
            continue;
        }

        // ダイクストラ法
        search(size_x, size_y, size_z, starts[i], goals[i], weights);
    }

    // 解導出できなかった場合
    /*if (isFinished(board) == false) {
        *status = 4;
        return false;
    }*/

    *status = 0;
    return true;
}
