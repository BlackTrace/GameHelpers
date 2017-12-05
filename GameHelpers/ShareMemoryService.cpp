#include "ShareMemoryService.h"

std::mutex CShareMemoryService::m_mutex;
CShareMemoryService* CShareMemoryService::m_pInstance = nullptr;
CShareMemoryService::CShareMemoryService()
{
	m_pSharedMemoryPointer = nullptr;
	m_hMapping = { 0 };
}


CShareMemoryService * CShareMemoryService::GetInstance()
{
	if (nullptr == m_pInstance)
	{
		if (nullptr == m_pInstance)
		{
			m_pInstance = new CShareMemoryService();
		}
	}
	return m_pInstance;
}

CShareMemoryService::~CShareMemoryService()
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

bool CShareMemoryService::Init()
{
	m_hMapping =  CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SHARED_MEMORY_ST), GAME_MAP_NAME);
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

PSHARED_MEMORY_ST CShareMemoryService::GetSharedMemoryPointer()
{
	return m_pSharedMemoryPointer;
}
