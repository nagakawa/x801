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
#include <GetTime.h>
#include <Area.h>
#include <ConcurrentCircularQueue.h>
#include <QualifiedAreaID.h>
#include "Database.h"
#include "Player.h"
#include "packet.h"
#include "combat/mob/MobInfo.h"
#include "combat/mob/MobManager.h"

namespace x801 {
  namespace game {
    class GameState;
    class ClientGameState;
    class SpellIndex;
    class AreaWithPlayers {
    public:
      AreaWithPlayers() {
      }
      // linkback to ClientGameState and read map data
      AreaWithPlayers(ClientGameState* g, std::istream& fh) :
          cg(g), area(new x801::map::Area(fh)) {
        initMobManager();
      }
      // linkback to GameState and read map data
      AreaWithPlayers(GameState* g, std::istream& fh) :
          g(g), area(new x801::map::Area(fh)) {
        initMobManager();
      }
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
      std::shared_ptr<x801::map::Area> getArea() { return area; }
      void setArea(std::shared_ptr<x801::map::Area> a) {
        area = a;
      }
      void initMobManager();
      // Mutex to make sure multiple threads aren't mutating
      // the set of players in this area simultaneously.
      // This is public so users of the class can use the mutex to
      // safely iterate over all elements of a map.
      mutable boost::shared_mutex playerMutex;
      GameState* g = nullptr;
      ClientGameState* cg = nullptr;
    private:
      std::unordered_set<uint32_t> players;
      // Unsure whether this is needed, since ClientGameState can hold
      // only one AreaWithPlayers.
      std::shared_ptr<x801::map::Area> area = nullptr;
      MobManager* mman = nullptr;
      friend class Client;
      friend class Server;
      friend class GameState;
    };
    class Server;
    class GameState {
    public:
      ~GameState();
      LoginStatus login(Credentials& c, uint32_t& id);
      void logout(uint32_t id);
      auto findPlayer(uint32_t id) const {
        boost::shared_lock<boost::shared_mutex> guard(playerMutex);
        return allPlayers.find(id);
      }
      bool findPlayer(uint32_t id, Player*& player) {
        boost::shared_lock<boost::shared_mutex> guard(playerMutex);
        auto it = allPlayers.find(id);
        if (it != allPlayers.end()) {
          player = &(it->second);
          return true;
        } else return false;
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
      MobInfo* getMobInfo(std::string name);
      void advanceFrame(float s);
      // void addArea(x801::map::QualifiedAreaID);
      mutable boost::shared_mutex playerMutex;
      mutable boost::shared_mutex miMutex;
      SpellIndex* spells = nullptr;
      Server* s = nullptr;
    private:
      Database db;
      std::unordered_map<uint32_t, Player> allPlayers;
      std::unordered_map<uint32_t, std::string> usernamesByID;
      std::unordered_map<std::string, uint32_t> idsByUsername;
      std::unordered_map<
        x801::map::QualifiedAreaID, std::unique_ptr<AreaWithPlayers>,
        x801::map::QualifiedAreaIDHash, x801::map::QualifiedAreaIDEqual
      > areas;
      std::unordered_map<std::string, std::unique_ptr<MobInfo>> mobInfos;
      friend class Server;
    };
    class Client;
    class ClientGameState {
    public:
      ~ClientGameState();
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
        boost::shared_lock<boost::shared_mutex> guard(locationMutex);
        return playersByID[id];
      }
      Player& getPlayerUnsynchronised(uint32_t id) {
        return playersByID[id];
      }
      void purgePlayers() {
        boost::unique_lock<boost::shared_mutex> guard(locationMutex);
        purgePlayersUnsynchronised();
      }
      void purgePlayersUnsynchronised() {
        playersByID.clear();
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
      void fastForwardSelfClient(const KeyInput& ki);
      // Mutex to make sure multiple threads aren't changing
      // player maps simultaneously.
      // This is public so the client can add multiple ID-username
      // mapping with only one lock and unlock.
      // In addition, users of the class can use the mutex to
      // safely iterate over all elements of a map.
      mutable boost::shared_mutex lookupMutex;
      mutable boost::shared_mutex locationMutex;
      mutable boost::shared_mutex historyMutex;
      mutable std::mutex selfPositionMutex;
      //mutable boost::shared_mutex keyHistoryMutex;
      SpellIndex* spells = nullptr;
      Client* c = nullptr;
    private:
      AreaWithPlayers currentArea;
      std::unordered_map<uint32_t, std::string> usernamesByID;
      std::unordered_map<std::string, uint32_t> idsByUsername;
      std::unordered_set<uint32_t> alreadyRequestedIDs;
      std::unordered_map<uint32_t, Player> playersByID;
      x801::base::CircularQueue<KeyInput> history;
      Location selfPosition;
      RakNet::Time lastTimeServer = RakNet::GetTime();
      RakNet::Time lastTimeClient = RakNet::GetTime();
      uint32_t myID = 0;
      friend class Client;
      friend class ClientWindow;
      friend class TerrainRenderer;
      friend class ChunkBuffer;
      friend class EntityRenderer;
      friend class EntityBuffer;
    };
  }
}
