/*
	Hassan Assaf
	INFO-6016
	Project #2: Authentication Server
	Due 2022-11-09
*/
#include "AuthHelper.h"

#include <algorithm>
#include <iomanip>

// Method: Initialize
// Summary: Initialize the server.
// Params: void
// Return: int
int AuthHelper::Initialize()
{
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

	// Setup FD_SET
	FD_ZERO(&activeSockets);
	FD_ZERO(&socketsReadyForReading);

	// Setup ADDRINFO
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;		// IPV4
	hints.ai_socktype = SOCK_STREAM;	// Stream
	hints.ai_protocol = IPPROTO_TCP;	// TCP
	hints.ai_flags = AI_PASSIVE;
	// Check for failure
	result = getaddrinfo(NULL, DEFAULT_PORT, &hints, &info);
	if (result != 0)
	{
		std::cout << "getaddrinfo failed with error:\n" << result << std::endl;;
		WSACleanup();
		return 1;
	}
	// End of ADDRINFO setup

	// Setup Socket
	listenSocket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
	if (listenSocket == INVALID_SOCKET)
	{
		std::cout << "SOCKET failed with error:\n" << WSAGetLastError() << std::endl;
		freeaddrinfo(info);
		WSACleanup();
		return 1;
	}
	// End of Socket setup

	// Bind the Socket
	result = bind(listenSocket, info->ai_addr, (int)info->ai_addrlen);
	if (result == SOCKET_ERROR)
	{
		std::cout << "BIND failed with error:\n" << WSAGetLastError() << std::endl;
		freeaddrinfo(info);
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}
	// End of Bind setup

	// Setup Listen
	result = listen(listenSocket, SOMAXCONN);
	if (result == SOCKET_ERROR)
	{
		std::cout << "listen failed with error:\n" << WSAGetLastError() << std::endl;
		freeaddrinfo(info);
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}
	// End of Listen Setup

	// Print out server has been started and return
	std::cout << "Server started..." << std::endl;
	return 0;
} // End of Initialization

// Method: hashSHA256
// Summary: Hash a password using the SHA256 algorithm
// Params: const string
// Return: string
std::string AuthHelper::hashSHA256(const std::string password)
{
	unsigned char hash[SHA256_DIGEST_LENGTH];
	SHA256_CTX sha256;
	SHA256_Init(&sha256);
	SHA256_Update(&sha256, password.c_str(), password.size());
	SHA256_Final(hash, &sha256);
	std::stringstream ss;
	for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
	{
		ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
	}
	return ss.str();
}

// Method: salt
// Summary: Generate a salt randomly 
// Params: void
// Return: string
std::string AuthHelper::salt()
{
	static char charset[] = { "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!$%^&*():@~#'?/><.,|`" };
	std::random_shuffle(std::begin(charset), std::end(charset));
	std::string salt = "";
	for (int i = 0; i <= 8; i++)
	{
		salt += charset[i];
	}
	return salt;
}

// Method: ShutDown
// Summary: Shutdown the server.
// Params: void
// Return: void
void AuthHelper::ShutDown() {
	freeaddrinfo(info);
	closesocket(listenSocket);
	WSACleanup();
	std::cout << "Server has been shutdown..." << std::endl;
}
