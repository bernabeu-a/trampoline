#ifndef MATRIX_H
#define MATRIX_H
#include <stdint.h>

typedef int16_t dtype;

typedef struct {
    dtype *data;
    uint16_t numRows;
    uint16_t numCols;
} matrix;
// typedef struct matrix matrix;

#endif
