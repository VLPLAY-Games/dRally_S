#!/bin/sh
# Builds and runs the host tests (they need only a C compiler).
set -e
cd "$(dirname "$0")"
${CC:-cc} -Wall -Istub -I../include -o switch_input_test switch_input_test.c ../src/platform/switch/switch_input.c ../src/platform/switch/switch_steer.c ../src/platform/switch/switch_config.c -lm
./switch_input_test
${CC:-cc} -Wall -Istub -I../include -o switch_text_test switch_text_test.c ../src/platform/switch/switch_text.c ../src/platform/switch/switch_keys.c
./switch_text_test
${CC:-cc} -Wall -Istub -I../include -o switch_touch_test switch_touch_test.c ../src/platform/switch/switch_touch.c
./switch_touch_test
${CC:-cc} -Wall -I../include -o datacheck_test datacheck_test.c ../src/core/datacheck.c
./datacheck_test
${CC:-cc} -Wall -I../include -o frame_clock_test frame_clock_test.c ../src/core/frame_clock.c
./frame_clock_test
${CC:-cc} -Wall -I../include -o switch_config_test switch_config_test.c ../src/platform/switch/switch_config.c
./switch_config_test
${CC:-cc} -Wall -I../include -o switch_steer_test switch_steer_test.c ../src/platform/switch/switch_steer.c ../src/platform/switch/switch_config.c ../src/platform/switch/switch_rumble.c -lm
./switch_steer_test
python3 ipxnet_test.py
rm -f switch_steer_test frame_clock_test switch_config_test switch_input_test switch_text_test switch_touch_test datacheck_test
