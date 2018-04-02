
#ifndef __INITSOCKET_H__
#define __INITSOCKET_H__

class CInitSocket
{
public:
	CInitSocket()
	{
		//加载网络
		WSADATA WSAData;
		if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0)
		{
			printf("加载网络失败!");
		}
	}

	~CInitSocket()
	{
		WSACleanup();
	}

};

#endif