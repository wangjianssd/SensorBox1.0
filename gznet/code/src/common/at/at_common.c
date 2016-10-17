#include "at_common.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
uint8_t ustrtok(char *src, char **des, char *delim)
{
    uint8_t i = 0;
    char *p = src;
    while( (des[i] = strtok(p, delim)) != NULL )
    {
        i++;
        p = NULL;
    }
    return i;
}

uint8_t my_strlen(const uint8_t *str)
{
    uint8_t len = 0;

    while(*str++ != '\0')
    {
        len++;
    }
    return len;
}

bool_t ascii_to_hex(uint8_t hi, uint8_t lo, uint8_t *hex)
{
	*hex = 0;
	if ((hi >= 0x30) && (hi <= 0x39))
	{
		hi -= 0x30;
	}
	else if ((hi >= 0x41) && (hi <= 0x46))
	{
		hi -= 0x37;
	}
	else
	{
		return FALSE;
	}
	*hex |= (hi << 4);

	if ((lo >= 0x30) && (lo <= 0x39))
	{
		lo -= 0x30;
	}
	else if ((lo >= 0x41) && (lo <= 0x46))
	{
		lo -= 0x37;
	}
	else
	{
		return FALSE;
	}
	*hex |= lo;

	return TRUE;
}

bool_t hex_to_ascii(uint8_t *buf, uint8_t dat)
{
	uint8_t dat_buff;

	dat_buff = dat;
	dat = dat & 0x0f;
	if (dat <= 9)
	{
		dat += 0x30;
	}
	else
	{
		dat += 0x37;
	}

	buf[1] = dat;

	dat = dat_buff;
	dat >>= 4;
	dat = dat & 0x0f;
	if (dat <= 9)
	{
		dat += 0x30;
	}
	else
	{
		dat += 0x37;
	}
	buf[0] = dat;
	return TRUE;
}
