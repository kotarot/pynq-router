/**
 * main_tb.cpp
 *
 * for Vivado HLS
 */

#ifndef SOFTWARE
#include <stdio.h>
#include <string.h>
#include <ap_int.h>
#endif

#ifdef SOFTWARE
#include "ap_int.h"
#else
#include <ap_int.h>
#endif

#include "main.hpp"


int main() {
    using namespace std;

    // テストデータ (文字列形式)
    // NL_Q00.txt
    char boardstr[BOARDSTR_SIZE] = "X10Y05Z3L0000107041L0004107002L0102102021L0900100003";
    // NL_Q06.txt
    //char boardstr[BOARDSTR_SIZE] = "X10Y18Z2L0900109002L0901105012L0902103052L0903103062L0904100102L0905106012L0906109022L0717109102L0808109112L0017209172L0401200072L0912208152L0009201092L0709209092L0901206052L0309204092L0701209072L0101201022L0011202152L0016202162";
    // NL_Q08.txt
    //char boardstr[BOARDSTR_SIZE] = "X17Y20Z2L0000103022L1603115052L0916107032L0302108012L1104111042L1002100002L0919116162L1616113182L1001115012L0500201182L1603213152L0600210022";
    // 20x36x4の問題 (壊れている)
    //char boardstr[BOARDSTR_SIZE] = "X20Y36Z4L0917109172L1119111192L0715107152L0207108153L0720107202L0913109132L1501115012L1002110024L0419109273L0304103042L1514115142L1617113231L0911109112L0406104064L0510105102L1708117082L0202102022L1820118202L1207109033L1507101311L0625106252L1805104081L0605106054L1702117022L0826111311L0924116253L0911413043L0202409083L0510403203L0207202074L0419405293L1527312283L0830318204L1002213093L0419208253L1019309223L0304403103L1617215223L0427312074L0406204033L1125309244L1702412023L0720409193L0516305193L1207210033L1507215074L1223310273L1805214063L060525063L1617416203L0826208264L0924202183L0202302073L0702303053L0820309174L1702315014L1218310223L1514414193L1615312173L1503314113L0604318054L1119413263L0805304083L1105307073L1429311303L1708417123L1207311103L1707317173L0718304223L1008309133L0609304133L0715407123L0626306313L0511309134L0212303153L1718316303L1212315133L0916314163L0625403253";
cout << boardstr << endl;
    ap_uint<7> size_x = (boardstr[1] - '0') * 10 + (boardstr[2] - '0');
    ap_uint<7> size_y = (boardstr[4] - '0') * 10 + (boardstr[5] - '0');
    ap_uint<3> size_z = (boardstr[7] - '0');

    // ソルバ実行
    ap_int<8> status;
    bool result = pynqrouter(boardstr, &status);
    if (result) {
        cout << "Test Passed!" << endl;
    } else {
        cout << "Test Failed!" << endl;
    }
    cout << "status = " << status << endl << endl;

    // 解表示
    cout << "SOLUTION" << endl;
    cout << "========" << endl;
    cout << "SIZE " << size_x << "X" << size_y << "X" << size_z << endl;
    int i = 0;
    for (ap_uint<3> z = 0; z < size_z; z++) {
        cout << "LAYER " << (z + 1) << endl;
        for (ap_uint<7> y = 0; y < size_y; y++) {
            for (ap_uint<7> x = 0; x < size_x; x++) {
                if (x != 0) {
                    cout << ",";
                }
                //cout << setfill('0') << setw(2) << right << (int)(boardstr[i]);
                i++;
            }
            cout << endl;
        }
    }

    return 0;
}
