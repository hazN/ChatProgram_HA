/*
	Hassan Assaf
	INFO-6016
	Project #1: Chat Program
	Due 2022-10-19
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
	int recvMessage(bool recvLoop);
};

// Make it easier to choose msg id
enum MsgType { JOIN = 1, LEAVE = 2, SEND = 3 };

struct PacketHeader
{
	int length;
	int id;
};

struct UserRoomData
{
	std::string roomName;
	std::string username;
};

struct SendMessageData
{
	std::string roomName;
	std::string message;
};

struct UserRoomPacket
{
	PacketHeader header;
	UserRoomData content;
};
struct SendMessagePacket
{
	PacketHeader header;
	SendMessageData content;
};