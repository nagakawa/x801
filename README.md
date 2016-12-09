## Experiment801

A customizable MMORPG with Wizard101-like mechanics that anyone can modify or
run a server for.

### Building instructions

#### Note about compilers

If you are using g++, then at least use a new version. 7.0.0 is known to work,
but 5.4.0 will *not* compile the program.

Alternatively, use Clang, which will (as of 3.8.0) successfully compile the
program.

#### Getting dependencies

Right now the list of dependencies isn't really set, but the current CMake
files ask for the same libraries as TDR, plus ZLib. On Debian / Ubuntu,
simply run the following command:

    sudo apt install libglfw3-dev libglew-dev libsoil-dev libglm-dev \
      libcairo2-dev libpango1.0-dev libglib2.0-dev zlib1g-dev cmake

#### Using CMake

To generate the makefile:

    cmake . # debug (default)
    cmake -DCMAKE_BUILD_TYPE=Release . # release

And to actually build the project:

    make

To run tests:

    ./test # a shell script that runs make to build the test executable

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
