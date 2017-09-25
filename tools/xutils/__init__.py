def writeInt(f, n, b):
  f.write((n % (1 << 8 * b)).to_bytes(b, byteorder='little'))

def writeString16(f, s):
  b = s.encode()
  writeInt(f, len(b), 2)
  f.write(b)

def byteLength(n):
  return (n.bit_length() + 7) >> 3

def writeBigint(f, n):
  isneg = n < 0
  n = abs(n)
  l = byteLength(n)
  netl = l
  if isneg: netl |= 0x80000000
  writeInt(f, netl, 4)
  writeInt(f, n, l)