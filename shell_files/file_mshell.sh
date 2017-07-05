#!/bin/sh
# file_mshell.sh
# Last Change: 2017/07/05 (Wed) 14:41:59.

if [ $# -ne 1 ] || [ ! -e $1 ] ; then
    echo -e "input shell file correctly\nexitting....."
    exit
fi

SERVER_LIST=./server.txt
while read server
do
    cmd=$1
    echo "-----------------------------------------------"
    echo $server
    echo "-----------------------------------------------"
    ssh $server 'sh ' < $1

done < $SERVER_LIST
