#!/bin/bash

# install packages necessary for running the imembench test systems
# potentially disruptive, use with caution!!

### Begin: for ramcloud
sudo apt-get install gcc-4.4 g++-4.4 build-essential git-core doxygen libpcre3-dev protobuf-compiler libprotobuf-dev libcrypto++-dev libevent-dev libboost-dev libboost-program-options-dev libboost-filesystem-dev libboost-system-dev libssl-dev libzookeeper-mt-dev

sudo ln -s /usr/lib/x86_64-linux-gnu/libzookeeper_mt.a  /usr/local/lib/libzookeeper_mt.a

sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.4 40
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-4.4 40
### End: for ramcloud
