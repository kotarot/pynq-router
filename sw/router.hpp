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
#define NOT_USE -1

#define MAX_BOXES 72        // X, Y �̍ő�l (7�r�b�g�Ŏ��܂�)
#define BITWIDTH_X 7
#define BITWIDTH_Y 7
#define BITMASK_X  127
#define BITMASK_Y  127
#define MAX_LAYER 8         // Z �̍ő�l (3�r�b�g�Ŏ��܂�)
#define BITWIDTH_Z 3
#define BITMASK_Z  7

#define MAX_CELLS 131072    // �Z���̑��� =128*128*8 (17�r�b�g�Ŏ��܂�)
#define MAX_LINES 128       // ���C�����̍ő�l (7�r�b�g�Ŏ��܂�)
#define MAX_PATH 256        // 1�̃��C�����Ή�����Z�����̍ő�l (8�r�b�g�Ŏ��܂�)

#define BOARDSTR_SIZE 32768 // �{�[�h�X�g�����O�̍ő啶���� (15�r�b�g�Ŏ��܂� : �l�͂Ƃ肠����)

#define MAX_PQ 512          // �T�����̃v���C�I���e�B�E�L���[�̍ő�T�C�Y

#define MAX_WEIGHT  255      // �d�݂̍ő�l (8�r�b�g�Ŏ��܂�)
#define COST_WEIGHT 4        // �d�݂̍X�V�l

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
