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
#include <openssl/sha.h>
#include <sstream>
// Linking Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

// Setting up port
#define DEFAULT_PORT "5556"

struct ClientInfo;

class AuthHelper
{
public:
	void ShutDown();
	int Initialize();

	struct addrinfo* info = nullptr;
	struct addrinfo hints;
	SOCKET listenSocket = INVALID_SOCKET;
	fd_set activeSockets;
	fd_set socketsReadyForReading;
	std::vector<ClientInfo> clients;
	std::string hashSHA256(const std::string password);
	std::string salt();

};

// Make it easier to choose msg id
enum MsgType { REGISTER = 1, LOGIN = 2};

struct ClientInfo {
	SOCKET socket;
	bool connected;
	std::string name;
};

struct PacketHeader
{
	int length;
	int id;
};

struct Packet
{
	PacketHeader header;
	std::string serializedData;
};