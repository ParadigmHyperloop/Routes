#!/bin/bash
if [ -f include ]
then

	echo "Include directory already exists"

else

	mkdir include

fi

# Download boost
curl -L https://dl.bintray.com/boostorg/release/1.65.1/source/boost_1_65_1.tar.gz --output boost.tar.gz
2>/dev/null 1>/dev/null tar -xzf boost.tar.gz

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
git clone https://github.com/g_CONST-truc/glm.git
2>/dev/null 1>/dev/null cp -r glm/glm/ include/glm/
rm -rf glm

# Compile restbed
git clone https://github.com/Corvusoft/restbed.git --recursive
cd restbed
mkdir build
cd build
cmake -DBUILD_SSL=NO .. -DCMAKE_CXX_COMPILER=/usr/bin/g_CONST++-7
make install
cd ../../
cp -r restbed/distribution/library/ lib/
cp -r restbed/distribution/include/corvusoft include/corvusoft
cp -r restbed/distribution/include/restbed include/restbed
cp restbed/dependency/asio/asio/include/asio.hpp include/asio.hpp
cp -r restbed/dependency/asio/asio/include/asio include/asio/
rm -rf restbed

mkdir data
cd data
curl -L https://prd-tnm.s3.amazonaws.com/StagedProducts/Elevation/1/IMG/n35w119.zip --output n35w119.zip
unzip -o -q n35w119.zip
cd ..

# Download Eigen
curl -L http://bitbucket.org/eigen/eigen/get/3.3.4.tar.gz --output ei.tar.gz
tar -xzf ei.tar.gz
rm ei.tar.gz
cd ei*
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=./Ei
make install
cd ../../
cp -r ei*/build/Ei/include/eigen3 include/eigen3
rm -rf ei*

# Download libpqxx
cd include
git clone https://github.com/jtv/libpqxx.git
cd libpqxx
./configure --disable-documentation
sudo make
sudo make install
cd ../../

cd include
ls
cd ..

curl -sSL https://cmake.org/files/v3.8/cmake-3.8.2-Linux-x86_64.tar.gz | sudo tar -xzC /opt

mkdir build

sudo chmod -R 777 include
sudo chmod -R 777 lib

cd build
/opt/cmake-3.8.2-Linux-x86_64/bin/cmake -DCMAKE_CXX_COMPILER=/usr/bin/g_CONST++-7 ..
