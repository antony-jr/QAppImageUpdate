# Run it like so,
# docker run -v $PWD:/docker-share -it ubuntu:xenial bash scripts/docker_build.sh 
# To get libQAppImageUpdate.so Qt Plugin.

# Install dependencies
apt update 
apt upgrade -y 
apt install software-properties-common build-essential libarchive13 libarchive-dev wget expat -y
add-apt-repository ppa:beineri/opt-qt563-xenial -y
apt-get update -qq
apt install libgl1-mesa-dev xvfb qt56base -y

cd /docker-share/
rm -rf build
mkdir build 
cd build

# Build OpenSSL Static version
wget "https://www.openssl.org/source/openssl-1.1.1j.tar.gz"
tar -xf openssl-1.1.1j.tar.gz
cd openssl-1.1.1j
./config no-shared --prefix=/usr/ --openssldir=/usr/
make -j$(nproc)
make install -j$(nproc)
cd ..

# Install CURL
wget "https://curl.se/download/curl-7.75.0.tar.gz"
tar -xf curl-7.75.0.tar.gz 
cd curl-7.75.0
,/configure
make -j$(nproc)
make install -j$(nproc)
cd ..

# Install CMake 3.19.6
wget "https://github.com/Kitware/CMake/releases/download/v3.19.6/cmake-3.19.6.tar.gz"
tar -xf cmake-3.19.6.tar.gz
cd cmake-3.19.6
./bootstrap
make -j$(nproc)
make install -j$(nproc)
cd ..

# Install Boost libraries
wget "https://dl.bintray.com/boostorg/release/1.75.0/source/boost_1_75_0.tar.gz"
tar -xf boost_1_75_0.tar.gz
cd boost_1_75_0
./bootstrap.sh
cp b2 /usr/bin/
echo "using gcc ;" > ~/user-config.jam
export BOOST_ROOT=$PWD
export BOOST_BUILD_PATH=$PWD/tools/build
BOOST_ROOT=$PWD BOOST_BUILD_PATH=$PWD/tools/build b2 cxxflags="-std=c++14" variant=release link=static install -j$(nproc) --with-system --with-chrono --with-random > /dev/null
cd ..

# Install Torrent Rasterbar
wget "https://github.com/arvidn/libtorrent/releases/download/libtorrent-1.2.8/libtorrent-rasterbar-1.2.8.tar.gz"
tar -xvf libtorrent-rasterbar-1.2.8.tar.gz
cd libtorrent-rasterbar-1.2.8
cmake -DBUILD_SHARED_LIBS=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON .
make -j$(nproc)
make install -j$(nproc)
cd ..

# Now Build QAppImageUpdate Qt Plugin.
source /opt/qt56/bin/qt56-env.sh 
cmake -DDECENTRALIZED_UPDATE_ENABLED=ON -DBUILD_AS_PLUGIN=ON ..
make -j$(nproc)

cp libQAppImageUpdate.* ..
cd ..
rm -rf build
