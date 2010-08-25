#!/bin/sh -e

(test \! -d build && mkdir build)
(cd build && cmake -DCMAKE_BUILD_TYPE=Release -G"Xcode" ..)
