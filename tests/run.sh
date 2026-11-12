#!/bin/sh
# Builds and runs the host tests (they need only a C compiler).
set -e
cd "$(dirname "$0")"
${CC:-cc} -Wall -Istub -I../include -o switch_input_test switch_input_test.c ../src/platform/switch/switch_input.c
./switch_input_test
${CC:-cc} -Wall -Istub -I../include -o switch_text_test switch_text_test.c ../src/platform/switch/switch_text.c
./switch_text_test
rm -f switch_input_test switch_text_test
