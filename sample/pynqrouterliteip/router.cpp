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

// �Q�l https://highlevel-synthesis.com/2017/02/10/lfsr-in-hls/
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

// A����B�͈̔� (A��B���܂�) �̐����̗������~�����Ƃ�
// �Q�l http://www.sat.t.u-tokyo.ac.jp/~omi/random_variables_generation.html
/*ap_uint<32> lfsr_random_uint32(ap_uint<32> a, ap_uint<32> b) {
#pragma HLS INLINE
    return lfsr_random() % (b - a + 1) + a;
}*/

// 0����B�͈̔� (A��B���܂�) �̐����̗������~�����Ƃ�
// �Q�l http://www.sat.t.u-tokyo.ac.jp/~omi/random_variables_generation.html
/*ap_uint<32> lfsr_random_uint32_0(ap_uint<32> b) {
#pragma HLS INLINE
    return lfsr_random() % (b + 1);
}*/


// ================================ //
// ���C�����W���[��
// ================================ //

// �d�݂̍X�V
// min_uint8(r, MAX_WEIGHT) �Ɠ���
ap_uint<8> new_weight(ap_uint<16> x) {
#pragma HLS INLINE
    if (x < (ap_uint<16>)MAX_WEIGHT) { return x; }
    else { return MAX_WEIGHT; }
}

// �{�[�h�Ɋւ���ϐ�
static ap_uint<7> size_x;       // �{�[�h�� X �T�C�Y
static ap_uint<7> size_y;       // �{�[�h�� Y �T�C�Y
static ap_uint<4> size_z;       // �{�[�h�� Z �T�C�Y

static ap_uint<7> line_num = 0; // ���C���̑���

