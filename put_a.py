#!/usr/bin/env python
# -*- coding: utf-8 -*-

import argparse
import os
import subprocess
import sys
import time

def main():
    parser = argparse.ArgumentParser(description='put_a')
    parser.add_argument('-n', '--n', action='store', nargs='?', default=-1, type=int,
        help='Put Qn problem')
    args = parser.parse_args()

    n = args.n

    if n < 1:
        print('N must be a problem number!', file=sys.stderr)
        sys.exit(1)

    ansdir   = '/home/pi/pynq-router/answers'
    anspath  = ansdir + ('/T03_A%02d.txt' % (n))
    infopath = ansdir + ('/T03_A%02d_info.txt' % (n))
    print(anspath)
    print(infopath)

    if not os.path.exists(anspath):
        print('Answer file does not exist!', file=sys.stderr)
        sys.exit(1)
    if not os.path.exists(infopath):
        print('Info file does not exist!', file=sys.stderr)
        sys.exit(1)

    infolines = []
    with open(infopath, 'r') as f:
        infolines = f.readlines()

    cpu_time = '-1.0'
    memory = '-1'
    for infoline in infolines:
        info = infoline.strip().split(':')
        if info[0] == 'CPU_time':
            cpu_time = info[1]
        if info[0] == 'memory':
            memory = info[1]

    print(cpu_time)
    print(memory)

    # Submit answer
    cmd = '/home/pi/conmgr/client/adccli put-a {} {}'.format(n, anspath)
    print('`{}`'.format(cmd))
    subprocess.call(cmd.strip().split(' '))

    time.sleep(2)

    # Submit info
    cmd = "/home/pi/conmgr/client/adccli put-a-info {} {} {} FPGAsolver".format(n, cpu_time, memory)
    print('`{}`'.format(cmd))
    subprocess.call(cmd.strip().split(' '))


if __name__ == '__main__':
    main()
