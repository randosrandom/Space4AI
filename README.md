# SPACE4-AI Design Time

## Introduction

SPACE4-AI is a tool that tackles the component placement resource selection problem in the computing continuum at design time, while dealing with different application requirements and constraints.

It exploits an efficient randomized greedy algorithm that identifies the placement minimizing a certain loss, e.g. the total cost, across heterogeneous resources including edge devices, cloud GPU-based Virtual Machines and Function as a Service solutions, under Quality of Service (QoS) response time constraints.

| ![UseCase](images/useCase.png "A Use case of identifying wind turbines blade damage") |
|:--:|
| <b>Use Case: maintenance and inspection of a wind farm</b>|

## Dependencies

To use the library you have to install [pybind11](https://github.com/pybind/pybind11). You could refer to the original documentation for the installation but, since it can be tricky to understand which is the correct way to go, we strongly suggest you to perform the follwing steps.

First of all, please update your system to have the latest versions of the compilers, libraries and so on. Then make sure to have **cmake 3.16.3** or above installed. Moreover, you also ought to have **Python 3.7** or above. After that you need to install **python3-dev** packages, for which you can do
```bash
sudo apt-get install -y python3-dev
```
Now you can install pybind11 using the pip wheel:
```bash
pip3 install "pybind11[global]"
```

If moreover, you will use the PARALLEL version of the library, you need to have the **OpenMP** installed. In general, most modern compilers provide support for OpenMP. If you are using gcc, you can check the configuration by running
```bash
echo |cpp -fopenmp -dM |grep -i open
```

If OpenMP is not featured in the compiler, you can configure it using the command
```bash
sudo apt install libomp-dev
```

## Compile the library
After you have installed the dependencies, clone this repository, and enter the source root.
```bash
cd <YOUR_FOLDER>/SPACE4-AI_DT
```
Create a *build* sub-folder and navigate to it
```bash
mkdir build && cd build
```
Now you can generate the *Makefile* through the build system *cmake*:
```bash
cmake ..
```
By default, this will generate a Makefile which will then create a STATIC library, SERIAL version, and in RELEASE mode. For any other combination, we describe the most important *cmake* variables you can set:
- CMAKE_BUILD_TYPE (string). Specifies the build type, which can ```Debug```, ```Release```, ```RelWithDebInfo ``` or ```RelWithDebInfo```. Please refer to the original cmake [documentation](https://cmake.org/cmake/help/latest/), for further information.
- PARALLELIZATION (option). If set to ON, you will compile the parallel version of the library; otherwise it will be serial.
> :warning: The Parallelization is **not** supported if there is the need to call Python functions during the construction of the solution through the algorithm (see **GIL ISSUE** on the report). If you try to run parallel code in this situation, the code will automatically ignore the requested threads, and will run in serial (a warning message will pop out).

- SHARED (option). If set to ON, you will create a Shared library instead of Static one.

So, for instance, if you want to compile the RELEASE, in PARALLEL mode, creating the SHARED library, you would do:
```bash
cmake -DCMAKE_BUILD_TYPE=Release -DPARALLELIZATION=ON -DSHARED=ON ..
```
Actually there are many other things you can set in *cmake*, such as setting the compiler to use, destinations and so forth.
Again, please refer to the original [documentation](https://cmake.org/cmake/help/latest/).

After you have generated the Makefile, you are ready to compile the library by doing:
```bash
make VERBOSE=1
```
Eventually, to ensure that everything went well, please do
```bash
make test
```
to run a bunch of tests, checking the correct compilation of the library and installation of the dependencies.
If some tests go wrong, please run the tests in VERBOSE mode to try to understand the problem.
```bash
ctest --verbose
```
<br />

> :exclamation: We provided indications for ```Ubuntu``` machines, but the flow should be quite similar for any ```UNIX``` distribution, adapting some installation commands. If you use ```Windows``` or you encounter any problem in the installation or compilation processes, we suggest to use the library through a ```Docker``` container (further information in the dedicated section).

> :hammer_and_wrench: At the moment we did not provide instruction for the installation of our library, mainly because it is still under development and it is very likely that in the future many things would change. So at this stage, it is pointless to manage the installation too.



DOCKERFILE
