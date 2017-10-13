# Routes

## Dependencies
Dependencies are automatically downloaded and installed with setup.sh.

- Geospatial Data Abstraction Library (GDAL)  http://www.gdal.org
- Boost  http://www.boost.org

## Setup Instructions
Currently we have no automated build script for Windows, but looking in setup.sh should be fairly self explanatory.

Run these commands
```
git clone https://github.com/BlueSpud/Routes
cd Routes
sudo ./setup.sh
```
This will clone GDAL, compile it, and install it. This will take a while and is also why `./setup.sh` requires sudo. It will also download bosot 1.6.5 and copy all of the headers into the include directory.


After setup finishes, run these commands to actually build the Routes project
```
mkdir build
cd build
cmake ..
make
```
You should now see a built binary in the build directory.
