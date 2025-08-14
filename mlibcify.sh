#!/bin/bash

set -e

if [ "$#" -ne 1 ]; then
	echo "usage: INTERPRETER=<path to mlibc ld.so> mlibcify.sh <path to executable>"
	exit 1
fi

if [[ -z "${INTERPRETER}" ]]; then
	echo "INTERPRETER is unset" 1>&2
	exit 1
fi

work="$1_mlibc"

echo "Making $1 compatible with mlibc (result path $work)"

cp "$1" "$work"

patchelf --set-interpreter "$INTERPRETER" "$work"
patchelf --set-os-abi solaris "$work"

syms=$(nm "$work" --dynamic | grep -e "[Uw] .*@GLIBC" | sed -E -e 's/U (.*)@GLIBC.*/\1/g' -e 's/ //g')

while IFS= read -r line; do
	patchelf --clear-symbol-version "$line" "$work"
done <<< "$syms"

