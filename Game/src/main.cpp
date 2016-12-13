#include "main.h"

/*
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
*/

using namespace x801::game;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <portable_endian.h>

int main(int argc, char** argv) {
  CLineConfig c;
  int res = readSettings(c, argc, argv);
  if (res != 0) return res;
  std::cout << "Hello from Athena V.\n";
  if (c.mode == CLIENT) {
    std::cout << "You intend to connect to a server.\n";
  } else {
    std::cout << "You intend to start a server.\n";
  }
  return 0;
}

const char* x801::game::USAGE =
  "Usage:\n"
  "  Game --client <address> <port>\n"
  "  Game --server <port>\n";

int x801::game::readSettings(CLineConfig& cn, int argc, char** argv) {
  bool ok = true;
  ClientOrServer mode = HUH;
  for (int i = 1; i < argc && ok; ++i) {
    char* arg = argv[i];
    if (mode != HUH) {
      cn.mode = mode;
      if (mode == CLIENT) {
        // get an IP address and a port
        int a, b, c, d;
        int received = sscanf(arg, "%d.%d.%d.%d", &a, &b, &c, &d);
        if (received != 4 || i == argc - 1) ok = false;
        else {
          uint32_t ip = (a << 24) | (b << 16) | (c << 8) | d;
          int port = atoi(argv[++i]); // Come on. SLAP ME.
          if (port == 0) ok = false;
          else {
            cn.ip = htobe32(ip);
            cn.port = (uint16_t) port;
          }
        }
      } else if (mode == SERVER) {
        int port = atoi(argv[i]);
        if (port == 0) ok = false;
        else cn.port = (uint16_t) port;
      }
      mode = HUH;
    } else if (arg[0] == '-') {
      if (arg[1] == '-') {
        if (!strcmp(arg + 2, "client")) mode = CLIENT;
        else if (!strcmp(arg + 2, "server")) mode = SERVER;
        else ok = false;
      } else {
        for (int j = 1; arg[j] != '\0'; ++j) {
          if (mode != HUH) ok = false;
          switch (arg[j]) {
          case 'c':
            mode = CLIENT; break;
          case 's':
            mode = SERVER; break;
          default:
            ok = false;
          }
          if (!ok) break;
        }
      }
    }
    else ok = false;
  }
  if (!ok || cn.mode == HUH) {
    fputs(USAGE, stderr);
    return -1;
  }
  return 0;
}
