#pragma once
#include <mutex>
#include "CShareMemoryStruct.h"

class CSharedMemoryClient
{
public:
	static CSharedMemoryClient* GetInstance();
	~CSharedMemoryClient();
	bool Init();
	PSHARED_MEMORY_ST  GetSharedMemoryPointer();

private:
	CSharedMemoryClient();
	CSharedMemoryClient(const CSharedMemoryClient&) = delete;
	void operator= (const CSharedMemoryClient&) = delete;
	static std::mutex m_mutex;
	static CSharedMemoryClient* m_pInstance;
	HANDLE m_hMapping;
	PSHARED_MEMORY_ST m_pSharedMemoryPointer;
};

