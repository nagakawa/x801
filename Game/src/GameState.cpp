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
#include <fstream>
#include <mapErrors.h>
#include "Credentials.h"


namespace x801 {
  namespace game {
    void AreaWithPlayers::addPlayer(uint32_t id) {
      playerMutex.lock();
      players.insert(id);
      playerMutex.unlock();
    }
    
    void AreaWithPlayers::removePlayer(uint32_t id) {
      playerMutex.lock();
      players.erase(id);
      playerMutex.unlock();
    }
    
    AreaWithPlayers::~AreaWithPlayers() {
      delete mman;
    }
    
    void AreaWithPlayers::initMobManager() {
      mman = new MobManager(std::move(area->getPathSec()), g);
    }
    
    GameState::~GameState() {
      std::cerr << "Destroying GameState; saving player data\n";
      for (const auto& p : allPlayers) {
        uint32_t id = p.first;
        std::cerr << "  for player #" << id << '\n';
        const Player& player = p.second;
        const Location& location = player.getLocation();
        const StatsUser& su = player.getStatsU();
        db.savePlayerLocation(id, location);
        db.savePlayerStats(id, su);
      }
    }
    
    LoginStatus GameState::login(Credentials& c, uint32_t& id) {
      StoredCredentials sc;
      bool succeeded = db.getUserByName(c.getUsername(), sc);
      if (!succeeded) return LOGIN_INVALID_CREDENTIALS;
      assert(strcmp(c.getUsername(), sc.getUsername()) == 0);
      if (!c.matches(sc)) return LOGIN_INVALID_CREDENTIALS;
      id = sc.getUserID();
      playerMutex.lock_shared();
      int count = usernamesByID.count(id);
      playerMutex.unlock_shared();
      if (count != 0) return LOGIN_ALREADY_LOGGED_IN;
      playerMutex.lock();
      usernamesByID[id] = sc.getUsernameS();
      idsByUsername[sc.getUsernameS()] = id;
      // allPlayers[id] = Player(id, db);
      auto p = allPlayers.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(id),
        std::forward_as_tuple(id, db)
      );
      x801::map::QualifiedAreaID aid = p.first->second.getLocation().areaID;
      if (areas.count(aid) == 0) {
        std::string fname = "assets/maps/map.";
        // Here's a gotcha!
        // If you just append `aid.worldID`,
        // this will call the operator+=(char) overload.
        fname += std::to_string(aid.worldID);
        fname += ".";
        fname += std::to_string(aid.areaID);
        fname += ".map";
        std::ifstream fh(fname, std::ios::binary);
        std::unique_ptr<AreaWithPlayers> a =
          std::make_unique<AreaWithPlayers>(this, fh);
        if (a->getArea()->getError() != x801::map::MAPERR_OK) {
          throw x801::map::IllegalMap(a->getArea()->getError());
        }
        areas[aid] = std::move(a);
      }
      areas[aid]->players.insert(id);
      playerMutex.unlock();
      return LOGIN_OK;
    }
    
    void GameState::logout(uint32_t id) {
      playerMutex.lock();
      const Player& player = allPlayers[id];
      const Location& location = player.getLocation();
      const StatsUser& su = player.getStatsU();
      db.savePlayerLocation(id, location);
      db.savePlayerStats(id, su);
      x801::map::QualifiedAreaID aid = location.areaID;
      areas[aid]->players.erase(id);
      allPlayers.erase(id);
      idsByUsername.erase(usernamesByID[id]);
      usernamesByID.erase(id);
      playerMutex.unlock();
    }
    
    MobInfo* GameState::getMobInfo(std::string name) {
      MobInfo* mi;
      miMutex.lock_shared();
      auto it = mobInfos.find(name);
      if (it != mobInfos.end()) {
        mi = it->second.get();
        miMutex.unlock_shared();
        return mi;
      }
      miMutex.unlock_shared();
      std::ifstream fh("assets/mobs/" + name + ".mob");
      std::unique_ptr<MobInfo> umi = std::make_unique<MobInfo>(fh);
      miMutex.lock();
      mobInfos[name] = std::move(umi);
      mi = mobInfos[name].get();
      miMutex.unlock();
      return mi;
    }
    
    void ClientGameState::addUser(uint32_t id, const std::string& name) {
      lookupMutex.lock();
      addUserUnsynchronised(id, name);
      lookupMutex.unlock();
    }
    
    void ClientGameState::addUserUnsynchronised(
        uint32_t id, const std::string& name) {
      usernamesByID[id] = name;
      idsByUsername[name] = id;
      alreadyRequestedIDs.erase(id);
    }
    
    void ClientGameState::addRequest(uint32_t id) {
      lookupMutex.lock();
      alreadyRequestedIDs.insert(id);
      lookupMutex.unlock();
    }
    
    void ClientGameState::populateRequested(uint32_t* ids, size_t n) {
      lookupMutex.lock();
      size_t i = 0;
      for (uint32_t id : alreadyRequestedIDs) {
        if (i >= n) break;
        ids[i] = id;
        ++i;
      }
      lookupMutex.unlock();
    }
    #include <iostream>
    /*
      disclaimer: this below rant is clearly tongue-in-cheek, so don't hate me
      Shit. Vanessa isn't even in one of the top PvP schools, and while I'm
      here fapping with my 420 (SNOOP DOGG) rating on my life, she's already
      at 660. I want revenge. And with this envy, I write this method in good
      old C++! ~ Uruwi
    */
    void ClientGameState::fastForwardSelf(RakNet::Time t) {
      x801::map::Area& a = *(currentArea.getArea());
      // Find the first (backmost) element in the queue greater than or
      // equal to t, using binary search.
      historyMutex.lock_shared();
      size_t start = 0;
      size_t end = history.size();
      while (end - start > 1) {
        size_t mid = (end + start) >> 1;
        RakNet::Time midTime = history[mid].time;
        if (t < midTime) end = mid;
        else start = mid;
      }
      historyMutex.unlock_shared();
      historyMutex.lock();
      history.popFront(end);
      historyMutex.unlock();
      historyMutex.lock_shared();
      size_t size = history.size();
      // Replay key inputs starting from this element.
      RakNet::Time tp = t;
      selfPosition = playersByID[myID].getLocation();
      //std::cout << "Fast-forwarding: ";
      for (size_t i = 0; i < size; ++i) {
        //std::cout << "{" << selfPosition.x << ", " << selfPosition.y << ", " << selfPosition.z << ", " << selfPosition.rot << "} ";
        //std::cout << i << "[" << history[i].inputs << ", " << (ssize_t) (history[i].time - tp) << "] ";
        bool stat = selfPosition.applyKeyInput(history[i], tp, a);
        if (stat) tp = history[i].time;
      }
      //std::cout << "{" << selfPosition.x << ", " << selfPosition.y << ", " << selfPosition.z << ", " << selfPosition.rot << "} ";
      //std::cout << "\n";
      RakNet::Time present = RakNet::GetTime();
      KeyInput last = {
        present,
        (size > 0) ? history[size - 1].inputs : 0
      };
      selfPosition.applyKeyInput(last, tp, a);
      lastTimeServer = t;
      historyMutex.unlock_shared();
    }
    
    void ClientGameState::fastForwardSelfClient(const KeyInput& ki) {
      x801::map::Area& a = *(currentArea.getArea());
      selfPosition.applyKeyInput(ki, lastTimeClient, a);
      lastTimeClient = ki.time;
    }    
  }
}
