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

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <exception>
#include <iostream>
#include <string>
#include <curl/curl.h>
#include <portable_endian.h>
#include "Client.h"
#include "Credentials.h"
#include "Server.h"

static void handleTerminate() {
  try {
    std::rethrow_exception(std::current_exception());
  } catch (std::exception& e) {
    std::cerr << "Exception thrown:\n"
      << e.what() << '\n';
  } catch (const char* c) {
    std::cerr << c << '\n';
    exit(-1);
  } catch (std::string& c) {
    std::cerr << c << '\n';
    exit(-1);
  } catch (...) {
    std::cerr << "something weird happened\n";
    exit(-1);
  }
  exit(-2);
}

int lmain(int argc, char** argv) {
  setlocale(LC_ALL, "");
  std::set_terminate(handleTerminate);
  CLineConfig c;
  int res = readSettings(c, argc, argv);
  if (res != 0) return res;
  std::cout << "Hello from Athena V.\n";
  if (c.mode == CLIENT) {
    curl_global_init(CURL_GLOBAL_ALL);
    std::cout << "You intend to connect to a server.\n";
    std::string username, password;
    if (c.username.length() != 0) username = c.username;
    else {
      std::cout << "Username: ";
      std::cin >> username;
    }
    if (c.password.length() != 0) password = c.password;
    else {
    std::cout << "Password: ";
      std::cin >> password;
    }
    Credentials cred(username, password);
    Client client(c.ip, c.port, c.useIPV6);
    client.login(cred);
    client.getListenThread().join();
    curl_global_cleanup();
  } else {
    std::cout << "You intend to start a server.\n";
    Server server(c.port, DEFAULT_MAX_CONNECTIONS, c.useIPV6);
  }
  return 0;
}

const char* x801::game::USAGE =
  "Usage:\n"
  "  Game [-6] [-d] --client <address> <port>\n"
  "  Game [-6] --server <port>\n"
  "  -6 (--use-ipv6): use IPV6\n"
  "  --client <address> <port> (-c): start a client\n"
  "  --server <port> (-s): start a server\n"
  "  -d (--debug): for clients, enable OpenGL live debugging\n"
  "    (requires OpenGL 4.5 or later)\n"
  "  --username <username>: pass in username from command line\n"
  "  --password <password>: pass in password from command line\n"
  "    The above two options are intended for compatibility with\n"
  "    graphical debuggers such as bugle and are not recommended\n"
  "    for actually playing the game.\n"
  ;

int x801::game::readSettings(CLineConfig& cn, int argc, char** argv) {
  bool ok = true;
  ClientOrServer mode = HUH;
  for (int i = 1; i < argc && ok; ++i) {
    char* arg = argv[i];
    if (mode != HUH) {
      cn.mode = mode;
      if (mode == CLIENT) {
        // get an IP address and a port
        cn.ip = argv[i];
        if (i + 1 >= argc) ok = false;
        else {
          int port = atoi(argv[++i]); // Come on. SLAP ME.
          if (port == 0) ok = false;
          else cn.port = (uint16_t) port;
        }
      } else if (mode == SERVER) {
        int port = atoi(argv[i]);
        if (port == 0) ok = false;
        else cn.port = (uint16_t) port;
      }
      mode = HUH;
    } else if (arg[0] == '-') {
      if (arg[1] == '-') {
        const char* name = arg + 2;
        if (!strcmp(name, "client") || !strcmp(name, "cheryl")) mode = CLIENT;
        else if (!strcmp(name, "server") || !strcmp(name, "samantha")) mode = SERVER;
        else if (!strcmp(name, "use-ipv6")) cn.useIPV6 = true;
        else if (!strcmp(name, "debug")) cn.debug = true;
        else if (!strcmp(name, "username")) {
          if (i == argc - 1) ok = false;
          else {
            cn.username = argv[i + 1];
            ++i;
          }
        }
        else if (!strcmp(name, "password")) {
          if (i == argc - 1) ok = false;
          else {
            cn.password = argv[i + 1];
            ++i;
          }
        }
        else ok = false;
      } else {
        for (int j = 1; arg[j] != '\0'; ++j) {
          if (mode != HUH) ok = false;
          switch (arg[j]) {
          case 'c':
            mode = CLIENT; break;
          case 's':
            mode = SERVER; break;
          case '6':
            cn.useIPV6 = true; break;
          case 'd':
            cn.debug = true; break;
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
    std::cerr << USAGE;
    return -1;
  }
  return 0;
}
