#########################################################################
# File Name: test.sh
# Author: Sues
# mail: sumory.kaka@foxmail.com
# Created Time: 2018年07月19日 星期四 11时15分51秒
# Version : 1.0
#########################################################################
#!/bin/bash


nu=100

for((i=1;i<=$nu;i=i+1))
do
    sudo insmod simplefifo.ko
    sudo insmod simplefifo_dev.ko
    ls /dev/simplefifo -l #>> file_out
    sleep 1
    sudo rmmod simplefifo_dev simplefifo
    sleep 1
done

