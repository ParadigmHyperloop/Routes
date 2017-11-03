# Routes

## Documentation
Full C++ documentation can be found at https://paradigmhyperloop.github.io/Routes/index.html

## Purpose
This project is intended to calculate realistic possible hyperloop routes. We build a representation of the terrain that a hyperloop will be crossing with GIS elevation data. 

## The Algorithm
The algorithm itself is a genetic algorithm. This essentially simulates natural selection like organic life would undergo in nature. First it generates an initial population of routes from the desired start to the destination. It then ranks these paths based on a number of factors including how high the track is above the land, how much tunneling would be required, how curved the track is, etc.. The best individuals are then "bred" together to create a new generation of (hopefully) better solutions. This process is repeated for a desired number of iterations. Upon completion the most "fit" path will be given.

## Dependencies

- Geospatial Data Abstraction Library (GDAL)  http://www.gdal.org
- Boost  http://www.boost.org
- OpenGL Mathematics (GLM) https://glm.g-truc.net/0.9.8/index.html
- OpenCL https://www.khronos.org/opencl/

## Setup Instructions

### Linux / macOS
If you are on Linux, you need to download the OpenCL SDK from https://software.intel.com/en-us/intel-opencl. Also make sure your graphics card drivers are installed and up to date.


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
Unfortunately the setup for Windows is a bit more complicated We were able to successfully compile the project using MSYS. Other compilers should work but the CMake file is specifically built for using MSYS, at least for now, so if you do use another compiler you will have to change CMakeLists.txt.
1. Download and install MinGW. The easiest way to do this is to use the installer, which can be found at https://sourceforge.net/projects/mingw/files/Installer/. Make sure MinGW is installed directly onto C:\. After installing you should now have a folder C:\MinGW\
2. Make sure MSYS, gcc, g++ and binutils are installed. This can also be done in the installer fairly easily by checking the msys-base, mingw32-gcc, mingw32-gcc-g++ and mingw32-binutils, then going to Installation > Apply Changes
3. Find MSYS. It should be in C:\MinGW\msys\1.0 and its a batch file called MSYS. Open it.
4. Clone the repo into any folder on your C drive
5. Setup GDAL. GDAL has a page on how to compile it for MinGW / MSYS which can be found at https://trac.osgeo.org/gdal/wiki/BuildingWithMinGW. We had to replace the cpl_config.h with the one in the instructions under the Misc section on the tutorial to get it to compile.
6. Install the Intel OpenCL SDK from https://software.intel.com/en-us/intel-opencl. You'll also need to make sure your graphics drivers are up to date.
7. Install curl if you don't already have it. A great installer can be found at http://www.confusedbycode.com/curl/
8. Type in these commands:
```
cd /c/path/to/repo/Routes/
./setup-win.sh
```
6. After setup finishes, run these commands to actually build the Routes project. Make sure this is done in MSYS.
```
mkdir build
cd build
cmake -G "MSYS Makefiles" ..
make
```
You should now see a built binary in the build directory. In order to run the program, open back up MSYS and run it from there.

## Running
The Routes executable takes in the start and destination for the route from the command line. Here is an example:
```
./Routes --start=-119.001666666700030,35.001666666664143,550.0 
         --dest=-118.13173828125,34.08877925439021,145.0
```
This will calculate a route from -119.001666666700030°W 35.001666666664143°N 550.0m above sea level to 118.13173828125°W 34.08877925439021°N 145.0m above sea level. The format of the arguments is --[start or dest]=lon,lat,h where lon is the desired longitude, lat is the desired latitude and h is the distance in meters above sea level.
 

## Data
Data is pulled from the USGS, and is 1 arc second 3D elevation products. You can find all USGS data at https://viewer.nationalmap.gov/basic/

Currently the algorithm is being tested on these pieces of data:
- https://prd-tnm.s3.amazonaws.com/StagedProducts/Elevation/13/IMG/USGS_NED_13_n42w071_IMG.zip
- https://prd-tnm.s3.amazonaws.com/StagedProducts/Elevation/1/IMG/n35w119.zip

Once you download data, create a "data" folder in the repo. Unzip the downloads and copy in the .img file. After you have all of the data in the data folder, you'll need to build the database. To do this run:
```
cd build
./Routes --rebuild
```

## OpenCL
OpenCL is used in this project because the genetic algorithm benefits from parallel processing on the GPU. If your system does not have a dedicated GPU (or at least a decent integrated one), running the algorithm on a CPU will most likely be very slow, and is not recommended.

## Reference
This algorithm is an OpenCL implementation the paper by Casey J. Handmer. This paper can be found at https://arxiv.org/pdf/1503.01524.pdf
