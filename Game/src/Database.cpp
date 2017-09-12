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
#include <boost/filesystem.hpp>
#include <utils.h>
#define SHA2_USE_INTTYPES_H
#include "sha2.h"

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

#pragma GCC diagnostic push                // we DO want an explicit ctor
#pragma GCC diagnostic ignored "-Weffc++"  // since it has complex behaviour
x801::game::Database::Database() {
  if (!boost::filesystem::is_directory(DB_DIR)) {
    std::cout <<
      "Warning: overwriting " << DB_DIR <<
      " because it is not a directory\n";
    boost::filesystem::remove(DB_DIR);
    boost::filesystem::create_directories(DB_DIR);
  }
  open(me, DB_MAIN_PATH);
  createAuthTable();
  createPlayerLocationTable();
  createPlayerStatsTable();
}

x801::game::Database::~Database() {
  sqlite3_close(me);
}
#pragma GCC diagnostic pop

void x801::game::Database::open(sqlite3*& handle, const char* path) {
  int status = sqlite3_open(path, &handle);
  if (handle == nullptr) throw "Can't open the database.";
  if (status != SQLITE_OK) throw sqlite3_errmsg(handle);
}

static const char* CREATE_AUTH_TABLE_QUERY =
  "CREATE TABLE IF NOT EXISTS "
  "Logins("
  "  userID INTEGER PRIMARY KEY ASC,"
  "  username STRING UNIQUE NOT NULL,"
  "  hash BLOB NOT NULL,"
  "  salt BLOB NOT NULL"
  ");"
  "CREATE INDEX IF NOT EXISTS "
  "  loginsByUsername ON Logins (username);"
  ;

void x801::game::Database::createAuthTable() {
  char* errMessage;
  int stat = sqlite3_exec(
    me,
    CREATE_AUTH_TABLE_QUERY,
    nullptr, nullptr, // don't callback
    &errMessage
  );
  if (stat != SQLITE_OK) throw errMessage;
}

static const char* CREATE_USER_QUERY =
  "INSERT INTO Logins (username, hash, salt) VALUES (?, ?, ?);"
  ;

