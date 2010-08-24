#!/bin/sh -e

(cd build && cmake -DCMAKE_BUILD_TYPE=Release -G"Xcode" ..)
