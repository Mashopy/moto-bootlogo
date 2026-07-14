# SPDX-License-Identifier: AGPL-3.0-or-later

OBJS = build/main.o build/BinFile.o build/BinHeader.o build/BinImage.o

CC = g++

DEBUG = -g

CFLAGS = -Wall -c $(DEBUG) $(shell pkg-config --cflags libpng)
LFLAGS = -Wall $(DEBUG)
LIBS = $(shell pkg-config --libs libpng)

all: build

build: $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) $(LIBS) -o build/moto-bootlogo

build/main.o: src/main.cpp
	$(CC) $(CFLAGS) -o $@ $<

build/BinFile.o: src/BinFile.cpp
	$(CC) $(CFLAGS) -o $@ $<

build/BinHeader.o: src/BinHeader.cpp
	$(CC) $(CFLAGS) -o $@ $<

build/BinImage.o: src/BinImage.cpp
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f build/*
