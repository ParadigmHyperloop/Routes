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

if [ -f include ]
then

	echo "Include directory already exists"

else

	mkdir include
	sudo chmod 777 include

fi

# Download boost
curl -L https://dl.bintray.com/boostorg/release/1.65.1/source/boost_1_65_1.tar.gz --output boost.tar.gz
tar -xvzf boost.tar.gz

# Copy libraries
cp -r boost_1_65_1/boost/ include/boost/

# Compile boost filesystem and program options 
cd boost_1_65_1
./bootstrap.sh --with-libraries=program_options,filesystem,test
./b2 link=static
cp -r stage/lib ../
cd ..

rm -rf boost_1_65_1
rm -rf boost.tar.gz

# Remove libraries that get built and we dont need
rm lib/*boost_prg_exec_monitor*
rm lib/*boost_test_exec_monitor*
rm lib/*boost_timer*

# Download GLM
git clone https://github.com/g-truc/glm.git
cp -r glm/glm/ include/glm/
rm -rf glm

# Compile restbed
git clone https://github.com/Corvusoft/restbed.git --recursive
cd restbed 
mkdir build
cd build
cmake -DBUILD_SSL=NO .. 
make install 
cd ../../
cp -r restbed/distribution/library/* lib/
cp -r restbed/distribution/include/* include/
rm -rf restbed

# Download Eigen
curl -L http://bitbucket.org/eigen/eigen/get/3.3.4.tar.gz --output ei.tar.gz
tar -xvzf ei.tar.gz
rm ei.tar.gz
cd ei*
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=./Ei
make install
cd ../../
cp -r ei*/build/Ei/include/eigen3 include/eigen3
rm -rf ei*

