#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
ライン数がたくさんあるような問題を生成する。
ボードサイズは N x N x 8 になる。
1, 2層目は端点が隣接する層、3--8層は端点は離れているけど直線でつながるように作る。
"""

import argparse
import sys

def main():
    parser = argparse.ArgumentParser(description='NLHubeGenerator')
    parser.add_argument('--n', '-n', default=32, type=int,
                        help='N size')
    args = parser.parse_args()

    n = args.n
    if n < 4 or 72 < n:
        print('N must be between 4--72.', file=sys.stderr)
        sys.exit(1)
    if n % 2 != 0:
        print('N must be even.', file=sys.stderr)
        sys.exit(1)

    # ラインリスト生成
    lines = []

    # 1, 2層目
    for x in range(2):
        for y in range(n):
            lines.append(( (x, y, 1), (x, y, 2) ))
    for x in range(2, n, 2):
        for y in range(n):
            lines.append(( (x, y, 1), (x + 1, y, 1) ))
    for x in range(2, n):
        for y in range(0, n, 2):
            lines.append(( (x, y, 2), (x, y + 1, 2) ))

    # 3--8層目
    for x in range(2):
        for y in range(n):
            lines.append(( (x, y, 3), (x, y, 8) ))
    for z in range(3, 6):
        for y in range(n):
            lines.append(( (2, y, z), (n - 1, y, z) ))
    for z in range(6, 9):
        for x in range(2, n):
            lines.append(( (x, 0, z), (x, n - 1, z) ))

    print(lines)

    # 問題文字列生成 & ファイル書き込み
    str = []
    str.append('SIZE %dX%dX8' % (n, n))
    str.append('LINE_NUM %d' % (len(lines)))
    str.append('')
    count = 1
    for line in lines:
        str.append('LINE#%d (%d,%d,%d) (%d,%d,%d)' %
            (count, line[0][0], line[0][1], line[0][2], line[1][0], line[1][1], line[1][2]))
        count += 1

    with open('NL_HUGE%d.txt' % (n), 'w') as f:
        f.write('\n'.join(str) + '\n')


if __name__ == '__main__':
    main()
