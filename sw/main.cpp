/**
 * main.cpp
 *
 * for Vivado HLS
 */

#ifndef SOFTWARE
#include <stdio.h>
#include <string.h>
#endif

#ifdef SOFTWARE
#include "ap_int.h"
#else
#include <ap_int.h>
#endif

#include "./main.hpp"


#ifdef USE_MT
// ================================ //
// メルセンヌ・ツイスタ
// ================================ //
#include "mt19937ar.hpp"

void mt_init_genrand(unsigned long s) {
#pragma HLS INLINE
    init_genrand(s);
}

// AからBの範囲 (AとBを含む) の整数の乱数が欲しいとき
// 参考 http://www.sat.t.u-tokyo.ac.jp/~omi/random_variables_generation.html
unsigned long mt_genrand_int32(int a, int b) {
#pragma HLS INLINE
    return genrand_int32() % (b - a + 1) + a;
}


#else
// ================================ //
// LFST
// ================================ //

// 参考 https://highlevel-synthesis.com/2017/02/10/lfsr-in-hls/
static ap_uint<32> lfsr;

void lfsr_random_init(ap_uint<32> seed) {
#pragma HLS INLINE
    lfsr = seed;
}

ap_uint<32> lfsr_random() {
#pragma HLS INLINE
    bool b_32 = lfsr.get_bit(32-32);
    bool b_22 = lfsr.get_bit(32-22);
    bool b_2 = lfsr.get_bit(32-2);
    bool b_1 = lfsr.get_bit(32-1);
    bool new_bit = b_32 ^ b_22 ^ b_2 ^ b_1;
    lfsr = lfsr >> 1;
    lfsr.set_bit(31, new_bit);

    return lfsr.to_uint();
}

// AからBの範囲 (AとBを含む) の整数の乱数が欲しいとき
// 参考 http://www.sat.t.u-tokyo.ac.jp/~omi/random_variables_generation.html
ap_uint<32> lfsr_random_uint32(ap_uint<32> a, ap_uint<32> b) {
#pragma HLS INLINE
    return lfsr_random() % (b - a + 1) + a;
}
#endif


// ================================ //
// メインモジュール
// ================================ //

