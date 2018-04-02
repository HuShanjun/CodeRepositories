#include "StdAfx.h"
#include "flashaccredit.h"
#include "..\Include\InitSocket.h"
#include <assert.h>
#include "TCPNetWorkEngine.h"

//////////////////////////////////////////////////////////////////////////
//宏定义
#define RECV_POSTED      101
#define SEND_POSTED      102


CInitSocket g_InitSocket;

CIOCP* CIOCP::m_sInstance = NULL;
int CIOCP::m_sInstanceCount = 0;

//////////////////////////////////////////////////////////////////////////
CIOCP::CIOCP(void)
{
	m_hServerSocket = INVALID_SOCKET;
	m_lIOCPHandle = INVALID_HANDLE_VALUE;
	m_wServicePort = 843;
	m_pTcpNetWorkEngine = NULL;
	m_IOSendOperationItemList.clear();
	m_IORecvOperationItemList.clear();
	m_IdleIOOperationItemList.clear();
}

CIOCP::~CIOCP(void)
{
	StopService();
	WSACleanup();
}

//单利模式
CIOCP* CIOCP::GetInstance()
{
	if (m_sInstanceCount == 0)
	{
		m_sInstance = new CIOCP;

		++m_sInstanceCount;
	}

	return m_sInstance;
}

void CIOCP::Release()
{
	if (m_sInstanceCount != 0)
	{
		delete m_sInstance;
	}
}

//启动服务
bool CIOCP::StartService()
{
	//建立网络
	SOCKADDR_IN SocketAddr;
	ZeroMemory(&SocketAddr,sizeof(SocketAddr));

	m_pTcpNetWorkEngine = CTCPNetWorkEngine::GetInstance();

	//建立网络
	SocketAddr.sin_family=AF_INET;
	SocketAddr.sin_addr.s_addr=INADDR_ANY;
	SocketAddr.sin_port=htons(m_wServicePort);
	m_hServerSocket=WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,NULL,0,WSA_FLAG_OVERLAPPED);

	//错误判断
	if (m_hServerSocket==INVALID_SOCKET) 
	{
		LPCTSTR pszString=TEXT("系统资源不足或者 TCP/IP 协议没有安装，网络启动失败");
		printf(pszString);
		return false;
	}

	//绑定链接
	if (bind(m_hServerSocket,(SOCKADDR*)&SocketAddr,sizeof(SocketAddr))==SOCKET_ERROR)
	{
		LPCTSTR pszString=TEXT("网络绑定发生错误，网络启动失败");
		printf(pszString);
		return false;
	}

	//监听端口
	if (listen(m_hServerSocket,200)==SOCKET_ERROR)
	{
		TCHAR szString[512]=TEXT("");
		_sntprintf(szString,CountArray(szString),TEXT("端口正被其他服务占用，监听 %d 端口失败"), m_wServicePort);
		printf(szString);
		return false;
	}

	//系统信息
	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);

	//完成端口
	m_lIOCPHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,NULL,SystemInfo.dwNumberOfProcessors);
	if(m_lIOCPHandle == NULL)
	{
		LPCTSTR pszString=TEXT("创建网络资源失败，网络启动失败");
		printf(pszString);
		return false;
	}

	//绑定SOCKET
	if(NULL == CreateIoCompletionPort((HANDLE)m_hServerSocket, m_lIOCPHandle, 0, 0))
	{
		LPCTSTR pszString=TEXT("绑定监听SOCKET失败，网络启动失败");
		printf(pszString);
		return false;
	}

	//创建工作线程
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AcceptWork, (LPVOID)this, 0, 0);
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ReadWriteWork, (LPVOID)this, 0, 0);

	return true;
}

//停止服务
bool CIOCP::StopService()
{
	//清理记录
	
	for (auto pos = m_IORecvOperationItemList.begin(); pos != m_IORecvOperationItemList.end(); ++pos)
	{
		LPPER_IO_OPERATION_DATA pIOOperdata= (*pos);
		if(pIOOperdata->socket)
		{
			//关闭连接
			closesocket(pIOOperdata->socket);
			delete pIOOperdata;
			pIOOperdata = NULL;
		}
	}
	m_IORecvOperationItemList.clear();

	//清理记录
	
	for (auto pos = m_IOSendOperationItemList.begin(); pos != m_IOSendOperationItemList.end(); ++pos)
	{
		LPPER_IO_OPERATION_DATA pIOOperdata=(*pos);
		if(pIOOperdata->socket)
		{
			//关闭连接
			closesocket(pIOOperdata->socket);
			delete pIOOperdata;
			pIOOperdata = NULL;
		}
	}
	m_IOSendOperationItemList.clear();

	//清理记录
	for (auto pos = m_IdleIOOperationItemList.begin(); pos != m_IdleIOOperationItemList.end(); ++pos)
	{
		LPPER_IO_OPERATION_DATA pIOOperdata=(*pos);
		if(pIOOperdata->socket)
		{
			//关闭连接
			closesocket(pIOOperdata->socket);
			delete pIOOperdata;
			pIOOperdata = NULL;
		}
	}
	m_IdleIOOperationItemList.clear();

	return true;
}

