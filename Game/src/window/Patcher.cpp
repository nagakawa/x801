#include "window/Patcher.h"

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

#include <assert.h>
#include <iostream>
#include <sstream>
#include <string>
#include <boost/filesystem.hpp>
#include <rapidjson/document.h>
#include <RakNetTypes.h>
#include "Database.h"

using namespace x801::game;

const char* x801::game::PATCHER_DIR = "gamedat/";

x801::game::Patcher::Patcher(std::string u) {
  if (!boost::filesystem::is_directory(PATCHER_DIR)) {
    std::cout <<
      "Warning: overwriting " << PATCHER_DIR <<
      " because it is not a directory\n";
    boost::filesystem::remove(PATCHER_DIR);
    boost::filesystem::create_directories(PATCHER_DIR);
  }
  curl = curl_easy_init();
  if (curl == nullptr) throw "Could not initialise CURL";
#ifndef NDEBUG
  curl_easy_setopt(curl, CURLOPT_VERBOSE, true);
#endif
  std::string urishadow = u;
  std::cout << urishadow.c_str() << '\n';
  size_t index = urishadow.find("://") + 3;
  urishadow[urishadow.find(':', index)] = '|';
  RakNet::SystemAddress addr;
  bool stat = addr.FromString(urishadow.c_str() + index, '|');
  char aname[64];
  addr.ToString(true, aname, '_');
  if (!stat) throw "Address of connected server unknown";
  // This is to be a filename as such:
  // gd_127.0.0.1_9001.dat
  std::string dfname = std::string(PATCHER_DIR) + "gd_" + aname + ".dat";
  open(conn, dfname.c_str());
  createFileTable();
  uri = u;
  // XXX this is blocking
  updateAllFiles();
}

x801::game::Patcher::~Patcher() {
  done = true;
  fetchThread.join();
  int stat = sqlite3_close(conn);
  curl_easy_cleanup(curl);
  conn = nullptr;
  (void) stat;
  assert(stat == SQLITE_OK);
}

void x801::game::Patcher::open(sqlite3*& handle, const char* path) {
  int status = sqlite3_open(path, &handle);
  if (handle == nullptr) throw "Can't open the database.";
  if (status != SQLITE_OK) throw sqlite3_errmsg(handle);
}

static const char* CREATE_FILE_TABLE_QUERY =
  "CREATE TABLE IF NOT EXISTS "
  "Files("
  "  fileID INTEGER PRIMARY KEY ASC,"
  "  fname STRING UNIQUE NOT NULL,"
  "  version INTEGER NOT NULL,"
  "  contents BLOB"
  ");"
  "CREATE INDEX IF NOT EXISTS "
  "  filesByName ON Files (fname);"
  ;

static const char* DEFAULT_ERROR = "patcher default error";

void x801::game::Patcher::createFileTable() {
  const char* errMessage = DEFAULT_ERROR;
  int stat = sqlite3_exec(
    conn,
    CREATE_FILE_TABLE_QUERY,
    nullptr, nullptr, // don't callback
    (char**) &errMessage
  );
  if (stat != SQLITE_OK) throw errMessage;
}

static const char* CREATE_FILE_ENTRY_QUERY =
  "INSERT OR REPLACE INTO Files (fname, version, contents) VALUES (?, ?, ?)"
  ;

void x801::game::Patcher::createFileEntry(
    const char* fname,
    uint32_t version,
    uint32_t contentLength,
    const uint8_t* contents) {
  sqlite3_stmt* statement;
  int stat = sqlite3_prepare_v2(
    conn,
    CREATE_FILE_ENTRY_QUERY, -1,
    &statement,
    nullptr
  );
  if (stat != SQLITE_OK) throw sqlite3_errmsg(conn);
  stat = sqlite3_bind_text(statement, 1, fname, -1, SQLITE_STATIC);
  if (stat != SQLITE_OK) throw sqlite3_errmsg(conn);
  stat = sqlite3_bind_int(statement, 2, version);
  if (stat != SQLITE_OK) throw sqlite3_errmsg(conn);
  stat = sqlite3_bind_blob(
    statement, 3, static_cast<const void*>(contents), contentLength,
    SQLITE_STATIC
  );
  if (stat != SQLITE_OK) throw sqlite3_errmsg(conn);
  stepBlock(statement, conn);
  sqlite3_finalize(statement);
}

