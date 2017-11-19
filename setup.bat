: Build GDAL
git clone https://github.com/OSGeo/gdal.git
cd gdal/gdal
nmake /f makefile.vc devinstall MSVC_VER=1900
cd ../../

echo cleaning up gdal...
RMDIR /S /Q gdal

: Download boost 
curl -L http://dl.bintray.com/boostorg/release/1.65.1/source/boost_1_65_1.zip --output boost.zip
curl -L http://www.stahlworks.com/dev/unzip.exe --output unzip.exe
unzip.exe boost.zip

: Compile boost
cd boost_1_65_1
call bootstrap.bat
bjam variant=debug threading=single link=static --with-filesystem --with-program_options --with-system --with-chrono --with-test

: Copy Boost
cd ..

set source="boost_1_65_1\boost"
set destination="include\boost\"
xcopy %source% %destination% /y /s

set source="boost_1_65_1\stage\lib"
set destination="lib\"
xcopy %source% %destination% /y /s

: Delete boost 
echo cleaning up boost...
RMDIR /S /Q boost_1_65_1
DEL /S /Q boost.zip

# Remove libraries that get built and we dont need
DEL /S /Q lib\*boost_prg_exec_monitor*
DEL /S /Q lib\*boost_test_exec_monitor*
DEL /S /Q lib\*boost_timer*

: Grab GLM
git clone https://github.com/g-truc/glm.git
set source="glm\glm"
set destination="include\glm\"
xcopy %source% %destination% /y /s
RMDIR /S /Q glm

: Grab restbed
git clone https://github.com/Corvusoft/restbed.git --recursive
cd restbed 
mkdir build

: Patch CMakeLists.txt
Echo set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")>>"CMakeLists.txt"
mkdir build
cd build
cmake -G "NMake Makefiles" -DBUILD_SSL=NO .. 
nmake install
cd ../..

: Copy
set source="restbed\\distribution\include"
set destination="include\"
xcopy %source% %destination% /y /s

set source="restbed\\distribution\library"
set destination="lib\"
xcopy %source% %destination% /y /s

RMDIR /S /Q restbed

: Get Eigen
curl -L http://bitbucket.org/eigen/eigen/get/3.3.4.zip --output ei.zip
unzip.exe ei.zip
cd eigen-eigen-5a0156e40feb
mkdir build
cd build
cmake .. -G "NMake Makefiles" -DCMAKE_INSTALL_PREFIX=./Ei
nmake install
cd ../../

set source="eigen-eigen-5a0156e40feb\build\Ei\include\eigen3"
set destination="include\eigen3\"
xcopy %source% %destination% /y /s 
RMDIR /S /Q eigen-eigen-5a0156e40feb


DEL /S /Q unzip.exe
DEL /S /Q ei.zip