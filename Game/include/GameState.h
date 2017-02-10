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

#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <boost/thread/shared_mutex.hpp>
#include <Area.h>
#include <CircularQueue.h>
#include <QualifiedAreaID.h>
#include "Database.h"
#include "Player.h"

namespace x801 {
  namespace game {
    enum LoginStatus {
      LOGIN_OK = 0,
      LOGIN_INVALID_CREDENTIALS = 1,
      LOGIN_SERVER_FULL = 2,
      LOGIN_BANNED = 3,
      LOGIN_ALREADY_LOGGED_IN = 4,
      LOGIN_FAILED = 5,
      LOGIN_NOT_ENOUGH_DATA = 6,
    };
    class GameState;
    class ClientGameState;
    class AreaWithPlayers {
    public:
      // dummy
      AreaWithPlayers() {}
      // linkback to ClientGameState and read map data
      AreaWithPlayers(ClientGameState* g, std::istream& fh) :
          cg(g), area(new x801::map::Area(fh)) {}
      // linkback to GameState and read map data
      AreaWithPlayers(GameState* g, std::istream& fh) :
          g(g), area(new x801::map::Area(fh)) {}
      AreaWithPlayers(const AreaWithPlayers& other) = delete;
      AreaWithPlayers& operator=(const AreaWithPlayers& other) = delete;
      ~AreaWithPlayers();
      // return (players.cbegin(), players.cend())
      auto playerMapEndpoints() const {
        boost::shared_lock<boost::shared_mutex> guard(playerMutex);
        return std::pair<decltype(players.cbegin()), decltype(players.cend())>(
            players.cbegin(), players.cend());
      }
      auto playerBegin() const {
        boost::shared_lock<boost::shared_mutex> guard(playerMutex);
        return players.cbegin();
      }
      auto playerEnd() const {
        boost::shared_lock<boost::shared_mutex> guard(playerMutex);
        return players.cend();
      }
      size_t playerCount() const {
        boost::shared_lock<boost::shared_mutex> guard(playerMutex);
        return players.size();
      }
      auto findPlayer(uint32_t id) const {
        boost::shared_lock<boost::shared_mutex> guard(playerMutex);
        return players.find(id);
      }
      void addPlayer(uint32_t id);
      void removePlayer(uint32_t id);
      // Mutex to make sure multiple threads aren't mutating
      // the set of players in this area simultaneously.
      // This is public so users of the class can use the mutex to
      // safely iterate over all elements of a map.
      mutable boost::shared_mutex playerMutex;
    private:
      std::unordered_set<uint32_t> players;
      GameState* g = nullptr;
      // Unsure whether this is needed, since ClientGameState can hold
      // only one AreaWithPlayers.
      ClientGameState* cg = nullptr;
      x801::map::Area* area = nullptr;
      friend class Client;
      friend class Server;
    };
    
    class GameState {
    public:
      LoginStatus login(Credentials& c, uint32_t& id);
      void logout(uint32_t id);
      auto findPlayer(uint32_t id) const {
        boost::shared_lock<boost::shared_mutex> guard(playerMutex);
        return allPlayers.find(id);
      }
      auto findUsernameByID(uint32_t id) const {
        boost::shared_lock<boost::shared_mutex> guard(playerMutex);
        return usernamesByID.find(id);
      }
      auto findIDByUsername(const std::string& name) const {
        boost::shared_lock<boost::shared_mutex> guard(playerMutex);
        return idsByUsername.find(name);
      }
      auto endOfUsernameMap() const {
        boost::shared_lock<boost::shared_mutex> guard(playerMutex);
        return usernamesByID.cend();
      }
      auto endOfIDMap() const {
        boost::shared_lock<boost::shared_mutex> guard(playerMutex);
        return idsByUsername.cend();
      }
      auto endOfPlayerMap() const {
        boost::shared_lock<boost::shared_mutex> guard(playerMutex);
        return allPlayers.cend();
      }
      mutable boost::shared_mutex playerMutex;
    private:
      Database db;
      std::unordered_map<uint32_t, Player> allPlayers;
      std::unordered_map<uint32_t, std::string> usernamesByID;
      std::unordered_map<std::string, uint32_t> idsByUsername;
      std::unordered_map<
        x801::map::QualifiedAreaID, std::unique_ptr<AreaWithPlayers>,
        x801::map::QualifiedAreaIDHash, x801::map::QualifiedAreaIDEqual
      > areas;
      friend class Server;
    };

    class ClientGameState {
    public:
      auto findUsernameByID(uint32_t id) const {
        boost::shared_lock<boost::shared_mutex> guard(lookupMutex);
        return usernamesByID.find(id);
      }
      auto findIDByUsername(const std::string& name) const {
        boost::shared_lock<boost::shared_mutex> guard(lookupMutex);
        return idsByUsername.find(name);
      }
      auto endOfUsernameMap() const {
        boost::shared_lock<boost::shared_mutex> guard(lookupMutex);
        return usernamesByID.cend();
      }
      bool isIDRequested(uint32_t id) const {
        boost::shared_lock<boost::shared_mutex> guard(lookupMutex);
        return alreadyRequestedIDs.count(id) != 0;
      }
      void addUser(uint32_t id, const std::string& name);
      // does not lock the mutex, so if you use this manually lock the mutex
      void addUserUnsynchronised(uint32_t id, const std::string& name);
      void addRequest(uint32_t id);
      size_t totalRequested() const {
        boost::shared_lock<boost::shared_mutex> guard(lookupMutex);
        return alreadyRequestedIDs.size();
      }
      AreaWithPlayers& getCurrentArea() {
        return currentArea;
      }
      Player& getPlayer(uint32_t id) {
        boost::unique_lock<boost::shared_mutex> guard(locationMutex);
        return playersByID[id];
      }
      // Write all of the elements of alreadyRequestedIDs into
      // a buffer. It should be big enough to fit the total
      // number of elements in the set; use totalRequested()
      // to get the count.
      void populateRequested(uint32_t* ids, size_t n);
      uint32_t getID() const { return myID; }
      void setID(uint32_t id) {
        myID = id;
      }
      // Use previous key inputs to fast-forward the player position
      // and compensate for latency. Discard all key inputs before
      // the specified time.
      void fastForwardSelf(RakNet::Time t);
      // Mutex to make sure multiple threads aren't changing
      // player maps simultaneously.
      // This is public so the client can add multiple ID-username
      // mapping with only one lock and unlock.
      // In addition, users of the class can use the mutex to
      // safely iterate over all elements of a map.
      mutable boost::shared_mutex lookupMutex;
      mutable boost::shared_mutex locationMutex;
    private:
      AreaWithPlayers currentArea;
      std::unordered_map<uint32_t, std::string> usernamesByID;
      std::unordered_map<std::string, uint32_t> idsByUsername;
      std::unordered_set<uint32_t> alreadyRequestedIDs;
      std::unordered_map<uint32_t, Player> playersByID;
      x801::base::CircularQueue<KeyInput> history;
      Location selfPosition;
      RakNet::Time lastTime = 0;
      uint32_t myID = 0;
      friend class Client;
      friend class ClientWindow;
    };
  }
}
