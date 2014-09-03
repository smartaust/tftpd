/**********************************************************
filename : packetopt.c
**********************************************************/
#include <arpa/inet.h>
#include <string.h>
#include "packetopt.h"

PACKET_OPT_TYPE  getoptcode(char *buf)  
{
	
	U16  optcode ;
	memcpy(&optcode ,buf,2) ;
	optcode = ntohs(optcode);
	return optcode ;
}

int getRWRQparm (char *pfilename,char *model,char *buf)
{
	int len ;
	strcpy(pfilename,buf+2);
	len = strlen(buf+2);
	strcpy(model,buf+3+len);
	return 0 ;
}

int getAckparm  (char * buf)
{
	U16 blocks ;
 	memcpy(&blocks,buf+2,2);
	blocks = ntohs(blocks);
	return blocks ;
}
	
int getErrparm  (U16 errno , char *errmsg) 
{	
	return 0 ;
	
}


int getDataparm (char * buf)
{
	U16  blocks ;
	memcpy(&blocks , buf+2 , 2);
	blocks = ntohs(blocks);
	return  blocks ;
}	
int packetack (U16 blocks,char *buf)
{
	 U16   optcode = ACK ;
	 optcode = htons(optcode) ;
	 blocks= htons(blocks);
	 memcpy(buf,&optcode,2);	
	 memcpy(buf+2 ,&blocks ,2);
	 return 4 ;
}
	
int packetdata (U16 blocks , char *buf , int fd )
{ 		
	 int ret ;
	 char data[1024] = {0}  ;
	 U16  optcode = DATA ;
	 optcode = htons(optcode);
	 blocks  = htons(blocks);
         ret = read(fd ,data ,512);
	 memcpy(buf,&optcode,2);
	 memcpy(buf+2,&blocks,2);
	 memcpy(buf+4,data,ret);
	 return ret+4 ;
}

int packeterr (char *buf , ERR_TYPE errtype , char * errmsg) 
{
	U16  optcode  =  ERR ;
	U16  errno = errtype ;
	optcode = htons(optcode);
	errno = htons (errno);
	memcpy(buf, &optcode,2);
	memcpy (buf+2,&errno,2);
	strcpy(buf+4,errmsg);
	return strlen(errmsg) + 4;
}
