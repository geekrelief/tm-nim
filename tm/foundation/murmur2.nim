# from https://github.com/cwpearson/nim-murmurhash/blob/master/src/murmurhash/murmur2.nim
template `^=`(a, b) = a = a xor b

template `+`[T](p: ptr T, off: int): ptr T =
  cast[ptr T](cast[ByteAddress](p) +% off * sizeof(T))

template `[]`[T](p: ptr T, off: int): T =
  (p + off)[]


proc murmurHash64A*(key: ptr uint8, len: int, seed: uint64 = 0): uint64 =
  ## MurmurHash2
  ## https://github.com/aappleby/smhasher/blob/master/src/MurmurHash2.cpp
  let m = 0xc6a4a7935bd1e995'u64
  let r = 47

  var h = seed xor uint64(uint64(len) * m)

  let blocks = cast[ptr uint64](key)
  let nblocks = len div sizeof(uint64)

  for i in 0 ..< nblocks:
    var k = blocks[i]
    # echo &"i,k: {i},{k}"
    k *= m
    k ^= k shr r
    k *= m
    h ^= k
    h *= m

  let tail = key + nblocks * sizeof(uint64)
  var state = len and 7
  while state > 0:
    case state:
    of 7: h ^= uint64(tail[6]) shl 48
    of 6: h ^= uint64(tail[5]) shl 40
    of 5: h ^= uint64(tail[4]) shl 32
    of 4: h ^= uint64(tail[3]) shl 24
    of 3: h ^= uint64(tail[2]) shl 16
    of 2: h ^= uint64(tail[1]) shl 8
    of 1:
      h ^= uint64(tail[0])
      h *= m
    of 0: break
    else:
      assert false
    dec(state)

  h ^= h shr r
  h *= m
  h ^= h shr r

  result = h
  # echo &"{bytes},{seed} -> {result}"

proc murmurHash64A*(x: string, seed = 0'u64): auto =
  murmurHash64A(cast[ptr uint8](x[0].unsafeAddr), x.len, seed)