#!/bin/sh

dir=$1
echo $1

ls $dir > temp.file

while read line
do
    echo $line
    rm -rf $1/$line
done < temp.file

rm -rf temp.file

