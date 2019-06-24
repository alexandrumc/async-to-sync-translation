#!/bin/bash

# This script runs ATHOS on all examples and checks results against references
# Run: ./regression_script.sh y/n y/n
# First y/n: keep results or no after execution
# second y/n: run with Verifast or no

# Delete already existing results folder
rm -rf results

if [ $# -ne 2 ]; then
	echo "Wrong number of parameters"
	exit 1
fi

disVerif=0
keep=0

# Check input
if [ "$1" == "y" ]; then
	keep=1
elif [ "$1" != "n" ]; then
	echo "Illegal first parameter"
	exit 1
fi

if [ "$2" == "y" ]; then
	disVerif=1
elif [ "$2" != "n" ]; then
	echo "Illegal second parameter"
	exit 1
fi

if [ $disVerif -eq 1 ]; then
	command="./run-translation -disableVerifast -allExamples"
else
	command="./run-translation -allExamples"
fi

echo "Creating results folder"

$command 2>/dev/null 1>/dev/null

echo "Start output checking:"

overall=1

FILES=./results/*.sync.c

for f in $FILES
do

	filename=$(echo $f | cut -d / -f 3)

	algo_name=$(echo $filename | cut -d . -f 1)
	
	diff "results/$filename" "examples/outputs-sync/$filename" >/dev/null 2>/dev/null
	
	if [ $? -ne 0 ]; then
		tput setaf 1; echo "Algorithm $algo_name has wrong output"
		overall=0
	else
		tput setaf 2; echo "Algorithm $algo_name is ok"
	fi
done

tput sgr0
echo ""

if [ $overall -eq 1 ]; then
	echo "All files OK"
else
	echo "Outputs differ. Check implementation"
fi

if [ $keep -eq 0 ]; then
	rm -rf results
fi
