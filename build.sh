#!/usr/bin/env bash

set -xe

CC=gcc
$CC src/textscroller.c -O2 -o textscroller