bool pynqrouter(char boardstr[BOARDSTR_SIZE], ap_int<8> *status) {
#pragma HLS INTERFACE s_axilite port=boardstr bundle=AXI4LS
#pragma HLS INTERFACE s_axilite port=status bundle=AXI4LS
#pragma HLS INTERFACE s_axilite port=return bundle=AXI4LS

    *status = -127;

    // ボードに関する変数
    ap_uint<7> size_x;                      // ボードの X サイズ
    ap_uint<7> size_y;                      // ボードの Y サイズ
    ap_uint<3> size_z;                      // ボードの Z サイズ

    ap_uint<8> line_num = 0;                // ラインの総数

    ap_uint<17> starts[MAX_LINES];          // ラインのスタートリスト
//#pragma HLS ARRAY_PARTITION variable=starts complete dim=0
    ap_uint<17> goals[MAX_LINES];           // ラインのゴールリスト
//#pragma HLS ARRAY_PARTITION variable=goals complete dim=0

    ap_uint<8> weights[MAX_CELLS];          // セルの重み
    ap_uint<8> paths_size[MAX_LINES];       // ラインが対応するセルIDのサイズ
//#pragma HLS ARRAY_PARTITION variable=paths_size complete dim=0
    ap_uint<17> paths[MAX_LINES][MAX_PATH]; // ラインが対応するセルIDの集合 (スタートとゴールは除く)
    bool adjacents[MAX_LINES];              // スタートとゴールが隣接しているライン
//#pragma HLS ARRAY_PARTITION variable=adjacents complete dim=0

    ap_int<9> boardmat[MAX_CELLS];          // ボードマトリックス

    // ================================
    // 初期化 BEGIN
    // ================================

    // ループカウンタは1ビット余分に用意しないと終了判定できない
    for (ap_uint<8> i = 0; i < (ap_uint<8>)(MAX_LINES); i++) {
//#pragma HLS UNROLL
        adjacents[i] = false;
    }

    // ボードストリングの解釈
    for (ap_uint<16> idx = 0; idx < BOARDSTR_SIZE; ) {
//#pragma HLS PIPELINE
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
            ap_uint<7> s_x = (boardstr[idx+1] - '0') * 10 + (boardstr[idx+2] - '0');
            ap_uint<7> s_y = (boardstr[idx+3] - '0') * 10 + (boardstr[idx+4] - '0');
            ap_uint<3> s_z = (boardstr[idx+5] - '0') - 1;
            ap_uint<7> g_x = (boardstr[idx+6] - '0') * 10 + (boardstr[idx+7] - '0');
            ap_uint<7> g_y = (boardstr[idx+8] - '0') * 10 + (boardstr[idx+9] - '0');
            ap_uint<3> g_z = (boardstr[idx+10] - '0') - 1;
            //cout << "L " << line_num << ": (" << s_x << ", " << s_y << ", " << s_z << ") "
            //                              "(" << g_x << ", " << g_y << ", " << g_z << ")" << endl;
            idx += 11;

            // スタートとゴール
            starts[line_num] = ((ap_uint<17>)s_z << (BITWIDTH_X + BITWIDTH_Y)) | ((ap_uint<17>)s_y << BITWIDTH_X) | (ap_uint<17>)s_x;
            goals[line_num]  = ((ap_uint<17>)g_z << (BITWIDTH_X + BITWIDTH_Y)) | ((ap_uint<17>)g_y << BITWIDTH_X) | (ap_uint<17>)g_x;

            // 初期状態で数字が隣接しているか判断
            ap_int<8> dx = g_x - s_x;
            ap_int<8> dy = g_y - s_y;
            ap_int<4> dz = g_z - s_z;
            if ((dx == 0 && dy == 0 && (dz == 1 || dz == -1)) || (dx == 0 && (dy == 1 || dy == -1) && dz == 0) ||
                ((dx == 1 || dx == -1) && dy == 0 && dz == 0)) {
                adjacents[line_num] = true;
            } else {
                adjacents[line_num] = false;
            }

            line_num++;
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
        // スタートとゴールの重みは最大にしておく
        weights[starts[i]] = MAX_WEIGHT;
        weights[goals[i]]  = MAX_WEIGHT;
    }

    // 乱数の初期化
#if USE_MT
    mt_init_genrand(12345);
#else
    lfsr_random_init(12345);
#endif

    // 乱数テスト
    //for (int a = 0; a < 100; a++)
    //    cout << static_cast<std::bitset<32> >(lfsr_random()) << endl;
    //for (int a = 0; a < 100; a++)
    //    cout << lfsr_random_uint32(10, 100) << endl;

    // ================================
    // 初期化 END
    // ================================

    // ================================
    // ルーティング BEGIN
    // ================================

    // ルーティングイテレーション
    for (ap_uint<8> iter = 0; iter < 100; iter++) {

        // 初期ルーティング
        for (ap_uint<8> i = 0; i < (ap_uint<8>)(line_num); i++) {
//#pragma HLS UNROLL
#pragma HLS LOOP_TRIPCOUNT min=2 max=128 avg=50

            // 数字が隣接する場合スキップ
            if (adjacents[i] == true) {
                continue;
            }

            // ダイクストラ法
            search(&(paths_size[i]), paths[i], size_x, size_y, size_z, starts[i], goals[i], weights);
        }

        // 全ラインのルーティング後
        // (1) オーバーラップのチェック
        bool has_overlap = false;
        ap_uint<8> overlaps[MAX_CELLS];
        for (ap_uint<18> i = 0; i < (ap_uint<18>)(MAX_CELLS); i++) {
            overlaps[i] = 0;
        }
        for (ap_uint<8> i = 0; i < (ap_uint<8>)(line_num); i++) {
            (overlaps[starts[i]])++;
            (overlaps[goals[i]])++;
            for (ap_uint<9> j = 0; j < (ap_uint<9>)(paths_size[i]); j++) {
                ap_uint<17> cell_id = paths[i][j];
                if (0 < overlaps[cell_id]) {
                    has_overlap = true;
                    (overlaps[cell_id])++;
                }
            }
        }
        // (1') オーバーラップなければ探索終了
        if (has_overlap == false) {
            break;
        }
        // (2) 重みの更新
        for (ap_uint<18> i = 0; i < (ap_uint<18>)(MAX_CELLS); i++) {
            weights[i] = 1;
        }

    }

    // 解導出できなかった場合
    /*if (isFinished(board) == false) {
        *status = 4;
        return false;
    }*/

    // ================================
    // ルーティング END
    // ================================

    // ================================
    // 解生成 BEGIN
    // ================================

    // 空白
    for (ap_uint<18> i = 0; i < (ap_uint<18>)(MAX_CELLS); i++) {
        boardmat[i] = 0;
    }
    // ライン
    for (ap_uint<8> i = 0; i < (ap_uint<8>)(line_num); i++) {
        boardmat[starts[i]] = (i + 1);
        boardmat[goals[i]]  = (i + 1);
        for (ap_uint<9> j = 0; j < (ap_uint<9>)(paths_size[i]); j++) {
            boardmat[paths[i][j]] = (i + 1);
        }
    }

    ap_uint<17> i = 0;
    for (ap_uint<4> z = 0; z < (ap_uint<4>)(size_z); z++) {
        for (ap_uint<8> y = 0; y < (ap_uint<8>)(size_y); y++) {
            for (ap_uint<8> x = 0; x < (ap_uint<8>)(size_x); x++) {
                ap_uint<17> cell_id = ((ap_uint<17>)z << (BITWIDTH_X + BITWIDTH_Y)) | ((ap_uint<17>)y << BITWIDTH_X) | (ap_uint<17>)x;
                boardstr[i] = boardmat[cell_id];
                i++;
            }
        }
    }

    // ================================
    // 解生成 END
    // ================================

    *status = 0;
    return true;
}


