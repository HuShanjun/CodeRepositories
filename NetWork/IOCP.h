#pragma once

#include <list>

//////////////////////////////////////////////////////////////////////////
//结构定义
typedef struct _PER_IO_OPERATION_DATA
{
	OVERLAPPED   Overlapped;
	WSABUF       WSABuff;
	SOCKET       socket;
	int          cbOperationType;
	char         cbDataBuff[4096];
}PER_IO_OPERATION_DATA,* LPPER_IO_OPERATION_DATA;

//////////////////////////////////////////////////////////////////////////

typedef std::list<LPPER_IO_OPERATION_DATA> CIOOperationItemList;
class CTCPNetWorkEngine;

//////////////////////////////////////////////////////////////////////////
class CIOCP
{
public:
	SOCKET                          m_hServerSocket;                    //连接句柄
	HANDLE                          m_lIOCPHandle;                      //连接句柄
	WORD							m_wServicePort;						//监听端口
	CIOOperationItemList            m_IOSendOperationItemList;
	CIOOperationItemList            m_IORecvOperationItemList;
	CIOOperationItemList            m_IdleIOOperationItemList;

private:
	CIOCP(void);
	~CIOCP(void);

public:
	static CIOCP* GetInstance();
	static void Release();

public:
	//启动服务
	bool StartService();
	//停止服务
	bool StopService();
	//设置服务
	void SetServiceParameter(WORD wServicePort);
	//获取缓存
	LPPER_IO_OPERATION_DATA GetIOOperationItem(SOCKET Socket, BYTE cbOperationType);
	//关闭连接
	bool CloseSocket(SOCKET Socket);
	//发送请求
	bool PostSend(SOCKET Socket, CHAR szBuf[]);
	//接受请求
	bool PostRecv(SOCKET Socket);

private:
	//接受线程
	static void AcceptWork(LPVOID pParam);
	//读写线程
	static void ReadWriteWork(LPVOID pParam);

private:
	//查询缓存
	LPPER_IO_OPERATION_DATA QueryIOOperationItem(SOCKET Socket, BYTE cbOperationType);

private:
	static CIOCP* m_sInstance;
	static int m_sInstanceCount;

	CTCPNetWorkEngine*	m_pTcpNetWorkEngine;

};
