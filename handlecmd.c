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
#include <errno.h>
#include "packetopt.h"
#include "translate.h"

static  void handle_rrq(Remote_information * premote) ;
static  void handle_wrq(Remote_information * premote) ;

void  handlecmd(PACKET_OPT_TYPE cmdtype,Remote_information *premote)
{
	
	int ret ;
	int sock = socket(AF_INET,SOCK_DGRAM,0);
	switch(cmdtype)
		{
			case RRQ :
				handle_rrq(premote) ;
				break ;

			case WRQ :
				handle_wrq(premote);
				break ;
      
			default  :
				ret = packeterr(premote->buf,UNKNOWERR,UNKNOWNERROR);
				ret = mysenddata(sock,premote,ret);
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
	
		if(errno == ENOENT)
		{
			printf("file not exit\n");
			memset(premote->buf,0 ,sizeof(premote->buf));
			len = packeterr(premote->buf,FILE_UNFIND,FILENOTFIND);
		        ret = mysenddata(sock,premote,len);
		}
		else if(errno == EEXIST)
		{
			memset(premote->buf,0 ,sizeof(premote->buf));
			len = packeterr(premote->buf,FILE_EXIST,FILEEXIST);
		        ret = mysenddata(sock,premote,len);		
		}
		else if (errno == EDQUOT)
		{
			printf("file not eixist\n");
			memset(premote->buf,0 ,sizeof(premote->buf));
			len = packeterr(premote->buf,FILE_UNFIND,FILENOTFIND);
		        ret = mysenddata(sock,premote,len);
		}	
		printf("open file erro %s\n",strerror(errno));
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
						block++ ;
					}
					break ;
				case  ERR  :
					printf("we really can got error packet\n");
					break ;
				default :
					break ;

			}
		 
		}		
		if( 0 == flag )
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
	int flag =  0 ;//定义一个标志，判断是否是最后一个数据包
	char filename[30]= {0};//暂存文件名
	char model[10] = {0} ;//暂存模式octet 和 netascii模式

	sock = socket(AF_INET,SOCK_DGRAM,0);
	getRWRQparm (filename,model,premote->buf) ;
	fd = open(filename ,O_WRONLY|O_CREAT,0666 );

	if(fd < 0)
	{
		memset(premote->buf,0 ,sizeof(premote->buf));
		len = packeterr(premote->buf,UNKNOWERR,UNKNOWNERROR);
		ret = mysenddata(sock,premote,len);	
		close(fd) ;
		close(sock);
		printf("open file erro %d\n",ret);		
		return ;
	}

	while(1)
	{
		/*判断收到的包的序号是否正确 */
		if(0 != block)
		{
			len  = myrecvdata(sock,premote);
			ret = getDataparm(premote->buf);

			if(ret != block)
			{
				printf("this is a erro packet, abandon it\n");
		                memset(premote->buf,0 ,BUFLEN) ;
				continue ;			
			}

		        ret = write(fd ,premote->buf+4,len -4);

			if( ret > 0 && ret < 512)
			{
		      		memset (premote->buf,0,BUFLEN) ;
		  		flag = 1 ;
			}		
		}	

		memset (premote->buf,0,BUFLEN) ;
        	len = packetack(block, premote->buf) ;
		ret = mysenddata(sock,premote,len);
		block ++ ;

		if(flag)	
		{
			close(fd);
			close(sock);
			break ;
		}
	}
		
	printf("WRQ\n");
}


