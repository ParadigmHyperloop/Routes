# Routes
![Build status](https://travis-ci.org/ParadigmHyperloop/Routes.svg?branch=master "Build status (passing I hope)")

## Documentation
Full C++ documentation can be found at https://paradigmhyperloop.github.io/Routes/index.html

## Purpose
This project is intended to calculate realistic potential hyperloop routes. Realistic constraints are applied such as change in elevation of the track, cost of tunneling, pylon cost, change in speed around curves, etc.

## The Algorithm
Routes are internally represented by n-degree bezier curves, where n is determined at runtime based on the length of the route being calculated. A GPU based cost function is used to determine how "good" a particular bezier curve is at being a hyperloop route, given the current terrain. The CMA-ES algorithm (Covariance Matrix Adaptation Evolution Strategy) is then applied to find the minima of this cost function. This involves randomly sampling bezier curves, ranking them based on their cost, adjusting the distribution based on the best solutions and sampling again. This process constitutes a single generation and is then repeated over and over until the minima is located.

## Screenshots

![Input](https://i.imgur.com/N6k5ifE.png "Input")
![Stats](https://i.imgur.com/qdtndiu.png "Stats")
![Route](https://i.imgur.com/AfYYfJU.jpg "Route")

## Dependencies

- Geospatial Data Abstraction Library (GDAL) [ http://www.gdal.org]

- Boost  [http://www.boost.org]
- OpenGL Mathematics (GLM) [https://glm.g-truc.net/0.9.8/index.html]
- OpenCL [https://www.khronos.org/opencl/]
- Eigen [http://eigen.tuxfamily.org/]

## Setup Instructions

### Linux / macOS
If you are on Linux, download the OpenCL SDK from https://software.intel.com/en-us/intel-opencl. Also ensure your graphics card drivers are installed and up to date.

Run these commands in the directory of your choice
```
git clone https://github.com/ParadigmHyperloop/Routes.git
cd Routes
sudo ./setup.sh
```
`sudo` is required for this process because GDAL will be installed such that it can be used from the command line. All other dependencies will automatically be downloaded, compiled and installed. When the script is finished, there should now be a lib directory with several statically compiled libraries and an include directory with header files.

Run these commands to build the Routes project after `setup.sh` is complete:
```
mkdir build
cd build
cmake ..
make
```
The build directory will now contain built binaries, `Routes-Exec`, `Routes-Server` and `Routes-Tests`. These are all various extensions of the `Routes` static library that is also found in the build folder.

### Windows
Note: We had some issues getting the project to compile under Windows with the newest version of CMake. CMake 3.8.2 has been confirmed to work.

Make sure Visual Studio 2017 for C++ development is installed. The Community edition is free and will suffice.
1. Open up the Visual Studio 2017 Developer Command Prompt. If you search "Developer" in Cortana it should be one of the results.
2. Download and install curl from http://www.confusedbycode.com/curl/. Ensure that you select the option to add it to your `PATH` during installation.
3. Make sure that the Intel OpenCL SDK is installed from https://software.intel.com/en-us/intel-opencl
4. Run these commands from the developer command prompt:
```
cd C:/Users/<Your username>/Desktop/
git clone https://github.com/ParadigmHyperloop/Routes.git
cd Routes
setup.bat
```
 All other dependencies will automatically be downloaded, compiled and installed. When the script is finished, there should now be a lib directory with several statically compiled libraries and an include directory with header files.

 Once the setup has finished, run these commands to build the Routes project:
```
mkdir build
cd build
cmake -G "NMake Makefiles" ..
nmake
```
The build directory will now contain built binaries, `Routes-Exec`, `Routes-Server` and `Routes-Tests`. These are all various extensions of the `Routes` static library that is also found in the build folder.

## Running
The Routes-Exec executable takes in the start and destination for the route from the command line. This should be run in the build directory Here is an example:
```
./Routes-Exec --start=-119.001666666700030,35.001666666664143 --dest=-118.13173828125,34.08877925439021
```
Windows:
```
Routes-Exec.exe --start=-119.001666666700030,35.001666666664143 --dest=-118.13173828125,34.08877925439021
```

This will calculate a route from -119.001666666700030°W 35.001666666664143°N to 118.13173828125°W 34.08877925439021°N. Elevation for both the start and the destination will be automatically sampled. The format of the arguments is:
```
--[start or dest]=lon,lat
```
where lon is the desired longitude, lat is the desired latitude.

## Server
For web-based computation of routes, Routes-Server provides a simple REST API. The server must be run out of the build directory
First make sure that the database has been build by running:
```
./Routes-Exec --rebuild
```
Windows:
```
Routes-Exec.exe --rebuild
```

Then run the REST server with:
```
./Routes-Server
```
Windows:
```
Routes-Server.exe
```

This will start the server on the port 8080 and provides several API calls.

To queue the computation of a route, use the request:
```
GET http://localhost:8080/compute?start=lat,long&dest=lat,long
```
Where lon is the desired longitude, lat is the desired latitude. This request will return a number, which should be saved by your client. To retrieve the route call
```
GET http://localhost:8080/retrieve?id=unique
```
Where unique is the number that was returned from the request to queue the computation. This will return one of two things. It will either return "false", which indicates that the route is still in the queue or is still computing, or it will return JSON with all of the information that was calculated about the route including the control points for the bezier curve and a collection of points on the curve.

Since we use OpenCL textures to represent the elevation data on the GPU, we are limited by the max texture size allowed by the GPU hardware. In order to get that limit, use the following request:
```
GET http://localhost:8080/max-route-length
````
The resultant length is in meters.

By editing the url field htdocs/scripts/config.js to `http://localhost` you can ensure that when using the website it will point to your own machine.

## Data
In order to run the algorithm, 1 Arc Second elevation data is required from the USGS. You can find all USGS data at https://viewer.nationalmap.gov/basic/

Once you download data, create a "data" folder inside of the cloned repository. Unzip any downloads and copy in the .img file to the data folder. After you have all of the data in the data folder, you'll need to build the database. To do this run:
```
cd build
./Routes-Exec --rebuild
```
Windows:
```
cd build
Routes-Exec.exe --rebuild
```

## Tests
In order to run the tests, switch to the build directory and run:
```
make check
```
In order to run the tests, the n35w119 elevation data must be downloaded from the USGS. After this is done, the instructions above to build the database must be run.

## OpenCL
Because the algorithm deals with large population sizes, it benefits from high levels of parallelism. We use the compute horsepower of your system's GPU to evaluate the cost function for hundreds of curves simultaneously. The OpenCL development SDK is required to compile the program, and up-to-date graphics drivers are required to run the built products. If your system does not have a dedicated GPU (or at least a decent integrated one), running the program on a CPU will most likely be very slow, and is not recommended.

## Reference
- https://en.wikipedia.org/wiki/CMA-ES
- https://www.lri.fr/~hansen/gecco2011-CMA-ES-tutorial.pdf
