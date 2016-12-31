#include "Database.h"

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

#include <string.h>
#include <iostream>
#include <random>
#include <boost/filesystem.hpp>
#include "sha1.h"

int x801::game::stepBlock(sqlite3_stmt* statement, sqlite3* conn) {
  volatile int stat;
  while ((stat = sqlite3_step(statement)) == SQLITE_BUSY) {
    if (stat != SQLITE_BUSY && stat != SQLITE_DONE && stat != SQLITE_ROW)
      throw sqlite3_errmsg(conn);
  }
  return stat;
}

const char* x801::game::DB_DIR = "saves/";
const char* x801::game::DB_MAIN_PATH = "saves/me.x8d";
const char* x801::game::DB_AUTH_PATH = "saves/auth.x8d";

x801::game::Database::Database() {
  if (!boost::filesystem::is_directory(DB_DIR)) {
    std::cout <<
      "Warning: overwriting " << DB_DIR <<
      " because it is not a directory\n";
    boost::filesystem::remove(DB_DIR);
    boost::filesystem::create_directories(DB_DIR);
  }
  open(me, DB_MAIN_PATH);
  open(auth, DB_AUTH_PATH);
  createAuthTable();
}

x801::game::Database::~Database() {
  sqlite3_close(me);
  sqlite3_close(auth);
}

void x801::game::Database::open(sqlite3*& handle, const char* path) {
  int status = sqlite3_open(path, &handle);
  if (handle == nullptr) throw "Can't open the database.";
  if (status != SQLITE_OK) throw sqlite3_errmsg(handle);
}

static const char* CREATE_AUTH_TABLE_QUERY =
  "CREATE TABLE IF NOT EXISTS "
  "logins("
  "  userID INTEGER PRIMARY KEY ASC,"
  "  username STRING UNIQUE NOT NULL,"
  "  hash BLOB NOT NULL,"
  "  salt BLOB NOT NULL"
  ");"
  "CREATE INDEX IF NOT EXISTS "
  "  loginsByUsername ON logins (username);"
  ;

void x801::game::Database::createAuthTable() {
  char* errMessage;
  int stat = sqlite3_exec(
    auth,
    CREATE_AUTH_TABLE_QUERY,
    nullptr, nullptr, // don't callback
    &errMessage
  );
  if (stat != SQLITE_OK) throw errMessage;
}

static const char* CREATE_USER_QUERY =
  "INSERT INTO logins (username, hash, salt) VALUES (?, ?, ?);"
  ;

void x801::game::Database::createUser(
    const char* username,
    const uint8_t* hash) {
  if (username == nullptr || hash == nullptr)
    throw "x801::game::Database::createUser: "
      "username and hash must not be null";
  sqlite3_stmt* statement;
  int stat = sqlite3_prepare_v2(
    auth,
    CREATE_USER_QUERY, -1,
    &statement,
    nullptr
  );
  if (stat != SQLITE_OK) throw sqlite3_errmsg(auth);
  stat = sqlite3_bind_text(statement, 1, username, -1, SQLITE_STATIC);
  if (stat != SQLITE_OK) throw sqlite3_errmsg(auth);
  uint8_t salt[SALT_LENGTH];
  std::random_device random;
  std::uniform_int_distribution<> dist(0, 255);
  for (int i = 0; i < SALT_LENGTH; ++i)
    salt[i] = static_cast<uint8_t>(dist(random));
  stat = sqlite3_bind_blob(
    statement, 3, static_cast<const void*>(salt), SALT_LENGTH,
    SQLITE_STATIC
  );
  if (stat != SQLITE_OK) throw sqlite3_errmsg(auth);
  // Generate SHA-1 hash
  SHA1_CTX sha1;
  sha1_init(&sha1);
  sha1_update(&sha1, hash, RAW_HASH_LENGTH);
  sha1_update(&sha1, salt, SALT_LENGTH);
  uint8_t cooked[COOKED_HASH_LENGTH];
  sha1_final(&sha1, cooked);
  stat = sqlite3_bind_blob(
    statement, 2, static_cast<const void*>(cooked), COOKED_HASH_LENGTH,
    SQLITE_STATIC
  );
  // All parameters bound.
  stepBlock(statement, auth);
  sqlite3_finalize(statement);
}

