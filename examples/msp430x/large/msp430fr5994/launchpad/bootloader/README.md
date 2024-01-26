# Example Update Over the Air with MSP430FR

To build for host (sender of then application):
```sh
cd bootloader_host
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=debug
make
```
To build for target (receiver of then application):
```sh
cd bootloader_target
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=debug
make
```