static const char* UPDATE_FILE_VERSION_QUERY =
  "BEGIN TRANSACTION;"
  "INSERT OR IGNORE INTO Files"
  "  (fname, version, contents) VALUES (?1, ?2, NULL);"
  "UPDATE Files"
  "  SET version = ?2"
  "  WHERE fname = ?1;"
  "COMMIT;"
  ;

void x801::game::Patcher::updateFileVersion(
    const char* fname,
    uint32_t version) {
  sqlite3_stmt* statement;
  int stat = sqlite3_prepare_v2(
    conn,
    UPDATE_FILE_VERSION_QUERY, -1,
    &statement,
    nullptr
  );
  if (stat != SQLITE_OK) throw sqlite3_errmsg(conn);
  stat = sqlite3_bind_text(statement, 1, fname, -1, SQLITE_STATIC);
  if (stat != SQLITE_OK) throw sqlite3_errmsg(conn);
  stat = sqlite3_bind_int(statement, 2, version);
  if (stat != SQLITE_OK) throw sqlite3_errmsg(conn);
  stepBlock(statement, conn);
  sqlite3_finalize(statement);
}

static const char* FIND_FILE_BY_NAME_QUERY =
  "SELECT * from Files"
  "  WHERE fname = ?;"
  ;

bool x801::game::Patcher::getFileEntry(
    const char* fname,
    uint32_t& version,
    uint32_t& contentLength,
    uint8_t*& contents) {
  sqlite3_stmt* statement;
  int stat = sqlite3_prepare_v2(
    conn,
    FIND_FILE_BY_NAME_QUERY, -1,
    &statement,
    nullptr
  );
  if (stat != SQLITE_OK) throw sqlite3_errmsg(conn);
  stat = sqlite3_bind_text(statement, 1, fname, -1, SQLITE_STATIC);
  if (stat != SQLITE_OK) throw sqlite3_errmsg(conn);
  stat = stepBlock(statement, conn);
  if (stat != SQLITE_ROW) {
    sqlite3_finalize(statement);
    return false;
  }
  version = sqlite3_column_int(statement, 2);
  int size = sqlite3_column_bytes(statement, 3);
  contentLength = (uint32_t) size;
  uint8_t* bytes = new uint8_t[size];
  memcpy(bytes, sqlite3_column_blob(statement, 3), size);
  contents = bytes;
  sqlite3_finalize(statement);
  return true;
}

void x801::game::Patcher::updateEntry(
    const char* fname,
    uint32_t version,
    uint32_t contentLength,
    const uint8_t* contents) {
  if (fname == nullptr || contents == nullptr)
    throw "x801::game::Patcher::createFileEntry: "
      "fname and contents must not be null";
  if (version > latestVersion) latestVersion = version;
  if (contentLength == 0) updateFileVersion(fname, version);
  else createFileEntry(fname, version, contentLength, contents);
}

static size_t patcherWriteFunction(char *ptr, size_t size, size_t nmemb, void *userdata) {
  std::stringstream* output = (std::stringstream*) userdata;
  output->write(ptr, size * nmemb);
  return size * nmemb;
}

bool x801::game::Patcher::refetchFile(const char* fname, uint32_t version) {
  std::lock_guard<std::mutex> lock(mutex);
  std::stringstream ss;
  curl_easy_setopt(curl, CURLOPT_URL, (uri + "/content?fname=" + fname).c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, patcherWriteFunction);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*) &ss);
  CURLcode res = curl_easy_perform(curl);
  if (res != CURLE_OK) return false;
  long responseCode;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
  if (responseCode != 200) return false;
  std::string s = ss.str();
  createFileEntry(fname, version, s.length(), (const uint8_t*) s.c_str());
  return true;
}

