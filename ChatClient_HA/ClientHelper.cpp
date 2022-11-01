/*
	Hassan Assaf
	INFO-6016
	Project #2: Authentication Server
	Due 2022-11-09
*/
#define WIN32_LEAN_AND_MEAN

#include <vector>
#include <iostream>
#include "Buffer.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "ClientHelper.h"
#include <sstream>
#include <future>
#include <thread>

// Linking Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

// Constructor to initialize connectSocket
ClientHelper::ClientHelper(SOCKET& connectSocket)
{
	this->connectSocket = connectSocket;
}

// Method: Initialize
// Summary: Initialize the client.
// Params: void
// Return: int
int ClientHelper::Initialize()
{
	// Set username first 
	std::cout << "Please enter your username: ";
	std::cin >> name;

	// Variable to use for error checks
	int result;

	// Setup WSADATA
	WSADATA wsaData;
	// Check for failure
	result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0)
	{
		std::cout << "WSAStartup failed with error:\n" << result << std::endl;
		return 1;
	}
	// End of WSADATA setup

	// Setup ADDRINFO
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;		// IPV4
	hints.ai_socktype = SOCK_STREAM; // Stream
	hints.ai_protocol = IPPROTO_TCP; // TCP
	result = getaddrinfo("127.0.0.1", "5555", &hints, &info);
	// Error Check
	if (result != 0)
	{
		std::cout << "getaddrinfo failed with error: " << result << std::endl;
		WSACleanup();
		return 1;
	}
	// Initialize socket
	connectSocket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
	// Error Check 
	if (connectSocket == INVALID_SOCKET)
	{
		std::cout << "socket failed with error: " << WSAGetLastError() << std::endl;
		freeaddrinfo(info);
		WSACleanup();
		return 1;
	}

	// Now connect that to the server
	result = connect(connectSocket, info->ai_addr, (int)info->ai_addrlen);
	Buffer* buffer = new Buffer(4);
	buffer->WriteInt32LE(sizeof(name));
	buffer->WriteString(name);
	result = send(connectSocket, (const char*)&(buffer->getBuffer()[0]), name.size() + 4, 0);

	// IO Control Socket
	DWORD NonBlock = 1;
	result = ioctlsocket(connectSocket, FIONBIO, &NonBlock);
	if (result == SOCKET_ERROR)
	{
		std::cout << "ioctlsocket failed with error: " << WSAGetLastError() << std::endl;
		closesocket(connectSocket);
		freeaddrinfo(info);
		WSACleanup();
		return 1;
	}
	return 0;
}

// Method: ShutDown
// Summary: Shutdown the client.
// Params: void
// Return: void
int ClientHelper::ShutDown()
{
	int result = shutdown(connectSocket, SD_SEND);
	if (result == SOCKET_ERROR)
	{
		std::cout << "shutdown failed with error: " << WSAGetLastError() << std::endl;
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}
	else
	{
		std::cout << "client has been shutdown..." << std::endl;
	}

	closesocket(connectSocket);
	WSACleanup();
	return 0;
}

// Method: SendMessage
// Summary: Send message to the server
// Params: Buffer, SendMessagePacket
// Return: int
int ClientHelper::sendMessage(Buffer buffer, SendMessagePacket packet)
{
	// Send the data
	int result = send(connectSocket, (const char*)&(buffer.getBuffer()[0]), packet.header.length, 0);
	if (result == SOCKET_ERROR)
	{
		std::cout << "send failed with error: " << WSAGetLastError() << std::endl;
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}
}

// Method: SentData
// Summary: Join or Leave a room
// Params: Buffer, size_t
// Return: int
int ClientHelper::sendData(Buffer buffer, size_t length)
{
	// Send the data
	int result = send(connectSocket, (const char*)&(buffer.getBuffer()[0]), length, 0);
	if (result == SOCKET_ERROR)
	{
		std::cout << "send failed with error: " << WSAGetLastError() << std::endl;
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}
}

int ClientHelper::recvMessage(bool recvLoop)
{
	// Loop until message is received back
	int recvLength = 128;
	char buf[128];
	int result;
	do 
	{
		// Try to receive
		result = recv(connectSocket, buf, recvLength, 0);
		// Error Check
		if (result == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK) {
				//std::cout << "WouldBlock!" << std::endl;
				break;
			}
			else
			{
				std::cout << "recv failed with error: " << WSAGetLastError() << std::endl;
				closesocket(connectSocket);
				WSACleanup();
				system("Pause");
				return 1;
			}
		}
		else // Otherwise exit loop
		{
			std::vector<uint8_t> vecBuf(buf, buf + result);
			Buffer* reply = new Buffer(result);
			reply->setBuffer(vecBuf);
			std::string msg = reply->ReadString(reply->ReadInt32LE());
			std::cout << msg << reply->ReadString(reply->ReadInt32LE()) << std::endl;
			recvLoop = false;
		}
	} while (recvLoop); // End of receive loop

	return result;
}
