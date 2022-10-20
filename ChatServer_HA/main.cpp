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
#include "ServerHelper.h"

// Linking Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

int main(int argc, char** argv) {
	ServerHelper SERVER;
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
	for(;;)
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

				// Take the ID and act accordingly
				switch (header.id)
				{
				case JOIN: // Join Room
				{
					std::string msgReply;
					UserRoomPacket packet;
					packet.header = header;
					packet.content.username = buffer->ReadString(buffer->ReadInt32LE());
					packet.content.roomName = buffer->ReadString(buffer->ReadInt32LE());

					// Check if vec is empty, if so add the room
					if (client.rooms.empty())
					{
						client.rooms.push_back(packet.content.roomName);
						msgReply = "You have successfully joined room ";
					}
					else // Otherwise check if the room exists already
					{
						bool roomExists = false;
						for (std::string room : client.rooms)
						{
							if (room == packet.content.roomName)
								room = true;
						}
						if (!roomExists) // Doesn't exist so add the room
						{
							client.rooms.push_back(packet.content.roomName);
							msgReply = "You have successfully joined room ";
						}
						else
						{
							msgReply = "You have already joined room ";

						}
					}
					// Create and serialize reply back to the client
					size_t length = sizeof(msgReply.size()) + msgReply.size() + sizeof(packet.content.roomName.size()) + packet.content.roomName.size();
					Buffer* reply = new Buffer(length);
					reply->WriteInt32LE(0, msgReply.size());
					reply->WriteString(msgReply);
					reply->WriteInt32LE(packet.content.roomName.size());
					reply->WriteString(packet.content.roomName);
					// Send reply
					result = send(client.socket, (const char*)&(reply->getBuffer()[0]), length, 0);
					// Print to the server that X user has joined Y room
					std::cout << "User " << client.name << " has joined room " << packet.content.roomName << std::endl;
					// Now print to each user in that room that a user has joined
					std::string userJoined = "User " + client.name + " has joined the room!(" + packet.content.roomName + ")";
					// Clear buffer
					reply->setBuffer(*new std::vector<uint8_t>);
					reply->WriteInt32LE(0, userJoined.size());
					reply->WriteString(userJoined);
					length = sizeof(userJoined.size()) + userJoined.size();
					for (ClientInfo& xClient : SERVER.clients)
					{
						// Check if client is in that room
						for (std::string room : xClient.rooms)
						{
							if (room == packet.content.roomName && xClient.name != packet.content.username)
							{
								// Send message
								result = send(xClient.socket, (const char*)&(reply->getBuffer()[0]), length, 0);
							}
						}
					}
					break;
				}
				case LEAVE: // Leave Room
				{
					std::string msgReply;
					UserRoomPacket packet;
					packet.header = header;
					packet.content.username = buffer->ReadString(buffer->ReadInt32LE());
					packet.content.roomName = buffer->ReadString(buffer->ReadInt32LE());

					// Check if vec is empty
					if (client.rooms.empty())
						msgReply = "You aren't in room ";
					else // Otherwise check if the room exists already
					{
						bool roomExists = false;
						for (size_t i = 0; i < client.rooms.size(); i++)
						{
							if (client.rooms[i] == packet.content.roomName)
							{
								roomExists = true;
								client.rooms.erase(client.rooms.begin() + i);
							}
						}
						if (!roomExists) // Doesn't exist so add the room
							msgReply = "You aren't in room ";
						else
						{
							msgReply = "You have successfully left room ";
						}
					}
					// Create and serialize reply back to the client
					size_t length = sizeof(msgReply.size()) + msgReply.size() + sizeof(packet.content.roomName.size()) + packet.content.roomName.size();
					Buffer* reply = new Buffer(length);
					reply->WriteInt32LE(0, msgReply.size());
					reply->WriteString(msgReply);
					reply->WriteInt32LE(packet.content.roomName.size());
					reply->WriteString(packet.content.roomName);
					// Send reply
					result = send(client.socket, (const char*)&(reply->getBuffer()[0]), length, 0);
					// Print to the server that X user has joined Y room
					std::cout << "User " << client.name << " has left room " << packet.content.roomName << std::endl;
					// Now print to each user in that room that a user has joined
					std::string userJoined = "User " + client.name + " has left the room!(" + packet.content.roomName + ")";
					// Clear buffer
					reply->setBuffer(*new std::vector<uint8_t>);
					reply->WriteInt32LE(0, userJoined.size());
					reply->WriteString(userJoined);
					length = sizeof(userJoined.size()) + userJoined.size();
					for (ClientInfo& xClient : SERVER.clients)
					{
						// Check if client is in that room
						for (std::string room : xClient.rooms)
						{
							if (room == packet.content.roomName && xClient.name != packet.content.username)
							{
								// Send message
								result = send(xClient.socket, (const char*)&(reply->getBuffer()[0]), length, 0);
							}
						}
					}
					break;
				}
				case SEND: // Send Message
					{
					std::string msgReply;
					SendMessagePacket packet;
					packet.header = header;
					packet.content.roomName = buffer->ReadString(buffer->ReadInt32LE());
					packet.content.message = buffer->ReadString(buffer->ReadInt32LE());

					// Broadcast msg to server
					std::string message = client.name + " > " + packet.content.roomName + ": " + packet.content.message;
					std::cout << message << std::endl;
					// Create buffer
					int length = sizeof(message.size()) + message.size();
					Buffer* reply = new Buffer(length);
					reply->WriteInt32LE(0, message.size());
					reply->WriteString(message);
					for (ClientInfo& xClient : SERVER.clients)
					{
						// Check if client is in that room
						for (std::string room : xClient.rooms)
						{
							if (room == packet.content.roomName)
							{
								// Send message
								result = send(xClient.socket, (const char*)&(reply->getBuffer()[0]), length, 0);
							}
						}
					}
					break;
					}
				default:
						break;
				}
			}
		}
	}
	SERVER.ShutDown();
}