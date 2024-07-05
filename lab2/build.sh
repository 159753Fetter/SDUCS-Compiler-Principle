#! /bin/bash
g++ -c main.cpp -o main.o -O2
g++ -c parser.cpp -o parser.o -O2

g++ main.o parser.o -o Main -lm