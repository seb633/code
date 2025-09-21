#ifndef _USER_TYPES_H_
#define _USER_TYPES_H_

#ifdef __cplusplus
extern "C"
{
#endif

typedef char INT8;
typedef unsigned char UINT8;
typedef short INT16;
typedef unsigned short UINT16;
typedef long INT32;
typedef unsigned long UINT32;
typedef float FLOAT32;

typedef union
{
    FLOAT32 valFloat;
    struct
    {
      UINT16 byte0 : 8;
      UINT16 byte1 : 8;
      UINT16 byte2 : 8;
      UINT16 byte3 : 8;      
    }byte;
    
    INT32 valLong;
    UINT32 valULong;
}B4_B1_Union;

typedef union
{
    UINT16 valShort;
    struct
    {
      UINT16 byte0 : 8;
      UINT16 byte1 : 8;    
    }byte;
}B2_B1_Union;


typedef struct
{
    UINT8 jump_flag;
    UINT8 update_flag;
    UINT8 boot_info[8];
    UINT8 app_info[8];
    UINT8 other_info[256-8-8-2];
}RamDateType;

typedef void (*Void_Func_Void)();        //无返回、无参数函数指针
#define INLINE_USER  inline
#define FORCE_INLINE_USER  __forceinline


#ifdef __cplusplus
}
#endif
#endif
