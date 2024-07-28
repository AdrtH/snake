#!/bin/bash

cc -Wall -Wextra -pedantic main.c `sdl2-config --cflags --libs` -o main