// ================================ //
// 探索
// ================================ //

// Pythonでダイクストラアルゴリズムを実装した - フツーって言うなぁ！
// http://lethe2211.hatenablog.com/entry/2014/12/30/011030
// をベース
void search(ap_uint<8> *path_size, ap_uint<17> path[MAX_PATH], ap_uint<7> size_x, ap_uint<7> size_y, ap_uint<3> size_z,
    ap_uint<17> start, ap_uint<17> goal, ap_uint<8> w[]) {

    ap_uint<16> dist[MAX_CELLS]; // 始点から各頂点までの最短距離を格納する
    ap_uint<17> prev[MAX_CELLS]; // 最短経路における，その頂点の前の頂点のIDを格納する
    for (ap_uint<18> i = 0; i < MAX_CELLS; i++) {
        dist[i] = 65535; // = (2^16 - 1)
    }

    // プライオリティ・キュー
    int pq_len = 0;
    int pq_size = 0;
    int pq_nodes_priority[MAX_PQ];
    int pq_nodes_data[MAX_PQ];

    // プライオリティ・キューのテスト
    //pq_push(10, 1, &pq_len, &pq_size, pq_nodes_priority, pq_nodes_data);
    //pq_push(50, 2, &pq_len, &pq_size, pq_nodes_priority, pq_nodes_data);
    //pq_push(20, 3, &pq_len, &pq_size, pq_nodes_priority, pq_nodes_data);
    //pq_push(40, 4, &pq_len, &pq_size, pq_nodes_priority, pq_nodes_data);
    //pq_push(30, 5, &pq_len, &pq_size, pq_nodes_priority, pq_nodes_data);
    //cout << "p " << pq_pop(&pq_len, &pq_size, pq_nodes_priority, pq_nodes_data) << " " << pq_len << " " << pq_size << endl;
    //cout << "p " << pq_pop(&pq_len, &pq_size, pq_nodes_priority, pq_nodes_data) << " " << pq_len << " " << pq_size << endl;
    //cout << "p " << pq_pop(&pq_len, &pq_size, pq_nodes_priority, pq_nodes_data) << " " << pq_len << " " << pq_size << endl;
    //pq_push(5,   101, &pq_len, &pq_size, pq_nodes_priority, pq_nodes_data);
    //pq_push(100, 102, &pq_len, &pq_size, pq_nodes_priority, pq_nodes_data);
    //cout << "p " << pq_pop(&pq_len, &pq_size, pq_nodes_priority, pq_nodes_data) << " " << pq_len << " " << pq_size << endl;
    //cout << "p " << pq_pop(&pq_len, &pq_size, pq_nodes_priority, pq_nodes_data) << " " << pq_len << " " << pq_size << endl;
    //cout << "p " << pq_pop(&pq_len, &pq_size, pq_nodes_priority, pq_nodes_data) << " " << pq_len << " " << pq_size << endl;
    //cout << "p " << pq_pop(&pq_len, &pq_size, pq_nodes_priority, pq_nodes_data) << " " << pq_len << " " << pq_size << endl;
    //cout << pq_len << endl;
    //cout << "p " << pq_pop(&pq_len, &pq_size, pq_nodes_priority, pq_nodes_data) << " " << pq_len << " " << pq_size << endl;

    dist[start] = 0;
    pq_push(0, start, &pq_len, &pq_size, pq_nodes_priority, pq_nodes_data); // 始点をpush

    while (0 < pq_len) {
        int prov_cost;
        int src;
        pq_pop(&prov_cost, &src, &pq_len, &pq_size, pq_nodes_priority, pq_nodes_data);

        // プライオリティキューに格納されている最短距離が，現在計算できている最短距離より大きければ，distの更新をする必要はない
        // そうでない場合は計算する
        if (dist[src] >= prov_cost) {
            // 隣接する他の頂点の探索
            // (0) コスト
            ap_uint<8> cost = w[src];
            // (1) ノードIDから3次元座標をマスクして抜き出す
            int src_x =  src &  BITMASK_X;
            int src_y = (src & (BITMASK_Y << BITWIDTH_X)) >> BITWIDTH_X;
            int src_z = (src & (BITMASK_Z << (BITWIDTH_X + BITWIDTH_Y))) >> (BITWIDTH_X + BITWIDTH_Y);
            //cout << src << " " << src_x << " " << src_y << " " << src_z << endl;
            // (2) 3次元座標で隣接するノード (6個) を調べる
            for (int a = 0; a < 6; a++) {
                int dest_x = src_x;
                int dest_y = src_y;
                int dest_z = src_z;
                if (a == 0) { dest_x -= 1; }
                if (a == 1) { dest_x += 1; }
                if (a == 2) { dest_y -= 1; }
                if (a == 3) { dest_y += 1; }
                if (a == 4) { dest_z -= 1; }
                if (a == 5) { dest_z += 1; }

                int dest = ((ap_uint<17>)dest_z << (BITWIDTH_X + BITWIDTH_Y)) | ((ap_uint<17>)dest_y << BITWIDTH_X) | (ap_uint<17>)dest_x;
                if (0 <= dest_x && dest_x < size_x && 0 <= dest_y && dest_y < size_y && 0 <= dest_z && dest_z < size_z) {
                    if (dist[dest] > dist[src] + cost) {
                        dist[dest] = dist[src] + cost;
                            // distの更新
                        pq_push(dist[dest], dest, &pq_len, &pq_size, pq_nodes_priority, pq_nodes_data);
                            // キューに新たな仮の距離の情報をpush
                        prev[dest] = src;
                            // 前の頂点を記録
                    }
                }
             }
        }
    }

    // 経路を出力
    // ゴールからスタートへの順番で表示される (ゴールとスタートは含まれない)
    ap_uint<17> t = goal;
    cout << start << " -> " << goal << endl;
    int p = 0;
    while (prev[t] != start) {
        int t_x =  prev[t] &  BITMASK_X;
        int t_y = (prev[t] & (BITMASK_Y << BITWIDTH_X)) >> BITWIDTH_X;
        int t_z = (prev[t] & (BITMASK_Z << (BITWIDTH_X + BITWIDTH_Y))) >> (BITWIDTH_X + BITWIDTH_Y);
        cout << "  via " << prev[t] << " (" << t_x << ", " << t_y << ", " << t_z << ")" << endl;

        // マッピングへ記録
        path[p] = prev[t];

        t = prev[t];
        p++;
    }
    *path_size = p;
}

