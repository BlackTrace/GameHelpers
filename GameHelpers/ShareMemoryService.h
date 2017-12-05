#pragma once
#include <mutex>
#include "CShareMemoryStruct.h"
class CShareMemoryService
{
	CShareMemoryService();
	CShareMemoryService(const CShareMemoryService&) = delete;
	void operator= (const CShareMemoryService&) = delete;
public:
	static CShareMemoryService* GetInstance();
	~CShareMemoryService();
	bool Init();
	PSHARED_MEMORY_ST GetSharedMemoryPointer();
private:
	static std::mutex m_mutex;
	static CShareMemoryService* m_pInstance;
	PSHARED_MEMORY_ST m_pSharedMemoryPointer;
	HANDLE m_hMapping;
};

