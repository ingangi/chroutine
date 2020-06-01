#!/bin/sh


SCRIPT_PWD=`cd $(dirname $0); pwd`
OUTPUT_DIR=$SCRIPT_PWD/build
echo "output to " $OUTPUT_DIR
mkdir $OUTPUT_DIR
cd $OUTPUT_DIR
echo "pwd = " `pwd`
cmake ..
make
