#!/bin/bash

TEST_PROGRAM="../compiler467 -Dsa"

TEST_FILE="test.out"
FAILURES_FILE="failures.out"

# Empty the failures file
rm -f $FAILURES_FILE
> $FAILURES_FILE

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

# Interpret argument 1 as overwrite if its -o
# Interpret all other arguments as specific tests cases that we want to run
if [[ $1 == '-o' ]]; then
  OVERWRITE=1
  # Take all arguments after the first if they aren't empty
  if [[ ${@:2} != "" ]]; then
    REGEX_TESTS="${@:2}"
  fi
else
  # Take all arguments if they aren't empty
  if [[ $@ != "" ]]; then
    REGEX_TESTS="$@"
  fi
fi

if [[ $REGEX_TESTS != "" ]]; then
  # Clear all tests but keep them in the variable ALL_TESTS
  ALL_TESTS="$TESTS"
  TESTS=""
  for REGEX_TEST in $REGEX_TESTS; do
    for TEST in $ALL_TESTS; do
      # Test each regex against each test and if it matches, add it to TESTS
      if [[ $TEST =~ $REGEX_TEST.* ]]; then
        TESTS="$TESTS $TEST"
      fi
    done
  done
fi

# Return the number of failed cases
EXIT_CODE=0

for TEST in $TESTS; do
  TEST_IN="${TEST}.in"
  TEST_OUT="${TEST}.out"

  # Get the output from the input file
  $TEST_PROGRAM $TEST_IN >> $TEST_FILE 2>&1

  # If the -o option is specified, overwrite the expected file
  if [[ $OVERWRITE -eq 1 ]]; then
    cp $TEST_FILE $TEST_OUT
  fi

  if [ ! -f $TEST_OUT ]; then
    > $TEST_OUT
  fi

  # Perform the diff
  FAILURES=$(diff $TEST_OUT $TEST_FILE)

  # Determine the status of the test
  if [[ $? -eq 0 ]]; then
    STATUS="${GREEN}PASS${DEFAULT}"
  else
    STATUS="${RED}FAIL${DEFAULT}"

    # Output the failure information to the failure file and to stdout
    echo "-------------" | tee -a $FAILURES_FILE
    echo $TEST | tee -a $FAILURES_FILE
    echo "$FAILURES" | tee -a $FAILURES_FILE
    echo "-------------" | tee -a $FAILURES_FILE

    # Increment the number of errors
    ((EXIT_CODE++))
  fi

  # Print the status nicely
  RESULTS+="$(printf "%-${MIN_OUTPUT_LENGTH}s : ${STATUS}" $TEST)"$'\n'

  # Cleanup
  rm -f $TEST_FILE
done

printf "$RESULTS"

exit $EXIT_CODE

