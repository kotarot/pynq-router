#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
ライン数を指定して必ずそのライン数の問題を生成する。
"""

import argparse
import sys

def main():
    parser = argparse.ArgumentParser(description='NLLineGenerator')
    parser.add_argument('--l', '-l', default=32, type=int,
                        help='Line size')
    args = parser.parse_args()

    l = args.l

    # ラインリスト生成
    lines = []

    for i in range(l):
        lines.append(( (0, (i % 72), (i // 72 + 1)), (9, (i % 72), (i // 72 + 1)) ))

    print(lines)

    # 問題文字列生成 & ファイル書き込み
    str = []
    str.append('SIZE 72X72X8')
    str.append('LINE_NUM %d' % (l))
    str.append('')
    count = 1
    for line in lines:
        str.append('LINE#%d (%d,%d,%d) (%d,%d,%d)' %
            (count, line[0][0], line[0][1], line[0][2], line[1][0], line[1][1], line[1][2]))
        count += 1

    with open('NL_QLINE%d.txt' % (l), 'w') as f:
        f.write('\n'.join(str) + '\n')


if __name__ == '__main__':
    main()