bool pynqrouter(char boardstr[BOARDSTR_SIZE], ap_uint<32> seed, ap_int<8> *status) {
#pragma HLS INTERFACE s_axilite port=boardstr bundle=AXI4LS
#pragma HLS INTERFACE s_axilite port=seed bundle=AXI4LS
#pragma HLS INTERFACE s_axilite port=status bundle=AXI4LS
#pragma HLS INTERFACE s_axilite port=return bundle=AXI4LS

    *status = -127;

    ap_uint<16> starts[MAX_LINES];          // ���C���̃X�^�[�g���X�g
#pragma HLS ARRAY_PARTITION variable=starts complete dim=1
    ap_uint<16> goals[MAX_LINES];           // ���C���̃S�[�����X�g
#pragma HLS ARRAY_PARTITION variable=goals complete dim=1

    ap_uint<8> weights[MAX_CELLS];          // �Z���̏d��
//#pragma HLS ARRAY_PARTITION variable=weights cyclic factor=8 dim=1 partition
// Note: weights �͗l�X�ȏ��ԂŃA�N�Z�X����邩��p�[�e�B�V�������Ă��S�R���ʂȂ�

    ap_uint<8> paths_size[MAX_LINES];       // ���C�����Ή�����Z��ID�̃T�C�Y
#pragma HLS ARRAY_PARTITION variable=paths_size complete dim=1
    ap_uint<16> paths[MAX_LINES][MAX_PATH]; // ���C�����Ή�����Z��ID�̏W�� (�X�^�[�g�ƃS�[���͏���)
#pragma HLS ARRAY_PARTITION variable=paths cyclic factor=16 dim=2 partition
    bool adjacents[MAX_LINES];              // �X�^�[�g�ƃS�[�����אڂ��Ă��郉�C��
#pragma HLS ARRAY_PARTITION variable=adjacents complete dim=1

    // ================================
    // Lite ��
    // ================================

    for (ap_uint<16> i = 0; i < (ap_uint<16>)(MAX_CELLS); i++) {
        boardstr[i] = boardstr[i] + seed;
    }
    // MMIO �����p
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
// �T��
// ================================ //

#ifdef USE_ASTAR
// A* �q���[���X�e�B�b�N�p
// �ő�71 �ŏ�0
ap_uint<7> abs_uint7(ap_uint<7> a, ap_uint<7> b) {
#pragma HLS INLINE
    if (a < b) { return b - a; }
    else  { return a - b; }
}
// �ő�7 �ŏ�0
ap_uint<3> abs_uint3(ap_uint<3> a, ap_uint<3> b) {
#pragma HLS INLINE
    if (a < b) { return b - a; }
    else  { return a - b; }
}
#endif

// * Python�Ń_�C�N�X�g���A���S���Y������������ - �t�c�[���Č����Ȃ��I
//   http://lethe2211.hatenablog.com/entry/2014/12/30/011030
// * Implementation of A*
//   http://www.redblobgames.com/pathfinding/a-star/implementation.html
// ���x�[�X
void search(ap_uint<8> *path_size, ap_uint<16> path[MAX_PATH], ap_uint<16> start, ap_uint<16> goal, ap_uint<8> w[MAX_CELLS]) {
//#pragma HLS INLINE // search�֐��̓C�����C������ƒx���Ȃ邵BRAM����Ȃ��Ȃ�
//#pragma HLS FUNCTION_INSTANTIATE variable=start
//#pragma HLS FUNCTION_INSTANTIATE variable=goal

    ap_uint<16> dist[MAX_CELLS]; // �n�_����e���_�܂ł̍ŒZ�������i�[����
#pragma HLS ARRAY_PARTITION variable=dist cyclic factor=64 dim=1 partition
// Note: dist �̃p�[�e�B�V������ factor �� 128 �ɂ����BRAM������Ȃ��Ȃ�
    ap_uint<16> prev[MAX_CELLS]; // �ŒZ�o�H�ɂ�����C���̒��_�̑O�̒��_��ID���i�[����

    SEARCH_INIT_DIST:
    for (ap_uint<16> i = 0; i < MAX_CELLS; i++) {
#pragma HLS UNROLL factor=64
        dist[i] = 65535; // = (2^16 - 1)
    }

    // �v���C�I���e�B�E�L���[
    ap_uint<12> pq_len = 0;
    ap_uint<32> pq_nodes[MAX_PQ];
//#pragma HLS ARRAY_PARTITION variable=pq_nodes complete dim=1
//#pragma HLS ARRAY_PARTITION variable=pq_nodes cyclic factor=2 dim=1 partition

#ifdef DEBUG_PRINT
    // �L���[�̍ő咷���`�F�b�N�p
    ap_uint<12> max_pq_len = 0;
#endif

#ifdef USE_ASTAR
    // �S�[���̍��W
    ap_uint<13> goal_xy = (ap_uint<13>)(goal >> BITWIDTH_Z);
    ap_uint<7> goal_x = (ap_uint<7>)(goal_xy / MAX_WIDTH);
    ap_uint<7> goal_y = (ap_uint<7>)(goal_xy % MAX_WIDTH);
    ap_uint<3> goal_z = (ap_uint<3>)(goal & BITMASK_Z);
#endif

    dist[start] = 0;
    pq_push(0, start, &pq_len, pq_nodes); // �n�_��push
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
        // �v���C�I���e�B�L���[�Ɋi�[����Ă���ŒZ�������C���݌v�Z�ł��Ă���ŒZ�������傫����΁Cdist�̍X�V������K�v�͂Ȃ�
        if (dist_src < prov_cost) {
            continue;
        }
#endif

        // PQ�̐擪���S�[���̏ꍇ��PQ���܂��󂶂�Ȃ��Ă��T���I��点���܂�
        if (src == goal) {
            break;
        }

        // �אڂ��鑼�̒��_�̒T��
        // (0) �R�X�g
        ap_uint<8> cost = w[src];
        // (1) �m�[�hID����3�������W���}�X�N���Ĕ����o��
        ap_uint<13> src_xy = (ap_uint<13>)(src >> BITWIDTH_Z);
        ap_uint<7> src_x = (ap_uint<7>)(src_xy / MAX_WIDTH);
        ap_uint<7> src_y = (ap_uint<7>)(src_xy % MAX_WIDTH);
        ap_uint<3> src_z = (ap_uint<3>)(src & BITMASK_Z);
        //cout << src << " " << src_x << " " << src_y << " " << src_z << endl;
        // (2) 3�������W�ŗאڂ���m�[�h (6��) �𒲂ׂ�
        SEARCH_ADJACENTS:
        for (ap_uint<3> a = 0; a < 6; a++) {
//#pragma HLS PIPELINE
//#pragma HLS UNROLL factor=2
            ap_int<8> dest_x = (ap_int<8>)src_x; // �ŏ�-1 �ő�72 (->�����t��8�r�b�g)
            ap_int<8> dest_y = (ap_int<8>)src_y; // �ŏ�-1 �ő�72 (->�����t��8�r�b�g)
            ap_int<5> dest_z = (ap_int<5>)src_z; // �ŏ�-1 �ő�8  (->�����t��5�r�b�g)
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
                    dist[dest] = dist_new; // dist�̍X�V
                    prev[dest] = src; // �O�̒��_���L�^
#ifdef USE_ASTAR
                    dist_new += abs_uint7(dest_x, goal_x) + abs_uint7(dest_y, goal_y) + abs_uint3(dest_z, goal_z); // A* �q���[���X�e�B�b�N
#endif
                    pq_push(dist_new, dest, &pq_len, pq_nodes); // �L���[�ɐV���ȉ��̋����̏���push
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

    // �o�H���o��
    // �S�[������X�^�[�g�ւ̏��Ԃŕ\������� (�S�[���ƃX�^�[�g�͊܂܂�Ȃ�)
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

    // �o�b�N�g���b�N
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
        // �}�b�s���O�֋L�^
        path[p] = t;
        p++;
    }
    *path_size = p;

#ifdef DEBUG_PRINT
    cout << "max_path_len = " << p << endl;
    cout << "max_pq_len = " << max_pq_len << endl;
#endif

}

// �v���C�I���e�B�E�L���[ (�q�[�v�Ŏ���)
// �D��x�̍ŏ��l���q�[�v�̃��[�g�ɗ���
// �Q�l
//   * �q�[�v - Wikipedia https://ja.wikipedia.org/wiki/%E3%83%92%E3%83%BC%E3%83%97
//   * �񕪃q�[�v - Wikipedia https://ja.wikipedia.org/wiki/%E4%BA%8C%E5%88%86%E3%83%92%E3%83%BC%E3%83%97
//   * �q�[�v�̐��� - http://www.maroontress.com/Heap/
//   * Priority queue - Rosetta Code https://rosettacode.org/wiki/Priority_queue#C
// Note
// �C���f�b�N�X��0����n�܂�Ƃ� (0-origin index)
//   --> �e: (n-1)/2, ���̎q: 2n+1, �E�̎q: 2n+2
// �C���f�b�N�X��1����n�܂�Ƃ� (1-origin index)
//   --> �e: n/2, ���̎q: 2n, �E�̎q: 2n+1
// FPGA�I�ɂ͂ǂ�����x���͓��������� 1-origin �̕���LUT���\�[�X���Ȃ��čς� (�������z���0�v�f�����ʂɂȂ�)

// �m�[�h�̑}���́C�����ɒǉ����Ă���D��x�������������̈ʒu�܂Ńm�[�h���グ�Ă���
// �T���̓s����C�����D��x�ł͌ォ����ꂽ�����ɏo����������C
// ���[�v�̏I�������͑}���m�[�h�̗D��x����r�Ώۂ̗D��x�����������Ȃ����Ƃ�
void pq_push(ap_uint<16> priority, ap_uint<16> data, ap_uint<12> *pq_len, ap_uint<32> pq_nodes[MAX_PQ]) {
#pragma HLS INLINE

    (*pq_len)++;
    ap_uint<12> i = *pq_len;
    ap_uint<12> p = (*pq_len) >> 1; // i.e., (*pq_len) / 2; // �e
    PQ_PUSH_LOOP:
    while (i > 1 && (ap_uint<16>)(pq_nodes[p] & PQ_PRIORITY_MASK) >= priority) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=8 avg=4
#pragma HLS PIPELINE
//#pragma HLS UNROLL factor=2
        pq_nodes[i] = pq_nodes[p];
        i = p;
        p = p >> 1; // i.e., p / 2; // �e
    }
    pq_nodes[i] = ((ap_uint<32>)data << 16) | (ap_uint<32>)priority;
}

