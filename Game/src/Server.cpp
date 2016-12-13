#include "Server.h"

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

#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

void x801::game::Server::initialise() {
  peer = RakNet::RakPeerInterface::GetInstance();
  updateKeyFiles();
  RakNet::SocketDescriptor socket(port, 0);
  peer->Startup(maxConnections, &socket, 1);
  peer->SetMaximumIncomingConnections(maxConnections);
}

x801::game::Server::~Server() {
  if (publicKey != nullptr) delete[] publicKey;
  if (privateKey != nullptr) delete[] privateKey;
}

const char* x801::game::KEY_DIR = "keys";
const char* x801::game::KEY_PUBLIC = "keys/public.bin";
const char* x801::game::KEY_PRIVATE = "keys/private.bin";

void x801::game::Server::updateKeyFiles() {
  if (!boost::filesystem::exists(KEY_DIR) ||
      boost::filesystem::is_directory(KEY_DIR)) {
    std::cout <<
      "Warning: overwriting " << KEY_DIR <<
      " because it is not a directory\n";
    boost::filesystem::remove(KEY_DIR);
    boost::filesystem::create_directories(KEY_DIR);
  }
  int pubSize = cat::EasyHandshake::PUBLIC_KEY_BYTES;
  publicKey = new char[pubSize];
  int privSize = cat::EasyHandshake::PRIVATE_KEY_BYTES;
  privateKey = new char[privSize];
  bool ok = false;
  if (boost::filesystem::exists(KEY_PUBLIC) &&
      boost::filesystem::is_directory(KEY_PUBLIC)) {
    ok = true;
    boost::filesystem::ifstream input(
      KEY_PUBLIC,
      std::ios_base::in | std::ios_base::binary
    );
    int bytesRead = input.readsome(publicKey, pubSize);
    if (bytesRead < pubSize) ok = false;
  }
  if (boost::filesystem::exists(KEY_PRIVATE) &&
      boost::filesystem::is_directory(KEY_PRIVATE)) {
    ok = true;
    boost::filesystem::ifstream input(
      KEY_PRIVATE,
      std::ios_base::in | std::ios_base::binary
    );
    int bytesRead = input.readsome(privateKey, privSize);
    if (bytesRead < privSize) ok = false;
  } else {
    ok = false;
  }
  if (!ok) {
    std::cout <<
      "Key files missing or corrupt; [re]generating them\n";
    cat::EasyHandshake handshake;
    handshake.GenerateServerKey(publicKey, privateKey);
    boost::filesystem::remove_all(KEY_PUBLIC);
    boost::filesystem::ofstream output1(
      KEY_PUBLIC,
      std::ios_base::out | std::ios_base::binary
    );
    output1.write(publicKey, pubSize);
    boost::filesystem::remove_all(KEY_PRIVATE);
    boost::filesystem::ofstream output2(
      KEY_PRIVATE,
      std::ios_base::out | std::ios_base::binary
    );
    output2.write(privateKey, privSize);
  }
  peer->InitializeSecurity(publicKey, privateKey, false);
}