# Module.

# Still thinking of how to do this,
# especially since 3D is artistically intense.

# a list of triangles.
# each triangle is a list of 3 vertices plus an occlusion face
# each vertex consists of [x, y, z, u, v]
# each value is [constant, [waveFunc, waveParameter, waveAmplitude, wavePeriod, waveLag]*]?

faces = []

def addTriangleVert(v1, v2, v3, occlusion=''):
  faces.append([v1, v2, v3, occlusion])

def addQuadVert(v1, v2, v3, v4, occlusion=''):
  addTriangleVert(v1, v2, v3, occlusion)
  addTriangleVert(v2, v3, v4, occlusion)


def fixed(c):
  return [c]