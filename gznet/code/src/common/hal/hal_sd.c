
#include "hal.h"
#include "platform/platform.h"
#include "common/fats/ff.h"

static uint32_t hwrite = 0;	//已写入次数

static char_t *strtok_s(register char_t *s, register const char_t *delim, int lay) {
	static char_t *last;
	char_t *tok;
	char_t *ucdelim;

	/*(1)s为空，并且上次剩余值也为空，则直接返回NULL，否则s为last或当前值中有值的一方*/
	/*if (s == NULL && (s = last) == NULL)
			return NULL;*/
	/*(2)以上内容与下面等价：
			 下面意为：如果s==NULL的时候，s则为last（上次执行该项的时候的剩余值）；
			 若这时候s还为空（即上次没有剩余），则返回NULL。
	 */

	if (s == NULL)
		s = last;
	if (s == NULL)
		return NULL;

	/*(3)也意为：若后续值为NULL则不进行操作，否则继续操作。
			 其中，后续值的界定方法为: 若last不为空的时候，则s为last，否则为输入值
	 * */
	/*
	if(last != NULL)
			s = last;
	else if(s == NULL)
			return NULL;*/

	tok = s;

	int found = 0;
	while (lay>0 && *s != '\0')
	{
		ucdelim = (char *) delim;
		while (*ucdelim)
		{
			if (*s == *ucdelim)
			{
				found ++;
				if (lay==1)
				{
					*s = '\0';
					last = s + 1;
				} else
				{
					found=0;
					lay--;
				}
				break;
			}
			if (*s == '.')
			{
				return NULL;
			}
			ucdelim++;
		}

		if (!found)
		{
			s++;
			if (*s == '\0')
			{
				return NULL;
			}
		}
	}

	return tok;
}

static void memcpy_s(void *dst, const void *const src, uint16_t len)
{
	uint8_t *tmp = (uint8_t *)dst;
	uint8_t *s = (uint8_t *)src;

	while (len--)
	{
		*tmp++ = *s++;
	}
}

static int strlen_s(const char_t * p) {
	int i=0;
	while (*p!='\0')
	{
		i++;
		p++;
	}
	return i;
}

static char_t *strcat_s(char_t *str1, char_t *str2)
{
	char_t *pt;
	pt = str1;
	if ((str1!=NULL)||(str2!=NULL))
	{
		while (*str1!='\0')	str1++;
		while (*str2!='\0')	*str1++ = *str2++;
		*str1 = '\0';
	}

	return pt;
}

bool_t hal_mkdir(const char_t *path)
{
	char_t separator[] ="/";
	char_t *result = NULL;
	FRESULT res;
	char_t pth[50]={0};
	int lay=0;
	do
	{
		lay++;
		memcpy_s(pth,path,strlen_s(path));
		result = strtok_s( pth, separator, lay);
		if (result != NULL)
		{
			res = f_mkdir(result);
			if (res!=FR_OK && res!=FR_EXIST)
			{
				return FALSE;
			}
		}
	} while (result != NULL);
	return TRUE;
}

bool_t hal_fmount(uint8_t vol, FATFS *fs)
{
	FRESULT res;
	res=f_mount(vol,fs);
	if (res!=FR_OK)
	{
		return FALSE;
	}
	return TRUE;
}

bool_t hal_fopen(FIL *fp, const char_t *path, uint8_t mode)
{
	//创建文件路径
	FRESULT res;
	if (!hal_mkdir(path))
	{
		return FALSE;
	}
	//打开文件
	res=f_open(fp,path,mode);
	if (res!=FR_OK)
	{
		return FALSE;
	}

	return TRUE;
}

bool_t hal_fclose(FIL *fp)
{
	FRESULT res;
	res=f_close(fp);
	if (res!=FR_OK)
	{
		return FALSE;
	}
	return TRUE;
}

