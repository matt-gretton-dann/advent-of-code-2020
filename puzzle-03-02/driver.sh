#!/bin/sh

set -eu

exe="$1"
input="$2"

output=1
for i in "1 1" "3 1" "5 1" "7 1" "1 2"; do
  result="$("$exe" $i < $input)"
  output="$((${output} * ${result}))"
done

echo ${output}