#!/bin/bash

if [ -f include ]
then

	echo "Include directory already exists"

else

	mkdir include

fi

# Download boost
curl -L https://dl.bintray.com/boostorg/release/1.65.1/source/boost_1_65_1.tar.gz --output boost.tar.gz
tar -xvzf boost.tar.gz

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
cp -r glm/glm/ include/glm/
rm -rf glm