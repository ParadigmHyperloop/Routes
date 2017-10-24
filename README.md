# Routes

## Purpose
This project is intended to calculate realistic possible hyperloop routes. We build a representation of the terrain that a hyperloop will be crossing with GIS elevation data. 

## The Algorithm
The algorithm itself is a genetic algorithm. This essentially simulates natural selection like organic life would undergo in nature. First it generates an initial population of routes from the desired start to the destination. It then ranks these paths based on a number of factors including how high the track is above the land, how much tunneling would be required, how curved the track is, etc.. The best individuals are then "bred" together to create a new generation of (hopefully) better solutions. This process is repeated for a desired number of iterations. Upon completion the most "fit" path will be given.

## Dependencies
Dependencies are automatically downloaded and installed with setup.sh, except for OpenCL.

- Geospatial Data Abstraction Library (GDAL)  http://www.gdal.org
- Boost  http://www.boost.org
- OpenGL Mathematics (GLM) https://glm.g-truc.net/0.9.8/index.html
- OpenCL https://www.khronos.org/opencl/

## Setup Instructions

### Linux / macOS
Run these commands
```
git clone https://github.com/ParadigmHyperloop/Routes.git
cd Routes
sudo ./setup.sh
```
This will clone GDAL, compile it, and install it. This will take a while and is also why `./setup.sh` requires sudo. It will also download boost 1.6.5 and copy all of the headers into the include directory.


After setup finishes, run these commands to actually build the Routes project.
```
mkdir build
cd build
cmake ..
make
```
You should now see a built binary in the build directory.

### Windows
Unfortunately the setup for Windows is a bit more complicated. For simplicity we used MinGW to compile the project so the setup.sh script could be reused. Other compilers should work but the CMake file is specifically built for using MinGW, at least for now.
1. Download and install MinGW. The easiest way to do this is to use the installer, which can be found at https://sourceforge.net/projects/mingw/files/Installer/. Make sure MinGW is installed directly onto C:\. After installing you should now have a folder C:\MinGW\
2. Make sure MSYS and binutils are installed. This can also be done in the installer fairly easily by checking the msys-base and mingw32-binutils and then going to Installation > Apply Changes
3. Find MSYS. It should be in C:\MinGW\msys\1.0 and its a batch file called MSYS. Open it.
4. Clone the repo into any folder on your C drive
5. Type in these commands:
```
cd /c/path/to/repo/Routes/
./setup.sh
```
This will start compiling GDAL and download and place header dependencies in the correct place.
6. After setup finishes, run these commands to actually build the Routes project. Make sure this is done in MSYS.
```
mkdir build
cd build
cmake -G "MSYS Makefiles" ..
make
```
You should now see a built binary in the build directory. In order to run the program, open back up MSYS and run it from there.

## Data
Data is pulled from the USGS, and is 1 arc second 3D elevation products. You can find all USGS data at https://viewer.nationalmap.gov/basic/

Currently the algorithm is being tested on these pieces of data:
- https://prd-tnm.s3.amazonaws.com/StagedProducts/Elevation/13/IMG/USGS_NED_13_n42w071_IMG.zip
- https://prd-tnm.s3.amazonaws.com/StagedProducts/Elevation/1/IMG/n35w119.zip

Once you download data, create a "data" folder in the repo. Unzip the downloads and copy in the .img file.

## OpenCL
OpenCL is used in this project because the genetic algorithm benefits from parallel processing on the GPU. If your system does not have a dedicated GPU (or at least a decent integrated one), running the algorithm on a CPU will most likely be very slow, and is not recommended.

## Reference
This algorithm is an OpenCL implementation the paper by Casey J. Handmer. This paper can be found at https://arxiv.org/pdf/1503.01524.pdf