bool_t hal_freadline(FIL *fp, void *buf ,uint16_t bufsize, uint16_t *hr_num)	//常用读取一行
{
	FRESULT res;
	uint16_t br;
	uint8_t ch[2]={0};
	uint16_t size=bufsize;
	uint8_t *pbuf=(uint8_t *) buf;
	bool_t mark = TRUE;
	do
	{
		if (size == 0)
		{
			return FALSE;
		}
		res=f_read(fp,pbuf,1,&br);
		*hr_num += br;
		if (res!=FR_OK)
		{
			return FALSE;
		}

		if (*pbuf != 0x0D && *pbuf!= 0x0A)
		{
			if (mark==FALSE)
			{
				mark = TRUE;
			}
		} else
		{
			if (mark==TRUE)
			{
				ch[0]=0x0D;
				mark=FALSE;
			} else
			{
				ch[1]=0x0A;
				*pbuf=0x00;
				*(--pbuf)=0x00;
			}
		}
		pbuf++;size--;
	} while ((ch[1]!=0x0A));
	*hr_num-=2;
	return TRUE;
}

bool_t hal_fwriteline(FIL *fp, const void *buf, uint16_t w_num, uint16_t *hw_num)	////常用写入一行
{
	FRESULT res;
	TCHAR ch='\n';
	res=f_write(fp,buf,w_num,hw_num);
	if (res!=FR_OK)
	{
		return FALSE;
	}
	if (!f_putc(ch,fp))
	{
		return FALSE;
	}
#if FSYNC
	if (hwrite == FSYNC_NUM)
	{
		if (!hal_fsync(fp))
		{
			return FALSE;
		}
		hwrite = 0;
	}
	hwrite++;
#else
	if (!hal_fsync(fp))
	{
		return FALSE;
	}
#endif
	return TRUE;
}

bool_t hal_fread(FIL *fp, void *buf, uint16_t r_num, uint16_t *hr_num)
{
	FRESULT res;
	res=f_read(fp,buf,r_num,hr_num);
	if (res!=FR_OK)
	{
		return FALSE;
	}
	return TRUE;
}

bool_t hal_fwrite(FIL *fp, const void *buf, uint16_t w_num, uint16_t *hw_num)
{
	FRESULT res;
	res=f_write(fp,buf,w_num,hw_num);
	if (res!=FR_OK)
	{
		return FALSE;
	}
	return TRUE;
}

bool_t hal_fsync(FIL *fp)
{
	FRESULT res;
	res=f_sync(fp);
	if (res!=FR_OK)
	{
		return FALSE;
	}
	return TRUE;
}

bool_t hal_flseek(FIL *fp, uint32_t ofs)
{
	FRESULT res;
	res=f_lseek(fp, ofs);
	if (res!=FR_OK)
	{
		return FALSE;
	}
	return TRUE;
}

bool_t hal_fgetfree(FATFS *fs, uint32_t *free)
{
	//uint32_t tot=0;
	uint32_t fre_clust=0;
	FATFS *f;
	FRESULT res;
	f = fs;
	res=f_getfree("/", &fre_clust, &f);
	if (res!=FR_OK)
	{
		return FALSE;
	}
	//tot = (fs->n_fatent - 2) * fs->csize;	//磁盘容量
	*free = (fre_clust * fs->csize);
	return TRUE;
}

bool_t hal_fremove(const char_t *path)
{
	FRESULT res;
	res=f_unlink(path);
	if (res!=FR_OK)
	{
		return FALSE;
	}
	return TRUE;
}


bool_t hal_ftimestamp(sd_time_t *timestamp)
{
	set_fattime(timestamp);
//  FILINFO fno;
//  FRESULT res;
//  fno.fdate = (WORD)(((timestamp->year - 1980) * 512U) | timestamp->month * 32U | timestamp->day);
//  fno.ftime = (WORD)(timestamp->hour * 2048U | timestamp->min * 32U | timestamp->sec / 2U);
//  res = f_utime(path, &fno);
//  if (res!=FR_OK)
//  {
//  	return FALSE;
//  }
	return TRUE;
}

bool_t hal_fremovedir(const char_t *path)
{
	FRESULT res;
	DIR dj;
	FILINFO fno;
	uint8_t len=strlen_s(path);
	res=f_opendir(&dj,path);
	do
	{
		char_t pth[50]={0};
		memcpy_s(pth,path,len);
		res=f_readdir(&dj,&fno);
		strcat_s(pth,"/");
		strcat_s(pth,fno.fname);
		res=f_unlink(pth);
	} while ((res==0));
	res=f_unlink(path);
	return TRUE;
}

bool_t hal_sd_init()
{//SD卡初始化
	return sd_init();
}
