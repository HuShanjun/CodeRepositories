
#pragma once

struct TCP_PacketHead
{
	WORD			wMainCmd;			//协议主码
	WORD			wSubCmd;			//协议子码
	DWORD			nDataPacketSize;	//协议数据包大小
}; 

struct TCP_DataPacket
{

	TCP_PacketHead		TcpHead;		//协议头
	BYTE				cbBuff[4096];	//协议数据
};