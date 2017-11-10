#!/bin/bash
if [ -f include ]
then

	echo "Include directory already exists"

else

	mkdir include
	sudo chmod 777 include

fi

# Download boost
curl -L https://dl.bintray.com/boostorg/release/1.65.1/source/boost_1_65_1.tar.gz --output boost.tar.gz
2>/dev/null 1>/dev/null tar -xvzf boost.tar.gz

# Copy libraries
cp -r boost_1_65_1/boost/ include/boost/

# Compile boost filesystem and program options 
cd boost_1_65_1
./bootstrap.sh --with-libraries=program_options,filesystem
./b2 link=static
cp -r stage/lib ../
cd ..

rm -rf boost_1_65_1
rm -rf boost.tar.gz

# Download GLM
git clone https://github.com/g-truc/glm.git
2>/dev/null 1>/dev/null cp -r glm/glm/ include/glm/
rm -rf glm

# Compile restbed
git clone https://github.com/Corvusoft/restbed.git --recursive
cd restbed 
mkdir build
cd build
cmake -DBUILD_SSL=NO .. -DCMAKE_CXX_COMPILER=/usr/bin/g++-7 
make install 
cd ../../
cp -r restbed/distribution/library/ lib/
cp -r restbed/distribution/include/corvusoft include/corvusoft
cp -r restbed/distribution/include/restbed include/restbed
rm -rf restbed

curl -sSL https://cmake.org/files/v3.8/cmake-3.8.2-Linux-x86_64.tar.gz | sudo tar -xzC /opt

mkdir build
cd build 
/opt/cmake-3.8.2-Linux-x86_64/bin/cmake -DCMAKE_CXX_COMPILER=/usr/bin/g++-7 .. 


