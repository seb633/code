#include "func.h"
#include "arch.h"

UINT16 CalcCrc(UINT8 *buf, UINT16 len)
{
    UINT16 crcIn = 0xFFFF;
    UINT8 Char = 0;
    volatile UINT16 t;

    while(len--)
    {
        Char = *(buf++);
        crcIn = crcIn ^ Char;
        for(UINT8 i = 0; i<8; i++)
        {
            t = crcIn & 0x0001;
            if(t != 0)
            {
                crcIn = (crcIn >> 1) ^ 0xA001;
            }
            else
            {
                crcIn = crcIn >> 1;
            }
        }
    }
    return crcIn;

}
UINT16 IsEqual(FLOAT32 a, FLOAT32 b, FLOAT32 gap)
{
    FLOAT32 temp;

    temp = ARCH_Abs(a - b);
    return (temp < gap);
}

#define		CRC_POLY_16		        (0xA001)
#define		CRC_START_MODBUS	    (0xFFFF)

static UINT16 crc_tab16_init = 0;
static UINT16 crc_tab16[256];

static void init_crc16_tab( void ) 
{
	UINT16 i;
	UINT16 j;
	UINT16 crc;
	UINT16 c;

	for (i=0; i<256; i++) 
    {
		crc = 0;
		c   = i;
		for (j=0; j<8; j++) 
        {
			if ( (crc ^ c) & 0x0001 )
            {
                crc = ( crc >> 1 ) ^ CRC_POLY_16;
            }
			else
            {
                crc =   crc >> 1;
            }
			c = c >> 1;
		}
		crc_tab16[i] = crc;
	}
	crc_tab16_init = 1;
}  /* init_crc16_tab */

UINT16 crc_modbus(UINT8 *input_str, UINT16 num_bytes ) 
{
	UINT16 crc;
	UINT16 tmp;
	UINT16 short_c;
	const unsigned char *ptr;
	UINT16 a;
	if ( ! crc_tab16_init )
    {
        init_crc16_tab();
    }
	crc = CRC_START_MODBUS;
	ptr = input_str;
	if ( ptr != 0 )
    {
        for (a=0; a<num_bytes; a++) 
        {
            short_c = 0x00ff & ((UINT16) (*ptr));
            tmp     =  crc       ^ short_c;
            crc     = (crc >> 8) ^ crc_tab16[ tmp & 0xff ];
            ptr++;
        }
    }
	return crc;
}


