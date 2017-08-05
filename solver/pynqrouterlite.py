#!/opt/python3.6/bin/python3.6
# -*- coding: utf-8 -*-

"""
PYNQ 上で pynqrouterlite を実行する。
"""

from pynq import Overlay
from pynq import PL
from pynq import MMIO


def main():
    # Settings
    IP = 'SEG_pynqrouter_0_Reg'
    OFFSET_BOARD = 65536   # 0x10000 ~ 0x1ffff
    OFFSET_SEED  = 131072  # 0x20000
    MAX_X = 72
    MAX_Y = 72
    MAX_Z = 8

    # Overlay 読み込み
    OL = Overlay('pynqrouterlite.bit')
    OL.download()
    print(OL.ip_dict)
    print('Overlay loaded!')

    # MMIO 接続
    mmio = MMIO(int(PL.ip_dict[IP][0]), int(PL.ip_dict[IP][1]))

    # 入力データをセット
    mmio.write(OFFSET_BOARD + 0, 0)
    mmio.write(OFFSET_BOARD + 4, 4)
    mmio.write(OFFSET_BOARD + (MAX_X * MAX_Y * MAX_Z) - 4, 0)

    mmio.write(OFFSET_SEED, 1)

    # スタート
    # ap_start (0 番地の 1 ビット目 = 1)
    mmio.write(0, 1)
    print('Start!')

    # ap_done (0 番地の 2 ビット目 = 2) が立つまで待ってもいいが
    #   done は一瞬だけ立つだけのことがあるから
    # ap_idle (0 番地の 3 ビット目 = 4) を待ったほうが良い
    while (mmio.read(0) & 4) == 0:
        # 動いてるっぽく見えるようにLチカさせる
        pass

    # 完了の確認
    print('control =', mmio.read(0))
    print('Done!')

    # 出力
    print(mmio.read(OFFSET_BOARD + 0))
    print(mmio.read(OFFSET_BOARD + 4))
    print(mmio.read(OFFSET_BOARD + (MAX_X * MAX_Y * MAX_Z) - 4))
    print(mmio.read(OFFSET_BOARD + (MAX_X * MAX_Y * MAX_Z)))


if __name__ == '__main__':
    main()
