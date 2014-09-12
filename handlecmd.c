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
#include <unistd.h>
#include "packetopt.h"
#include "translate.h"
#define  MAXTIME  5 
static  void handle_rrq(Remote_information *premote);
static  void handle_wrq(Remote_information *premote);
static  int  myselect(int sock,int timeout);

void  handlecmd(PACKET_OPT_TYPE cmdtype,Remote_information *premote)
{
	
	int ret ;
	int sock = socket(AF_INET,SOCK_DGRAM,0);//请求码不正确,同过该socket发送错误信息 
	switch(cmdtype)
		{
			case RRQ :
				handle_rrq(premote);
				break;

			case WRQ :
				handle_wrq(premote);
				break;
      
			default  :
				ret = packeterr(premote->buf,UNKNOWERR);
				ret = mysenddata(sock,premote,ret);
				printf("UNKNOWN CMD\n");
				break;
		}
}

static  void handle_rrq(Remote_information *premote) 
{
	int retranstime = 0;
	int flag = 0;
	unsigned short int  block = 1;
	int sock;
	int len;
	int ret;
	int fd;
	char filename[30] = {0};
	char model[10] = {0};
	PACKET_OPT_TYPE opttype;

	sock = socket(AF_INET,SOCK_DGRAM,0);
	getRWRQparm (filename,model,premote->buf);
	fd = open(filename ,O_RDONLY,0666);
	if(fd < 0)
	{
	
		memset(premote->buf,0,sizeof(premote->buf));
	        len = packeterr(premote->buf,errno);
		ret = mysenddata(sock,premote,len);
		printf("open file erro %s\n",strerror(errno));
		return;
	}

	while(1)
	{
		memset(premote->buf,0,BUFLEN);
        	len = packetdata(block, premote->buf,fd);
		ret = mysenddata(sock,premote,len);
		if (0 < ret && ret < 516)
		{	
			flag = 1;
			printf("the last packet\n");
		}

		ret = myselect(sock,3);
		if (ret <=0)
		{
			if(retranstime == MAXTIME)
			{
				printf("unreached\n");
				return;
			}
			
			retranstime ++;
			continue;
		}


		ret  = myrecvdata(sock,premote);

		if(ret >0)
		{
		  opttype = getoptcode(premote->buf);
		  switch(opttype)
		       {
				case  ACK :
					if(block == getAckparm(premote->buf))
					{
						block++;
					}
					break;
				case  ERR  :
					printf("we really can got error packet\n");
					break;
				default :
					break;

			}
		 
		}		
		if(flag)
		 {
			close(fd);
			close(sock);
			printf("translate complete...\n");
			break;
		 }
	}
		
	
	printf("RRQ\n");
}


static  void handle_wrq(Remote_information *premote) 
{
	unsigned short int  block = 0;//记录数据包的编号
	int sock;
	int len;
	int ret;
	int fd;
	int times = 0;//记录超时次数
	int flag = 0;//定义一个标志，判断是否是最后一个数据包
	char filename[30]= {0};//暂存文件名
	char model[10] = {0};//暂存模式octet 和 netascii模式	
	char isasciimodel;
	int  pret;

	sock = socket(AF_INET,SOCK_DGRAM,0);
	getRWRQparm(filename,model,premote->buf);
	printf("model:%s\n",model);
	isasciimodel = !strcmp(model,"netascii");
	fd = open(filename,O_WRONLY|O_CREAT,0666);

	if(fd < 0)
	{
		memset(premote->buf,0,sizeof(premote->buf));
		len = packeterr(premote->buf,UNKNOWERR);
		ret = mysenddata(sock,premote,len);	
		close(sock);
		printf("open file erro %d\n",ret);		
		return;
	}

	while(1)
	{
		pret = 0;
		/*判断收到的包的序号是否正确 */
		if(0 != block)
		{
			ret  = myselect(sock,3);
			if(ret <= 0)	
			{
				if (times == MAXTIME)
				{
					printf("file recv %s\n",filename);
					unlink(filename);
					break ;
				}
				times ++;
				continue;
			}
			
			len  = myrecvdata(sock,premote);
			if(len < 0 )
			{
				printf("recv errror\n");
				continue;
			}
			
			if(getDataparm(premote->buf) != block)
			{
				printf("this is a erro packet, abandon it\n");
		                memset(premote->buf,0,BUFLEN);
				continue;			
			}
		
			if(isasciimodel)
			{
				printf("do you working\n");
				pret=packrtoh(premote->buf+4,len -4);
				
			}	
		        ret = write(fd,premote->buf+4,len -4-pret);
			if(ret == -1)
			{
				printf("write error\n");
				return;
			}

			if( len > 0 && len < 516)
			{
		  		flag = 1;
			}		
		}	

		memset(premote->buf,0,BUFLEN);
        	len = packetack(block,premote->buf);
		ret = mysenddata(sock,premote,len);

		if(flag)	
		{
			close(fd);
			close(sock);
			break;
		}
		block ++;
	}
		
	printf("WRQ\n");
}


static  int  myselect(int sock,int timeout) 
{
	
	int ret;
	fd_set fds;
        struct timeval  tm;
        tm.tv_sec = timeout;
        tm.tv_usec = 0;
        FD_ZERO(&fds);
        FD_SET(sock, &fds);
	ret = select(sock+1,&fds,NULL,NULL,&tm);
	return ret;
}
