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
ap_uint<32> lfsr_random_uint32(ap_uint<32> a, ap_uint<32> b) {
#pragma HLS INLINE
    return lfsr_random() % (b - a + 1) + a;
}


// ================================ //
// ���C�����W���[��
// ================================ //

// �{�[�h�Ɋւ���ϐ�
static ap_uint<7> size_x;       // �{�[�h�� X �T�C�Y
static ap_uint<7> size_y;       // �{�[�h�� Y �T�C�Y
static ap_uint<3> size_z;       // �{�[�h�� Z �T�C�Y

static ap_uint<7> line_num = 0; // ���C���̑���

bool pynqrouter(char boardstr[BOARDSTR_SIZE], ap_uint<32> seed, ap_int<8> *status) {
#pragma HLS INTERFACE s_axilite port=boardstr bundle=AXI4LS
#pragma HLS INTERFACE s_axilite port=status bundle=AXI4LS
#pragma HLS INTERFACE s_axilite port=return bundle=AXI4LS

    *status = -127;

    ap_uint<17> starts[MAX_LINES];          // ���C���̃X�^�[�g���X�g
//#pragma HLS ARRAY_PARTITION variable=starts complete dim=0
    ap_uint<17> goals[MAX_LINES];           // ���C���̃S�[�����X�g
//#pragma HLS ARRAY_PARTITION variable=goals complete dim=0

    ap_uint<8> weights[MAX_CELLS];          // �Z���̏d��
    ap_uint<8> paths_size[MAX_LINES];       // ���C�����Ή�����Z��ID�̃T�C�Y
#pragma HLS ARRAY_PARTITION variable=paths_size complete dim=0
    ap_uint<17> paths[MAX_LINES][MAX_PATH]; // ���C�����Ή�����Z��ID�̏W�� (�X�^�[�g�ƃS�[���͏���)
    bool adjacents[MAX_LINES];              // �X�^�[�g�ƃS�[�����אڂ��Ă��郉�C��
#pragma HLS ARRAY_PARTITION variable=adjacents complete dim=0

#ifdef SOFTWARE
    ap_int<9> boardmat[MAX_CELLS];          // �{�[�h�}�g���b�N�X
#endif

    // ================================
    // ������ BEGIN
    // ================================

    // ���[�v�J�E���^��1�r�b�g�]���ɗp�ӂ��Ȃ��ƏI������ł��Ȃ�
    for (ap_uint<8> i = 0; i < (ap_uint<8>)(MAX_LINES); i++) {
//#pragma HLS UNROLL
        adjacents[i] = false;
    }

    // �{�[�h�X�g�����O�̉���
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

            // �X�^�[�g�ƃS�[��
            starts[line_num] = ((ap_uint<17>)s_z << (BITWIDTH_X + BITWIDTH_Y)) | ((ap_uint<17>)s_y << BITWIDTH_X) | (ap_uint<17>)s_x;
            goals[line_num]  = ((ap_uint<17>)g_z << (BITWIDTH_X + BITWIDTH_Y)) | ((ap_uint<17>)g_y << BITWIDTH_X) | (ap_uint<17>)g_x;

            // ������ԂŐ������אڂ��Ă��邩���f
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

        // �I�[ (null) ����
        if (boardstr[idx] == 0) {
            break;
        }
    }
    //cout << size_x << " " << size_y << " " << size_z << endl;

    // ���[�v�J�E���^��1�r�b�g�]���ɗp�ӂ��Ȃ��ƏI������ł��Ȃ�
    for (ap_uint<18> i = 0; i < (ap_uint<18>)(MAX_CELLS); i++) {
        weights[i] = 1;
    }

    // ���[�v�J�E���^��1�r�b�g�]���ɗp�ӂ��Ȃ��ƏI������ł��Ȃ�
    for (ap_uint<8> i = 0; i < (ap_uint<8>)(MAX_LINES); i++) {
        paths_size[i] = 0;
        // �X�^�[�g�ƃS�[���̏d�݂͍ő�ɂ��Ă���
        weights[starts[i]] = MAX_WEIGHT;
        weights[goals[i]]  = MAX_WEIGHT;
    }

    // �����̏�����
    lfsr_random_init(seed);

    // ================================
    // ������ END
    // ================================

    // ================================
    // ���[�e�B���O BEGIN
    // ================================

    bool has_overlap = false;
    //ap_uint<8> overlaps[MAX_CELLS];
    ap_uint<1> overlap_checks[MAX_CELLS];

    // �������[�e�B���O
    for (ap_uint<8> i = 0; i < (ap_uint<8>)(line_num); i++) {
//#pragma HLS UNROLL
#pragma HLS LOOP_TRIPCOUNT min=2 max=127 avg=50

        // �������אڂ���ꍇ�X�L�b�v�A�����łȂ��ꍇ�͎��s
        if (adjacents[i] == false) {

            // �_�C�N�X�g���@
#ifdef DEBUG_PRINT
            cout << "LINE #" << (i + 1) << endl;
#endif
            search(&(paths_size[i]), paths[i], starts[i], goals[i], weights);

        }
    }

    // ���[�e�B���O�C�e���[�V���� (���E���h�ƌĂ�)
    for (ap_uint<32> round = 0; round < 1000000; round++) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=100 avg=50

