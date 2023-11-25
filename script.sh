#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Utilizare: $0 <c>"
    exit 1
fi

contor=0
caracter=$1

while IFS= read -r line
do
    if [[ $line =~ ^[[:upper:]][[:alnum:][:space:],.!?]*[[:upper:]]*[.!?]$ && ! $line =~ ,\s*și ]]; then
        contor=$((contor+1))
    fi
done

echo "$contor"
