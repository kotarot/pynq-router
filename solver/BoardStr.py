#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
問題ファイルを boardstr に変換したりするクラス。
"""

import random
import re

def conv_boardstr(lines, terminals='initial', _seed=12345):
    """
    問題ファイルを boardstr に変換
    """

    random.seed(_seed)

    boardstr = ''
    for line in lines:
        if 'SIZE' in line:
            x, y, z = line.strip().split(' ')[1].split('X')
            boardstr += ('X%02dY%02dZ%d' % (int(x), int(y), int(z)))
        if 'LINE_NUM' in line:
            pass
        if 'LINE#' in line:
            _line = re.sub(r', +', ',', line)
            _line = re.sub(r' +', ' ', _line)
            sp = _line.strip().replace('-', ' ').replace('(', '').replace(')', '').split(' ')
            print(sp)

            # s (スタート) -> g (ゴール)
            s_str = sp[1].split(',')
            g_str = sp[2].split(',')
            s_tpl = (int(s_str[0].strip()), int(s_str[1].strip()), int(s_str[2].strip()) - 1)
            g_tpl = (int(g_str[0].strip()), int(g_str[1].strip()), int(g_str[2].strip()) - 1)

            # 端に近い方をスタートにしたいから各端までの距離計算する
            # (探索のキューを小さくしたいから)
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

            # start と goal
            start_term = '%02d%02d%d' % (int(s_str[0]), int(s_str[1]), int(s_str[2]))
            goal_term  = '%02d%02d%d' % (int(g_str[0]), int(g_str[1]), int(g_str[2]))

            # 端に近い方をスタートにするオプションがオンのときは距離に応じて端点を選択する
            if terminals == 'edgefirst':
                if s_dist <= g_dist:
                    boardstr += ('L' + start_term + goal_term)
                else:
                    boardstr += ('L' + goal_term + start_term)
            # ランダムにスタート・ゴールを選ぶ
            elif terminals == 'random':
                if random.random() < 0.5:
                    boardstr += ('L' + start_term + goal_term)
                else:
                    boardstr += ('L' + goal_term + start_term)
            # 問題ファイルに出てきた順
            else:
                boardstr += ('L' + start_term + goal_term)

    return boardstr
