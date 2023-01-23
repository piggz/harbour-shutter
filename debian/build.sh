#!/bin/bash
. debian/vars
set -e
set -x

qmake VERSION='0.0.1-1' FLAVOR=kirigami
make