// �m�[�h�̎��o���́C���[�g������Ă��邾��
// ���ɍŏ��̗D��x�����m�[�h�����[�g�Ɉړ������邽�߂ɁC
// �܂��C�����̃m�[�h�����[�g�Ɉړ�����
// �����̎q�ŗD��x��������������ɂ����Ă��� (���[�g��K�؂ȍ����܂ŉ�����)
// ������ċA�I�ɌJ��Ԃ�
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
        ap_uint<12> c1 = i << 1;       // i.e., 2 * i;     // ���̎q
        ap_uint<12> c2 = (i << 1) + 1; // i.e., 2 * i + 1; // �E�̎q
        // ���̎q
        if (c1 < *pq_len && (ap_uint<16>)(pq_nodes[c1] & PQ_PRIORITY_MASK) <= (ap_uint<16>)(pq_nodes[t] & PQ_PRIORITY_MASK)) {
            t = c1;
        }
        // �E�̎q
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

    // ��
    for (int i = 0; i < MAX_CELLS; i++) {
        boardmat[i] = 0;
    }
    // ���C��
    // ���̃\���o�ł̃��C��ID��+1���ĕ\������
    // �Ȃ��Ȃ�󔒂� 0 �ŕ\�����Ƃɂ��邩�烉�C��ID�� 1 �ȏ�ɂ�����
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