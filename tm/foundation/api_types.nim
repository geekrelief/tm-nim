import murmur2
import macros {.all.}
{.push hint[XDeclaredButNotUsed]:false.}
{.pragma: struct, bycopy, completeStruct.}
{.pragma: impapi_typesHdr, header: tm_headers_dir & "foundation/api_types.h".}

type
  tm_vec2_t* {.struct, impapi_typesHdr, importc: "struct tm_vec2_t".} = object
    x*, y*: cfloat

  tm_vec3_t* {.struct, impapi_typesHdr, importc: "struct tm_vec3_t".} = object
    x*, y*, z*: cfloat

  tm_vec4_t* {.struct, impapi_typesHdr, importc: "struct tm_vec4_t".} = object
    x*, y*, z*, w*: cfloat

  tm_mat44_t* {.struct, impapi_typesHdr, importc: "struct tm_mat44_t".} = object
    xx*, xy*, xz*, xw*, yx*, yy*, yz*, yw*, zx*, zy*, zz*, zw*, wx*, wy*, wz*, ww*: cfloat

  tm_transform_t* {.struct, impapi_typesHdr, importc: "struct tm_transform_t".} = object
    pos*, rot*, scl*: tm_vec3_t

  tm_rect_t* {.struct, impapi_typesHdr, importc: "struct tm_rect_t".} = object
    x*, y*, w*, h*: cfloat

  tm_str_t* {.struct, impapi_typesHdr, importc: "struct tm_str_t".} = object
    data*: cstring
    size*: uint32
    null_terminated*: uint32
  
  tm_clock_o* {.struct, impapi_typesHdr, importc: "struct tm_clock_o".} = object
    opaque*: uint64

  tm_uuid_t* {.struct, impapi_typesHdr, importc: "struct tm_uuid_t".} = object
    a*, b*: uint64

  tm_color_srgb_t* {.struct, impapi_typesHdr, importc: "struct tm_color_srgb_t".} = object
    r*, g*, b*, a*: uint8

  tm_tt_type_t* {.struct, impapi_typesHdr, importc: "struct tm_tt_type_t".} = object
    u64*: uint64

  tm_tt_id_t* {.struct, union, impapi_typesHdr, importc: "struct tm_tt_id_t".} = object
    u64*: uint64 
    `type`*: uint64
    generation*: uint64
    index*: uint64

  tm_tt_undo_scope_t* {.struct, impapi_typesHdr, importc: "struct tm_tt_undo_scope_t".} = object
    u64*: uint64

  tm_version_t* {.struct, impapi_typesHdr, importc:"struct tm_version_t".} = object
    major*, minor*, patch*: uint32 

  tm_strhash_t* {.importc.} = distinct uint64

proc tt_id*(`type`, generation, index: uint64): tm_tt_id_t {.inline.} = 
  result.`type` = `type`
  result.generation = generation
  result.index = index

converter to_tt_type*(id: tm_tt_id_t): tm_tt_type_t {.inline.} =
  tm_tt_type_t(u64: id.`type`)

proc TM_VERSION*(major, minor, patch: uint32): tm_version_t {.inline.} =
  tm_version_t(major: major, minor: minor, patch: patch)

template TM_PAD*(n: uint32) =
  padding: array[n, char]

macro TM_STATIC_HASH*(x: string, h: uint64 = 0): untyped =
  var hashLit = 
    if h.intVal == 0: 
      newLit(murmurHash64A(x.strVal)) 
    else: 
      h
  result = newTree(nnkCast, ident("tm_strhash_t"), hashLit)

const TM_PAGE_SIZE* = 4096

proc vec2*(x, y: float): tm_vec2_t {.inline.} =
  tm_vec2_t(x: x, y: y)

proc vec3*(x, y, z: float): tm_vec3_t {.inline.} =
  tm_vec3_t(x: x, y: y, z: z)

proc vec4*(x, y, z, w: float): tm_vec4_t {.inline.} =
  tm_vec4_t(x: x, y: y, z: z, w: w)

proc mat44*(xx, xy, xz, xw,
              yx, yy, yz, yw,
              zx, zy, zz, zw,
              wx, wy, wz, ww: float): tm_mat44_t {.inline.} =
  tm_mat44_t(
    xx: xx, xy: xy, xz: xz, xw: xw,
    yx: yx, yy: yy, yz: yz, yw: yw,
    zx: zx, zy: zy, zz: zz, zw: zw,
    wx: wx, wy: wy, wz: xw, ww: ww)

converter toStr*(s: string): tm_str_t {.inline.} =
  tm_str_t(data: s, size: (uint32)s.len, null_terminated: 1)

# TRGB where T is transparency (0x00 is fully opaque and 0xff fully transparent).
proc trgb*(c: uint32): tm_color_srgb_t {.inline.} =
  tm_color_srgb_t(
    r: uint8(0xff and (c shr 16)), 
    g: uint8(0xff and (c shr 8)), 
    b: uint8(0xff and c), 
    a: uint8(0xff - (0xff and (c shr 24))))
