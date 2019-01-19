#!/bin/bash

FILES=($(find src -name '*.hpp'))

if test "${#FILES[@]}" -eq "$(head -qn1 "${FILES[@]}" | grep -Fxc '#pragma once')" ; then
exit 0
fi

IDX1=$(head -qn1 "${FILES[@]}" | grep -Fxvn '#pragma once' | head -n1 | sed 's/\(^[[:digit:]]*\):.*/\1/1;')
IDX0=$(( $IDX1 - 1))

echo "missing '#pragma once' in line 1 : ${FILES[$IDX0]}"

exit 1
