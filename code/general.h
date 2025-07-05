#include <stdint.h>

#define internal static 
#define local_persist static 
#define global_variable static 
#define Pi32 3.1415926535f
#define WrapColor(Value) ((uint8)(((Value) % 256 + 256) % 256))

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;