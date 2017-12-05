#include "stdafx.h"
#include "SharedMemoryClient.h"

std::mutex CSharedMemoryClient::m_mutex;
CSharedMemoryClient* CSharedMemoryClient::m_pInstance = nullptr;

CSharedMemoryClient::CSharedMemoryClient()
{
	m_hMapping = { 0 };
	m_pSharedMemoryPointer = nullptr;
}


CSharedMemoryClient * CSharedMemoryClient::GetInstance()
{
	if (nullptr == m_pInstance)
	{
		m_mutex.lock();
		if (nullptr == m_pInstance)
		{
			m_pInstance = new CSharedMemoryClient();
		}
		m_mutex.unlock();
	}
	return m_pInstance;
}

CSharedMemoryClient::~CSharedMemoryClient()
{
	if (m_pSharedMemoryPointer)
	{
		UnmapViewOfFile(m_pSharedMemoryPointer);
		m_pSharedMemoryPointer = nullptr;
	}

	if (m_hMapping)
	{
		CloseHandle(m_hMapping);
	}
}

bool CSharedMemoryClient::Init()
{
	m_hMapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, false, GAME_MAP_NAME);
	if (!m_hMapping)
	{
		return false;
	}

	m_pSharedMemoryPointer = (PSHARED_MEMORY_ST)(MapViewOfFile(m_hMapping, FILE_MAP_ALL_ACCESS, NULL, NULL, NULL));
	if (!m_pSharedMemoryPointer)
	{
		return false;
	}

	return true;
}

PSHARED_MEMORY_ST CSharedMemoryClient::GetSharedMemoryPointer()
{
	return m_pSharedMemoryPointer;
}
