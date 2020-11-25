#!/bin/bash
PROCESS=$(ipcs -s | grep 644 | awk '{print $2}')
for i in $PROCESS
do
    ipcrm -s $i
done
PROCESS=$(ipcs -m | grep 644 | awk '{print $2}')
for i in $PROCESS
do
    ipcrm -m $i
done
PROCESS=$(ipcs -q | grep 644 | awk '{print $2}')
for i in $PROCESS
do
    ipcrm -q $i
done
