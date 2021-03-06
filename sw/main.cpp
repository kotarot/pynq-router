/**
 * main.cpp
 *
 * for Vivado HLS
 */

#ifdef SOFTWARE
#include "ap_int.h"
#else
#include <ap_int.h>
#endif

#ifdef CALCTIME
#include <stdio.h>
#include <time.h>
#endif

#include "router.hpp"


int main(int argc, char *argv[]) {
    using namespace std;

    // テストデータ (文字列形式)
    // NL_Q00.txt
    //char boardstr[BOARDSTR_SIZE] = "X10Y05Z3L0000107041L0004107002L0102102021L0900100003";
    // NL_Q06.txt
    char boardstr[BOARDSTR_SIZE] = "X10Y18Z2L0900109002L0901105012L0902103052L0903103062L0904100102L0905106012L0906109022L0717109102L0808109112L0017209172L0401200072L0912208152L0009201092L0709209092L0901206052L0309204092L0701209072L0101201022L0011202152L0016202162";
    // NL_Q08.txt
    //char boardstr[BOARDSTR_SIZE] = "X17Y20Z2L0000103022L1603115052L0916107032L0302108012L1104111042L1002100002L0919116162L1616113182L1001115012L0500201182L1603213152L0600210022";

    // 指定されてればコマンドラインから問題文字列を読み込む
    if (1 < argc) {
        //先頭がXではないならば標準入力から読み込む
        if(argv[1][0]!='X')
        {
            char* c_p=fgets(boardstr, BOARDSTR_SIZE, stdin);
            int length=strlen(c_p);
            boardstr[length-1]=0;
        }
        else
        {
            strcpy(boardstr, argv[1]);
        }
    }

    // 指定されてればシード値を読み込む
    int seed = 12345;
    if (2 < argc) {
        seed = atoi(argv[2]);
    }

    int size_x = (boardstr[1] - '0') * 10 + (boardstr[2] - '0');
    int size_y = (boardstr[4] - '0') * 10 + (boardstr[5] - '0');
    int size_z = (boardstr[7] - '0');

    // ソルバ実行
    ap_int<32> status;
    clock_t clock_start, clock_done;
    clock_start = clock();
    bool result = pynqrouter(boardstr, seed, &status);
    clock_done = clock();
    if (result) {
        cout << endl << "Test Passed!" << endl;
    } else {
        cout << endl << "Test Failed!" << endl;
    }
    cout << "status = " << (int)status << endl;
    cout << "elapsed = " << ((double)(clock_done - clock_start) / CLOCKS_PER_SEC) << endl << endl;

    // 解表示
    cout << "SOLUTION" << endl;
    cout << "========" << endl;
    cout << "SIZE " << size_x << "X" << size_y << "X" << size_z << endl;
    for (int z = 0; z < size_z; z++) {
        cout << "LAYER " << (z + 1) << endl;
        for (int y = 0; y < size_y; y++) {
            for (int x = 0; x < size_x; x++) {
                if (x != 0) {
                    cout << ",";
                }
                int i = ((x * MAX_WIDTH + y) << BITWIDTH_Z) | z;
                //cout << setfill('0') << setw(2) << right << (unsigned int)(unsigned char)(boardstr[i]);
                cout << (unsigned int)(unsigned char)(boardstr[i]);
            }
            cout << endl;
        }
    }

    return 0;
}
