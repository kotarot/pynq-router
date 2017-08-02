/**
 * main.cpp
 */

#include <ap_int.h>

bool avg(ap_uint<32> t[5], ap_uint<32> *ret);

int main(int argc, char *argv[]) {
	using namespace std;

	ap_uint<32> ret;
	ap_uint<32> t[5];
	t[0] = 50;
	t[1] = 200;
	t[2] = 300;
	t[3] = 400;
	t[4] = 550;

	bool status = avg(t, &ret);
	cout << ret << endl;

	return 0;
}
