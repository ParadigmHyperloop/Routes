#!/bin/bash
git clone https://github.com/OSGeo/gdal.git
cd gdal/gdal
./configure
make
sudo make install
cd ../..
rm gdal