//设置服务
void CIOCP::SetServiceParameter(WORD wServicePort)
{
	m_wServicePort=wServicePort;
}

//查询缓存
LPPER_IO_OPERATION_DATA CIOCP::QueryIOOperationItem(SOCKET Socket, BYTE cbOperationType)
{
	//参数效验
	assert(cbOperationType == SEND_POSTED || cbOperationType == RECV_POSTED);
	if (cbOperationType != SEND_POSTED && cbOperationType != RECV_POSTED) return NULL;

	CIOOperationItemList QueryList = (RECV_POSTED == cbOperationType ? m_IORecvOperationItemList : m_IOSendOperationItemList);

	for (auto pos = QueryList.begin(); pos != QueryList.end(); ++pos)
	{
		if ((*pos)->socket == Socket)
		{
			return *pos;
		}
	}

	return NULL;

}

//获取缓存
LPPER_IO_OPERATION_DATA CIOCP::GetIOOperationItem(SOCKET Socket, BYTE cbOperationType)
{
	//参数效验
	assert(cbOperationType==SEND_POSTED || cbOperationType==RECV_POSTED);
	if(cbOperationType!=SEND_POSTED && cbOperationType!=RECV_POSTED) return NULL;

	//定义变量
	LPPER_IO_OPERATION_DATA pIOOperdata = NULL;


	//查询buff
	if (RECV_POSTED == cbOperationType || SEND_POSTED == cbOperationType)
	{
		LPPER_IO_OPERATION_DATA pIOData = QueryIOOperationItem(Socket, cbOperationType);
		if (NULL != pIOData)
		{
			return pIOData;
		}
	}

	//分配buff
	if(m_IdleIOOperationItemList.size() > 0)
	{
		pIOOperdata = (*m_IdleIOOperationItemList.begin());

		m_IdleIOOperationItemList.pop_front();
	}
	else //分配缓存
	{
		pIOOperdata = new PER_IO_OPERATION_DATA;
		if( pIOOperdata == NULL) return NULL;
	}

	//设置缓存
	pIOOperdata->socket = Socket;
	memset(&(pIOOperdata->Overlapped),0,sizeof(OVERLAPPED));
	memset(pIOOperdata->cbDataBuff,0,sizeof(pIOOperdata->cbDataBuff));
	pIOOperdata->WSABuff.len = sizeof(pIOOperdata->cbDataBuff);
	pIOOperdata->WSABuff.buf = pIOOperdata->cbDataBuff;
	pIOOperdata->cbOperationType = cbOperationType;

	//记录缓存
	if(cbOperationType == RECV_POSTED)
	{
		m_IORecvOperationItemList.push_back(pIOOperdata);
	}
	else if(cbOperationType == SEND_POSTED)
	{
		m_IOSendOperationItemList.push_back(pIOOperdata);
	}

	return pIOOperdata;
}

//关闭连接
bool CIOCP::CloseSocket(SOCKET Socket)
{
	//关闭连接
	closesocket(Socket);

	//清理记录
	for (auto pos = m_IORecvOperationItemList.begin();
		pos != m_IORecvOperationItemList.end(); ++pos)
	{
		LPPER_IO_OPERATION_DATA tempPos = (*pos);
		if(tempPos->socket == Socket)
		{
			m_IORecvOperationItemList.erase(pos);
			m_IdleIOOperationItemList.push_back(tempPos);
			break;
		}
	}

	//清理记录
	//pos=m_IOSendOperationItemList.GetHeadPosition();
	for (auto pos = m_IOSendOperationItemList.begin();
		pos != m_IOSendOperationItemList.end(); ++pos)
	{
		LPPER_IO_OPERATION_DATA pIOOperdata=(*pos);
		if(pIOOperdata->socket == Socket)
		{
			m_IOSendOperationItemList.erase(pos);
			m_IdleIOOperationItemList.push_back(pIOOperdata);
			break;
		}
	}

	return true;
}

