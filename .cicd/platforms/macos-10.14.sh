#!/bin/bash
brew update
brew install git boost@1.70 cmake python@2 python libtool libusb graphviz automake wget gmp llvm@4 pkgconfig doxygen openssl

if [[ $PINNED != false || $UNPINNED != true ]]; then
    git clone --single-branch --branch release_80 https://git.llvm.org/git/llvm.git clang8
    cd clang8
    git checkout 18e41dc
    cd tools
    git clone --single-branch --branch release_80 https://git.llvm.org/git/lld.git
    cd lld
    git checkout d60a035
    cd ../
    git clone --single-branch --branch release_80 https://git.llvm.org/git/polly.git
    cd polly
    git checkout 1bc06e5
    cd ../
    git clone --single-branch --branch release_80 https://git.llvm.org/git/clang.git clang
    cd clang
    git checkout a03da8b
    cd tools
    mkdir extra
    cd extra
    git clone --single-branch --branch release_80 https://git.llvm.org/git/clang-tools-extra.git
    cd clang-tools-extra
    git checkout 6b34834
    cd ../../../../../projects/
    git clone --single-branch --branch release_80 https://git.llvm.org/git/libcxx.git
    cd libcxx
    git checkout 1853712
    cd ../
    git clone --single-branch --branch release_80 https://git.llvm.org/git/libcxxabi.git
    cd libcxxabi
    git checkout d7338a4
    cd ../
    git clone --single-branch --branch release_80 https://git.llvm.org/git/libunwind.git
    cd libunwind
    git checkout 57f6739
    cd ../
    git clone --single-branch --branch release_80 https://git.llvm.org/git/compiler-rt.git
    cd compiler-rt
    git checkout 5bc7979
    mkdir ../../build
    cd ../../build
    cmake -G 'Unix Makefiles' -DCMAKE_INSTALL_PREFIX='/usr/local' -DLLVM_BUILD_EXTERNAL_COMPILER_RT=ON -DLLVM_BUILD_LLVM_DYLIB=ON -DLLVM_ENABLE_LIBCXX=ON -DLLVM_ENABLE_RTTI=ON -DLLVM_INCLUDE_DOCS=OFF -DLLVM_OPTIMIZED_TABLEGEN=ON -DLLVM_TARGETS_TO_BUILD=X86 -DCMAKE_BUILD_TYPE=Release ..
    make -j $(getconf _NPROCESSORS_ONLN)
    sudo make install
    cd ../..
    rm -rf clang8
fi

cd /tmp
curl -OL https://fastdl.mongodb.org/osx/mongodb-osx-ssl-x86_64-3.6.3.tgz
tar -xzf mongodb-osx-ssl-x86_64-3.6.3.tgz
mkdir -p ~/mongodb
touch ~/mongodb/mongod.log
rm -f mongodb-osx-ssl-x86_64-3.6.3.tgz
mkdir -p ~/mongodb
ln -s ~/mongodb-osx-x86_64-3.6.3 ~/mongodb
ln -s ~/mongodb/bin/mongod /usr/local/bin/mongod

cd /tmp
curl -LO https://github.com/mongodb/mongo-c-driver/releases/download/1.13.0/mongo-c-driver-1.13.0.tar.gz
tar -xzf mongo-c-driver-1.13.0.tar.gz
cd mongo-c-driver-1.13.0
mkdir -p cmake-build
cd cmake-build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX='/usr/local' -DENABLE_BSON=ON -DENABLE_SSL=DARWIN -DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF -DENABLE_STATIC=ON -DENABLE_ICU=OFF -DENABLE_SASL=OFF -DENABLE_SNAPPY=OFF ..
make -j $(getconf _NPROCESSORS_ONLN)
sudo make install
cd ../..
rm mongo-c-driver-1.13.0.tar.gz

cd /tmp
curl -L https://github.com/mongodb/mongo-cxx-driver/archive/r3.4.0.tar.gz -o mongo-cxx-driver-r3.4.0.tar.gz
tar -xzf mongo-cxx-driver-r3.4.0.tar.gz
cd mongo-cxx-driver-r3.4.0/build
cmake -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX='/usr/local' ..
make -j $(getconf _NPROCESSORS_ONLN) VERBOSE=1
sudo make install
cd ../..
rm -f mongo-cxx-driver-r3.4.0.tar.gz
