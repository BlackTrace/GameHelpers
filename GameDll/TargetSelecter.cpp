#include "stdafx.h"
#include "TargetSelecter.h"
#include "Monster.h"
#include "utils.h"
#include "baseaddr.h"
#include "Player.h"
 CTargetSelecter* CTargetSelecter::m_pInstance = nullptr;
 std::mutex CTargetSelecter::m_mutex;

 //交换数据
 std::vector<DWORD> g_personList;
 std::vector<DWORD> g_monsterList;
 std::vector<DWORD> g_personList2;
 std::mutex g_mutex;
 HANDLE g_evt = { 0 };
CTargetSelecter::CTargetSelecter()
{
	g_evt = CreateEvent(NULL, FALSE, FALSE, NULL);
}


CTargetSelecter::~CTargetSelecter()
{
}

CTargetSelecter * CTargetSelecter::GetInstance()
{
	if (nullptr == m_pInstance)
	{
		m_mutex.lock();
		if (nullptr == m_pInstance)
		{
			m_pInstance = new CTargetSelecter();
		}
		m_mutex.unlock();
	}

	return m_pInstance;
}

bool CTargetSelecter::travse()
{

	m_personList.clear();
	m_monsterList.clear();
	m_personList2.clear();
	auto startAddr = utils::read<DWORD>(Base_MonsterArrayAddr);
	auto endAddr = utils::read<DWORD>(Base_MonsterArrayAddr + 0x4);
	if (!startAddr || !endAddr)
	{
		return false;
	}

	for (auto i = startAddr; i <= endAddr; i += 4)
	{
		auto temp = utils::read<DWORD>(i);
		if (!temp) {
			continue;
		}

		if (Monster::GetCamp(temp) != Player::GetCamp())
		{
			//如果不是友军，并且是玩家对象
			if (Monster::GetType(temp) == 0x4) 
			{
				m_personList.push_back(temp);
				m_personList2.push_back(temp);
			}
			//如果是小兵
			if (Monster::GetType(temp) == 0x6) 
			{

				//过滤未知的东子
				auto MaxHp = Monster::GetMaxHealth(temp);
				if (MaxHp< 1001.0f && MaxHp > 999.0f)
				{
					continue;
				}

				if (MaxHp < 101.0f && MaxHp > 99.0f)
				{
					continue;
				}

				//过滤和谐死亡后的眼尸体
				if (MaxHp < 77.0f && MaxHp > 75.0f)
				{
					continue;
				}


				if (MaxHp < 5.0f && MaxHp > 0.0f)
				{
					continue;
				}


				if (utils::GetDistance(Monster::GetCurrentPos(temp), Player::GetCurrPostion()) < 2000.0f * 2000.0f &&
					!Monster::BDead(temp) &&
					Monster::BVisable(temp))
				{
					m_monsterList.push_back(temp);
				}
			}
		}
	}

	swapPersonList();
	swapMonsterList();
	swapPersonList2();
	SetEvent(g_evt);

	return true;
}

void CTargetSelecter::swapPersonList2()
{
	g_mutex.lock();
	g_personList2.swap(m_personList2);
	g_mutex.unlock();
}

void CTargetSelecter::swapPersonList()
{
	g_mutex.lock();
	g_personList.swap(m_personList);
	g_mutex.unlock();
}

void CTargetSelecter::swapMonsterList()
{
	g_mutex.lock();
	g_monsterList.swap(m_monsterList);
	g_mutex.unlock();
}


DWORD GetHealthLowerestPerson(std::vector<DWORD>& list, float dis)
{
	if (dis <= 0)
	{
		return 0;
	}
	float minHealth = FLT_MAX;
	DWORD obj = 0;

	for (auto &temp : list)
	{
		if (!temp)
		{
			continue;
		}
		auto disTemp = dis + Monster::GetModelWidth(temp);
		if (Monster::BVisable(temp) && 
			!Monster::BDead(temp) &&
			utils::GetDistance(Monster::GetCurrentPos(temp), Player::GetCurrPostion()) < disTemp  * disTemp)
		{
			auto currentHealth = Monster::GetCurrentHealth(temp);
			if (currentHealth< minHealth)
			{
				minHealth = currentHealth;
				obj = temp;
			}
		}
	}

	return obj;
}

DWORD GetCassiopeiaPoisonPerson(std::vector<DWORD>& list, float dis)
{

	if (dis <= 0)
	{
		return 0;
	}
	float minHealth = FLT_MAX;
	DWORD obj = 0;

	float PoisonHealth = FLT_MAX;
	DWORD poisonObj = 0;

	for (auto temp : list)
	{
		if (!temp)
		{
			continue;
		}
		auto disTemp = dis + Monster::GetModelWidth(temp);
		if (Monster::BVisable(temp) &&
			!Monster::BDead(temp) &&
			utils::GetDistance(Monster::GetCurrentPos(temp), Player::GetCurrPostion()) < disTemp  * disTemp)
		{
			auto currentHealth = Monster::GetCurrentHealth(temp);
			if (currentHealth< minHealth)
			{
				minHealth = currentHealth;
				obj = temp;
			}


			//获取怪物buff
			CBufferService cbf(temp);
			for (auto obj : cbf.GetBufferList())
			{
				if (!Buffer::GetName(obj))
				{
					continue;
				}

				if (strstr(Buffer::GetName(obj), "assiopeia") != NULL)
				{
					//说明怪物已经中毒了
					if (currentHealth < PoisonHealth)
					{
						PoisonHealth = currentHealth;
						poisonObj = temp;
					}
				}
			}
		}
	}

	if (poisonObj)
	{
		return poisonObj;
	}

	return obj;

}

bool IsCoveredByMonster(std::vector<DWORD>& list, PEP_POINT_3D pnt, float dis, float width)
{
	//技能的弧度角大小
	float anglesTarget = atan2(pnt->y - Player::GetCurrPostion()->y, pnt->x - Player::GetCurrPostion()->x);
	//获取到小兵列表
	for (auto &temp : list)
	{

		//如果小兵死亡，或者小兵不可见，就继续找下一个小兵
		if (Monster::BDead(temp) || !Monster::BVisable(temp))
		{
			continue;
		}

		//计算小兵到玩家的圆半径
		auto disMons2Player = utils::GetDistance(Player::GetCurrPostion(), Monster::GetCurrentPos(temp));

		//如果小兵在技能范围外，就不判断处理
		auto modelWidth = Monster::GetModelWidth(temp);
		if (disMons2Player > (dis + modelWidth)* (dis + modelWidth))
		{
			continue;
		}

		//计算小兵的弧度角大小
		float angles = atan2(Monster::GetCurrentPos(temp)->y - Player::GetCurrPostion()->y, Monster::GetCurrentPos(temp)->x - Player::GetCurrPostion()->x);

		//计算小兵和技能之间的夹角,
		auto destAngles = abs(anglesTarget - angles);
		if (destAngles > PI_2)
		{
			continue;
		}
		//根据夹角求垂线距离
		auto dis2Pnt = sin(destAngles) * sqrt(disMons2Player);

		//如果小兵中心点的位置 到 技能中心垂线的距离 < 技能宽度 + 怪物半径，说明技能碰撞
		if (dis2Pnt <= (modelWidth + width))
		{
			return true;
		}

	}

	return false;
}
