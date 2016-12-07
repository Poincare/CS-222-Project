#!/bin/bash
FILES=../enron/spam/*
for f in $FILES
do
	./sequitur_simple < $f > test.out
done