//#ifdef DEBUG_PRINT
if (round % 100 == 0) {
        cout << "ROUND " << round << endl;
        //show_board(line_num, paths_size, paths, starts, goals);
}
//#endif

        // �ă��[�e�B���O
        // �Ώۃ��C����I��
        ap_uint<8> target = lfsr_random_uint32(0, line_num - 1);

        // �������אڂ���ꍇ�X�L�b�v�A�����łȂ��ꍇ�͎��s
        if (adjacents[target] == true) {
            continue;
        }

        // (1) �d�݂����Z�b�g
        for (ap_uint<18> i = 0; i < (ap_uint<18>)(MAX_CELLS); i++) {
            weights[i] = 1;
        }
        // (2) �d�݂��X�V
        for (ap_uint<8> i = 0; i < (ap_uint<8>)(line_num); i++) {
#pragma HLS LOOP_TRIPCOUNT min=2 max=127 avg=50

            weights[starts[i]] = MAX_WEIGHT;
            weights[goals[i]]  = MAX_WEIGHT;

            // �������אڂ���ꍇ�X�L�b�v�A�����łȂ��ꍇ�͎��s
            if (adjacents[i] == false && i != target) {
                for (ap_uint<9> j = 0; j < (ap_uint<9>)(paths_size[i]); j++) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=255 avg=50
                    weights[paths[i][j]] = MAX_WEIGHT;
                }
            }

        }

        // �_�C�N�X�g���@
#ifdef DEBUG_PRINT
        cout << "LINE #" << (target + 1) << endl;
#endif
        search(&(paths_size[target]), paths[target], starts[target], goals[target], weights);

        // ���[�e�B���O��
        // �I�[�o�[���b�v�̃`�F�b�N
        has_overlap = false;
        for (ap_uint<18> i = 0; i < (ap_uint<18>)(MAX_CELLS); i++) {
            overlap_checks[i] = 0;
        }
        for (ap_uint<8> i = 0; i < (ap_uint<8>)(line_num); i++) {
#pragma HLS LOOP_TRIPCOUNT min=2 max=127 avg=50
            overlap_checks[starts[i]] = 1;
            overlap_checks[goals[i]] = 1;

            // �������אڂ���ꍇ�X�L�b�v�A�����łȂ��ꍇ�͎��s
            if (adjacents[i] == false) {

                for (ap_uint<9> j = 0; j < (ap_uint<9>)(paths_size[i]); j++) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=255 avg=50
                    ap_uint<17> cell_id = paths[i][j];
                    if (overlap_checks[cell_id] == 1) {
                        has_overlap = true;
                        break;
                    }
                    overlap_checks[cell_id] = 1;
                }

            }

        }
        // �I�[�o�[���b�v�Ȃ���ΒT���I��
        if (has_overlap == false) {
            break;
        }
    }

    // �𓱏o�ł��Ȃ������ꍇ
    if (has_overlap == true) {
        *status = 1;
        return false;
    }

    // ================================
    // ���[�e�B���O END
    // ================================

    // ================================
    // �𐶐� BEGIN
    // ================================
