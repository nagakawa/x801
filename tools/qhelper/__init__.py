import fparser

from pyquaternion import Quaternion

def quaternionFromCommands(comm):
  i = 0
  n = len(comm)
  q = Quaternion()
  while i < n:
    label = comm[i]
    i += 1
    # if we have "identity", then we need not do anything
    if label == 'identity': continue
    flag = -1
    # axis rotations: this is degrees ccw, looking from
    # origin toward the positive end of the appropriate axis
    if label == 'aroundXAxis': flag = 0
    if label == 'aroundYAxis': flag = 1
    if label == 'aroundZAxis': flag = 2
    if flag == -1:
      fparser.error("Unknown label " + str(label) + "! Check docs for details.")
    # These commands take one argument; gobble that up
    if i >= n: # no more entries
      fparser.error("Reached end of argument list!")
    angle = 0
    try:
      angle = float(comm[i])
    except Exception:
      fparser.error(comm[i] + " is not a number")
    i += 1
    axis = [
      [1, 0, 0],
      [0, 1, 0],
      [0, 0, 1]
    ][flag]
    # the argument is the rotation CCW when our line of sight is parallel
    # to the positive axis, so CW when it's pointing toward negative
    rotation = Quaternion(axis=axis, degrees=-angle)
    # NOTE: the order matters!
    q = (rotation * q).normalised
  return q