// プライオリティ・キュー (ヒープ)
// 参考 https://rosettacode.org/wiki/Priority_queue#C
void pq_push(int priority, int data, int *pq_len, int *pq_size, int pq_nodes_priority[MAX_PQ], int pq_nodes_data[MAX_PQ]) {

    (*pq_len)++;
    if (*pq_len >= *pq_size) {
        if (0 < *pq_size) {
            *pq_size *= 2;
        } else {
            *pq_size = 4;
        }
    }
    int i = *pq_len;
    int j = i / 2;
    while (i > 1 && pq_nodes_priority[j] > priority) {
        pq_nodes_priority[i] = pq_nodes_priority[j];
        pq_nodes_data[i]     = pq_nodes_data[j];
        i = j;
        j /= 2;
    }
    pq_nodes_priority[i] = priority;
    pq_nodes_data[i]     = data;
}

void pq_pop(int *ret_priority, int *ret_data, int *pq_len, int *pq_size, int pq_nodes_priority[MAX_PQ], int pq_nodes_data[MAX_PQ]) {
    *ret_priority = pq_nodes_priority[1];
    *ret_data = pq_nodes_data[1];
    pq_nodes_priority[1] = pq_nodes_priority[*pq_len];
    pq_nodes_data[1]     = pq_nodes_data[*pq_len];
    (*pq_len)--;
    int i = 1;
    while (1) {
        int k = i;
        int j = 2 * i;
        if (j <= *pq_len && pq_nodes_priority[j] < pq_nodes_priority[k]) {
            k = j;
        }
        if (j + 1 <= *pq_len && pq_nodes_priority[j + 1] < pq_nodes_priority[k]) {
            k = j + 1;
        }
        if (k == i) {
            break;
        }
        pq_nodes_priority[i] = pq_nodes_priority[k];
        pq_nodes_data[i]     = pq_nodes_data[k];
        i = k;
    }
    pq_nodes_priority[i] = pq_nodes_priority[*pq_len + 1];
    pq_nodes_data[i]     = pq_nodes_data[*pq_len + 1];
}
