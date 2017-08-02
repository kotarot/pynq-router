/**
 * avg.cpp
 */

#include <ap_int.h>

bool avg(ap_uint<32> t[5], ap_uint<32> *ret) {
#pragma HLS INTERFACE s_axilite register port=t bundle=AXI4LS
#pragma HLS INTERFACE s_axilite register port=ret bundle=AXI4LS
#pragma HLS INTERFACE s_axilite register port=return bundle=AXI4LS

	ap_uint<32> maxval = 0;
	ap_uint<32> minval = 0xffff;
	ap_uint<64> sum = 0;

	for (ap_uint<3> i = 0; i < 5; i++) {
		sum += t[i];
		if (maxval < t[i])
			maxval = t[i];
		if (t[i] < minval)
			minval = t[i];
	}
	*ret = (sum - maxval - minval) / 3;
	return true;
}
