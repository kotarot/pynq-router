#!/bin/sh
# file_mshell.sh
# Last Change: 2017/07/05 (Wed) 15:50:27.

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
    scp $1 $server:~

done < $SERVER_LIST
