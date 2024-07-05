#! /bin/bash
g++ -c lexer.cpp -o lexer.o -O2

g++ lexer.o -o Main -lm