uint32_t x801::game::Patcher::getVersionFromServer(const char* fname) {
  std::lock_guard<std::mutex> lock(mutex);
  std::stringstream ss;
  curl_easy_setopt(curl, CURLOPT_URL, (uri + "/version?fname=" + fname).c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, patcherWriteFunction);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*) &ss);
  CURLcode res = curl_easy_perform(curl);
  if (res != CURLE_OK) return -1;
  long responseCode;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
  if (responseCode != 200) return -1;
  //std::cerr << "*** File " << fname << " ***\n";
  //std::cerr << "Response code: " << responseCode << "\n";
  std::string s = ss.str();
  //std::cerr << "Contents: " << s << "\n";
  return std::stoul(s);
}

void x801::game::Patcher::fetchFile(const char* fname) {
  uint32_t lversion;
  uint32_t contentLength;
  uint8_t* contents;
  getFileEntry(fname, lversion, contentLength, contents);
  uint32_t sversion = getVersionFromServer(fname);
  if (sversion != lversion) refetchFile(fname, sversion);
}

void x801::game::Patcher::requestFile(const char* fname) {
  files.enqueue(std::string(fname));
}

void x801::game::Patcher::startFetchThread() {
  auto cback = [this]() {
    while (!this->done) {
      std::string fname;
      bool stat = files.wait_dequeue_timed(fname, 5000);
      if (stat) {
        std::cerr << "Fetching file " << fname << "!\n";
        fetchFile(fname.c_str());
        std::cerr << "Done fetching file " << fname << "!\n";
      }
    }
  };
  fetchThread = std::thread(cback);
}

bool x801::game::Patcher::fetchIndex(std::stringstream& ss) {
  std::lock_guard<std::mutex> lock(mutex);
  curl_easy_setopt(curl, CURLOPT_URL, (uri + "/v0tgil-sucks").c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, patcherWriteFunction);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*) &ss);
  CURLcode res = curl_easy_perform(curl);
  if (res != CURLE_OK) return false;
  long responseCode;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
  if (responseCode != 200) return false;
  return true;
}

bool x801::game::Patcher::updateAllFiles() {
  std::stringstream ss;
  bool stat = fetchIndex(ss);
  if (!stat) return false;
  std::string indexStr = ss.str();
  rapidjson::Document index;
  if (index.Parse(indexStr.c_str()).HasParseError())
    return false;
  if (!index.IsObject()) return false;
  rapidjson::Value::ConstMemberIterator begin = index.MemberBegin();
  rapidjson::Value::ConstMemberIterator end = index.MemberEnd();
  bool ok = true;
  for (auto it = begin; it != end; ++it) {
    auto fnameWithAssetsSlash = it->name.GetString();
    auto fname =
      (strncmp(fnameWithAssetsSlash, "assets/", 7) == 0) ?
      fnameWithAssetsSlash + 7 :
      fnameWithAssetsSlash;
    const rapidjson::Value& props = it->value;
    if (!props.IsObject()) {
      ok = false;
      continue;
    }
    auto verNode = props.FindMember("version");
    auto endNode = props.MemberEnd();
    if (verNode == endNode || !verNode->value.IsInt()) {
      ok = false;
      continue;
    }
    uint32_t version = verNode->value.GetInt();
    uint32_t oldVersion, oldLength;
    uint8_t* oldContents;
    bool fileExists = getFileEntry(fname, oldVersion, oldLength, oldContents);
    bool needsToUpdate = !fileExists || (oldVersion < version);
    if (needsToUpdate) {
#ifndef NDEBUG
      std::cerr << "Updating file " << fname << "\n";
#endif
      bool stat = refetchFile(fname, version);
      if (!stat) {
        std::cerr << "ERROR! Couldn't fetch file " << fname << '\n';
      }
    } else {
#ifndef NDEBUG
      std::cerr << fname << " doesn't need to update\n";
#endif
    }
    if (fileExists) delete[] oldContents;
  }
  return ok;
}