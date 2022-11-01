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

// Setting up port
#define DEFAULT_PORT "5555"

struct ClientInfo;

class ServerHelper
{
public:
	void ShutDown();
	int Initialize();

	struct addrinfo* info = nullptr;
	struct addrinfo hints;
	SOCKET listenSocket = INVALID_SOCKET;
	SOCKET connectSocket = INVALID_SOCKET;
	fd_set activeSockets;
	fd_set socketsReadyForReading;
	std::vector<ClientInfo> clients;
};

// Make it easier to choose msg id
enum MsgType { JOIN = 1, LEAVE = 2, SEND = 3, CREATE = 4, AUTHENTICATE = 5};

struct ClientInfo {
	SOCKET socket;
	bool connected;
	std::string name;
	std::vector<std::string> rooms;
};

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

struct CreateAccountPacket
{
	PacketHeader header;
	std::string serializedString;
};