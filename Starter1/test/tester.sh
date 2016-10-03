#!/bin/bash

TEST_FILE="test.out"
FAILURES_FILE="failures.out"

rm -f $FAILURES_FILE

# Colours for output
DEFAULT="\e[39m"
GREEN="\e[32m"
RED="\e[31m"

# Find all tests in this folder
# Remove the './' prefix and '.in' suffix
TESTS=$(find . | grep '^.*\.in$' | sed -e 's/^\.\///' | sed -e 's/\.in$//')

# Determine the length of the longest test name
MIN_OUTPUT_LENGTH=0
for TEST in $TESTS; do
  if [[ ${#TEST} -gt $MIN_OUTPUT_LENGTH ]]; then
    MIN_OUTPUT_LENGTH=${#TEST}
  fi 
done

for TEST in $TESTS; do
  TEST_IN="${TEST}.in"
  TEST_OUT="${TEST}.out"

  # Get the output from the input file
  ../compiler467 -Tn $TEST_IN &> $TEST_FILE

  # If the -o option is specified, overwrite the expected file
  if [[ "-o" == $1 ]]; then
    cp $TEST_FILE $TEST_OUT
  fi

  # Perform the diff
  FAILURES=$(diff $TEST_OUT $TEST_FILE)

  # Determine the status of the test
  if [[ $? -eq 0 ]]; then
    STATUS="${GREEN}PASS${DEFAULT}"
  else
    STATUS="${RED}FAIL${DEFAULT}"

    # Output the failure information to the failure file
    echo "-------------" | tee -a $FAILURES_FILE
    echo $TEST | tee -a $FAILURES_FILE
    echo "$FAILURES" | tee -a $FAILURES_FILE
    echo "-------------" | tee -a $FAILURES_FILE
  fi

  # Print the status nicely
  printf "%-${MIN_OUTPUT_LENGTH}s : ${STATUS}\n" $TEST

  # Cleanup
  rm -f $TEST_FILE
done

