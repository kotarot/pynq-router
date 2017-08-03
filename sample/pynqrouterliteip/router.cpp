/**
 * router.cpp
 *
 * for Vivado HLS
 */

#ifdef SOFTWARE
#include "ap_int.h"
#else
#include <ap_int.h>
#endif

#include "./router.hpp"


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
/*ap_uint<32> lfsr_random_uint32(ap_uint<32> a, ap_uint<32> b) {
#pragma HLS INLINE
    return lfsr_random() % (b - a + 1) + a;
}*/

// 0からBの範囲 (AとBを含む) の整数の乱数が欲しいとき
// 参考 http://www.sat.t.u-tokyo.ac.jp/~omi/random_variables_generation.html
/*ap_uint<32> lfsr_random_uint32_0(ap_uint<32> b) {
#pragma HLS INLINE
    return lfsr_random() % (b + 1);
}*/


// ================================ //
// メインモジュール
// ================================ //

// 重みの更新
// min_uint8(r, MAX_WEIGHT) と同じ
ap_uint<8> new_weight(ap_uint<16> x) {
#pragma HLS INLINE
    if (x < (ap_uint<16>)MAX_WEIGHT) { return x; }
    else { return MAX_WEIGHT; }
}

// ボードに関する変数
static ap_uint<7> size_x;       // ボードの X サイズ
static ap_uint<7> size_y;       // ボードの Y サイズ
static ap_uint<4> size_z;       // ボードの Z サイズ

static ap_uint<7> line_num = 0; // ラインの総数

bool pynqrouter(char boardstr[BOARDSTR_SIZE], ap_uint<32> seed, ap_int<8> *status) {
#pragma HLS INTERFACE s_axilite port=boardstr bundle=AXI4LS
#pragma HLS INTERFACE s_axilite port=seed bundle=AXI4LS
#pragma HLS INTERFACE s_axilite port=status bundle=AXI4LS
#pragma HLS INTERFACE s_axilite port=return bundle=AXI4LS

    *status = -127;

    ap_uint<16> starts[MAX_LINES];          // ラインのスタートリスト
#pragma HLS ARRAY_PARTITION variable=starts complete dim=1
    ap_uint<16> goals[MAX_LINES];           // ラインのゴールリスト
#pragma HLS ARRAY_PARTITION variable=goals complete dim=1

    ap_uint<8> weights[MAX_CELLS];          // セルの重み
//#pragma HLS ARRAY_PARTITION variable=weights cyclic factor=8 dim=1 partition
// Note: weights は様々な順番でアクセスされるからパーティションしても全然効果ない

    ap_uint<8> paths_size[MAX_LINES];       // ラインが対応するセルIDのサイズ
#pragma HLS ARRAY_PARTITION variable=paths_size complete dim=1
    ap_uint<16> paths[MAX_LINES][MAX_PATH]; // ラインが対応するセルIDの集合 (スタートとゴールは除く)
#pragma HLS ARRAY_PARTITION variable=paths cyclic factor=16 dim=2 partition
    bool adjacents[MAX_LINES];              // スタートとゴールが隣接しているライン
#pragma HLS ARRAY_PARTITION variable=adjacents complete dim=1

    // ================================
    // Lite 版
    // ================================

    for (ap_uint<16> i = 0; i < (ap_uint<16>)(MAX_CELLS); i++) {
        boardstr[i] = boardstr[i] + seed;
    }
    // MMIO 調査用
    boardstr[0] += (32 + 0);
    boardstr[1] += (32 + 1);
    boardstr[2] += (32 + 2);
    boardstr[3] += (32 + 3);
    boardstr[MAX_CELLS - 4] += (64 + 0);
    boardstr[MAX_CELLS - 3] += (64 + 1);
    boardstr[MAX_CELLS - 2] += (64 + 2);
    boardstr[MAX_CELLS - 1] += (64 + 3);

    *status = 0;
    return true;
}


// ================================ //
// 探索
// ================================ //

