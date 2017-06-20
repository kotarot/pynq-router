#!/usr/bin/env python
# -*- coding: utf-8 -*-

import argparse

if __name__ == '__main__':

    parser = argparse.ArgumentParser(description='gen_boardstr')
    parser.add_argument('input', nargs=None, default=None, type=str,
        help='Path to input problem file')
    args = parser.parse_args()

    # 問題ファイルの読み込み
    with open(args.input, 'r') as f:
        lines = f.readlines()

    # 問題ファイルを boardstr に変換
    boardstr = ''
    for line in lines:
        if 'SIZE' in line:
            x, y, z = line.strip().split(' ')[1].split('X')
            boardstr += ('X%02dY%02dZ%d' % (int(x), int(y), int(z)))
        if 'LINE_NUM' in line:
            pass
        if 'LINE#' in line:
            sp = line.strip().replace('-', ' ').replace('(', '').replace(')', '').split(' ')
            sp_s = sp[1].split(',')
            sp_t = sp[2].split(',')
            boardstr += ('L%02d%02d%d%02d%02d%d' % (int(sp_s[0]), int(sp_s[1]), int(sp_s[2]), int(sp_t[0]), int(sp_t[1]), int(sp_t[2])))

    # 表示
    print(boardstr)