#ifdef SOFTWARE
    // ��
    for (ap_uint<18> i = 0; i < (ap_uint<18>)(MAX_CELLS); i++) {
        boardmat[i] = 0;
    }
    // ���C��
    // ���̃\���o�ł̃��C��ID��+1���ĕ\������
    // �Ȃ��Ȃ�󔒂� 0 �ŕ\�����Ƃɂ��邩�烉�C��ID�� 1 �ȏ�ɂ�����
    for (ap_uint<8> i = 0; i < (ap_uint<8>)(line_num); i++) {
#pragma HLS LOOP_TRIPCOUNT min=2 max=127 avg=50
        boardmat[starts[i]] = (i + 1);
        boardmat[goals[i]]  = (i + 1);
        for (ap_uint<9> j = 0; j < (ap_uint<9>)(paths_size[i]); j++) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=255 avg=50
            boardmat[paths[i][j]] = (i + 1);
        }
    }

    // boardmat �𕶎��� (�������A�\���ł��镶���Ƃ͌���Ȃ�)
    ap_uint<17> i = 0;
    for (ap_uint<4> z = 0; z < (ap_uint<4>)(size_z); z++) {
#pragma HLS LOOP_TRIPCOUNT min=4 max=72 avg=20
        for (ap_uint<8> y = 0; y < (ap_uint<8>)(size_y); y++) {
#pragma HLS LOOP_TRIPCOUNT min=4 max=72 avg=20
            for (ap_uint<8> x = 0; x < (ap_uint<8>)(size_x); x++) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=8 avg=4
                ap_uint<17> cell_id = ((ap_uint<17>)z << (BITWIDTH_X + BITWIDTH_Y)) | ((ap_uint<17>)y << BITWIDTH_X) | (ap_uint<17>)x;
                boardstr[i] = boardmat[cell_id];
                i++;
            }
        }
    }
#endif
    // ================================
    // �𐶐� END
    // ================================

    *status = 0;
    return true;
}

/*ap_uint<8> new_weight(ap_uint<8> x) {
#pragma HLS INLINE
    ap_uint<8> ret = 1;
    for (ap_uint<9> i = 0; i < x; i++) {
#pragma HLS LOOP_TRIPCOUNT min=0 max=10 avg=4
        ret *= COST_WEIGHT;
    }
    return ret;
}*/


// ================================ //
// �T��
// ================================ //

