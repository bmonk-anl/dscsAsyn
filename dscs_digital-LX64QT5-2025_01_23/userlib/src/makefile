#*******************************************************************************
#  Project:        DSCS Control Library
#
#  Filename:       makefile
#
#  Purpose:        GCC makefile for trivial example
#
#  Author:         N-Hands GmbH & Co KG
#*******************************************************************************

example : example.o libdscs.so
	gcc -o example -L../lib example.o -ldscs

example.o : example.c
	gcc -c -g -Wall -std=gnu99 -O0 -I../inc -o example.o example.c

libdscs.so: ../lib/libdscs.so
	cp ../lib/libdscs.so .

clean:
	rm -f example example.o libdscs.so