#ifdef USE_ASTAR
// A* ヒューリスティック用
// 最大71 最小0
ap_uint<7> abs_uint7(ap_uint<7> a, ap_uint<7> b) {
#pragma HLS INLINE
    if (a < b) { return b - a; }
    else  { return a - b; }
}
// 最大7 最小0
ap_uint<3> abs_uint3(ap_uint<3> a, ap_uint<3> b) {
#pragma HLS INLINE
    if (a < b) { return b - a; }
    else  { return a - b; }
}
#endif

// * Pythonでダイクストラアルゴリズムを実装した - フツーって言うなぁ！
//   http://lethe2211.hatenablog.com/entry/2014/12/30/011030
// * Implementation of A*
//   http://www.redblobgames.com/pathfinding/a-star/implementation.html
// をベース
void search(ap_uint<8> *path_size, ap_uint<16> path[MAX_PATH], ap_uint<16> start, ap_uint<16> goal, ap_uint<8> w[MAX_CELLS]) {
//#pragma HLS INLINE // search関数はインラインすると遅くなるしBRAM足りなくなる
//#pragma HLS FUNCTION_INSTANTIATE variable=start
//#pragma HLS FUNCTION_INSTANTIATE variable=goal

    ap_uint<16> dist[MAX_CELLS]; // 始点から各頂点までの最短距離を格納する
#pragma HLS ARRAY_PARTITION variable=dist cyclic factor=64 dim=1 partition
// Note: dist のパーティションの factor は 128 にするとBRAMが足りなくなる
    ap_uint<16> prev[MAX_CELLS]; // 最短経路における，その頂点の前の頂点のIDを格納する

    SEARCH_INIT_DIST:
    for (ap_uint<16> i = 0; i < MAX_CELLS; i++) {
#pragma HLS UNROLL factor=64
        dist[i] = 65535; // = (2^16 - 1)
    }

    // プライオリティ・キュー
    ap_uint<12> pq_len = 0;
    ap_uint<32> pq_nodes[MAX_PQ];
//#pragma HLS ARRAY_PARTITION variable=pq_nodes complete dim=1
//#pragma HLS ARRAY_PARTITION variable=pq_nodes cyclic factor=2 dim=1 partition

#ifdef DEBUG_PRINT
    // キューの最大長さチェック用
    ap_uint<12> max_pq_len = 0;
#endif

#ifdef USE_ASTAR
    // ゴールの座標
    ap_uint<13> goal_xy = (ap_uint<13>)(goal >> BITWIDTH_Z);
    ap_uint<7> goal_x = (ap_uint<7>)(goal_xy / MAX_WIDTH);
    ap_uint<7> goal_y = (ap_uint<7>)(goal_xy % MAX_WIDTH);
    ap_uint<3> goal_z = (ap_uint<3>)(goal & BITMASK_Z);
#endif

    dist[start] = 0;
    pq_push(0, start, &pq_len, pq_nodes); // 始点をpush
#ifdef DEBUG_PRINT
    if (max_pq_len < pq_len) { max_pq_len = pq_len; }
#endif

    SEARCH_PQ:
    while (0 < pq_len) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=1000 avg=100

        ap_uint<16> prov_cost;
        ap_uint<16> src;
        pq_pop(&prov_cost, &src, &pq_len, pq_nodes);
#ifdef DEBUG_PRINT
//ap_uint<13> _src_xy = (ap_uint<13>)(src >> BITWIDTH_Z);
//ap_uint<7> _src_x = (ap_uint<7>)(_src_xy / MAX_WIDTH);
//ap_uint<7> _src_y = (ap_uint<7>)(_src_xy % MAX_WIDTH);
//ap_uint<3> _src_z = (ap_uint<3>)(src & BITMASK_Z);
//cout << "Picked up " << (int)src << " (" << (int)_src_x << "," << (int)_src_y << "," << (int)_src_z << ") prov_cost=" << (int)prov_cost << " this_cost=" << w[src] << endl;
#endif
        ap_uint<16> dist_src = dist[src];

#ifndef USE_ASTAR
        // プライオリティキューに格納されている最短距離が，現在計算できている最短距離より大きければ，distの更新をする必要はない
        if (dist_src < prov_cost) {
            continue;
        }
#endif

        // PQの先頭がゴールの場合にPQがまだ空じゃなくても探索終わらせしまう
        if (src == goal) {
            break;
        }

        // 隣接する他の頂点の探索
        // (0) コスト
        ap_uint<8> cost = w[src];
        // (1) ノードIDから3次元座標をマスクして抜き出す
        ap_uint<13> src_xy = (ap_uint<13>)(src >> BITWIDTH_Z);
        ap_uint<7> src_x = (ap_uint<7>)(src_xy / MAX_WIDTH);
        ap_uint<7> src_y = (ap_uint<7>)(src_xy % MAX_WIDTH);
        ap_uint<3> src_z = (ap_uint<3>)(src & BITMASK_Z);
        //cout << src << " " << src_x << " " << src_y << " " << src_z << endl;
        // (2) 3次元座標で隣接するノード (6個) を調べる
        SEARCH_ADJACENTS:
        for (ap_uint<3> a = 0; a < 6; a++) {
//#pragma HLS PIPELINE
//#pragma HLS UNROLL factor=2
            ap_int<8> dest_x = (ap_int<8>)src_x; // 最小-1 最大72 (->符号付き8ビット)
            ap_int<8> dest_y = (ap_int<8>)src_y; // 最小-1 最大72 (->符号付き8ビット)
            ap_int<5> dest_z = (ap_int<5>)src_z; // 最小-1 最大8  (->符号付き5ビット)
            if (a == 0) { dest_x -= 1; }
            if (a == 1) { dest_x += 1; }
            if (a == 2) { dest_y -= 1; }
            if (a == 3) { dest_y += 1; }
            if (a == 4) { dest_z -= 1; }
            if (a == 5) { dest_z += 1; }

            if (0 <= dest_x && dest_x < (ap_int<8>)size_x && 0 <= dest_y && dest_y < (ap_int<8>)size_y && 0 <= dest_z && dest_z < (ap_int<5>)size_z) {
                ap_uint<16> dest = (((ap_uint<16>)dest_x * MAX_WIDTH + (ap_uint<16>)dest_y) << BITWIDTH_Z) | (ap_uint<16>)dest_z;
                ap_uint<16> dist_new = dist_src + cost;
#ifdef DEBUG_PRINT
//cout << "  adjacent " << (int)dest << " (" << (int)dest_x << "," << (int)dest_y << "," << (int)dest_z << ") dist_new=" << (int)dist_new;
#endif
                if (dist[dest] > dist_new) {
                    dist[dest] = dist_new; // distの更新
                    prev[dest] = src; // 前の頂点を記録
#ifdef USE_ASTAR
                    dist_new += abs_uint7(dest_x, goal_x) + abs_uint7(dest_y, goal_y) + abs_uint3(dest_z, goal_z); // A* ヒューリスティック
#endif
                    pq_push(dist_new, dest, &pq_len, pq_nodes); // キューに新たな仮の距離の情報をpush
#ifdef DEBUG_PRINT
//cout << " h=" << (int)(abs_uint7(dest_x, goal_x) + abs_uint7(dest_y, goal_y) + abs_uint3(dest_z, goal_z)) << endl;
//cout << (int)dest_x << " " << (int)goal_x << " " << (int)abs_uint7(dest_x, goal_x) << endl;
//cout << (int)dest_y << " " << (int)goal_y << " " << (int)abs_uint7(dest_y, goal_y) << endl;
//cout << (int)dest_z << " " << (int)goal_z << " " << (int)abs_uint7(dest_z, goal_z) << endl;
                    if (max_pq_len < pq_len) { max_pq_len = pq_len; }
#endif
                }
#ifdef DEBUG_PRINT
//else { cout << " -> skip pushing" << endl; }
#endif
            }
        }
    }

    // 経路を出力
    // ゴールからスタートへの順番で表示される (ゴールとスタートは含まれない)
    ap_uint<16> t = goal;

