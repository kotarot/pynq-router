#!/usr/bin/env python
# -*- coding: utf-8 -*-

import argparse
import subprocess


def main():
    parser = argparse.ArgumentParser(description='get_q')
    parser.add_argument('-n', '--n', action='store', nargs='?', default=10, type=int,
        help='Get problems from Q1 to Qn')
    args = parser.parse_args()

    n = args.n

    for i in range(1, n + 1):
        cmd = '/home/pi/conmgr/client/adccli --output /home/pi/pynq-router/problems/NL_Q%02d.txt get-q %d' % (i, i)
        print('`{}`'.format(cmd))
        subprocess.call(cmd.strip().split(' '))


if __name__ == '__main__':
    main()
