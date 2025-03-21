CONST(convol_t, AUTOMATIC) conv1d_2_layer = {
	 .activation = 2,
	 .numFilter = 32,
	 .numChannels = 3,
	 .filterRows = 3,
	 .filterCols = 1,
	 .strideRows = 1,
	 .strideCols = 1,
	 .filterLen = 288,
	 .padding = 0,
	 .weight = conv1d_2_weight,
	 .bias = conv1d_2_bias
};
CONST(pooling_t, AUTOMATIC) max_pooling1d_2_layer = {
	 .poolRows = 2,
	 .poolCols = 1,
	 .strideRows = 2,
	 .strideCols = 1,
	 .padding = 0
};
CONST(convol_t, AUTOMATIC) conv1d_3_layer = {
	 .activation = 2,
	 .numFilter = 64,
	 .numChannels = 3,
	 .filterRows = 3,
	 .filterCols = 1,
	 .strideRows = 1,
	 .strideCols = 1,
	 .filterLen = 6144,
	 .padding = 0,
	 .weight = conv1d_3_weight,
	 .bias = conv1d_3_bias
};
CONST(pooling_t, AUTOMATIC) max_pooling1d_3_layer = {
	 .poolRows = 2,
	 .poolCols = 1,
	 .strideRows = 2,
	 .strideCols = 1,
	 .padding = 0
};
CONST(convol_t, AUTOMATIC) conv1d_4_layer = {
	 .activation = 2,
	 .numFilter = 128,
	 .numChannels = 3,
	 .filterRows = 3,
	 .filterCols = 1,
	 .strideRows = 1,
	 .strideCols = 1,
	 .filterLen = 24576,
	 .padding = 0,
	 .weight = conv1d_4_weight,
	 .bias = conv1d_4_bias
};
CONST(dense_t, AUTOMATIC) dense_4_layer = {
	 .activation = 2,
	 .kernel_numRows = 64,
	 .kernel_numCols = 128,
	 .bias_numRows = 64,
	 .bias_numCols = 1,
	 .weight = dense_4_weight,
	 .bias = dense_4_bias
};
CONST(dense_t, AUTOMATIC) dense_5_layer = {
	 .activation = 1,
	 .kernel_numRows = 1,
	 .kernel_numCols = 64,
	 .bias_numRows = 1,
	 .bias_numCols = 1,
	 .weight = dense_5_weight,
	 .bias = dense_5_bias
};
