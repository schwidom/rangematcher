#!/bin/bash

set -x

cd doxygen
rm -f rmDoxygenConfigFile.txt rmDoxygenConfigFile_original.txt
doxygen -g rmDoxygenConfigFile.txt
cp -i rmDoxygenConfigFile.txt rmDoxygenConfigFile_original.txt

SED_COMMANDS='s+^\(INPUT\)\>.*+\1 = "../src"+1; '

for i in EXTRACT_ALL EXTRACT_PRIVATE EXTRACT_PACKAGE EXTRACT_STATIC EXTRACT_LOCAL_METHODS EXTRACT_ANON_NSPACES RECURSIVE; do
SED_COMMANDS+='s+^\('"$i"'\)\>.*+\1 = YES+1; '
done

sed "$SED_COMMANDS" -i rmDoxygenConfigFile.txt

