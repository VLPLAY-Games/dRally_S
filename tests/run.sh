#!/bin/sh
# Builds and runs the host tests (they need only a C compiler).
set -e
cd "$(dirname "$0")"
${CC:-cc} -Wall -Istub -o switch_input_test switch_input_test.c ../switch_input.c
./switch_input_test
rm -f switch_input_test
