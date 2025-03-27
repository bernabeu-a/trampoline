# Compiled Tensorflow-microlite


Step 1:
  - Clone tflite-micro
```sh
git clone https://github.com/tensorflow/tflite-micro.git
```
Step 2:
  - Compile library for cortex-m4
```sh
make -f tensorflow/lite/micro/tools/make/Makefile TARGET=cortex_m_generic TARGET_ARCH=cortex-m4 OPTIMIZED_KERNEL_DIR=cmsis_nn microlite
```
Step 3:
  - Copy generate here
```sh
cp -r <path_to_tflite-micro/gen/cortex_m_generic_cortex-m4_default_cmsis_nn_gcc/> <here>
```
# You can now use tflite-micro library with cortex m4
