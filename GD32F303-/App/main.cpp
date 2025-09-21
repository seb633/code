#include "main.h"
#include "appTaskOs.h"
#include "gd32f30x.h"

int main()
{
    nvic_vector_table_set(NVIC_VECTTAB_FLASH, 0x5000);
    TaskCreat();
}

