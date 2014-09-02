#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "translate.h"
#include "packetopt.h"
#include "handlecmd.h"

/*定义处理客户端发来的申请线程处理函数*/
void *Handle_Apply (void * arg)
{
	PACKET_OPT_TYPE opttypes;
        Remote_information remote = *(Remote_information *) arg ;		
	opttypes = getoptcode (remote.buf) ;
	handlecmd(opttypes,&remote);
}


/*生成一个与本地69端口绑定的socket*/
static int  getlistensock() 
{	
		int ret;
		int sock ;	
		struct sockaddr_in lisaddr ;
		bzero(&lisaddr,sizeof(lisaddr));
		lisaddr.sin_family = AF_INET;
		lisaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	      //lisaddr.sin_addr.s_addr = inet_addr("192.168.4.182");
		lisaddr.sin_port = htons(69); 
		
		sock = socket(AF_INET, SOCK_DGRAM ,0);
		if(sock < 0 )
			return -1  ;
		ret = bind(sock, (struct sockaddr * )&lisaddr , sizeof (lisaddr));
		if(ret < 0)
			return -2 ;
		return sock;
}


int main()
{
	int len ;
	int ret ;
	int sock;
	fd_set fdset ;
	pthread_t pid ;
	char buf[1024] = {0} ;
	Remote_information remote ;

	bzero(&remote,sizeof(remote));
	len = sizeof(len);
	sock  = getlistensock(); /*get a UDP type sock decriptor*/
	if(sock < 0)	
	{
		printf("socket no\n");
		return 0;
	}
	
	while(1)
         {
		FD_ZERO(&fdset);
		FD_SET(sock, &fdset);
		ret = select(sock +1 ,&fdset ,NULL ,NULL,NULL);
		if(ret> 0 )
		{
			
			ret = myrecvdata(sock,&remote);
			if(ret < 0 )
			{
				printf("error\n");
			}
			else
			{			
				ret = pthread_create(&pid ,NULL,Handle_Apply,&remote);
				if(ret < 0)
				{
		                        //TODO:添加创建线程出错处理代码 
					printf("error\n");
					continue ;
				}
			}
		}
		else
		{

			
			printf("server died , restart...\n");
			
		}
	   }			

}
