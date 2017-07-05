#!/bin/sh
# mshell.sh
# Last Change: 2017/07/05 (Wed) 14:41:34.

cmd_cnf=""
cmd_exec=""

echo -e "type command:\ntype\n.<Enter>\nto execute:\n"
while read line
do
    if [ "$line" = "." ]; then
        break
    fi
    cmd_cnf=$cmd_cnf$line"\n"
    cmd_exec=$cmd_exec$line";"
done

echo "-----------------------------------------------"
echo "command"
echo "-----------------------------------------------"
echo -e "$cmd_cnf"

SERVER_LIST=./server.txt
while read server
do
    echo "-----------------------------------------------"
    echo $server
    echo "-----------------------------------------------"
    ssh -n $server "$cmd_exec"
    echo ""
done < $SERVER_LIST
