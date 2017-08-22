#!/usr/bin/env python
# -*- coding: utf-8 -*-

import argparse
import os, sys

import BoardStr

def main():
    parser = argparse.ArgumentParser(description='gen_boardstr')
    parser.add_argument('input', nargs=None, default=None, type=str,
        help='Path to input problem file')
    parser.add_argument('--edge-start', '-e', default=False, action='store_true', required=False,
        help='Turn on to set farther terminals from the center as starts')
    args = parser.parse_args()

    # 問題ファイルの読み込み
    with open(args.input, 'r') as f:
        lines = f.readlines()

    # 問題ファイルを boardstr に変換
    boardstr = BoardStr.conv_boardstr(lines, args.edge_start)

    # 表示
    print(boardstr)


if __name__ == '__main__':
    main()
