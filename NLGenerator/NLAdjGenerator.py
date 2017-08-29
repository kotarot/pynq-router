#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
すべてのラインが隣接している問題を生成する。
"""

import argparse
import sys

def main():
    parser = argparse.ArgumentParser(description='NLAdjGenerator')
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

    for z in range(1, 9, 2):
        for x in range(n):
            for y in range(n):
                lines.append(( (x, y, z), (x, y, z + 1) ))

    #print(lines)

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

    with open('NL_QADJ%d.txt' % (n), 'w') as f:
        f.write('\n'.join(str) + '\n')


if __name__ == '__main__':
    main()
