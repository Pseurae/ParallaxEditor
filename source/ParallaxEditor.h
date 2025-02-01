#pragma once

#include <stdio.h>
#include <stdlib.h>

#define LOG(_STR, ...) fprintf(stderr, _STR, __VA_ARGS__)

#define FATAL_ERROR(_STR, ...) \
    do { LOG(_STR, __VA_ARGS__); exit(EXIT_FAILURE); } while (0)

#define ASSERT(_COND, _STR, ...) \
    do { if (!(_COND)) FATAL_ERROR(_STR, __VA_ARGS__); } while (0)