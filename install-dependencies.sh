#!/bin/bash

# install portaudio

git clone --recursive git://git.assembla.com/portaudio.git
cd portaudio
./configure
make
make install
cd ..