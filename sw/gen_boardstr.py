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
            #print(sp)
            # s (スタート) -> g (ゴール)
            s_str = sp[1].split(',')
            g_str = sp[2].split(',')
            s_tpl = (int(s_str[0]), int(s_str[1]), int(s_str[2]) - 1)
            g_tpl = (int(g_str[0]), int(g_str[1]), int(g_str[2]) - 1)
            # 端に近い方をスタートにする (各端までの距離計算)
            s_dist_x = min(s_tpl[0], int(x) - 1 - s_tpl[0])
            s_dist_y = min(s_tpl[1], int(y) - 1 - s_tpl[1])
            s_dist_z = min(s_tpl[2], int(z) - 1 - s_tpl[2])
            s_dist = s_dist_x + s_dist_y + s_dist_z
            #print(s_dist_x, s_dist_y, s_dist_z, s_dist)
            g_dist_x = min(g_tpl[0], int(x) - 1 - g_tpl[0])
            g_dist_y = min(g_tpl[1], int(y) - 1 - g_tpl[1])
            g_dist_z = min(g_tpl[2], int(z) - 1 - g_tpl[2])
            g_dist = g_dist_x + g_dist_y + g_dist_z
            #print(g_dist_x, g_dist_y, g_dist_z, g_dist)
            # 端に近い方を選ぶ
            if s_dist <= g_dist:
                boardstr += ('L%02d%02d%d%02d%02d%d' % (int(s_str[0]), int(s_str[1]), int(s_str[2]), int(g_str[0]), int(g_str[1]), int(g_str[2])))
                #print('S')
            else:
                boardstr += ('L%02d%02d%d%02d%02d%d' % (int(g_str[0]), int(g_str[1]), int(g_str[2]), int(s_str[0]), int(s_str[1]), int(s_str[2])))
                #print('G')

    # 表示
    print(boardstr)
