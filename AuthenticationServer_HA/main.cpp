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
#include "AuthDB.h"
#include <openssl/sha.h>
#include "authentication.pb.h"
// Linking Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

int main(int argc, char** argv) {
	std::srand(unsigned(std::time(0)));
	AuthDB db;
	db.Connect();
	AuthHelper SERVER;
	//int authError = 0;
	//std::string salt = SERVER.salt();
	//std::string password = SERVER.hashSHA256("password" + salt);
	//std::cout << "Real password: " << password << std::endl << "Real salt: " << salt << std::endl;
	//db.CreateAccount("h_sodeassaf@fanshaweonline.ca", &salt, password.c_str(), &authError);
	//db.AuthenticateAccount("h_sodeassaf@fanshaweonline.ca",  "password", &authError, SERVER);
	// Creating buffer
	Buffer* buffer = new Buffer(1);

	// Creating timeval
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 500 * 1000;

	// Start server
	int result = SERVER.Initialize();
	if (result != 0)
		return result;

	// Server loop
	for (;;)
	{
		// Empty socketsReadyForReading
		FD_ZERO(&SERVER.socketsReadyForReading);
		// Check for new connections
		FD_SET(SERVER.listenSocket, &SERVER.socketsReadyForReading);
		// Add all connected sockets and check if there is any data to be recv'd
		for (const ClientInfo& client : SERVER.clients)
		{
			if (client.connected)
				FD_SET(client.socket, &SERVER.socketsReadyForReading);
		}
		result = select(0, &SERVER.socketsReadyForReading, NULL, NULL, &tv);
		if (result == SOCKET_ERROR)
		{
			std::cout << "select failed with error: " << WSAGetLastError() << std::endl;
			return 1;
		}
		// Check for any new clients
		if (FD_ISSET(SERVER.listenSocket, &SERVER.socketsReadyForReading))
		{
			// Then accept that client
			SOCKET clientSocket = accept(SERVER.listenSocket, NULL, NULL);
			// Error check
			if (clientSocket == INVALID_SOCKET)
				std::cout << "Accept failed with error: " << WSAGetLastError() << std::endl;
			else
			{	// Otherwise add the client to the server
				ClientInfo client;
				client.socket = clientSocket;
				client.connected = true;

				const int buflen = 128;
				char buf[buflen];
				int recvResult = recv(client.socket, buf, buflen, 0);
				std::vector<uint8_t> vecBuf(buf, buf + recvResult);
				buffer->setBuffer(vecBuf);

				// Get the username from the client
				int nameSize = buffer->ReadInt32LE(0);
				client.name = buffer->ReadString(nameSize);
				std::cout << client.name << " has connected!" << std::endl;
				// Push it onto the vector
				SERVER.clients.push_back(client);
			}
		}

		// Check if connected clients have sent data
		for (ClientInfo& client : SERVER.clients)
		{
			// Make sure it is connected
			if (!client.connected)
				continue;

			if (FD_ISSET(client.socket, &SERVER.socketsReadyForReading))
			{
				// Will act as the ping server
				const int buflen = 128;
				char buf[buflen];
				// Receive call
				int recvResult = recv(client.socket, buf, buflen, 0);
				// Make sure client is connected
				if (recvResult == 0)
				{
					std::cout << "Client \"" << client.name << "\" has disconnected!" << std::endl;
					client.connected = false;
					continue;
				}
				std::vector<uint8_t> vecBuf(buf, buf + buflen);
				buffer->setBuffer(vecBuf);
				PacketHeader header;
				header.length = buffer->ReadInt32LE(0); // total length
				header.id = buffer->ReadInt32LE(4); // id
				std::cout << header.id << std::endl;
				// Take the ID and act accordingly
				switch (header.id)
				{
				case REGISTER: // REGISTER USER
				{
					std::string msgReply;
					Packet packet;
					packet.header = header;
					packet.serializedData = buffer->ReadString(buffer->ReadInt32LE());
					authentication::CreateAccountWeb deserialized_account;
					if (!deserialized_account.ParseFromString(packet.serializedData))
					{
						std::cout << "Error Invalid Data..." << std::endl;
						break;
					}
					// Create user
					int* error;
					error = new int;
					int userId;
					std::string salt = SERVER.salt();
					std::string hashedPassword = SERVER.hashSHA256(deserialized_account.plaintextpassword() + salt);
					if (deserialized_account.plaintextpassword().length() < 8)
					{
						msgReply = "Failure to create account.";
						authentication::CreateAccountWebFailure failure;
						failure.set_error(authentication::CreateAccountWebFailure::INVALID_PASSWORD);
						failure.set_requestid(3);
						packet.serializedData = failure.SerializeAsString();
						// Create and serialize reply back to the client
						size_t length = 4 + sizeof(msgReply.size()) + msgReply.size() + sizeof(packet.serializedData.size()) + packet.serializedData.size();
						Buffer* reply = new Buffer(length);
						reply->WriteInt32LE(0, 0); // 1 = success, 0 = failure
						reply->WriteInt32LE(msgReply.size());
						reply->WriteString(msgReply);
						reply->WriteInt32LE(packet.serializedData.size());
						reply->WriteString(packet.serializedData);
						// Send reply
						result = send(client.socket, (const char*)&(reply->getBuffer()[0]), length, 0);
						break;
					}
					if (!db.CreateAccount(deserialized_account.email().c_str(), &salt, hashedPassword.c_str(), error, &userId))
					{
						msgReply = "Failure to create account.";
						authentication::CreateAccountWebFailure failure;
						failure.set_requestid(3);
						switch (*error)
						{
						case ACCOUNT_ALREADY_EXISTS:
							failure.set_error(authentication::CreateAccountWebFailure::ACCOUNT_ALREADY_EXISTS);
							break;
						case INVALID_PASSWORD:
							failure.set_error(authentication::CreateAccountWebFailure::INVALID_PASSWORD);
							break;
						case INTERNAL_SERVER_ERROR:
							failure.set_error(authentication::CreateAccountWebFailure::INTERNAL_SERVER_ERROR);
						}
						// Create and serialize reply back to the client
						size_t length = 4 + sizeof(msgReply.size()) + msgReply.size() + sizeof(packet.serializedData.size()) + packet.serializedData.size();
						Buffer* reply = new Buffer(length);
						packet.serializedData = failure.SerializeAsString();
						reply->WriteInt32LE(0, 0); // 1 = success, 0 = failure
						reply->WriteInt32LE(msgReply.size());
						reply->WriteString(msgReply);
						reply->WriteInt32LE(packet.serializedData.size());
						reply->WriteString(packet.serializedData);
						// Send reply
						result = send(client.socket, (const char*)&(reply->getBuffer()[0]), length, 0);
					}
					else
					{
						// Create and serialize reply back to the client
						msgReply = "Account successfully created.";
						size_t length = 4 + sizeof(msgReply.size()) + msgReply.size() + sizeof(packet.serializedData.size()) + packet.serializedData.size();
						Buffer* reply = new Buffer(length);
						authentication::CreateAccountWebSuccess success;
						success.set_requestid(4);
						success.set_userid(userId);
						reply->WriteInt32LE(0, 1); // 1 = success, 0 = failure
						reply->WriteInt32LE(msgReply.size());
						reply->WriteString(msgReply);
						reply->WriteInt32LE(packet.serializedData.size());
						reply->WriteString(packet.serializedData);
						// Send reply
						result = send(client.socket, (const char*)&(reply->getBuffer()[0]), length, 0);
					}
					break;
				}
				case LOGIN: // USER LOGIN
				{
					std::string msgReply;
					Packet packet;
					packet.header = header;
					packet.serializedData = buffer->ReadString(buffer->ReadInt32LE());
					authentication::AuthenticateWeb deserialized_account;
					if (!deserialized_account.ParseFromString(packet.serializedData))
					{
						std::cout << "Error Invalid Data..." << std::endl;
						break;
					}
					// Create user
					int* error;
					error = new int;
					int userId;
					std::string creation_date;
					if (!db.AuthenticateAccount(deserialized_account.email().c_str(), deserialized_account.plaintextpassword().c_str(), error, SERVER, &userId, &creation_date))
					{
						msgReply = "Failure to create account.";
						authentication::AuthenticateWebFailure failure;
						failure.set_requestid(3);
						switch (*error)
						{
						case INVALID_CREDENTIALS:
							failure.set_error(authentication::AuthenticateWebFailure::INVALID_CREDENTIALS);
							break;
						case INTERNAL_SERVER_ERROR:
							failure.set_error(authentication::AuthenticateWebFailure::INTERNAL_SERVER_ERROR);
							break;
						default:
							failure.set_error(authentication::AuthenticateWebFailure::INTERNAL_SERVER_ERROR);
							break;
						}
						// Create and serialize reply back to the client
						size_t length = 4 + sizeof(msgReply.size()) + msgReply.size() + sizeof(packet.serializedData.size()) + packet.serializedData.size();
						Buffer* reply = new Buffer(length);
						packet.serializedData = failure.SerializeAsString();
						reply->WriteInt32LE(0, 0); // 1 = success, 0 = failure
						reply->WriteInt32LE(msgReply.size());
						reply->WriteString(msgReply);
						reply->WriteInt32LE(packet.serializedData.size());
						reply->WriteString(packet.serializedData);
						// Send reply
						result = send(client.socket, (const char*)&(reply->getBuffer()[0]), length, 0);
					}
					else
					{
						// Create and serialize reply back to the client
						msgReply = "Account successfully created.";
						size_t length = 4 + sizeof(msgReply.size()) + msgReply.size() + sizeof(packet.serializedData.size()) + packet.serializedData.size();
						Buffer* reply = new Buffer(length);
						authentication::AuthenticateWebSuccess success;
						success.set_requestid(4);
						success.set_userid(userId);
						success.set_creationdate(creation_date);
						packet.serializedData = success.SerializeAsString();
						reply->WriteInt32LE(0, 1); // 1 = success, 0 = failure
						reply->WriteInt32LE(msgReply.size());
						reply->WriteString(msgReply);
						reply->WriteInt32LE(packet.serializedData.size());
						reply->WriteString(packet.serializedData);
						// Send reply
						result = send(client.socket, (const char*)&(reply->getBuffer()[0]), length, 0);
					}
					break;
				}
				default:
					break;
				}
			}
		}
	}
	//db.Disconnect();
	SERVER.ShutDown();
}