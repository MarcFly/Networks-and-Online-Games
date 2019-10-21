#include "Networks.h"
#include "ModuleNetworking.h"


static uint8 NumModulesUsingWinsock = 0;



void ModuleNetworking::reportError(const char* inOperationDesc)
{
	LPVOID lpMsgBuf;
	DWORD errorNum = WSAGetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorNum,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	ELOG("Error %s: %d- %s", inOperationDesc, errorNum, lpMsgBuf);
}

void ModuleNetworking::disconnect()
{
	for (SOCKET socket : sockets)
	{
		shutdown(socket, 2);
		closesocket(socket);
	}

	sockets.clear();
}

bool ModuleNetworking::init()
{
	if (NumModulesUsingWinsock == 0)
	{
		NumModulesUsingWinsock++;

		WORD version = MAKEWORD(2, 2);
		WSADATA data;
		if (WSAStartup(version, &data) != 0)
		{
			reportError("ModuleNetworking::init() - WSAStartup");
			return false;
		}
	}

	return true;
}

bool ModuleNetworking::preUpdate()
{
	if (sockets.empty()) return true;

	int err_ret;

	// NOTE(jesus): You can use this temporary buffer to store data from recv()
	const uint32 incomingDataBufferSize = Kilobytes(1);
	byte incomingDataBuffer[incomingDataBufferSize];

	// TODO(jesus): select those sockets that have a read operation available

	fd_set readSet; 
	FD_ZERO(&readSet);
	for (int i = 0; i < sockets.size(); ++i) FD_SET(sockets[i], &readSet);

	fd_set writeSet(readSet);

	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	err_ret = select(0, &readSet, &writeSet, nullptr, &timeout);
	if (err_ret == SOCKET_ERROR)
	{
		LOG("Error Selecting Sockets to read Data.");
		return false;
	}
	// TODO(jesus): for those sockets selected, check wheter or not they are
	// a listen socket or a standard socket and perform the corresponding
	

	for (SOCKET s : sockets)
	{
		if (FD_ISSET(s, &readSet))
		{
			if (isListenSocket(s))
			{
				// On accept() success, communicate the new connected socket to the
				// subclass (use the callback onSocketConnected()), and add the new
				// connected socket to the managed list of sockets.
				sockaddr_in in;
				int len = sizeof(in);
				SOCKET ret_sock = accept(s, (sockaddr*)&in, &len);
				if (ret_sock == INVALID_SOCKET)
				{
					LOG("Error Accepting Incoming Connection.");
					continue;
				}
				addSocket(ret_sock);
				
				onSocketConnected(ret_sock, in);
			}
			else
			{
				// On recv() success, communicate the incoming data received to the
				// subclass (use the callback onSocketReceivedData()).

				// TODO(jesus): handle disconnections. Remember that a socket has been
				// disconnected from its remote end either when recv() returned 0,
				// or when it generated some errors such as ECONNRESET.
				// Communicate detected disconnections to the subclass using the callback
				// onSocketDisconnected().

				InputMemoryStream packet;
				err_ret = recv(s, packet.GetBufferPtr(),packet.GetCapacity(),0);
				if (err_ret > 0)
				{
					packet.SetSize((uint32)err_ret);
					onSocketReceivedData(s, packet);
				}
				else
				{
					if (err_ret == SOCKET_ERROR)
					{
						LOG("Error Receiving data from a socket.");
					}

					onSocketDisconnected(s);
				}
					

			}
		}
	}

	// TODO(jesus): Finally, remove all disconnected sockets from the list
	// of managed sockets.

	return true;
}

bool ModuleNetworking::cleanUp()
{
	disconnect();

	NumModulesUsingWinsock--;
	if (NumModulesUsingWinsock == 0)
	{

		if (WSACleanup() != 0)
		{
			reportError("ModuleNetworking::cleanUp() - WSACleanup");
			return false;
		}
	}

	return true;
}

void ModuleNetworking::addSocket(SOCKET socket)
{
	sockets.push_back(socket);
}

// Packet Sending Properly
bool ModuleNetworking::sendPacket(const OutputMemoryStream & packet, SOCKET socket)
{
	int err_ret = send(socket, packet.GetBufferPtr(), packet.GetSize(), 0);
	if (err_ret == SOCKET_ERROR)
	{
		LOG("Error Sending Packet.");
		return false;
	}
	return true;
}