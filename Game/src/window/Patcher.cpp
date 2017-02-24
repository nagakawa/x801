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

#include <sstream>
#include <string>
#include <boost/filesystem.hpp>
#include <RakNetTypes.h>
#include "Database.h"

using namespace x801::game;

const char* x801::game::PATCHER_DIR = "gamedat/";

x801::game::Patcher::Patcher(Client& cli) {
  if (!boost::filesystem::is_directory(PATCHER_DIR)) {
    std::cout <<
      "Warning: overwriting " << PATCHER_DIR <<
      " because it is not a directory\n";
    boost::filesystem::remove(PATCHER_DIR);
    boost::filesystem::create_directories(PATCHER_DIR);
  }
  RakNet::SystemAddress addr;
  bool stat = cli.getServerAddress(addr);
  char aname[64];
  addr.ToString(true, aname, '_');
  if (!stat) throw "Address of connected server unknown";
  std::stringstream dfnamein;
  dfnamein << PATCHER_DIR << "gd_" << aname << ".dat";
  // This is to be a filename as such:
  // gd_127.0.0.1_9001.dat
  std::string dfname = dfnamein.str();
  open(conn, dfname.c_str());
  createFileTable();
  c = &cli;
}

x801::game::Patcher::~Patcher() {
  sqlite3_close(conn);
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

void x801::game::Patcher::createFileTable() {
  char* errMessage;
  int stat = sqlite3_exec(
    conn,
    CREATE_FILE_TABLE_QUERY,
    nullptr, nullptr, // don't callback
    &errMessage
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
  if (contentLength == 0) updateFileVersion(fname, version);
  else createFileEntry(fname, version, contentLength, contents);
}