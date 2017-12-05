#pragma once
#include <mutex>
#include <vector>
#include "Monster.h"
extern std::vector<DWORD> g_personList;
extern std::vector<DWORD> g_monsterList;
extern std::mutex g_mutex;
extern std::vector<DWORD> g_personList2;
extern  HANDLE g_evt;
class CTargetSelecter
{
public:
	~CTargetSelecter();
	static CTargetSelecter* GetInstance();

	bool travse();
	void swapPersonList2();
	void swapPersonList();
	void swapMonsterList();
private:
	CTargetSelecter();
	CTargetSelecter(const CTargetSelecter&)=delete;
	void operator= (const CTargetSelecter&) = delete;
	static CTargetSelecter* m_pInstance;
	static std::mutex m_mutex;
	std::vector<DWORD> m_personList;
	std::vector<DWORD> m_monsterList;
	std::vector<DWORD> m_personList2;
};

//��ȡѪ����͵Ĺ���
DWORD GetHealthLowerestPerson(std::vector<DWORD>& list, float dis);
//��ȡ��Ů�ж��Ĺ���
DWORD GetCassiopeiaPoisonPerson(std::vector<DWORD> &list, float dis);
//�ж��Ƿ���С���ڵ� (С���б� Ŀ������ , ���ܷ�Χ)
bool IsCoveredByMonster(std::vector<DWORD>& list, PEP_POINT_3D pnt , float dis, float width);