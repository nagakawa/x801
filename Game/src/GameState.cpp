#include "GameState.h"

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

#include <assert.h>
#include <string.h>
#include "Credentials.h"

x801::game::AreaWithPlayers::~AreaWithPlayers() {
  delete area;
}

LoginStatus x801::game::GameState::login(Credentials& c, uint32_t& id) {
  StoredCredentials sc;
  bool succeeded = db.getUserByName(c.getUsername(), sc);
  if (!succeeded) return LOGIN_INVALID_CREDENTIALS;
  assert(strcmp(c.getUsername(), sc.getUsername()) == 0);
  if (!c.matches(sc)) return LOGIN_INVALID_CREDENTIALS;
  id = sc.getUserID();
  if (usernamesByID.count(id) != 0) return LOGIN_ALREADY_LOGGED_IN;
  usernamesByID[id] = sc.getUsernameS();
  idsByUsername[sc.getUsernameS()] = id;
  // allPlayers[id] = Player(id, db);
  allPlayers.emplace(
    std::piecewise_construct,
    std::forward_as_tuple(id),
    std::forward_as_tuple(id, db)  
  );
  return LOGIN_OK;
}

void x801::game::GameState::logout(uint32_t id) {
  allPlayers.erase(id);
  idsByUsername.erase(usernamesByID[id]);
  usernamesByID.erase(id);
}

void x801::game::ClientGameState::addUser(uint32_t id, const std::string& name) {
  lookupMutex.lock();
  addUserUnsynchronised(id, name);
  lookupMutex.unlock();
}

void x801::game::ClientGameState::addUserUnsynchronised(
    uint32_t id, const std::string& name) {
  usernamesByID[id] = name;
  idsByUsername[name] = id;
  alreadyRequestedIDs.erase(id);
}

void x801::game::ClientGameState::addRequest(uint32_t id) {
  lookupMutex.lock();
  alreadyRequestedIDs.insert(id);
  lookupMutex.unlock();
}

void x801::game::ClientGameState::populateRequested(uint32_t* ids) {
  lookupMutex.lock();
  size_t i = 0;
  for (uint32_t id : alreadyRequestedIDs) {
    ids[i] = id;
    ++i;
  }
  lookupMutex.unlock();
}