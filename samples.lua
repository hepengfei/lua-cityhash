
cityhash = require("cityhash")

-- helper function
function hexlify(binstr)
   hexstr=""
   for i=1,#binstr do
      hexstr = hexstr .. string.format("%02x", binstr:byte(i))
   end
   return hexstr
end

function hexrevert(hexstr)
   hexrev=""
   for i=#hexstr,2,-2 do
      hexrev = hexrev .. hexstr:sub(i-1,i)
   end
   return hexrev
end

------ basic usage ------

-- cityhash64(text)
val = cityhash.cityhash64("text to hash")
print("cityhash64", #val, hexlify(val), val)

-- cityhash64(text, seed)
seed0 = 43
val = cityhash.cityhash64("text to hash", seed0)
print("cityhash64withseed", #val, hexlify(val), val)

-- cityhash64(text, seed0, seed1)
seed1 = 999
val = cityhash.cityhash64("text to hash", seed0, seed1)
print("cityhash64withseeds", #val, hexlify(val), val)

-- cityhash128(text)
val = cityhash.cityhash128("text to hash")
print("cityhash128", #val, hexlify(val), val)

-- cityhash128(text, seedlow64, seedhigh64)
seedlow64 = 43
seedhigh64 = 999
vallow64, valhigh64 = cityhash.cityhash128("text to hash", seedlow64, seedhigh64)
print("cityhash128withseed", #val, hexlify(val), val)

-- cityhash32(text)
val = cityhash.cityhash32("text to hash")
print("cityhash32", #val, hexlify(val), val)

-- hash128to64(hash128)
val128 = cityhash.cityhash128("text to hash")
val = cityhash.hash128to64(val128)
print("hash128to64", #val, hexlify(val), val)



------ advance usage ------

-- you can use hash128/hash64 value as seed(s). hash64 value for one
-- seed, hash128 value for two seeds.


-- cityhash64(text, seed_str_8bytes)
seed = cityhash.cityhash64("seed")
val = cityhash.cityhash64("text to hash", seed)
print("cityhash64withseed128", #val, hexlify(val), val)

-- cityhash64(text, seed0_str_8bytes, seed1_str_8bytes)
seed0 = cityhash.cityhash64("seed0")
seed1 = cityhash.cityhash64("seed1")
val = cityhash.cityhash64("text to hash", seed0, seed1)
print("cityhash64withseeds64", #val, hexlify(val), val)

-- cityhash64(text, seed_str_16bytes)
seed = cityhash.cityhash128("seed")
val = cityhash.cityhash64("text to hash", seed)
print("cityhash64withseed128", #val, hexlify(val), val)

-- cityhash128*text, seed_str_16bytes)
seed = cityhash.cityhash128("seed")
val = cityhash.cityhash128("text to hash", seed)
print("cityhash128withseed128", #val, hexlify(val), val)


-- one hash128 is same to two hash64
seed = cityhash.cityhash128("seed")


val1 = cityhash.cityhash128("text to hash", seed)
val2 = cityhash.cityhash128("text to hash", seed:sub(9,16), seed:sub(1,8))
print("cmp2:", hexlify(val1), "-", hexlify(val2))
assert(val1 == val2, "cityhash128 assert failed!")


val1 = cityhash.cityhash64("text to hash", seed)
val2 = cityhash.cityhash64("text to hash", seed:sub(1,8), seed:sub(9,16))
val3 = cityhash.cityhash64("text to hash", seed:sub(1,8))
print("cmp3:", hexlify(val1), "-", hexlify(val2), "-", hexlify(val3))
assert(val1 == val2, "cityhash64 assert1 failed!")
assert(val1 ~= val3, "cityhash64 assert2 failed!")
