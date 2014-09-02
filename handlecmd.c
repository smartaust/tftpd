/*************************************************
*filename : handlecmd.c
*function : cmd handle 
**************************************************/
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "packetopt.h"
#include "translate.h"

static  void handle_rrq(Remote_information * premote) ;
static  void handle_wrq(Remote_information * premote) ;

void  handlecmd(PACKET_OPT_TYPE cmdtype,Remote_information *premote)
{
	switch(cmdtype)
		{
			case RRQ :
				handle_rrq(premote) ;
				break ;
			case WRQ :
				handle_wrq(premote);
				break ;
	/*		case DATA :
				printf("DATA\n");
				break ;
			case ACK :
				printf("ACK\n");
				break ;	
			case ERR :
				printf("ERR\n");
				break ;
         */
			default  :
				printf("UNKNOWN CMD\n");
				break ;
		}
}

static  void handle_rrq(Remote_information * premote) 
{
	int flag = 1 ;
	unsigned short int  block = 1 ;
	int sock ;
	int len ;
	int ret ;
	int fd ;
	char filename[30]= {0};
	char model[10] = {0} ;
	PACKET_OPT_TYPE opttype ;

	sock = socket(AF_INET,SOCK_DGRAM,0);
	getRWRQparm (filename,model,premote->buf) ;
	fd = open(filename ,O_RDONLY,0666 );
	if(fd < 0)
	{	
		printf("open file erro \n");
		return ;
	}
	while(1)
	{
		memset (premote->buf,0,BUFLEN) ;
        	len = packetdata(block, premote->buf,fd) ;
		ret = mysenddata(sock,premote,len);
		if ( 0 < ret && ret < 516)
		{	
			flag = 0 ;
			printf("the last packet\n");
		}
		memset(premote->buf,0 ,BUFLEN) ;
		ret  = myrecvdata(sock,premote);

		if(ret >0)
		{
		  opttype = getoptcode(premote->buf);
		  switch(opttype)
		       {
				case  ACK :
					if(block == getAckparm(premote->buf))
					{
						printf("ack :%d\n",getAckparm(premote->buf));
						block++ ;
					}
					break ;
				case  ERR  :
					break ;
				default :
					break ;

			}
		 
		}		
		if(flag == 0)
		 {
			close(fd);
			close(sock);
			printf("translate complete...\n");
			break ;
		 }
	}
		
	
	printf("RRQ\n");
}


 static  void handle_wrq(Remote_information * premote) 
{
	unsigned short int  block = 0 ;//记录数据包的编号
	int sock ;
	int len ;
	int ret ;
	int fd ;
	char filename[30]= {0};//暂存文件名
	char model[10] = {0} ;//暂存模式octet 和 netascii模式

	sock = socket(AF_INET,SOCK_DGRAM,0);
	getRWRQparm (filename,model,premote->buf) ;

	fd = open(filename ,O_WRONLY|O_CREAT,0666 );
	if(fd < 0)
	{	
		printf("open file erro \n");
		return ;
	}
	while(1)
	{
		memset (premote->buf,0,BUFLEN) ;
        	len = packetack(block, premote->buf) ;
		ret = mysenddata(sock,premote,len);
		block ++ ;
		memset(premote->buf,0 ,BUFLEN) ;
		ret  = myrecvdata(sock,premote);
		ret = write(fd ,premote->buf+4,ret -4);
		if( ret > 0 && ret < 512)
		{
		      	memset (premote->buf,0,BUFLEN) ;
        	      	len = packetack(block, premote->buf) ;
		  	ret = mysenddata(sock,premote,len);
			close(fd);
	          	close(sock);
		  	break ;
		}		
	}
		
	printf("WRQ\n");
}


