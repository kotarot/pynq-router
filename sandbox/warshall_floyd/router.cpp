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

#include "router.hpp"


ap_uint<16> min_uint16(ap_uint<16> a, ap_uint<16> b) {
    if (a < b) {
        return a;
    } else {
        return b;
    }
}

ap_uint<16> pynqrouter(ap_uint<8> weights[MAX_CELLS]) {
#pragma HLS INTERFACE s_axilite port=weights bundle=AXI4LS
#pragma HLS INTERFACE s_axilite port=return bundle=AXI4LS

    ap_uint<16> dist[MAX_CELLS][MAX_CELLS];

    // èâä˙âª
    for (ap_uint<18> i = 0; i < (ap_uint<18>)(MAX_CELLS); i++) {
        for (ap_uint<18> j = 0; j < (ap_uint<18>)(MAX_CELLS); j++) {
            if (i == j) {
                dist[i][j] = 0;
            } else {
                dist[i][j] = MAX_DIST;
            }
        }
    }

    // éGÇ»ílê›íË
    for (ap_uint<18> i = 0; i < (ap_uint<18>)(MAX_CELLS - 1); i++) {
        dist[i][i + 1] = dist[i + 1][i] = (ap_uint<16>)(weights[i]);
    }

    // Warshall-Floyd
    for (ap_uint<18> k = 0; k < (ap_uint<18>)(MAX_CELLS); k++) {
        for (ap_uint<18> i = 0; i < (ap_uint<18>)(MAX_CELLS); i++) {
            for (ap_uint<18> j = 0; j < (ap_uint<18>)(MAX_CELLS); j++) {
                dist[i][j] = min_uint16(dist[i][j], dist[i][k] + dist[k][j]);
            }
        }
    }

    return dist[0][100];
}
