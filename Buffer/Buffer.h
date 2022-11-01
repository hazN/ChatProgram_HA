/*
	Hassan Assaf
	INFO-6016
	Project #2: Authentication Server
	Due 2022-11-09
*/
#pragma once

#include <vector>
#include <string>

class Buffer {
public:
	Buffer(size_t size);

	void WriteInt32LE(std::size_t index, int32_t value);
	void WriteInt32LE(int32_t value);
	void WriteInt16LE(std::size_t index, int16_t value);
	void WriteInt16LE(int16_t value);
	void WriteString(std::size_t index, std::string value);
	void WriteString(std::string value);
	int32_t ReadInt32LE(std::size_t index);
	int32_t ReadInt32LE();
	int16_t ReadInt16LE(std::size_t index);
	int16_t ReadInt16LE();
	std::string ReadString(std::size_t index, std::size_t size);
	std::string ReadString(std::size_t size);
	size_t writeIndex = 0;
	std::vector<uint8_t> getBuffer();
	void setBuffer(std::vector<uint8_t> m_Buffer);
private:
	std::vector<uint8_t> m_Buffer;
};
