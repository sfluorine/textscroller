#!/usr/bin/env bash

CC=${CC:-gcc}
$CC src/textscroller.c -Wall -Wextra -O2 -o textscroller
