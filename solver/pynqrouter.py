#!/opt/python3.6/bin/python3.6
# -*- coding: utf-8 -*-

"""
PYNQ 上で pynqrouter を実行する。
"""

from pynq import Overlay
from pynq import PL
from pynq import MMIO

import argparse
import sys
import time

import BoardStr


# Settings -- pynqrouter
IP = 'SEG_pynqrouter_0_Reg'
OFFSET_BOARD  = 65536   # 0x10000 ~ 0x1ffff
OFFSET_SEED   = 131072  # 0x20000
OFFSET_STATUS = 131080  # 0x20008
MAX_X = 72
MAX_Y = 72
MAX_Z = 8
BITWIDTH_Z = 3
# Settings -- LED
IP_LED = 'SEG_axi_gpio_0_Reg'


def main():
    parser = argparse.ArgumentParser(description="Solver with pynqrouter")

    # 入出力
    parser.add_argument('-i', '--input', action='store', nargs='?', default=None, type=str,
        help='Path to input problem file')
    parser.add_argument('-b', '--boardstr', action='store', nargs='?', default=None, type=str,
        help='Problem boardstr (if you want to solve directly)')
    parser.add_argument('-o', '--output', action='store', nargs='?', default=None, type=str,
        help='Path to output answer file')

    # ソルバオプション
    parser.add_argument('-s', '--seed', action='store', nargs='?', default=12345, type=int,
        help='Random seed')
    parser.add_argument('-e', '--edge-start', action='store_true', default=False,
        help='Enable to set farther terminals from the center as starts')

    # 出力オプション
    parser.add_argument('-p', '--print', action='store_true', default=False,
        help='Enable to print the solution')
    parser.add_argument('-z', '--zero-padding', action='store_true', default=False,
        help='Enable to do zero-padding')

    args = parser.parse_args()

    if args.input is not None:
        # 問題ファイルの読み込み
        with open(args.input, 'r') as f:
            lines = f.readlines()

        # 問題ファイルを boardstr に変換
        boardstr = BoardStr.conv_boardstr(lines, args.edge_start)

    elif args.boardstr is not None:
        boardstr = args.boardstr

    else:
        sys.stderr.write('Specify at least "input" or "boardstr"!\n')
        sys.exit(1)

    print('boardstr:')
    print(boardstr)
    print('seed:')
    print(args.seed)
    print('')

    # ボード文字列から X, Y, Z を読んでくる
    size_x = (ord(boardstr[1]) - ord('0')) * 10 + (ord(boardstr[2]) - ord('0'))
    size_y = (ord(boardstr[4]) - ord('0')) * 10 + (ord(boardstr[5]) - ord('0'))
    size_z = (ord(boardstr[7]) - ord('0'))

    # Overlay 読み込み
    OL = Overlay('pynqrouter.bit')
    OL.download()
    print(OL.ip_dict)
    print('Overlay loaded!')

    # MMIO 接続 (pynqrouter)
    mmio = MMIO(int(PL.ip_dict[IP][0]), int(PL.ip_dict[IP][1]))

    # MMIO 接続 & リセット (LED)
    mmio_led = MMIO(int(PL.ip_dict[IP_LED][0]), int(PL.ip_dict[IP_LED][1]))
    mmio_led.write(0, 0)

    # 入力データをセット
    imem = pack(boardstr)
    for i in range(len(imem)):
        mmio.write(OFFSET_BOARD + (i * 4), imem[i])

    mmio.write(OFFSET_SEED, args.seed)

    # スタート
    # ap_start (0 番地の 1 ビット目 = 1)
    mmio.write(0, 1)
    print('Start!')
    time_start = time.time()

    # ap_done (0 番地の 2 ビット目 = 2) が立つまで待ってもいいが
    #   done は一瞬だけ立つだけのことがあるから
    # ap_idle (0 番地の 3 ビット目 = 4) を待ったほうが良い
    iteration = 0
    while (mmio.read(0) & 4) == 0:
        # 動いてるっぽく見えるようにLチカさせる
        iteration += 1
        if iteration == 10000:
            mmio_led.write(0, 3)
        elif 20000 <= iteration:
            mmio_led.write(0, 12)
            iteration = 0

    # 完了の確認
    print('Done!')
    print('control:', mmio.read(0))
    time_done = time.time()
    elapsed = time_done - time_start
    print('elapsed:', elapsed)
    print('')

    # 状態の取得
    status = int(mmio.read(OFFSET_STATUS))
    print('status:', status)
    if status != 0:
        # 解けなかったらLEDを消す
        mmio_led.write(0, 0)
        sys.stderr.write('Cannot solve it!\n')
        sys.exit(1)
    print('Solved!')

    # 解けたらLEDを全部つける
    mmio_led.write(0, 15)

    # 出力
    omem = []
    for i in range(len(imem)):
        omem.append(mmio.read(OFFSET_BOARD + (i * 4)))
    boards = unpack(omem)

    # 回答の生成
    solution = ('SIZE ' + str(size_x) + 'X' + str(size_y) + 'X' + str(size_z) + '\n')
    for z in range(size_z):
        solution += ('LAYER ' + str(z + 1) + '\n')
        for y in range(size_y):
            for x in range(size_x):
                if x != 0:
                    solution += ','
                i = ((x * MAX_X + y) << BITWIDTH_Z) | z
                if args.zero_padding:
                    solution += '{0:0>2}'.format(boards[i])  # 2桁の0詰め
                else:
                    solution += str(boards[i])               # 普通に表示
            solution += '\n'

    # 表示 & ファイル出力
    if args.print:
        print('')
        print('SOLUTION')
        print('========')
        print(solution)
    if args.output is not None:
        with open(args.output, 'w') as f:
            f.write(solution)


def pack(_str):
    """
    ボードストリング文字列をMMIOメモリにパックする
    """
    _mem = [ 0 for _ in range(MAX_X * MAX_Y * MAX_Z // 4) ]
    for i in range(len(_str)):
        _index  = i // 4;
        _offset = i % 4;
        if _offset == 0:
            _mem[_index] = _mem[_index] | (ord(_str[i]))
        elif _offset == 1:
            _mem[_index] = _mem[_index] | (ord(_str[i]) << 8)
        elif _offset == 2:
            _mem[_index] = _mem[_index] | (ord(_str[i]) << 16)
        elif _offset == 3:
            _mem[_index] = _mem[_index] | (ord(_str[i]) << 24)
    return _mem


def unpack(_mem):
    """
    MMIOメモリからボード情報をアンパックする
    """
    _boards = [ 0 for _ in range(MAX_X * MAX_Y * MAX_Z) ]
    for i in range(len(_mem)):
        _boards[4 * i    ] = (_mem[i])       & 255
        _boards[4 * i + 1] = (_mem[i] >> 8)  & 255
        _boards[4 * i + 2] = (_mem[i] >> 16) & 255
        _boards[4 * i + 3] = (_mem[i] >> 24) & 255
    return _boards


if __name__ == '__main__':
    main()
