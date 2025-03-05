# Purr-hibition

CMPT 433 Spring project.


## Dependencies
* aarch64-linux-gnu-gcc  cross compiler
* cmake
* python3


## Development Setup

This setup assumes that you are developing on a Linux Debian 12 (Bookworm) Machine.

First, ensure you have the dependencies installed
```bash
(host) $ sudo apt update
(host) $ sudo apt install gcc-aarch64-linux-gnu cmake python3

# verify installation
(host) $ aarch64-linux-gnu-gcc --version
(host) $ python3 --version
(host) $ cmake --version
```

Next, we setup the pre commit
```bash
(host) $ pip install pre-commit
(host) $ pre-commit install

# run the hooks manually to verify setup
(host) $ pre-commit run --all-files
```

To make it easier to deploy each built to the BYAI, we have automated the process in the `./app/CMakeLists.txt`. To use it, create a file in the **root directory** of the project name `build_config.txt` and add the absolute path to the shared folder with the BYAI.
```bash
(host) $ echo BUILD_DIR=<absolute_path_to_shared_folder> > build_config.txt
```

## Build and Execute

To build the project we leverage CMake to simplify our life.
```bash
(host) $ mkdir build
(host) $ cd build
(host) $ cmake -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc ..
(host) $ make -j
(target) $ ./purr-hibition
```
