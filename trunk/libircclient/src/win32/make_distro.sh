#!/bin/sh

mkdir -p win32/lib
mkdir -p win32/include
mkdir -p win32/doc
cp libdynamic-release/*.lib win32/lib/
cp libdynamic-release/*.dll win32/lib/
cp libstatic-release/*.lib win32/lib/
cp ../../include/libircclient.h win32/include/
cp ../../include/libirc_errors.h	win32/include/
cp ../../include/libirc_events.h  win32/include/
cp ../../doc/html/* win32/doc
cd win32
zip -r ../libircclient-win32-vc-0.4.zip .
cd ..
rm -rf win32
