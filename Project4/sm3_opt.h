#ifndef SM3_H
#define SM3_H

#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int sm3_opt(unsigned char *msg, unsigned int msglen, unsigned char *out_hash);

#endif