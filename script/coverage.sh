#!/bin/bash

mkdir coverage
lcov -d . --capture -o coverage/all.info

cd coverage
lcov -e all.info '*zeppelin/src*' -o zepp.info
genhtml zepp.info
