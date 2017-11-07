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
This will clone GDAL, compile it, and install it. It will also compile restbed and add it to the lib folder. This will take a while and is also why `./setup.sh` requires sudo. It will also download boost 1.6.5 and glm, copying all of the headers into the include directory.


After setup finishes, run these commands to actually build the Routes project.
```
mkdir build
cd build
cmake ..
make
```
You should now see a built library and a set of binaries in the build directory.

### Windows
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

You should now see a built library and a set of binaries in the build directory.
```

You should now see a built binary in the build directory. In order to run the program, open back up MSYS and run it from there.

## Running
The Routes-Exec executable takes in the start and destination for the route from the command line. Here is an example:
```
./Routes-Exec --start=-119.001666666700030,35.001666666664143,550.0
              --dest=-118.13173828125,34.08877925439021,145.0
```
This will calculate a route from -119.001666666700030°W 35.001666666664143°N 550.0m above sea level to 118.13173828125°W 34.08877925439021°N 145.0m above sea level. The format of the arguments is --[start or dest]=lon,lat,h where lon is the desired longitude, lat is the desired latitude and h is the distance in meters above sea level.

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
http://localhost:8080/compute?start=lat,long,h&dest=lat,long,h
```
Where lon is the desired longitude, lat is the desired latitude and h is the distance in meters above sea level. This request will return a number, which should be saved by your client. To retrieve the route call
```
http://localhost:8080/retrieve?id=unique
```
Where unique is the number that was returned from the call to compute. This will return one of two things. It will either return "false", which indicates that the route is still in the queue or is still computing, or it will return a JSON string with the control points of the route. After the JSON is returned the server will discard the computed route and calling it again.

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
This algorithm is an OpenCL implementation the paper by Casey J. Handmer. This paper can be found at https://arxiv.org/pdf/1503.01524.pdf
