/*
	Hassan Assaf
	INFO-6016
	Project #1: Chat Program
	Due 2022-10-19
*/
#include "Buffer.h"
#include <string>
#include <iostream>

Buffer::Buffer(size_t size)
{
	m_Buffer.resize(size);
}

void Buffer::WriteInt32LE(std::size_t index, int32_t value)
{
	writeIndex = index;
	if (m_Buffer.size() < sizeof(value))
		m_Buffer.resize(m_Buffer.size() + sizeof(value));
	if (m_Buffer.size() < writeIndex + 4)
		m_Buffer.resize(m_Buffer.size() + 4);
	m_Buffer[writeIndex] = value;
	m_Buffer[++writeIndex] = value >> 8;
	m_Buffer[++writeIndex] = value >> 16;
	m_Buffer[++writeIndex] = value >> 24;
	writeIndex++;
}

void Buffer::WriteInt32LE(int32_t value)
{
	if (m_Buffer.size() < sizeof(value) || m_Buffer.size() < writeIndex + 4)
		m_Buffer.resize(m_Buffer.size() + sizeof(value));
	if (m_Buffer.size() < writeIndex + 4)
		m_Buffer.resize(m_Buffer.size() + 4);
	m_Buffer[writeIndex] = value;
	m_Buffer[++writeIndex] = value >> 8;
	m_Buffer[++writeIndex] = value >> 16;
	m_Buffer[++writeIndex] = value >> 24;
	writeIndex++;
}

void Buffer::WriteInt16LE(std::size_t index, int16_t value)
{
	writeIndex = index;
	if (m_Buffer.size() < sizeof(value))
		m_Buffer.resize(m_Buffer.size() + sizeof(value));
	if (m_Buffer.size() < writeIndex + 3)
		m_Buffer.resize(m_Buffer.size() + 3);
	m_Buffer[writeIndex] = value;
	m_Buffer[++writeIndex] = value >> 8;
	m_Buffer[++writeIndex] = value >> 8;
	writeIndex++;
}

void Buffer::WriteInt16LE(int16_t value)
{
	if (m_Buffer.size() < sizeof(value) || m_Buffer.size() < writeIndex + 3)
		m_Buffer.resize(m_Buffer.size() + sizeof(value));
	if (m_Buffer.size() < writeIndex + 3)
		m_Buffer.resize(m_Buffer.size() + 3);
	m_Buffer[writeIndex] = value;
	m_Buffer[++writeIndex] = value >> 8;
	m_Buffer[++writeIndex] = value >> 8;
	writeIndex++;
}

void Buffer::WriteString(std::size_t index, std::string value)
{
	writeIndex = index;
	if ((m_Buffer.size() - writeIndex) < value.length())
		m_Buffer.resize(m_Buffer.size() + value.length());
	for (char val : value)
	{
		m_Buffer[writeIndex++] = val;
	}
}

void Buffer::WriteString(std::string value)
{
	if ((m_Buffer.size() - writeIndex) < value.length())
		m_Buffer.resize(m_Buffer.size() + value.length());
	for (char val : value)
	{
		m_Buffer[writeIndex++] = val;
	}
}

int32_t Buffer::ReadInt32LE(std::size_t index)
{
	writeIndex = index;
	if (m_Buffer.size() - writeIndex < 4)
		writeIndex = 0;
	int32_t value = m_Buffer[writeIndex];		
	value |= m_Buffer[++writeIndex] << 8;
	value |= m_Buffer[++writeIndex] << 16;
	value |= m_Buffer[++writeIndex] << 24;
	writeIndex++;
	return value;
}

int32_t Buffer::ReadInt32LE()
{
	if (m_Buffer.size() - writeIndex < 4)
		writeIndex = 0;
	int32_t value = m_Buffer[writeIndex];
	value |= m_Buffer[++writeIndex] << 8;
	value |= m_Buffer[++writeIndex] << 16;
	value |= m_Buffer[++writeIndex] << 24;
	writeIndex++;
	return value;
}

int16_t Buffer::ReadInt16LE(std::size_t index)
{
	writeIndex = index;
	if (m_Buffer.size() - writeIndex < 3)
		writeIndex = 0;
	int16_t value = m_Buffer[writeIndex];
	value |= m_Buffer[++writeIndex] << 8;
	value |= m_Buffer[++writeIndex] << 16;
	writeIndex++;
	return value;
}

int16_t Buffer::ReadInt16LE()
{
	if (m_Buffer.size() - writeIndex < 3)
		writeIndex = 0;
	int16_t value = m_Buffer[writeIndex];
	value |= m_Buffer[++writeIndex] << 8;
	value |= m_Buffer[++writeIndex] << 16;
	writeIndex++;
	return value;
}

std::string Buffer::ReadString(std::size_t index, std::size_t size)
{
	writeIndex = index;
	std::string value = "";
	for (size_t i = 0; i < size; i++)
	{
		if (writeIndex >= m_Buffer.size())
			return value;
		value += m_Buffer[writeIndex++];
	}
	return value;
}

std::string Buffer::ReadString(std::size_t size)
{
	std::string value = "";
	for (size_t i = 0; i < size; i++)
	{
		if (writeIndex >= m_Buffer.size())
			return value;
		value += m_Buffer[writeIndex++];
	}
	return value;
}

std::vector<uint8_t> Buffer::getBuffer()
{
	return m_Buffer;
}

void Buffer::setBuffer(std::vector<uint8_t> m_Buffer)
{
	this->m_Buffer = m_Buffer;
}
