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
