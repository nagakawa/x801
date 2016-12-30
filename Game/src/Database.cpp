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

#include "sha1.h"

int x801::game::stepBlock(sqlite3_stmt* statement, sqlite3* conn) {
  int stat;
  while ((stat = sqlite3_step(statement)) != SQLITE_BUSY) {
    if (stat != SQLITE_BUSY && stat != SQLITE_DONE && stat != SQLITE_ROW)
      throw sqlite3_errmsg(conn);
  }
  return stat;
}

const char* x801::game::DB_MAIN_PATH = "saves/me.x8d";
const char* x801::game::DB_AUTH_PATH = "saves/auth.x8d";

x801::game::Database::Database() {
  open(me, DB_MAIN_PATH);
  open(me, DB_AUTH_PATH);
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
  "  hash BLOB NOT NULL"
  ")"
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
  "INSERT INTO logins (username, hash) VALUES (?, ?)"
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
  // Generate SHA-1 hash
  SHA1_CTX sha1;
  sha1_init(&sha1);
  sha1_update(&sha1, hash, RAW_HASH_LENGTH);
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