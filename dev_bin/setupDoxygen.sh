#!/bin/bash

set -x
set -u

cd doxygen
rm -f rmDoxygenConfigFile.txt rmDoxygenConfigFile_original.txt
doxygen -g rmDoxygenConfigFile.txt
cp -i rmDoxygenConfigFile.txt rmDoxygenConfigFile_original.txt

SED_COMMANDS=''

for i in INPUT STRIP_FROM_PATH; do
SED_COMMANDS+='s+^\('"$i"'\)\>.*+\1 = "../src"+1; '
done

for i in EXTRACT_ALL EXTRACT_PRIVATE EXTRACT_PACKAGE EXTRACT_STATIC EXTRACT_LOCAL_METHODS EXTRACT_ANON_NSPACES RECURSIVE; do
SED_COMMANDS+='s+^\('"$i"'\)\>.*+\1 = YES+1; '
done

sed "$SED_COMMANDS" -i rmDoxygenConfigFile.txt

