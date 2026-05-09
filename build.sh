#!/usr/bin/bash

gcc yezik.c -Wall -Wextra -Werror -o yezik && ./yezik $1 $1.c
