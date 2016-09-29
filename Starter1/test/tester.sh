#!/bin/bash

TESTS=$(find . | grep *.in)

rm -f failures.out

for TEST in $TESTS; do
  ../compiler467 -Tn $TEST &> test.out
  TEST_OUT=${TEST/.in/.out}
  if [[ "-o" == $1 ]]; then
    cp test.out $TEST_OUT
  fi
  diff $TEST_OUT test.out >> failures.out
  rm -f test.out
done