// Python�Ń_�C�N�X�g���A���S���Y������������ - �t�c�[���Č����Ȃ��I
// http://lethe2211.hatenablog.com/entry/2014/12/30/011030
// ���x�[�X
void search(ap_uint<8> *path_size, ap_uint<17> path[MAX_PATH], ap_uint<17> start, ap_uint<17> goal, ap_uint<8> w[MAX_CELLS]) {

    ap_uint<16> dist[MAX_CELLS]; // �n�_����e���_�܂ł̍ŒZ�������i�[����
    ap_uint<17> prev[MAX_CELLS]; // �ŒZ�o�H�ɂ�����C���̒��_�̑O�̒��_��ID���i�[����
    for (ap_uint<18> i = 0; i < MAX_CELLS; i++) {
        dist[i] = 65535; // = (2^16 - 1)
    }

    // �v���C�I���e�B�E�L���[
    ap_uint<16> pq_len = 0;
    ap_uint<8> pq_nodes_priority[MAX_PQ];
//#pragma HLS ARRAY_PARTITION variable=pq_nodes_priority complete dim=1
//#pragma HLS ARRAY_PARTITION variable=pq_nodes_priority cyclic factor=2 dim=1 partition
    ap_uint<17> pq_nodes_data[MAX_PQ];
//#pragma HLS ARRAY_PARTITION variable=pq_nodes_data complete dim=1
//#pragma HLS ARRAY_PARTITION variable=pq_nodes_data cyclic factor=2 dim=1 partition

    dist[start] = 0;
    pq_push(0, start, &pq_len, pq_nodes_priority, pq_nodes_data); // �n�_��push

    while (0 < pq_len) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=2000 avg=1000

        ap_uint<8> prov_cost;
        ap_uint<17> src;
        pq_pop(&prov_cost, &src, &pq_len, pq_nodes_priority, pq_nodes_data);

        // �v���C�I���e�B�L���[�Ɋi�[����Ă���ŒZ�������C���݌v�Z�ł��Ă���ŒZ�������傫����΁Cdist�̍X�V������K�v�͂Ȃ�
        // �����łȂ��ꍇ�͌v�Z����
        if (dist[src] >= prov_cost) {
            // �אڂ��鑼�̒��_�̒T��
            // (0) �R�X�g
            ap_uint<8> cost = w[src];
            // (1) �m�[�hID����3�������W���}�X�N���Ĕ����o��
            ap_uint<7> src_x =  src &  BITMASK_X;
            ap_uint<7> src_y = (src & (BITMASK_Y << BITWIDTH_X)) >> BITWIDTH_X;
            ap_uint<3> src_z = (src & (BITMASK_Z << (BITWIDTH_X + BITWIDTH_Y))) >> (BITWIDTH_X + BITWIDTH_Y);
            //cout << src << " " << src_x << " " << src_y << " " << src_z << endl;
            // (2) 3�������W�ŗאڂ���m�[�h (6��) �𒲂ׂ�
            for (ap_uint<3> a = 0; a < 6; a++) {
                ap_int<9> dest_x = (ap_int<9>)src_x;
                ap_int<9> dest_y = (ap_int<9>)src_y;
                ap_int<5> dest_z = (ap_int<9>)src_z;
                if (a == 0) { dest_x -= 1; }
                if (a == 1) { dest_x += 1; }
                if (a == 2) { dest_y -= 1; }
                if (a == 3) { dest_y += 1; }
                if (a == 4) { dest_z -= 1; }
                if (a == 5) { dest_z += 1; }

                if (0 <= dest_x && dest_x < (ap_int<9>)size_x && 0 <= dest_y && dest_y < (ap_int<9>)size_y && 0 <= dest_z && dest_z < (ap_int<5>)size_z) {
                    ap_uint<17> dest = ((ap_uint<17>)dest_z << (BITWIDTH_X + BITWIDTH_Y)) | ((ap_uint<17>)dest_y << BITWIDTH_X) | (ap_uint<17>)dest_x;
                    if (dist[dest] > dist[src] + cost) {
                        dist[dest] = dist[src] + cost;
                            // dist�̍X�V
                        pq_push(dist[dest], dest, &pq_len, pq_nodes_priority, pq_nodes_data);
                            // �L���[�ɐV���ȉ��̋����̏���push
                        prev[dest] = src;
                            // �O�̒��_���L�^
                    }
                }
             }
        }
    }

    // �o�H���o��
    // �S�[������X�^�[�g�ւ̏��Ԃŕ\������� (�S�[���ƃX�^�[�g�͊܂܂�Ȃ�)
    ap_uint<17> t = goal;

#ifdef DEBUG_PRINT
    int start_x =  start &  BITMASK_X;
    int start_y = (start & (BITMASK_Y << BITWIDTH_X)) >> BITWIDTH_X;
    int start_z = (start & (BITMASK_Z << (BITWIDTH_X + BITWIDTH_Y))) >> (BITWIDTH_X + BITWIDTH_Y);
    int goal_x  =  goal  &  BITMASK_X;
    int goal_y  = (goal  & (BITMASK_Y << BITWIDTH_X)) >> BITWIDTH_X;
    int goal_z  = (goal  & (BITMASK_Z << (BITWIDTH_X + BITWIDTH_Y))) >> (BITWIDTH_X + BITWIDTH_Y);
    cout << "(" << start_x << ", " << start_y << ", " << start_z << ") = " << start << " -> "
         << "(" << goal_x  << ", " << goal_y  << ", " << goal_z  << ") = " << goal << endl;
