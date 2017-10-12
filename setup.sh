#!/bin/bash

if [ -f /usr/local/lib/libgdal.a ] 
then

	echo "Found GDAL installed"
else

	git clone https://github.com/OSGeo/gdal.git
	cd gdal/gdal
	./configure
	sudo make
	sudo make install
	cd ../..
	rm -rf gdal

fi

if [ -f ./include ]
then

	echo "Include directory already exists"

else

	mkdir include

fi

git clone https://github.com/boostorg/compute.git
cp -r ./compute/include/boost ./include/boost
rm -rf compute