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

// Linking Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

struct UserRoomPacket;
struct SendMessagePacket;

class ClientHelper {
public:
	ClientHelper(SOCKET& connectSocket);

	struct addrinfo* info = nullptr;
	struct addrinfo* ptr = nullptr;
	struct addrinfo hints;
	std::string name = "";
	std::vector<std::string> rooms;
	SOCKET connectSocket = NULL;

	int ShutDown();
	int Initialize();
	int sendMessage(Buffer buffer, SendMessagePacket packet);
	int sendData(Buffer buffer, size_t length);
	int recvMessage(char* buf, const int length);
};
