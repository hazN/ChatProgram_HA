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
#include <thread>
#include <sstream>
#include "ClientHelper.h"

// Linking Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")
// to use for thread
bool endThread = false;

// METHOD TAKEN FROM STACKOVERFLOW: https://stackoverflow.com/questions/275404/splitting-strings-in-c
// Method: split
// Summary: splits a string based off a given delimiter
// Params: string, char
// Return: vector<string>
std::vector<std::string> split(std::string s, char delim) {
	std::vector<std::string> elems;
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

// Main
int main(int argc, char** argv)
{
	// Create the connectSocket and connect it to the ClientHelper
	SOCKET connectSocket;  
	ClientHelper CLIENT = ClientHelper(connectSocket);

	// Start client
	int result = CLIENT.Initialize();
	if (result != 0)
		return result;


	int input = 0;

	// Loop until client wants to exit so we can constantly be checking for msgs
	std::thread th([&]() {
		while (!endThread) {
			CLIENT.recvMessage(false);
			//std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
		});
	// Client Loop
	for (;;)
	{
		// User Input
		std::cout << "Choose an option: \n1. Join Room\n2. Leave Room\n3. Send Message\n4. Create Account \n5. Login\n6. Exit" << std::endl;
		std::cin >> input;

		switch (input)
		{
		case 1: // Join Room
		{
			std::cout << "Room list: \n1. Main Room\n2. Movie Room\n3. Game Room" << std::endl;
			std::cout << "Please type out the name of the room you wish to join(case sensitive), if you type a non existent room then a new one will be created." << std::endl;

			// Create packet and start setting the variables
			UserRoomPacket packet;
			packet.header.id = JOIN;

			std::cin >> packet.content.roomName;
			CLIENT.rooms.push_back(packet.content.roomName);
			packet.content.username = CLIENT.name;

			packet.header.length = sizeof(packet.header) + sizeof(packet.content.roomName.size()) + packet.content.roomName.size()
				+ sizeof(CLIENT.name.size()) + CLIENT.name.size();

			Buffer* buffer = new Buffer(packet.header.length);
			buffer->WriteInt32LE(packet.header.length);
			buffer->WriteInt32LE(packet.header.id);
			buffer->WriteInt32LE(packet.content.username.size());
			buffer->WriteString(packet.content.username);
			buffer->WriteInt32LE(packet.content.roomName.size());
			buffer->WriteString(packet.content.roomName);

			// Send data
			CLIENT.sendData(*buffer, packet.header.length);
			break;
		}
		case 2: // Leave Room
		{
			// Create Packet
			UserRoomPacket packet;
			std::cout << "Rooms you are in: " << std::endl;

			// List rooms
			for (int i = 0; i < CLIENT.rooms.size(); i++)
				std::cout << i + 1 << ". " << CLIENT.rooms[i] << std::endl;
			std::cout << "Enter the room NAME you'd like to leave: " << std::endl;
			// User input
			std::cin >> packet.content.roomName;
			for (int i = 0; i < CLIENT.rooms.size(); i++)
			{
				if (CLIENT.rooms[i] == packet.content.roomName)
					CLIENT.rooms.erase(CLIENT.rooms.begin() + i - 1);
			}
			// Fill packet with content
			packet.content.username = CLIENT.name;
			packet.header.id = 2;
			packet.header.length = sizeof(packet.header) + sizeof(packet.content.roomName.size()) + packet.content.roomName.size()
				+ sizeof(CLIENT.name.size()) + CLIENT.name.size();
			// Write to Buffer
			Buffer* buffer = new Buffer(packet.header.length);
			buffer->WriteInt32LE(packet.header.length);
			buffer->WriteInt32LE(packet.header.id);
			buffer->WriteInt32LE(packet.content.username.size());
			buffer->WriteString(packet.content.username);
			buffer->WriteInt32LE(packet.content.roomName.size());
			buffer->WriteString(packet.content.roomName);

			// Send data
			CLIENT.sendData(*buffer, packet.header.length);
			break;
		}
		case 3: // Send Message
		{
			// List rooms
			std::cout << "Rooms you are in: " << std::endl;
			for (int i = 0; i < CLIENT.rooms.size(); i++)
				std::cout << i + 1 << ". " << CLIENT.rooms[i] << std::endl;
				 // User Input
			std::string inputString;
			std::cout << "Enter room name and the message separated by a ':' (example -> Room:Hello Everyone!) " << std::endl;
			std::cin.ignore();
			std::getline(std::cin, inputString);
				// Create Packet
			SendMessagePacket packet;
			packet.header.id = 3;
				// Split with delimiter
			std::vector<std::string> temp = split(inputString, ':');
			packet.content.roomName = temp[0];
			packet.content.message = temp[1];

			packet.header.length = sizeof(packet.header) + sizeof(packet.content.roomName.size()) + packet.content.roomName.size()
				+ sizeof(packet.content.message) + packet.content.message.size();

				// Create buffer and set it up
			Buffer* buffer = new Buffer(packet.header.length);
			buffer->WriteInt32LE(packet.header.length);
			buffer->WriteInt32LE(packet.header.id);
			buffer->WriteInt32LE(packet.content.roomName.size());
			buffer->WriteString(packet.content.roomName);
			buffer->WriteInt32LE(packet.content.message.size());
			buffer->WriteString(packet.content.message);
			// Send data
			CLIENT.sendData(*buffer, packet.header.length);
			break;
		}
		case 4: // Create Account
			{
			// User Input
			std::string inputString;
			std::cout << "Enter email and password separated by a ':' (example -> h_sodeassaf@fanshaweonline.ca:password) " << std::endl;
			std::cin.ignore();
			std::getline(std::cin, inputString);
			// Create Packet
			CreateAccountPacket packet;
			packet.header.id = 4;
			// Split with delimiter
			std::vector<std::string> temp = split(inputString, ':');
			packet.content.email = temp[0];
			packet.content.password = temp[1];

			packet.header.length = sizeof(packet.header) + sizeof(packet.content.email.size()) + packet.content.email.size()
				+ sizeof(packet.content.password) + packet.content.password.size();

			// Create buffer and set it up
			Buffer* buffer = new Buffer(packet.header.length);
			buffer->WriteInt32LE(packet.header.length);
			buffer->WriteInt32LE(packet.header.id);
			buffer->WriteInt32LE(packet.content.email.size());
			buffer->WriteString(packet.content.email);
			buffer->WriteInt32LE(packet.content.password.size());
			buffer->WriteString(packet.content.password);
			// Send data
			CLIENT.sendData(*buffer, packet.header.length);
			break;
			}
		case 5: // Authenticate
			{
			// User Input
			std::string inputString;
			std::cout << "Enter email and password separated by a ':' (example -> h_sodeassaf@fanshaweonline.ca:password) " << std::endl;
			std::cin.ignore();
			std::getline(std::cin, inputString);
			// Create Packet
			CreateAccountPacket packet;
			packet.header.id = 5;
			// Split with delimiter
			std::vector<std::string> temp = split(inputString, ':');
			packet.content.email = temp[0];
			packet.content.password = temp[1];

			packet.header.length = sizeof(packet.header) + sizeof(packet.content.email.size()) + packet.content.email.size()
				+ sizeof(packet.content.password) + packet.content.password.size();

			// Create buffer and set it up
			Buffer* buffer = new Buffer(packet.header.length);
			buffer->WriteInt32LE(packet.header.length);
			buffer->WriteInt32LE(packet.header.id);
			buffer->WriteInt32LE(packet.content.email.size());
			buffer->WriteString(packet.content.email);
			buffer->WriteInt32LE(packet.content.password.size());
			buffer->WriteString(packet.content.password);
			// Send data
			CLIENT.sendData(*buffer, packet.header.length);
			break;			}
		case 6:	// Shut the client down
			endThread = true;
			CLIENT.ShutDown();
			break;
		default:
				break;
		}
	}
	CLIENT.ShutDown();
	return 0;
}