void x801::game::Database::createUser(Credentials& c) {
  createUser(c.getUsername(), c.getHash());
}

void x801::game::Database::createUserDebug(std::string username, std::string password) {
  Credentials c(username, password);
  createUser(c);
}

void x801::game::Database::userRowToSC(sqlite3_stmt* statement, StoredCredentials& sc) {
  // ID | Username | Hash | Salt
  uint32_t userID = sqlite3_column_int(statement, 0);
  std::string username(
    reinterpret_cast<const char*>(sqlite3_column_text(statement, 1)));
  const void* hash = sqlite3_column_blob(statement, 2);
  const void* salt = sqlite3_column_blob(statement, 3);
  sc = StoredCredentials(
    userID, username,
    reinterpret_cast<const uint8_t*>(hash),
    reinterpret_cast<const uint8_t*>(salt)
  );
}

static const char* GET_USER_BY_ID_QUERY =
  "SELECT * FROM logins"
  "  WHERE userID = ?;"
  ;

bool x801::game::Database::getUserByID(uint32_t id, StoredCredentials& sc) {
  sqlite3_stmt* statement;
  int stat = sqlite3_prepare_v2(
    auth,
    GET_USER_BY_ID_QUERY, -1,
    &statement,
    nullptr
  );
  if (stat != SQLITE_OK) throw sqlite3_errmsg(auth);
  stat = sqlite3_bind_int(statement, 1, id);
  if (stat != SQLITE_OK) throw sqlite3_errmsg(auth);
  stat = stepBlock(statement, auth);
  if (stat != SQLITE_ROW) {
    sqlite3_finalize(statement);
    return false;
  }
  userRowToSC(statement, sc);
  assert(sc.getUserID() == id);
  sqlite3_finalize(statement);
  return true;
}

static const char* GET_USER_BY_NAME_QUERY =
  "SELECT * FROM logins"
  "  WHERE username = ?;"
  ;

bool x801::game::Database::getUserByName(const char* username, StoredCredentials& sc) {
  sqlite3_stmt* statement;
  int stat = sqlite3_prepare_v2(
    auth,
    GET_USER_BY_NAME_QUERY, -1,
    &statement,
    nullptr
  );
  if (stat != SQLITE_OK) throw sqlite3_errmsg(auth);
  stat = sqlite3_bind_text(statement, 1, username, -1, SQLITE_STATIC);
  if (stat != SQLITE_OK) throw sqlite3_errmsg(auth);
  stat = stepBlock(statement, auth);
  if (stat != SQLITE_ROW) {
    sqlite3_finalize(statement);
    return false;
  }
  userRowToSC(statement, sc);
  assert(strcmp(sc.getUsername(), username) == 0);
  sqlite3_finalize(statement);
  return true;
}

static const char* GET_USER_ID_BY_NAME_QUERY =
  "SELECT userID FROM logins"
  "  WHERE username = ?;"
  ;

uint32_t x801::game::Database::getUserIDByName(const char* username) {
  sqlite3_stmt* statement;
  int stat = sqlite3_prepare_v2(
    auth,
    GET_USER_ID_BY_NAME_QUERY, -1,
    &statement,
    nullptr
  );
  if (stat != SQLITE_OK) throw sqlite3_errmsg(auth);
  stat = sqlite3_bind_text(statement, 1, username, -1, SQLITE_STATIC);
  if (stat != SQLITE_OK) throw sqlite3_errmsg(auth);
  stat = stepBlock(statement, auth);
  if (stat != SQLITE_ROW) {
    sqlite3_finalize(statement);
    return 0;
  }
  uint32_t userID = sqlite3_column_int(statement, 0);
  sqlite3_finalize(statement);
  return userID;
}