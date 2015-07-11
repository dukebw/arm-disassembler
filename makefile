# NOTE(brendan): set P using "export P=program_name" from shell
OBJECTS=
STD_FLAGS=gnu11
MORE_FLAGS=#-DSHA1TEST
CFLAGS=`pkg-config --cflags glib-2.0` -ggdb3 -std=$(STD_FLAGS) -Wall -Wextra -Werror -O0
LDLIBS=`pkg-config --libs glib-2.0` -lm
CC=gcc
CXX=g++

$(P):$(OBJECTS)
