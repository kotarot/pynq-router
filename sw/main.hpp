/**
 * main.hpp
 *
 * for Vivado HLS
 */

#ifndef __MAIN_HPP__
#define __MAIN_HPP__

#ifdef SOFTWARE
#include "ap_int.h"
#else
#include <ap_int.h>
#endif

#ifdef SOFTWARE
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

using namespace std;

#define NOT_USE -1

// �e��ݒ�l
#define MAX_BOXES 72     // X, Y �̍ő�l (7�r�b�g�Ŏ��܂�)
#define BITWIDTH_X 7
#define BITWIDTH_Y 7
#define MAX_LAYER 8      // Z �̍ő�l (3�r�b�g�Ŏ��܂�)
#define BITWIDTH_Z 3
#define MAX_CELLS 131072 // �Z���̑��� =128*128*8 (17�r�b�g�Ŏ��܂�)
#define MAX_LINES 128    // ���C�����̍ő�l (7�r�b�g�Ŏ��܂�)
#define MAX_PATH 256     // 1�̃��C�����Ή�����Z�����̍ő�l (8�r�b�g�Ŏ��܂�)

#define BOARDSTR_SIZE 32768 // �{�[�h�X�g�����O�̍ő啶���� (15�r�b�g�Ŏ��܂� : �l�͂Ƃ肠����)


// �����Z���k�E�c�C�X�^
void mt_init_genrand(unsigned long s);
unsigned long mt_genrand_int32(int a, int b);

bool pynqrouter(char boardstr[BOARDSTR_SIZE], ap_int<8> *status);
//void initialize(char boardstr[BOARDSTR_SIZE], Board *board);
//bool isFinished(Board *board);
//void generateSolution(char boardstr[BOARDSTR_SIZE], Board *board);


#endif /* __MAIN_HPP__ */
