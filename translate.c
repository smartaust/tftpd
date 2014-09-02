#include <sys/socket.h>
#include "translate.h"
	
int myrecvdata(int sock, Remote_information * remote)
{
	int ret ;
	int len ;
	len = sizeof(remote->remoteaddr);
	ret = recvfrom(sock ,remote->buf,BUFLEN,0,&(remote->remoteaddr),&len);
	return ret ;
	
}

int mysenddata(int sock ,Remote_information* remote,int buflen)
{
	int ret  ;
	ret = sendto(sock ,remote->buf,buflen,0,&(remote->remoteaddr),sizeof(remote->remoteaddr));
	return ret ;
}
