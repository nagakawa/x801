#include "EntityModel.h"

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

#include "mapErrors.h"

using namespace x801::map;

using namespace x801::base;

x801::map::Component::Component(std::istream& fh) {
  parent = readInt<uint32_t>(fh);
  offsetAngle = readQuaternion(fh);
  offsetCoordinates = readVec3(fh);
  axisScale = readVec3(fh);
}

void x801::map::Component::write(std::ostream& fh) const {
  writeInt<uint32_t>(fh, parent);
  writeQuaternion(fh, offsetAngle);
  writeVec3(fh, offsetCoordinates);
  writeVec3(fh, axisScale);
}

x801::map::PFaceVertex::PFaceVertex(std::istream& fh) {
  cindex = readInt<uint32_t>(fh);
  offset = readVec3(fh);
  uv = readVec2(fh);
}

void x801::map::PFaceVertex::write(std::ostream& fh) const {
  writeInt<uint32_t>(fh, cindex);
  writeVec3(fh, offset);
  writeVec2(fh, uv);
}

x801::map::PFace::PFace(std::istream& fh) {
  for (size_t i = 0; i < 3; ++i)
    vertices[i] = PFaceVertex(fh);
}

void x801::map::PFace::write(std::ostream& fh) const {
  for (size_t i = 0; i < 3; ++i)
    vertices[i].write(fh);
}

x801::map::Part::Part(std::istream& fh) {
  hitboxSize = readVec3(fh);
  size_t nComponents = readInt<uint32_t>(fh);
  size_t nFaces = readInt<uint32_t>(fh);
  size_t nControlAngles = readInt<uint32_t>(fh);
  for (size_t i = 0; i < nComponents; ++i) {
    components.push_back(Component(fh));
  }
  for (size_t i = 0; i < nFaces; ++i) {
    faces.push_back(PFace(fh));
  }
  for (size_t i = 0; i < nComponents; ++i) {
    std::string name = readString<uint16_t>(fh);
    componentIndicesByName[name] = i;
    componentNames.push_back(name);
  }
  for (size_t i = 0; i < nControlAngles; ++i) {
    std::string name = readString<uint16_t>(fh);
    size_t affectedCount = readInt<uint16_t>(fh);
    std::vector<size_t> affected;
    for (size_t j = 0; j < affectedCount; ++j) {
      size_t cid = readInt<uint32_t>(fh);
      affected.push_back(cid);
      controlAnglesByComponent[cid].push_back(name);
    }
    controlAngles[name] = std::move(affected);
  }
}

void x801::map::Part::write(std::ostream& fh) const {
  writeVec3(fh, hitboxSize);
  writeInt<uint32_t>(fh, components.size());
  writeInt<uint32_t>(fh, faces.size());
  writeInt<uint32_t>(fh, controlAngles.size());
  for (const Component& comp : components) {
    comp.write(fh);
  }
  for (const PFace& face : faces) {
    face.write(fh);
  }
  for (const std::string& name : componentNames) {
    writeString<uint16_t>(fh, name);
  }
  for (const auto& p : controlAngles) {
    // name
    writeString<uint16_t>(fh, p.first);
    // affected
    writeInt<uint16_t>(fh, p.second.size());
    for (size_t n : p.second)
      writeInt<uint32_t>(fh, n);
  }
}

x801::map::Blueprint::Elem::Elem(std::istream& fh) {
  name = readString<uint16_t>(fh);
  id = readString<uint16_t>(fh);
  textureName = readString<uint16_t>(fh);
  parent = readInt<uint32_t>(fh);
  angle = readQuaternion(fh);
  offset = readVec3(fh);
}

void x801::map::Blueprint::Elem::write(std::ostream& fh) const {
  writeString<uint16_t>(fh, name);
  writeString<uint16_t>(fh, id);
  writeString<uint16_t>(fh, textureName);
  writeInt<uint32_t>(fh, parent);
  writeQuaternion(fh, angle);
  writeVec3(fh, offset);
}

x801::map::Blueprint::Blueprint(std::istream& fh) {
  size_t count = readInt<uint32_t>(fh);
  for (size_t i = 0; i < count; ++i) {
    elems.emplace_back(fh);
  }
}

void x801::map::Blueprint::write(std::ostream& fh) const {
  writeInt<uint32_t>(fh, elems.size());
  for (const Elem& elem : elems) {
    elem.write(fh);
  }
}