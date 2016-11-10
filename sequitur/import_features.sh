#!/bin/bash
FILES=../enron1/ham/*
for f in $FILES
do
	./sequitur_simple < $f > test.out
done
