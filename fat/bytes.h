#ifndef __FAT_BYTES_H_INCLUDED__
#define __FAT_BYTES_H_INCLUDED__

#define __LE8(tp, ptr, op) ( (tp) op(ptr, 0) )

#define __LE16(tp, ptr, op) ( ((tp) op(ptr, 0)) | (((tp) op(ptr, 1)) << 8) )
#define __LE32(tp, ptr, op) ( \
        ((tp) op(ptr, 0)) | (((tp) op(ptr, 1)) << 8) | \
        (((tp) op(ptr, 2)) << 16) | (((tp) op(ptr, 3)) << 24) )
#define __LE64(tp, ptr) ( \
        ((tp) op(ptr, 0)) | (((tp) op(ptr, 1)) << 8) | \
        (((tp) op(ptr, 2)) << 16) | (((tp) op(ptr, 3)) << 24) \
        (((tp) op(ptr, 4)) << 32) | (((tp) op(ptr, 5)) << 40) \
        (((tp) op(ptr, 6)) << 48) | (((tp) op(ptr, 7)) << 56) )

#define __DEREFERENCE(ptr, i) ((ptr)[i])
#define __DEREFERENCE_MOVE(ptr, i) (*((ptr)++))

#define _LE_uint8_t(op, ptr) __LE8(uint8_t, ptr, op)
#define _LE_int8_t(op, ptr) __LE8(int8_t, ptr, op)
#define _LE_uint16_t(op, ptr) __LE16(uint16_t, ptr, op)
#define _LE_int16_t(op, ptr) __LE16(int16_t, ptr, op)
#define _LE_uint32_t(op, ptr) __LE32(uint32_t, ptr, op)
#define _LE_int32_t(op, ptr) __LE32(int32_t, ptr, op)
#define _LE_uint64_t(op, ptr) __LE64(uint64_t, ptr, op)
#define _LE_int64_t(op, ptr) __LE64(int64_t, ptr, op)

#define __GET(endian, type, ptr) _##endian##_##type(__DEREFERENCE, ptr)
#define __READ(endian, type, ptr) _##endian##_##type(__DEREFERENCE_MOVE, ptr)

#define GETLE(type, ptr) __GET(LE, type, ptr)
#define READLE(type, ptr) __READ(LE, type, ptr)


#define CHECK_MASK(v, mask) (((v) & (mask)) == (mask))

typedef unsigned int uint;

#endif
