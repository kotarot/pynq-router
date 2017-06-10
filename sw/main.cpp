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
// �����Z���k�E�c�C�X�^
// ================================ //
#include "mt19937ar.hpp"

void mt_init_genrand(unsigned long s) {
////#pragma HLS INLINE
    init_genrand(s);
}

// A����B�͈̔͂̐����̗������~�����Ƃ�
// �Q�l http://www.sat.t.u-tokyo.ac.jp/~omi/random_variables_generation.html
unsigned long mt_genrand_int32(int a, int b) {
////#pragma HLS INLINE
    return genrand_int32() % (b - a + 1) + a;
}

// ================================ //
// �T��
// ================================ //

void search(ap_uint<7> size_x, ap_uint<7> size_y, ap_uint<3> size_z,
    ap_uint<17> start, ap_uint<17> goal, ap_uint<8> *w) {

    cout << size_x << " " << size_y << " " << size_z << endl;
    cout << start << " -> " << goal << endl;

}


// ================================ //
// ���C�����W���[��
// ================================ //

bool pynqrouter(char boardstr[BOARDSTR_SIZE], ap_int<8> *status) {
#pragma HLS INTERFACE s_axilite port=boardstr bundle=AXI4LS
#pragma HLS INTERFACE s_axilite port=status bundle=AXI4LS
#pragma HLS INTERFACE s_axilite port=return bundle=AXI4LS

    *status = -127;

    // �{�[�h�Ɋւ���ϐ�
    ap_uint<7> size_x; // �{�[�h�� X �T�C�Y
    ap_uint<7> size_y; // �{�[�h�� Y �T�C�Y
    ap_uint<3> size_z; // �{�[�h�� Z �T�C�Y

    ap_uint<8> line_size = 0; // ���C���̑���

    ap_uint<17> starts[MAX_LINES]; // ���C���̃X�^�[�g���X�g
    ap_uint<17> goals[MAX_LINES];  // ���C���̃S�[�����X�g

    ap_uint<8> weights[MAX_CELLS];          // �Z���̏d��
    ap_uint<8> paths_size[MAX_LINES];       // ���C�����Ή�����Z��ID�̃T�C�Y
    ap_uint<17> paths[MAX_LINES][MAX_PATH]; // ���C�����Ή�����Z��ID�̏W�� (�X�^�[�g�ƃS�[���͏���)
    bool adjacents[MAX_LINES];              // �X�^�[�g�ƃS�[�����אڂ��Ă��郉�C��
//#pragma HLS ARRAY_PARTITION variable=adjacents complete dim=0

    // ================================
    // ������ BEGIN
    // ================================

    // ���[�v�J�E���^��1�r�b�g�]���ɗp�ӂ��Ȃ��ƏI������ł��Ȃ�
    for (ap_uint<8> i = 0; i < (ap_uint<8>)(MAX_LINES); i++) {
//#pragma HLS PIPELINE
        adjacents[i] = false;
    }

    // �{�[�h�X�g�����O�̉���
    // unsigned 15bit �� 32768 �Ɏ��܂邽��
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

            // �X�^�[�g�ƃS�[��
            starts[line_size] = (s_z << (BITWIDTH_X + BITWIDTH_Y)) & (s_y << BITWIDTH_X) & s_x;
            goals[line_size]  = (g_z << (BITWIDTH_X + BITWIDTH_Y)) & (g_y << BITWIDTH_X) & g_x;

            // ������ԂŐ������אڂ��Ă��邩���f
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
    for (ap_uint<8> i = 0; i < (ap_uint<18>)(MAX_LINES); i++) {
        paths_size[i] = 0;
    }

    // �����̏�����
    mt_init_genrand(12345);

    // ================================
    // ������ END
    // ================================

    //printBoard();

    ap_int<8> output;

    // �������[�e�B���O
    for (ap_uint<8> i = 0; i < (ap_uint<8>)(line_size); i++) {
#pragma HLS LOOP_TRIPCOUNT min=2 max=128 avg=50

        // �������אڂ���ꍇ�X�L�b�v
        if (adjacents[i] == false) {
            continue;
        }

        // �_�C�N�X�g���@
        search(size_x, size_y, size_z, starts[i], goals[i], weights);
    }

    // �𓱏o�ł��Ȃ������ꍇ
    /*if (isFinished(board) == false) {
        *status = 4;
        return false;
    }*/

    *status = 0;
    return true;
}
