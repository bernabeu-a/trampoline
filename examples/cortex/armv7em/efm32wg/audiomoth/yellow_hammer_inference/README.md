# Yellow_Hammer inference example

This example use the CMake build system with VSCode.

- First open a terminal from this folder
- Then run goil
```sh
goil --target=cortex/armv7em/efm32wg --templates=../../../../../../goil/templates/ main.oil
```
- Finally open vscode from this folder
```sh
code .
```
- Now use a toolkit to compile the application
- Finally flash and debug by preessing F5 within VSCode

# From TFLite model to TFLM model

Get TFLM from github repository
```sh
git clone https://github.com/tensorflow/tflite-micro.git
```

In ```tflite-micro/tensorflow/micro/tools```, use the ```generate_cc_arrays.py``` script such as:

```sh
generate_cc_arrays.py <output_directory> <input_tflite_model>
```

This will create 2 files, a header and a source file.