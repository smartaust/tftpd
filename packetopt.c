/**********************************************************
filename : packetopt.c
**********************************************************/
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "packetopt.h"

PACKET_OPT_TYPE  getoptcode(char *buf)  
{
	U16  optcode;
	memcpy(&optcode,buf,2);
	optcode = ntohs(optcode);
	return optcode;
}

int getRWRQparm(char *pfilename,char *model,char *buf)
{
	int len ;
	strcpy(pfilename,buf+2);
	len = strlen(buf+2);
	strcpy(model,buf+3+len);
	return 0;
}

int getAckparm(char *buf)
{
	U16 blocks ;
 	memcpy(&blocks,buf+2,2);
	blocks = ntohs(blocks);
	return blocks;
}
	
int getErrparm(U16 errno , char *errmsg) 
{	
	return 0;
	
}


int getDataparm(char *buf)
{
	U16  blocks ;
	memcpy(&blocks,buf+2,2);
	blocks = ntohs(blocks);
	return  blocks;
}	
int packetack(U16 blocks,char *buf)
{
	 U16   optcode = ACK ;
	 optcode = htons(optcode);
	 blocks= htons(blocks);
	 memcpy(buf,&optcode,2);	
	 memcpy(buf+2 ,&blocks ,2);
	 return 4;
}
	
int packetdata(U16 blocks , char *buf , int fd )
{ 		
	 int ret;
	 char data[1024] = {0};
	 U16  optcode = DATA ;
	 optcode = htons(optcode);
	 blocks  = htons(blocks);
         ret = read(fd ,data ,512);
	 memcpy(buf,&optcode,2);
	 memcpy(buf+2,&blocks,2);
	 memcpy(buf+4,data,ret);
	 return ret+4;
}

int packeterr(char *buf,int errnum) 
{
	
	U16  optcode = ERR;
 	ERR_TYPE errors;
	char *errmsg;

        if(errnum == ENOENT)
	{
	
		printf("file not exit\n");
		errors = FILE_UNFIND;
		errmsg = FILENOTFIND;
	}
	else if(errnum == EEXIST)
	{
		errors = FILE_EXIST;
		errmsg = FILEEXIST;
	}
	else if (errnum == EDQUOT)
	{
		errors = FILE_UNFIND;
		errmsg = FILENOTFIND;
		printf("file not eixist\n");
	}
	optcode = htons(optcode);
	errors = htons (errors);
	memcpy(buf, &optcode,2);
	memcpy (buf+2,&errors,2);
	strcpy(buf+4,errmsg);
	return strlen(errmsg)+4;


}


/*返回值是当前字符串长于转换之后串长的差值*/				
/*对接收到的包进行处理，将其行结束符转化成本机的行结束符*/
int packrtoh(char *buf,int len)
{
	int wide=0;
	const char *p = buf;
	char temp[1024] = {0};
	char *q = temp;
	int i = 0;
	while(i < len)
	{	
		if (*p == '\r')
		{
			if(*(p+1) == '\n')
			{
				*q = '\n';
				p += 2;
				q ++;
				i += 2;
				wide ++;
			}
			else
			{	
				*q = '\n';
				 p ++;
				 q ++;
				 i ++;
				 wide ++;
			}
			continue;
		}
		*q = *p;
		p ++;
		q ++;
		i ++;	
		wide ++;
			
	}
	memset(buf,0,len);	
	memcpy(buf,temp,wide);
	printf("leaving \n");
	return len -wide;
}
