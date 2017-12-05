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

//获取血量最低的怪物
DWORD GetHealthLowerestPerson(std::vector<DWORD>& list, float dis);
//获取蛇女中毒的怪物
DWORD GetCassiopeiaPoisonPerson(std::vector<DWORD> &list, float dis);
//判断是否有小兵遮挡 (小兵列表， 目标坐标 , 技能范围)
bool IsCoveredByMonster(std::vector<DWORD>& list, PEP_POINT_3D pnt , float dis, float width);