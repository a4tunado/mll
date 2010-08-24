#!/bin/sh -e

(cd build && cmake -DCMAKE_BUILD_TYPE=Release -G"Eclipse CDT4 - Unix Makefiles" ..)
