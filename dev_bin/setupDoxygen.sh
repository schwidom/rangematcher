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

DOXY_MANDATORY="RECURSIVE"
DOXY_EXTRACT="EXTRACT_ALL EXTRACT_PRIVATE EXTRACT_PACKAGE EXTRACT_STATIC EXTRACT_LOCAL_METHODS EXTRACT_ANON_NSPACES"
DOXY_MISC="UML_LOOK TEMPLATE_RELATIONS CALL_GRAPH CALLER_GRAPH SOURCE_BROWSER INLINE_SOURCES"

for i in $DOXY_MANDATORY $DOXY_EXTRACT $DOXY_MISC  ; do
SED_COMMANDS+='s+^\('"$i"'\)\>.*+\1 = YES+1; '
done

for i in STRIP_CODE_COMMENTS ; do
SED_COMMANDS+='s+^\('"$i"'\)\>.*+\1 = NO+1; '
done

SED_COMMANDS+='s+^\(PROJECT_NAME\)\>.*+\1 = "The Rangematcher"+1; '

sed "$SED_COMMANDS" -i rmDoxygenConfigFile.txt

