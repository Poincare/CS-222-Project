#!/bin/sh

# Runs the tree kernel code over the spam samples using SVM light.
./svm-light-TK-1.2/svm_learn -t 5 ham_tree.train trained_svm.output
