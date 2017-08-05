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

#include "router.hpp"


int main(int argc, char *argv[]) {
    using namespace std;

    ap_uint<8> weights[MAX_CELLS];
    for (int i = 0; i < MAX_CELLS; i++) {
        weights[i] = (ap_uint<8>)(i % 10);
    }

    ap_uint<16> ret = pynqrouter(weights);
    cout << ret << endl;

    return 0;
}