void x801::game::Database::createUser(
    const char* username,
    const uint8_t* hash) {
  if (username == nullptr || hash == nullptr)
    throw "x801::game::Database::createUser: "
      "username and hash must not be null";
  sqlite3_stmt* statement;
  int stat = sqlite3_prepare_v2(
    me,
    CREATE_USER_QUERY, -1,
    &statement,
    nullptr
  );
  if (stat != SQLITE_OK) throw sqlite3_errmsg(me);
  stat = sqlite3_bind_text(statement, 1, username, -1, SQLITE_STATIC);
  if (stat != SQLITE_OK) throw sqlite3_errmsg(me);
  uint8_t salt[SALT_LENGTH];
  x801::base::writeRandomBytes(salt, SALT_LENGTH);
  stat = sqlite3_bind_blob(
    statement, 3, static_cast<const void*>(salt), SALT_LENGTH,
    SQLITE_STATIC
  );
  if (stat != SQLITE_OK) throw sqlite3_errmsg(me);
  // Generate SHA-256 hash
  SHA256_CTX sha2;
  SHA256_Init(&sha2);
  SHA256_Update(&sha2, hash, RAW_HASH_LENGTH);
  SHA256_Update(&sha2, salt, SALT_LENGTH);
  uint8_t cooked[COOKED_HASH_LENGTH];
  SHA256_Final(cooked, &sha2);
  stat = sqlite3_bind_blob(
    statement, 2, static_cast<const void*>(cooked), COOKED_HASH_LENGTH,
    SQLITE_STATIC
  );
  if (stat != SQLITE_OK) throw sqlite3_errmsg(me);
  // All parameters bound.
  stepBlock(statement, me);
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
  "SELECT * FROM Logins"
  "  WHERE userID = ?;"
  ;

bool x801::game::Database::getUserByID(uint32_t id, StoredCredentials& sc) {
  sqlite3_stmt* statement;
  int stat = sqlite3_prepare_v2(
    me,
    GET_USER_BY_ID_QUERY, -1,
    &statement,
    nullptr
  );
  if (stat != SQLITE_OK) throw sqlite3_errmsg(me);
  stat = sqlite3_bind_int(statement, 1, id);
  if (stat != SQLITE_OK) throw sqlite3_errmsg(me);
  stat = stepBlock(statement, me);
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
  "SELECT * FROM Logins"
  "  WHERE username = ?;"
  ;

bool x801::game::Database::getUserByName(const char* username, StoredCredentials& sc) {
  sqlite3_stmt* statement;
  int stat = sqlite3_prepare_v2(
    me,
    GET_USER_BY_NAME_QUERY, -1,
    &statement,
    nullptr
  );
  if (stat != SQLITE_OK) throw sqlite3_errmsg(me);
  stat = sqlite3_bind_text(statement, 1, username, -1, SQLITE_STATIC);
  if (stat != SQLITE_OK) throw sqlite3_errmsg(me);
  stat = stepBlock(statement, me);
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
  "SELECT userID FROM Logins"
  "  WHERE username = ?;"
  ;

uint32_t x801::game::Database::getUserIDByName(const char* username) {
  sqlite3_stmt* statement;
  int stat = sqlite3_prepare_v2(
    me,
    GET_USER_ID_BY_NAME_QUERY, -1,
    &statement,
    nullptr
  );
  if (stat != SQLITE_OK) throw sqlite3_errmsg(me);
  stat = sqlite3_bind_text(statement, 1, username, -1, SQLITE_STATIC);
  if (stat != SQLITE_OK) throw sqlite3_errmsg(me);
  stat = stepBlock(statement, me);
  if (stat != SQLITE_ROW) {
    sqlite3_finalize(statement);
    return 0;
  }
  uint32_t userID = sqlite3_column_int(statement, 0);
  sqlite3_finalize(statement);
  return userID;
}

static const char* CREATE_PLAYER_LOCATION_TABLE_QUERY =
  "CREATE TABLE IF NOT EXISTS "
  "PlayerLocations("
  "  userID INTEGER UNIQUE NOT NULL,"
  "  worldID INTEGER NOT NULL,"
  "  areaID INTEGER NOT NULL,"
  "  x DOUBLE NOT NULL,"
  "  y DOUBLE NOT NULL,"
  "  z INTEGER NOT NULL,"
  "  rot INTEGER NOT NULL,"
  "  FOREIGN KEY(userID) REFERENCES Logins(userID)"
  "    ON DELETE CASCADE ON UPDATE CASCADE"
  ");"
  ;

void x801::game::Database::createPlayerLocationTable() {
  char* errMessage;
  int stat = sqlite3_exec(
    me,
    CREATE_PLAYER_LOCATION_TABLE_QUERY,
    nullptr, nullptr, // don't callback
    &errMessage
  );
  if (stat != SQLITE_OK) throw errMessage;
}

static const char* SAVE_PLAYER_LOCATION_QUERY =
  "INSERT OR REPLACE INTO PlayerLocations"
  "  (userID, worldID, areaID, x, y, z, rot)"
  "  VALUES (?, ?, ?, ?, ?, ?, ?);"
  ;

void x801::game::Database::savePlayerLocation(
    uint32_t userID,
    const Location& location) {
  if (userID == 0) throw "userID must not be 0";
  sqlite3_stmt* statement;
  int stat = sqlite3_prepare_v2(
    me,
    SAVE_PLAYER_LOCATION_QUERY, -1,
    &statement,
    nullptr
  );
  if (stat != SQLITE_OK) throw sqlite3_errmsg(me);
  stat = sqlite3_bind_int(statement, 1, userID);
  if (stat != SQLITE_OK) throw sqlite3_errmsg(me);
  stat = sqlite3_bind_int(statement, 2, location.areaID.worldID);
  if (stat != SQLITE_OK) throw sqlite3_errmsg(me);
  stat = sqlite3_bind_int(statement, 3, location.areaID.areaID);
  if (stat != SQLITE_OK) throw sqlite3_errmsg(me);
  stat = sqlite3_bind_double(statement, 4, location.x);
  if (stat != SQLITE_OK) throw sqlite3_errmsg(me);
  stat = sqlite3_bind_double(statement, 5, location.y);
  if (stat != SQLITE_OK) throw sqlite3_errmsg(me);
  stat = sqlite3_bind_int(statement, 6, location.z);
  if (stat != SQLITE_OK) throw sqlite3_errmsg(me);
  stat = sqlite3_bind_int(statement, 7, location.rot);
  if (stat != SQLITE_OK) throw sqlite3_errmsg(me);
  // All parameters bound.
  stepBlock(statement, me);
  sqlite3_finalize(statement);
}

uint32_t x801::game::Database::locationRowToStruct(
    sqlite3_stmt* statement,
    Location& location) {
  location.areaID.worldID = (uint16_t) sqlite3_column_int(statement, 1);
  location.areaID.areaID = (uint16_t) sqlite3_column_int(statement, 2);
  location.x = (float) sqlite3_column_double(statement, 3);
  location.y = (float) sqlite3_column_double(statement, 4);
  location.z = (uint8_t) sqlite3_column_int(statement, 5);
  location.rot = (uint8_t) sqlite3_column_int(statement, 6);
  return sqlite3_column_int(statement, 0);
}

static const char* LOAD_PLAYER_LOCATION_QUERY =
  "SELECT * FROM PlayerLocations"
  "  WHERE userID = ?;"
  ;

bool x801::game::Database::loadPlayerLocation(uint32_t userID, Location& location) {
  sqlite3_stmt* statement;
  int stat = sqlite3_prepare_v2(
    me,
    LOAD_PLAYER_LOCATION_QUERY, -1,
    &statement,
    nullptr
  );
  if (stat != SQLITE_OK) throw sqlite3_errmsg(me);
  stat = sqlite3_bind_int(statement, 1, userID);
  if (stat != SQLITE_OK) throw sqlite3_errmsg(me);
  stat = stepBlock(statement, me);
  if (stat != SQLITE_ROW) {
    sqlite3_finalize(statement);
    return false;
  }
  uint32_t actualID = locationRowToStruct(statement, location);
  assert(actualID == userID);
  (void) actualID; // assert not used in release mode
  sqlite3_finalize(statement);
  return true;
}

static const char* CREATE_PLAYER_STATS_TABLE_QUERY =
  "CREATE TABLE IF NOT EXISTS "
  "PlayerStats("
  "  userID INTEGER UNIQUE NOT NULL,"
  "  level INTEGER DEFAULT 0,"
  "  school INTEGER DEFAULT 0,"
  "  xp INTEGER DEFAULT 0,"
  "  FOREIGN KEY(userID) REFERENCES Logins(userID)"
  "    ON DELETE CASCADE ON UPDATE CASCADE"
  ");"
  ;
  
void x801::game::Database::createPlayerStatsTable() {
  char* errMessage;
  int stat = sqlite3_exec(
    me,
    CREATE_PLAYER_STATS_TABLE_QUERY,
    nullptr, nullptr, // don't callback
    &errMessage
  );
  if (stat != SQLITE_OK) throw errMessage;
}

static const char* SAVE_PLAYER_STATS_QUERY =
"INSERT OR REPLACE INTO PlayerStats"
"  (userID, level, school, xp)"
"  VALUES (?, ?, ?, ?);"
;

void x801::game::Database::savePlayerStats(
    uint32_t userID,
    const StatsUser& su) {
  if (userID == 0) throw "userID must not be 0";
  sqlite3_stmt* statement;
  int stat = sqlite3_prepare_v2(
    me,
    SAVE_PLAYER_STATS_QUERY, -1,
    &statement,
    nullptr
  );
  if (stat != SQLITE_OK) throw sqlite3_errmsg(me);
  stat = sqlite3_bind_int(statement, 1, userID);
  if (stat != SQLITE_OK) throw sqlite3_errmsg(me);
  stat = sqlite3_bind_int(statement, 2, su.level);
  if (stat != SQLITE_OK) throw sqlite3_errmsg(me);
  stat = sqlite3_bind_int(statement, 3, su.school);
  if (stat != SQLITE_OK) throw sqlite3_errmsg(me);
  stat = sqlite3_bind_int(statement, 4, su.xp);
  if (stat != SQLITE_OK) throw sqlite3_errmsg(me);
  // All parameters bound.
  stepBlock(statement, me);
  sqlite3_finalize(statement);
}

uint32_t x801::game::Database::statsRowToStruct(
    sqlite3_stmt* statement,
    StatsUser& su) {
  su.level = sqlite3_column_int(statement, 1);
  su.school = sqlite3_column_int(statement, 2);
  su.xp = sqlite3_column_int(statement, 3);
  return sqlite3_column_int(statement, 0);
}

static const char* LOAD_PLAYER_STATS_QUERY =
"SELECT * FROM PlayerStats"
"  WHERE userID = ?;"
;

bool x801::game::Database::loadPlayerStats(uint32_t userID, StatsUser& su) {
  sqlite3_stmt* statement;
  int stat = sqlite3_prepare_v2(
    me,
    LOAD_PLAYER_STATS_QUERY, -1,
    &statement,
    nullptr
  );
  if (stat != SQLITE_OK) throw sqlite3_errmsg(me);
  stat = sqlite3_bind_int(statement, 1, userID);
  if (stat != SQLITE_OK) throw sqlite3_errmsg(me);
  stat = stepBlock(statement, me);
  if (stat != SQLITE_ROW) {
    sqlite3_finalize(statement);
    return false;
  }
  uint32_t actualID = statsRowToStruct(statement, su);
  assert(actualID == userID);
  (void) actualID; // assert not used in release mode
  sqlite3_finalize(statement);
  return true;
}