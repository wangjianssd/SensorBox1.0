/**
 * @brief       : 实现string库函数的部分函数功能  
 *
 * @file        : gprs_str.c 
 * @author      : zhangzhan
 * @version     : v0.1
 * @date        : 2015/9/15
 *
 * Change Logs  : 
 *
 * Date           Version      Author      Notes
 * - 2015/9/15    v0.0.1      zhangzhan    文件初始版本
 */
#include <gznet.h> 
//#include <debug.h>

/**
  *实现strstr函数功能
  *
  *@param: 两个字符串
  *@return: 返回在str中出现sub_str之后的字符串。
  *
  */
char const *my_strstr(const char *str, const char *sub_str)  
{
    DBG_ASSERT(str != NULL __DBG_LINE);
    DBG_ASSERT(sub_str != NULL __DBG_LINE);

    for(int i = 0; str[i] != '\0'; i++)  
    {  
        int tem = i;    
        int j = 0;  
        
        while(str[i++] == sub_str[j++])  
        {  
            if(sub_str[j] == '\0')  
            {  
                return &str[tem];  
            }  
        }  
        i = tem;  
    }  
    return NULL;  
} 

int mystrlen(const char *str)
{
    DBG_ASSERT(str != NULL __DBG_LINE);
    
    int len = 0;
    
    while ((*str++) != '\0')
    {
        len++;
    }
    
    return len;
}
