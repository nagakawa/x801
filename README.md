## Experiment801

[*Join our discord!*](https://discord.gg/sDbNH5N)

> Thousands of candles can be lit from a single candle, and the life of the
> candle will not be shortened. Happiness never decreases by being shared.
> ~ Wizard101

A customisable MMORPG with Wizard101-like mechanics that anyone can modify or
run a server for.

### Building instructions

At this moment, compiling will likely work only on POSIX-based systems.

#### Note about compilers

If you are using g++, then at least use a new version. 7.0.0 is known to work,
but 5.4.0 will *not* compile the program.

Alternatively, use Clang, which will (as of 3.8.0) successfully compile the
program.

#### Getting dependencies

Right now the list of dependencies isn't really set, but the current CMake
files ask for the same libraries as TDR, plus a few other libraries. On Debian
/ Ubuntu, simply run the following command:

    sudo apt install libglfw3-dev libglew-dev libsoil-dev libglm-dev \
      libcairo2-dev libpango1.0-dev libglib2.0-dev zlib1g-dev libboost-dev \
      libsqlite3-dev libboost-filesystem-dev libboost-random-dev \
      libasound2-dev libogg-dev libvorbis-dev cmake

and build PortAudio yourself.

*(Boost.Process is also needed, but since Ubuntu's repositories don't have
Boost 1.64.0 yet, it's prepackaged in the repo for now.)*

This project also depends on a custom version of RakNet, but this dependency
is managed by the configuration files so no extra work for you.

In addition, the Python scripts depend on a few packages.

    sudo apt install python3-pip
    pip3 install numpy simpleeval

In addition, there are programs used to build assets:

* MuseScore 2.0.2 or later. You can get the package in the official repositories,
  but if you want the most up-to-date version, there's also a
  [PPA](https://launchpad.net/~mscore-ubuntu/+archive/ubuntu/mscore-stable).
  Other distributions might have their own ways to get the program.
* FluidSynth (used to convert .mid files to .wav). The package name is,
  unsurprisingly, `fluidsynth`.
* XCFTools (`xcftools`).

There is a bug with older versions of Valgrind that will prevent it from running
some tests. Valgrind 3.12.0 should resolve the issue, but you'll probably have
to build it from source.

#### Using CMake

To generate the makefile:

    cmake . # debug (default)
    cmake -DCMAKE_BUILD_TYPE=Release . # release

And to actually build the project:

    make

To run tests:

    ./test # a shell script that runs make to build the test executable

To start a server:

    build/x801 -s 9001

To start a client:

    build/x801 -c localhost 9001

### Other Links

* [The One and Only Style Guide](https://docs.google.com/document/d/1AskEPaRCH0A6xCgIYerogpheiXyx4UT886UwIGPR-vU/edit?usp=sharing)

### Licence

Copyright (C) 2016 AGC.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