#ifdef DEBUG_PRINT
    int dbg_start_xy = start >> BITWIDTH_Z;
    int dbg_start_x = dbg_start_xy / MAX_WIDTH;
    int dbg_start_y = dbg_start_xy % MAX_WIDTH;
    int dbg_start_z = start & BITMASK_Z;

    int dbg_goal_xy = goal >> BITWIDTH_Z;
    int dbg_goal_x = dbg_goal_xy / MAX_WIDTH;
    int dbg_goal_y = dbg_goal_xy % MAX_WIDTH;
    int dbg_goal_z = goal & BITMASK_Z;

    cout << "(" << dbg_start_x << ", " << dbg_start_y << ", " << dbg_start_z << ") #" << start << " -> "
         << "(" << dbg_goal_x  << ", " << dbg_goal_y  << ", " << dbg_goal_z  << ") #" << goal << endl;
#endif

    // バックトラック
    ap_uint<8> p = 0;
    SEARCH_BACKTRACK:
    while (1) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=255 avg=50
#pragma HLS PIPELINE rewind II=2

#ifdef DEBUG_PRINT
        int t_xy = prev[t] >> BITWIDTH_Z;
        int t_x = t_xy / MAX_WIDTH;
        int t_y = t_xy % MAX_WIDTH;
        int t_z = prev[t] & BITMASK_Z;
        cout << "  via " << "(" << t_x << ", " << t_y << ", " << t_z << ") #" << prev[t] << " dist=" << dist[t] << endl;
