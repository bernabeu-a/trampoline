#ifndef MODEL_H
#define MODEL_H
#include <stdint.h>

#define DENSE_LAYER 0
#define LEAKY_RELU_LAYER 1
#define CONV2D_LAYER 2
#define MAXPOOLING2D_LAYER 3
#define FLATTEN_LAYER 4
#define DROPOUT_LAYER 5

#define LINEAR_ACTIVATION 0
#define SIGMOID_ACTIVATION 1
#define RELU_ACTIVATION 2

#define NUM_OUTPUTS 1
#define FIXED_POINT_PRECISION 10

#define UNUSED(x) (void)(x)

typedef struct {
  uint16_t activation;
  uint16_t numFilter;
  uint16_t numChannels;
  uint16_t filterRows;
  uint16_t filterCols;
  uint16_t strideRows;
  uint16_t strideCols;
  uint16_t filterLen;
  uint16_t padding;
  int16_t *weight;
  int16_t *bias;
} convol_t;

typedef struct {
    uint16_t poolRows;
    uint16_t poolCols;
    uint16_t strideRows;
    uint16_t strideCols;
    uint16_t padding;
} pooling_t;

typedef struct {
    uint16_t activation;
    uint16_t kernel_numRows;
    uint16_t kernel_numCols;
    uint16_t bias_numRows;
    uint16_t bias_numCols;
    int16_t *weight;
    int16_t *bias;
} dense_t;

#endif
