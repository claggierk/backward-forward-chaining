#!/bin/bash

./clean.sh

echo " ***** backward-forward-chaining ***********************************"

g++ *.h *.cc -o chaining
compiled_status=$?

if [[ $compiled_status -eq 0 ]]
then
    echo "Compilation success!"
    command="./chaining"
    $command
else
    echo "failed compilation"
    exit 2
fi

echo " *******************************************************************"
echo ""