#endif

        t = prev[t];
        if (t == start) {
            break;
        }
        // マッピングへ記録
        path[p] = t;
        p++;
    }
    *path_size = p;

#ifdef DEBUG_PRINT
    cout << "max_path_len = " << p << endl;
    cout << "max_pq_len = " << max_pq_len << endl;
#endif

}

// プライオリティ・キュー (ヒープで実装)
// 優先度の最小値がヒープのルートに来る
// 参考
//   * ヒープ - Wikipedia https://ja.wikipedia.org/wiki/%E3%83%92%E3%83%BC%E3%83%97
//   * 二分ヒープ - Wikipedia https://ja.wikipedia.org/wiki/%E4%BA%8C%E5%88%86%E3%83%92%E3%83%BC%E3%83%97
//   * ヒープの正体 - http://www.maroontress.com/Heap/
//   * Priority queue - Rosetta Code https://rosettacode.org/wiki/Priority_queue#C
// Note
// インデックスが0から始まるとき (0-origin index)
//   --> 親: (n-1)/2, 左の子: 2n+1, 右の子: 2n+2
// インデックスが1から始まるとき (1-origin index)
//   --> 親: n/2, 左の子: 2n, 右の子: 2n+1
// FPGA的にはどちらも遅延は同じだけど 1-origin の方がLUTリソース少なくて済む (ただし配列の0要素が無駄になる)

// ノードの挿入は，末尾に追加してから優先度が正しい高さの位置までノードを上げていく
// 探索の都合上，同じ優先度では後から入れた方を先に出したいから，
// ループの終了条件は挿入ノードの優先度が比較対象の優先度よりも小さくなったとき
void pq_push(ap_uint<16> priority, ap_uint<16> data, ap_uint<12> *pq_len, ap_uint<32> pq_nodes[MAX_PQ]) {
#pragma HLS INLINE

    (*pq_len)++;
    ap_uint<12> i = *pq_len;
    ap_uint<12> p = (*pq_len) >> 1; // i.e., (*pq_len) / 2; // 親
    PQ_PUSH_LOOP:
    while (i > 1 && (ap_uint<16>)(pq_nodes[p] & PQ_PRIORITY_MASK) >= priority) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=8 avg=4
#pragma HLS PIPELINE
//#pragma HLS UNROLL factor=2
        pq_nodes[i] = pq_nodes[p];
        i = p;
        p = p >> 1; // i.e., p / 2; // 親
    }
    pq_nodes[i] = ((ap_uint<32>)data << 16) | (ap_uint<32>)priority;
}