#endif

    ap_uint<8> p = 0;
    while (prev[t] != start) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=255 avg=50

#ifdef DEBUG_PRINT
        ap_uint<7> t_x =  prev[t] &  BITMASK_X;
        ap_uint<7> t_y = (prev[t] & (BITMASK_Y << BITWIDTH_X)) >> BITWIDTH_X;
        ap_uint<3> t_z = (prev[t] & (BITMASK_Z << (BITWIDTH_X + BITWIDTH_Y))) >> (BITWIDTH_X + BITWIDTH_Y);
        cout << "  via " << "(" << t_x << ", " << t_y << ", " << t_z << ") = " << prev[t] << " " << dist[t] << endl;
#endif

        // �}�b�s���O�֋L�^
        path[p] = prev[t];

        t = prev[t];
        p++;
    }
    *path_size = p;
}

// �v���C�I���e�B�E�L���[ (�q�[�v)
// �Q�l https://rosettacode.org/wiki/Priority_queue#C
void pq_push(ap_uint<8> priority, ap_uint<17> data, ap_uint<16> *pq_len, ap_uint<8> pq_nodes_priority[MAX_PQ], ap_uint<17> pq_nodes_data[MAX_PQ]) {
#pragma HLS INLINE

    (*pq_len)++;
    ap_uint<16> i = *pq_len;
    ap_uint<16> j = (*pq_len) / 2;
    while (i > 1 && pq_nodes_priority[j] > priority) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=10 avg=5
//#pragma HLS PIPELINE
//#pragma HLS UNROLL factor=2
        pq_nodes_priority[i] = pq_nodes_priority[j];
        pq_nodes_data[i]     = pq_nodes_data[j];
        i = j;
        j /= 2;
    }
    pq_nodes_priority[i] = priority;
    pq_nodes_data[i]     = data;
}

void pq_pop(ap_uint<8> *ret_priority, ap_uint<17> *ret_data, ap_uint<16> *pq_len, ap_uint<8> pq_nodes_priority[MAX_PQ], ap_uint<17> pq_nodes_data[MAX_PQ]) {
#pragma HLS INLINE

    *ret_priority = pq_nodes_priority[1];
    *ret_data = pq_nodes_data[1];
    pq_nodes_priority[1] = pq_nodes_priority[*pq_len];
    pq_nodes_data[1]     = pq_nodes_data[*pq_len];
    (*pq_len)--;
    ap_uint<16> i = 1;
    while (1) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=10 avg=5
//#pragma HLS PIPELINE
//#pragma HLS UNROLL factor=2
        ap_uint<16> k = i;
        ap_uint<16> j = 2 * i;
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

#ifdef SOFTWARE
void show_board(ap_uint<7> line_num, ap_uint<8> paths_size[MAX_LINES], ap_uint<17> paths[MAX_LINES][MAX_PATH], ap_uint<17> starts[MAX_LINES], ap_uint<17> goals[MAX_LINES]) {
    int boardmat[MAX_CELLS];

    // ��
    for (int i = 0; i < MAX_CELLS; i++) {
        boardmat[i] = 0;
    }
    // ���C��
    // ���̃\���o�ł̃��C��ID��+1���ĕ\������
    // �Ȃ��Ȃ�󔒂� 0 �ŕ\�����Ƃɂ��邩�烉�C��ID�� 1 �ȏ�ɂ�����
    for (int i = 0; i < (int)(line_num); i++) {
#pragma HLS LOOP_TRIPCOUNT min=2 max=127 avg=50
        boardmat[starts[i]] = (i + 1);
        boardmat[goals[i]]  = (i + 1);
        for (int j = 0; j < (int)(paths_size[i]); j++) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=255 avg=50
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
                int id = (int)( ((ap_uint<17>)z << (BITWIDTH_X + BITWIDTH_Y)) | ((ap_uint<17>)y << BITWIDTH_X) | (ap_uint<17>)x );
                cout << setfill('0') << setw(2) << right << (int)(boardmat[id]);
            }
            cout << endl;
        }
    }
}
#endif
