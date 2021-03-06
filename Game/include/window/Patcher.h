#pragma once

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

#if !defined(__cplusplus) || __cplusplus < 201103L
#error Only C++11 or later supported.
#endif

#include <stdint.h>
#include <mutex>
#include <sstream>
#include <thread>
#include <curl/curl.h>
#include <sqlite3.h>
namespace x801 {
  namespace game {
    class Patcher;
  }
}
#include "ext/blockingconcurrentqueue.h"
#include "Client.h"

namespace x801 {
  namespace game {
    extern const char* PATCHER_DIR;
    class Patcher {
    public:
      Patcher(std::string u, std::string addressOfServer);
      ~Patcher();
      Patcher(const Patcher& other) = delete;
      Patcher& operator=(const Patcher& other) = delete;
      void updateEntry(
        const char* fname,
        uint32_t version,
        uint32_t contentLength,
        const uint8_t* contents);
      /*
        returns true if file is there; otherwise returns false
        `contents` must be freed with delete[]
      */
      bool getFileEntry(
        const char* fname,
        uint32_t& version,
        uint32_t& contentLength,
        uint8_t*& contents);
      void requestFile(const char* fname);
      void startFetchThread();
      void stopFetchThread() {
        done = true;
        if (fetchThread.joinable())
          fetchThread.join();
      }
      void fetchFile(const char* fname);
      std::stringstream getSStream(const char* fname);
    private:
      sqlite3* conn;
      uint32_t latestVersion = 0;
      std::string uri;
      CURL* curl; // TODO use multi interface
      std::thread fetchThread;
      volatile bool done = false;
      moodycamel::BlockingConcurrentQueue<std::string> files;
      void open(sqlite3*& handle, const char* path);
      void createFileTable();
      void createFileEntry(
        const char* fname,
        uint32_t version,
        uint32_t contentLength,
        const uint8_t* contents);
      void updateFileVersion(
        const char* fname,
        uint32_t version);
      bool refetchFile(const char* fname, uint32_t version);
      uint32_t getVersionFromServer(const char* fname);
      bool fetchIndex(std::stringstream& ss);
      bool updateAllFiles();
      std::mutex mutex;
    };
  }
}