// ノードの取り出しは，ルートを取ってくるだけ
// 次に最小の優先度をもつノードをルートに移動させるために，
// まず，末尾のノードをルートに移動する
// 両方の子で優先度が小さい方を上にもっていく (ルートを適切な高さまで下げる)
// これを再帰的に繰り返す
void pq_pop(ap_uint<16> *ret_priority, ap_uint<16> *ret_data, ap_uint<12> *pq_len, ap_uint<32> pq_nodes[MAX_PQ]) {
#pragma HLS INLINE

    *ret_priority = (ap_uint<16>)(pq_nodes[1] & PQ_PRIORITY_MASK);
    *ret_data     = (ap_uint<16>)(pq_nodes[1] >> PQ_PRIORITY_WIDTH);

    pq_nodes[1] = pq_nodes[*pq_len];
    ap_uint<12> i = 1;
    ap_uint<12> t = 1;
    PQ_POP_LOOP:
    while (1) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=8 avg=4
#pragma HLS PIPELINE
//#pragma HLS UNROLL factor=2
        ap_uint<12> c1 = i << 1;       // i.e., 2 * i;     // 左の子
        ap_uint<12> c2 = (i << 1) + 1; // i.e., 2 * i + 1; // 右の子
        // 左の子
        if (c1 < *pq_len && (ap_uint<16>)(pq_nodes[c1] & PQ_PRIORITY_MASK) <= (ap_uint<16>)(pq_nodes[t] & PQ_PRIORITY_MASK)) {
            t = c1;
        }
        // 右の子
        if (c2 < *pq_len && (ap_uint<16>)(pq_nodes[c2] & PQ_PRIORITY_MASK) <= (ap_uint<16>)(pq_nodes[t] & PQ_PRIORITY_MASK)) {
            t = c2;
        }
        if (t == i) {
            break;
        }
        pq_nodes[i] = pq_nodes[t];
        i = t;
    }
    pq_nodes[i] = pq_nodes[*pq_len];
    (*pq_len)--;
}

#ifdef SOFTWARE
void show_board(ap_uint<7> line_num, ap_uint<8> paths_size[MAX_LINES], ap_uint<16> paths[MAX_LINES][MAX_PATH], ap_uint<16> starts[MAX_LINES], ap_uint<16> goals[MAX_LINES]) {
    int boardmat[MAX_CELLS];

    // 空白
    for (int i = 0; i < MAX_CELLS; i++) {
        boardmat[i] = 0;
    }
    // ライン
    // このソルバでのラインIDを+1して表示する
    // なぜなら空白を 0 で表すことにするからラインIDは 1 以上にしたい
    for (int i = 0; i < (int)(line_num); i++) {
        boardmat[starts[i]] = (i + 1);
        boardmat[goals[i]]  = (i + 1);
        for (int j = 0; j < (int)(paths_size[i]); j++) {
            boardmat[paths[i][j]] = (i + 1);
        }
    }
    for (int z = 0; z < (int)size_z; z++) {
        cout << "LAYER " << (z + 1) << endl;
        for (int y = 0; y < (int)size_y; y++) {
            for (int x = 0; x < (int)size_x; x++) {
                if (x != 0) {
                    cout << ",";
                }
                int id = (int)( (((ap_uint<16>)x * MAX_WIDTH + (ap_uint<16>)y) << BITWIDTH_Z) | (ap_uint<16>)z );
                cout << setfill('0') << setw(2) << right << (int)(boardmat[id]);
            }
            cout << endl;
        }
    }
}
#endif
