#!/bin/bash
if [[ "$#" -lt 4 ]]; then
  echo "Usage: bash iterateEikonal.sh <start> <diff> <count> <parameterToIterate> [rest of params to be passed to main]"
  echo "parameterToIterate <actual value> will be appended."
  echo "Output goes into iterated.txt."
  exit
fi
i=0
t=$1
d=$2
n=$3
p=$4
shift
shift
shift
shift
echo $p direction >iterated.txt
while [[ $i -lt $n ]]; do
  echo ./eikonal $p $t $*
  result=`./eikonal $p $t $* |grep 'start direction'|cut -d ':' -f 2`
  echo $t $result >>iterated.txt
  i=$((i+1))
  t=`awk "BEGIN{print $t + $d}" | tr ',' '.'`
done