//接受线程
void CIOCP::AcceptWork(LPVOID pParam)
{
	//变量定义
	CIOCP * pFlashAccredit = (CIOCP *)pParam;

	//线程循环
	while(true)
	{
		//接收用户连接，被和完成端口关联
		SOCKET sockAccept = WSAAccept(pFlashAccredit->m_hServerSocket,NULL,NULL,NULL,0);
		if (INVALID_SOCKET == sockAccept)
		{
			printf("无效socket \n");
			continue;
		}

		//分配缓存
		LPPER_IO_OPERATION_DATA pIOOperdata = pFlashAccredit->GetIOOperationItem(sockAccept, RECV_POSTED);
		if (pIOOperdata == NULL)
		{
			printf("内存获取失败\n");
			continue;
		}

		//关联SOCKET和完成端口
		CreateIoCompletionPort((HANDLE)sockAccept,pFlashAccredit->m_lIOCPHandle,NULL,1);

		//投递接收操作
		/*DWORD dwRecvBytes=0;
		DWORD dwFlags=0;
		INT nRet = WSARecv(pIOOperdata->socket,&(pIOOperdata->WSABuff),1,&dwRecvBytes,&dwFlags,&(pIOOperdata->Overlapped),NULL);
		if(nRet==SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		{
			pFlashAccredit->CloseSocket(sockAccept);
			continue;
		}*/

		bool bRet = pFlashAccredit->PostRecv(sockAccept);
		if (!bRet)
		{
			printf("[%s--%d] 投递接受请求失败\n", __FUNCTION__, __LINE__);
			continue;
		}
	}
}

//读写线程
void CIOCP::ReadWriteWork(LPVOID pParam)
{
	//变量定义
	CIOCP * pFlashAccredit = (CIOCP *)pParam;

	//线程循环
	while(true)
	{
		//等待完成端口上SOCKET的完成
		bool bRet = false;
		DWORD dwThancferred=0;
		DWORD  dwCompletionKey=0;
		LPPER_IO_OPERATION_DATA pIOOperdata=NULL;
		GetQueuedCompletionStatus(pFlashAccredit->m_lIOCPHandle,&dwThancferred,&dwCompletionKey,(LPOVERLAPPED *)&pIOOperdata,INFINITE);
		if(pIOOperdata==NULL) continue;

		//检查是否有错误产生
		if(dwThancferred == 0 && (pIOOperdata->cbOperationType == RECV_POSTED || pIOOperdata->cbOperationType == SEND_POSTED))
		{
			//关闭SOCKET
			pFlashAccredit->CloseSocket(pIOOperdata->socket);
			continue;
		}

		//为请求服务
		if(pIOOperdata->cbOperationType == RECV_POSTED)
		{
			//数据包解析
			
			pFlashAccredit->m_pTcpNetWorkEngine->ParseTCPPacket(pIOOperdata->socket, pIOOperdata->cbDataBuff);

			//
			bRet = pFlashAccredit->PostRecv(pIOOperdata->socket);
			if (!bRet)
			{
				printf("[%s--%d] 投递接受请求失败\n", __FUNCTION__, __LINE__);
				continue;
			}
		}
		else if(pIOOperdata->cbOperationType == SEND_POSTED)
		{
			bRet = pFlashAccredit->PostRecv(pIOOperdata->socket);
			if (!bRet)
			{
				printf("[%s--%d] 投递接受请求失败\n", __FUNCTION__, __LINE__);
				continue;
			}
		}
	}
}

//接受请求
bool CIOCP::PostSend(SOCKET Socket, CHAR szBuf[])
{

	LPPER_IO_OPERATION_DATA pIOOperdata = GetIOOperationItem(Socket, SEND_POSTED);

	//投递发送请求操作
	DWORD dwRecvBytes = 0;
	DWORD dwFlags = 0;
	CopyMemory(pIOOperdata->WSABuff.buf, szBuf, sizeof(CHAR)*strlen(szBuf));

	INT nRet = WSASend(pIOOperdata->socket, &(pIOOperdata->WSABuff), 1, &dwRecvBytes, dwFlags, &(pIOOperdata->Overlapped), NULL);
	if (nRet == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
	{
		CloseSocket(pIOOperdata->socket);
		return false;
	}
	return true;
}

//发送请求
bool CIOCP::PostRecv(SOCKET Socket)
{
	LPPER_IO_OPERATION_DATA pIOOperdata = GetIOOperationItem(Socket, RECV_POSTED);

	//投递发送请求操作
	DWORD dwRecvBytes = 0;
	DWORD dwFlags = 0;
	INT nRet = WSARecv(pIOOperdata->socket, &(pIOOperdata->WSABuff), 1, &dwRecvBytes, &dwFlags, &(pIOOperdata->Overlapped), NULL);
	if (nRet == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
	{
		CloseSocket(pIOOperdata->socket);

		return false;
	}

	return true;
}
