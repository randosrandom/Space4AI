# SPACE4-AI Design Time

## Introduction
SPACE4-AI is a tool that tackles the component placement resource selection problem in the computing continuum at design time, while dealing with different application requirements and constraints.

It exploits an efficient randomized greedy algorithm that identifies the placement minimizing a certain loss, e.g. the total cost, across heterogeneous resources including edge devices, cloud GPU-based Virtual Machines and Function as a Service solutions, under Quality of Service (QoS) response time constraints.

| ![UseCase](images/useCase.png "A Use case of identifying wind turbines blade damage") |
|:--:|
| <b>Use Case: maintenance and inspection of a wind farm</b>|

## Dependencies

> :exclamation: We provided indications for ```Ubuntu``` machines, but the flow should be quite similar for any ```UNIX``` distribution, adapting some installation commands. If you use ```Windows``` or you encounter any problem during dependencies installation (or the subsequent compilation process), we suggest to use the library through a ```Docker``` container (further information in the dedicated section).

### pybind11
To use our library you have to install [pybind11](https://github.com/pybind/pybind11). You could refer to the original documentation for the installation but, since it can be tricky to understand which is the correct way to go, we strongly suggest you to perform the follwing steps.

First of all, please update your system to have the latest versions of the compilers, libraries and so on. Then make sure to have **cmake 3.16.3** or above installed. Moreover, you also ought to have **Python 3.7** or above. After that you need to install **python3-dev** packages, for which you can do
```bash
sudo apt-get install -y python3-dev
```
Now you can install pybind11 using the pip wheel:
```bash
pip3 install "pybind11[global]"
```

### pacsltk
If you successfully installed pybind11, you can move to install the [pacsltk](https://github.com/pacslab/serverless-performance-modeling) package.

```bash
pip3 install pacsltk
```

### OpenMP
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
By default, this will generate a Makefile which will compile in RELEASE mode, SERIAL version, creating a SHARED library. For any other combination, we describe the most important *cmake* variables you can set:
- CMAKE_BUILD_TYPE (string). Specifies the build type, which can ```Debug```, ```Release```, ```RelWithDebInfo ``` or ```RelWithDebInfo```. Please refer to the original cmake [documentation](https://cmake.org/cmake/help/latest/), for further information.
- PARALLELIZATION (option). If set to ON, you will compile the parallel version of the library; otherwise it will be serial.
> :warning: The Parallelization is **not** supported if there is the need to call Python functions during the construction of the solution through the algorithm (see **GIL ISSUE** on the report). If you try to run parallel code in this situation, the code will automatically ignore the requested threads, and will run in serial (a warning message will pop out).

- SHARED (option). If set to OFF, you will create a static library instead of shared one.

So, for instance, if you want to compile for DEBUG, in PARALLEL mode, creating STATIC library, you would do:
```bash
cmake -DCMAKE_BUILD_TYPE=Debug -DPARALLELIZATION=ON -DSHARED=OFF ..
```
Actually there are many other things you can set in *cmake*, such as the compiler to use, destinations and so on.
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

> :hammer_and_wrench: At the moment we did not provide instruction for the installation of our library, mainly because it is still under development and it is very likely that in the future many things would change. So at this stage, it is pointless to manage the installation too.

## Docker container

If you have any problem installing dependencies or compiling the library correctly, you can use [Docker](https://docs.docker.com/) virtualization.

First, you need to create an image and a container, starting from our repository. From the source root (namely the folder in which the Dockerfile is located) do

```bash
docker build -t <IMAGE_NAME> .
docker create -it --name <CONTAINER_NAME> <IMAGE_NAME>
```

Once you created the container you can use it

```bash
docker start -i <CONTAINER_NAME>
```

Note that, at the moment of creation of the container, the code will be compiled following the default explained above.
If you need a different building configuration, you can recompile the code in the container.

## Usage

In the ```config``` folder of the project there are stored both the system descriptions files, and the input *.json* files requested by our main executable, namely ```dt_solver``` . In particular, the input has the following structure:
```json
{
  "ConfigFiles" : [
    "config/LargeScale/5-components/instance1/system_description1.json",
    "config/LargeScale/15-components/instance10/system_description10.json"
  ],

  "Algorithm" : {
    "n_iterations" : 5000,
    "max_num_sols" : 1
  },

  "Logger" : {
    "priority" : 5,
    "terminal_stream" : false,
    "file_stream" : false
  }
}
```
- **ConfigFiles**: list of system descriptions you want to solve. Note that the relative path with respect to the root folder of the project must be given for each system file;

- **Algorithm**: Here you can the total number of iterations to request, and the number of top solutions to retain;

- **Logger**: configure Logger messages
  - *priority*: 0 is the lowest priority (print everything possible, useful for hard debugging), 5 is the highest priority (print only the critical errors).

  - *terminal_stream*: set to true if you want to have the messages printed on the terminal, false otherwise.

  - *file_stream*: set to true if you want to have logger messages saved to file. Note that this will automatically create a folder named ```logs``` in the same location of the executable.

In the config folder, there is already a input file called ```config_dt_solver.json``` which you can modify (or you can create one from scratch).

Once you have built the library and configured the input files as you wish, from the ```build``` folder navigate to
```bash
cd apps
```
where you will find (other than building files) a symbolic link to the ```config``` folder introduced above. Now you can launch the solver by doing
```bash
./dt_solver config/config_dt_solver.json
```

If instead, you are using the parallel version, you can set the total number of threads by either exporting the following environmental variable
```bash
export OMP_NUM_THREADS=<NUM_THREADS>
```
or you can change it just for the specific executable launch
```bash
OMP_NUM_THREADS=<NUM_THREADS> ./dt_solver config/config_dt_solver.json

```

After the run, you will find a folder called ```OutputFiles```, in which there will be saved the solutions and some additional files containing the most important information about the saved solutions (like the cost, the number of threads used, the computing time etc...).

## References

<a id="1">[1]</a>
H. Sedghani, F. Filippini and D. Ardagna (2021). A Randomized Greedy Method for AI Applications Component Placement and Resource Selection in Computing Continua. https://doi.ieeecomputersociety.org/10.1109/JCC53141.2021.00022

<a id="2">[2]</a>
Wenzel Jakob, Jason Rhinelander and Dean Moldovan (2017).
Pybind11 -- Seamless operability between C++11 and Python. https://github.com/pybind/pybind11

<a id="3">[3]</a>
Lohmann, N. (2022). JSON for Modern C++. https://github.com/nlohmann

<a id="4">[4]</a>
Merkel, D. (2014). Docker: lightweight linux containers for consistent development and deployment. https://docs.docker.com/
