# Routes
![Build status](https://travis-ci.org/ParadigmHyperloop/Routes.svg?branch=master "Build status (passing I hope)")

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
This will clone GDAL, compile it, and install it. It will also compile restbed and add it to the lib folder. This will take a while and is also why `./setup.sh` requires sudo. It will also download boost 1.6.5, restbed and glm, copying all of the headers into the include directory and compiling the necessary libraries.


After setup finishes, run these commands to actually build the Routes project.
```
mkdir build
cd build
cmake ..
make
```
You should now see a built library and a set of binaries in the build directory.

To run the tests:
```
make check
```

### Windows
Note: We had some issues getting the project to compile under Windows with the newest version of CMake. CMake 3.8.2 has been confirmed to work. 

Make sure Visual Studio 2017 is installed. The Community edition is free and will suffice. 
1. Open up the Visual Studio 2017 Developer Command Prompt. If you search "Developer" in Cortana it should be one of the results. 
2. Download and install curl from http://www.confusedbycode.com/curl/. Ensure that you select the option to add it to your PATH.
3. Make sure that the Intel OpenCL SDK is installed from https://software.intel.com/en-us/intel-opencl
4. Run these commands from the developer command prompt:
```
cd C:/Users/<Your username>/Desktop/
git clone https://github.com/ParadigmHyperloop/Routes.git
cd Routes
setup.bat
```
This will download, compile and install GDAL. Restbed will also be downloaded and compiled. Boost 1.6.5 and glm will be downloaded and added to the include directory. Once the setup is finished (it will take a long time) run these commands:
```
mkdir build
cd build
cmake -G "NMake Makefiles" ..
nmake
```
You should now see a built library and a set of binaries in the build directory.

## Running
The Routes-Exec executable takes in the start and destination for the route from the command line. Here is an example:
```
./Routes-Exec --start=-119.001666666700030,35.001666666664143
              --dest=-118.13173828125,34.08877925439021
```
This will calculate a route from -119.001666666700030°W 35.001666666664143°N above sea level to 118.13173828125°W 34.08877925439021°N above sea level. The format of the arguments is --[start or dest]=lon,lat where lon is the desired longitude, lat is the desired latitude.

## Server
For web-based computation of routes, Routes-Server provides a simple REST API.
First make sure that the database has been build by running
```
./Routes-Exec --rebuild
```
Then run the REST server with
```
./Routes-Server
```
This will start the server on the port 8080 and provides two API calls. The first is to queue the calculation of a new route. To do this make a GET request with the format
```
GET http://localhost:8080/compute?start=lat,long&dest=lat,long
```
Where lon is the desired longitude, lat is the desired latitude. This request will return a number, which should be saved by your client. To retrieve the route call
```
GET http://localhost:8080/retrieve?id=unique
```
Where unique is the number that was returned from the call to compute. This will return one of two things. It will either return "false", which indicates that the route is still in the queue or is still computing, or it will return a JSON string with the control points of the route. After the JSON is returned the server will discard the computed route and calling it again.
```
GET http://localhost:8080/max-route-length
````
Since we use OpenCL textures to represent the elevation data on the GPU, we are limited by the max texture size allowed by the GPU hardware. This will return the longest possible route in meters based on that limit.

## Data
Data is pulled from the USGS, and is 1 arc second 3D elevation products. You can find all USGS data at https://viewer.nationalmap.gov/basic/

Currently the algorithm is being tested on these pieces of data:
- https://prd-tnm.s3.amazonaws.com/StagedProducts/Elevation/13/IMG/USGS_NED_13_n42w071_IMG.zip
- https://prd-tnm.s3.amazonaws.com/StagedProducts/Elevation/1/IMG/n35w119.zip

Once you download data, create a "data" folder in the repo. Unzip the downloads and copy in the .img file. After you have all of the data in the data folder, you'll need to build the database. To do this run:
```
cd build
./Routes-Exec --rebuild
```

## OpenCL
OpenCL is used in this project because the genetic algorithm benefits from parallel processing on the GPU. If your system does not have a dedicated GPU (or at least a decent integrated one), running the algorithm on a CPU will most likely be very slow, and is not recommended.

## Reference
This algorithm is originally based off of an algorithm demonstrated by Casey J. Handmer. This paper can be found at https://arxiv.org/pdf/1503.01524.pdf
