#include "stdafx.h"
#include "Player.h"
#include "baseaddr.h"
#include "utils.h"
#include "GameCall.h"
#include "HookToMainThread.h"

//全局变量
std::shared_ptr<CSkillService> m_skService = nullptr;
std::shared_ptr<CBufferService> m_buffService = nullptr;
float m_ZouKanTimeSec[2] = {0};


void Player::ZouA2Mons(DWORD target, bool tag)
{
	static bool bDefense = false;
	//timeSec[0]攻击后摇  timeSec[1]攻击前摇
	if (GameCall::GetGameTickTime() > m_ZouKanTimeSec[1])
	{
		if (target && tag)
		{
			//走砍并计算攻击前后摇
			DWORD monObj = target;
			CHookToMainThread::GetInstance()->SendMessageToMainThread(MSG_ATTACKCALL , (LPARAM)&monObj);

			auto attackSpeed = Player::GetAttackSpeed();

			m_ZouKanTimeSec[0] = GameCall::GetGameTickTime() + (attackSpeed - attackSpeed * sub_10008CC4(attackSpeed)) / 1.4f;
			m_ZouKanTimeSec[1] = GameCall::GetGameTickTime() + attackSpeed;

			bDefense = true;
		}
		else
		{
			CHookToMainThread::GetInstance()->SendMessageToMainThread(MSG_FINDWAY, NULL);
		}
	}
	else if (GameCall::GetGameTickTime() > m_ZouKanTimeSec[0])
	{
		//如果达到攻击后摇时间
		if (bDefense)
		{
			CHookToMainThread::GetInstance()->SendMessageToMainThread(MSG_DEFENSE, NULL);
			bDefense = false;
		}
		CHookToMainThread::GetInstance()->SendMessageToMainThread(MSG_FINDWAY, NULL);
	}
}

float Player::sub_10008CC4(float a1)
{

	if (a1 >= 1.7f) {
		return 0.65f;
	}
	else if (a1 < 1.7f && a1 > 0.3f) {
		return (a1 - 0.25f) * 0.2f + 0.4f;
	}
	else {
		return 0.2f;
	}
}

void Player::UseSkillToMons(DWORD mons, DWORD skIndex, float skDelay, float MissileSpeed, bool bAttack)
{

	//如果时间小于攻击后摇时间，不使用技能
	if (GameCall::GetGameTickTime() < m_ZouKanTimeSec[0])
	{
		return;
	}

	auto sk = GetSkillByIndex(skIndex);
	if (!sk)
	{
		return;
	}

	//根据技能类型确定技能参数
	switch (sk->GetSkillType())
	{
	case SK_SKILL_LINE :  //指向性技能，大嘴Q
	{
		//判断怪物是否存在
		if (!mons)
		{
			return;
		}
		//判断技能是否可用
		if (sk->GetLevel() < 1 || !sk->BCoolDown() || Player::GetCurrMp() < sk->GetExpendMp())
		{
			return;
		}

		//如果怪物死亡 || 怪物不可见
		if (Monster::BDead(mons) || !Monster::BVisable(mons))
		{
			return;
		}
		//如果怪物和人物之间的距离 > 技能范围
		auto dis = sk->GetDistance();
		if (utils::GetDistance(Player::GetCurrPostion(), Monster::GetCurrentPos(mons)) > dis * dis)
		{
			return;
		}
		SKILL_TO_MONS sk2mons = { 0 };
		memcpy(&sk2mons.pCurrPnt, Player::GetCurrPostion(), sizeof(EP_POINT_3D));
		//计算预判坐标
		if (Monster::BMoving(mons))
		{
			//获取到延迟后的坐标
			EP_POINT_3D pnt;
			float delay = 0.0f;
			if (MissileSpeed > 0)
			{
				delay = skDelay + sqrt(utils::GetDistance(Player::GetCurrPostion(), Monster::GetCurrentPos(mons)) / MissileSpeed );
			}
			else
			{
				delay = skDelay;
			}

			//计算预判坐标
			auto disMonsMove = Monster::GetMoveSpeed(mons) * delay;
			auto angles = atan2(Monster::GetOrientation(mons)->y, Monster::GetOrientation(mons)->x);
			pnt.x = Monster::GetCurrentPos(mons)->x + cos(angles) * disMonsMove ;
			pnt.z = Monster::GetCurrentPos(mons)->z;
			pnt.y = Monster::GetCurrentPos(mons)->y + sin(angles) * disMonsMove;
			memcpy(&sk2mons.pAskPnt, &pnt, sizeof(EP_POINT_3D));
		}
		else {
			memcpy(&sk2mons.pAskPnt, Monster::GetCurrentPos(mons), sizeof(EP_POINT_3D));
		}

		//然后把计算出来的圆和直线的交点作为第二个参数
		float angles = atan2(Monster::GetCurrentPos(mons)->y - Player::GetCurrPostion()->y, Monster::GetCurrentPos(mons)->x - Player::GetCurrPostion()->x);
		sk2mons.pDestPnt.x = cos(angles) * sk->GetDistance() + Player::GetCurrPostion()->x;
		sk2mons.pDestPnt.y = sin(angles) * sk->GetDistance() + Player::GetCurrPostion()->y;
		sk2mons.pDestPnt.z = Monster::GetCurrentPos(mons)->z;

		sk2mons.targetId = 0;
		sk2mons.skillIndex = skIndex;
		sk2mons.skillBase = sk->GetNodeBase();
		//组包完成，开始发包
		GameCall::UseSkill(sk2mons.targetId, &sk2mons.pAskPnt, &sk2mons.pDestPnt, sk2mons.skillIndex, sk2mons.skillBase);
		//CHookToMainThread::GetInstance()->SendMessageToMainThread(MSG_USESKILL, (LPARAM)(&sk2mons));
		if (bAttack)
		{
			auto dis = Player::GetAttackDistance() + Monster::GetModelWidth(mons);
			if (utils::GetDistance(Player::GetCurrPostion(), Monster::GetCurrentPos(mons)) < dis * dis)
			{
				Sleep(300);
				ZouA2Mons(mons, true);
			}
		}
		break;
	}
	case SK_SKILL_LOCK :   //锁定技能，比如安妮Q
	{
		//判断怪物是否存在
		if (!mons)
		{
			return;
		}
		//判断技能是否可用
		if (sk->GetLevel() < 1
			|| !sk->BCoolDown()
			|| Player::GetCurrMp() < sk->GetExpendMp())
		{
			return;
		}

		//如果怪物死亡 || 怪物不可见
		if (Monster::BDead(mons) || !Monster::BVisable(mons))
		{
			return;
		}

		//如果怪物和人物之间的距离 > 技能范围
		auto dis = sk->GetDistance();
		if (utils::GetDistance(Player::GetCurrPostion(), Monster::GetCurrentPos(mons)) > dis * dis)
		{
			return;
		}

		//组包
		SKILL_TO_MONS sk2mons = { 0 };
		memcpy(&sk2mons.pCurrPnt, Player::GetCurrPostion(), sizeof(EP_POINT_3D));
		memcpy(&sk2mons.pDestPnt, Monster::GetCurrentPos(mons), sizeof(EP_POINT_3D));
		sk2mons.targetId = Monster::GetObjectForSkill(mons);
		sk2mons.skillIndex = skIndex;
		sk2mons.skillBase = sk->GetNodeBase();

		//发包
		GameCall::UseSkill(sk2mons.targetId, &sk2mons.pAskPnt, &sk2mons.pDestPnt, sk2mons.skillIndex, sk2mons.skillBase);
		//CHookToMainThread::GetInstance()->SendMessageToMainThread(MSG_USESKILL, (LPARAM)(&sk2mons));
		if (bAttack)
		{
			auto dis = Player::GetAttackDistance() + Monster::GetModelWidth(mons);
			if (utils::GetDistance(Player::GetCurrPostion(), Monster::GetCurrentPos(mons)) < dis * dis)
			{
				Sleep(300);
				ZouA2Mons(mons, true);
			}
		}
		break;
	}
	case SK_SKILL_SECTOR :  //扇形技能，比如安妮W
	{
		//判断怪物是否存在
		if (!mons)
		{
			return;
		}
		//判断技能是否可用
		if (sk->GetLevel() < 1
			|| !sk->BCoolDown()
			|| Player::GetCurrMp() < sk->GetExpendMp())
		{
			return;
		}

		//如果怪物死亡 || 怪物不可见
		if (Monster::BDead(mons) || !Monster::BVisable(mons))
		{
			return;
		}

		//如果怪物和人物之间的距离 > 技能范围
		auto dis = sk->GetDistance();
		if (utils::GetDistance(Player::GetCurrPostion(), Monster::GetCurrentPos(mons)) > dis * dis)
		{
			return;
		}

		//组包
		SKILL_TO_MONS sk2mons = { 0 };
		memcpy(&sk2mons.pCurrPnt, Player::GetCurrPostion(), sizeof(EP_POINT_3D));
		//计算预判坐标
		if (Monster::BMoving(mons)) 
		{
			//获取到延迟后的坐标
			EP_POINT_3D pnt;
			float delay = 0.0f;
			if (MissileSpeed > 0)
			{
				delay = skDelay + sqrt(utils::GetDistance(Player::GetCurrPostion(), Monster::GetCurrentPos(mons)) / MissileSpeed);
			}
			else
			{
				delay = skDelay;
			}

			//计算预判坐标
			auto disMonsMove = Monster::GetMoveSpeed(mons) * delay;
			auto angles = atan2(Monster::GetOrientation(mons)->y, Monster::GetOrientation(mons)->x);
			pnt.x = Monster::GetCurrentPos(mons)->x + cos(angles) * disMonsMove;
			pnt.z = Monster::GetCurrentPos(mons)->z;
			pnt.y = Monster::GetCurrentPos(mons)->y + sin(angles) * disMonsMove;
			memcpy(&sk2mons.pDestPnt, &pnt, sizeof(EP_POINT_3D));
		}
		else 
		{
			memcpy(&sk2mons.pDestPnt, Monster::GetCurrentPos(mons), sizeof(EP_POINT_3D));
		}
		memcpy(&sk2mons.pAskPnt, Player::GetCurrPostion(), sizeof(EP_POINT_3D));
		sk2mons.targetId = 0;
		sk2mons.skillIndex = skIndex;
		sk2mons.skillBase = sk->GetNodeBase();
		//发包
		GameCall::UseSkill(sk2mons.targetId, &sk2mons.pAskPnt, &sk2mons.pDestPnt, sk2mons.skillIndex, sk2mons.skillBase);
		//CHookToMainThread::GetInstance()->SendMessageToMainThread(MSG_USESKILL, (LPARAM)(&sk2mons));
		if (bAttack)
		{
			auto dis = Player::GetAttackDistance() + Monster::GetModelWidth(mons);
			if (utils::GetDistance(Player::GetCurrPostion(), Monster::GetCurrentPos(mons)) < dis * dis)
			{
				Sleep(300);
				ZouA2Mons(mons, true);
			}
		}
		break;
	}
	case SK_SKILL_ROUND : //圆形技能，大嘴R ,安妮R
	case SK_SKILL_ROUND2: //泽拉斯R , 逆羽Q
	{
		//判断怪物是否存在
		if (!mons)
		{
			return;
		}
		//判断技能是否可用
		if (sk->GetLevel() < 1
			|| !sk->BCoolDown()
			|| Player::GetCurrMp() < sk->GetExpendMp())
		{
			return;
		}

		//如果怪物死亡 || 怪物不可见
		if (Monster::BDead(mons) || !Monster::BVisable(mons))
		{
			return;
		}

		//如果怪物和人物之间的距离 > 技能范围
		auto dis = sk->GetDistance();
		if (utils::GetDistance(Player::GetCurrPostion(), Monster::GetCurrentPos(mons)) > dis * dis)
		{
			return;
		}

		//组包
		SKILL_TO_MONS sk2mons = { 0 };
		memcpy(&sk2mons.pCurrPnt, Player::GetCurrPostion(), sizeof(EP_POINT_3D));
		//计算预判坐标
		if (Monster::BMoving(mons))
		{
			//获取到延迟后的坐标
			//获取到延迟后的坐标
			EP_POINT_3D pnt;
			float delay = 0.0f;
			if (MissileSpeed > 0)
			{
				delay = skDelay + sqrt(utils::GetDistance(Player::GetCurrPostion(), Monster::GetCurrentPos(mons)) / MissileSpeed);
			}
			else
			{
				delay = skDelay;
			}

			//计算预判坐标
			auto disMonsMove = Monster::GetMoveSpeed(mons) * delay;
			auto angles = atan2(Monster::GetOrientation(mons)->y, Monster::GetOrientation(mons)->x);
			pnt.x = Monster::GetCurrentPos(mons)->x + cos(angles) * disMonsMove;
			pnt.z = Monster::GetCurrentPos(mons)->z;
			pnt.y = Monster::GetCurrentPos(mons)->y + sin(angles) * disMonsMove;
			memcpy(&sk2mons.pDestPnt, &pnt, sizeof(EP_POINT_3D));
		}
		else
		{
			memcpy(&sk2mons.pDestPnt, Monster::GetCurrentPos(mons), sizeof(EP_POINT_3D));
		}

		sk2mons.targetId = 0;
		sk2mons.skillIndex = skIndex;
		sk2mons.skillBase = sk->GetNodeBase();
		//发包
		GameCall::UseSkill(sk2mons.targetId, &sk2mons.pAskPnt, &sk2mons.pDestPnt, sk2mons.skillIndex, sk2mons.skillBase);
		//CHookToMainThread::GetInstance()->SendMessageToMainThread(MSG_USESKILL, (LPARAM)(&sk2mons));
		if (bAttack)
		{
			auto dis = Player::GetAttackDistance() + Monster::GetModelWidth(mons);
			if (utils::GetDistance(Player::GetCurrPostion(), Monster::GetCurrentPos(mons)) < dis * dis)
			{
				Sleep(300);
				ZouA2Mons(mons, true);
			}
		}
		break;
	}
	case SK_SKILL_TOSELF: //对自己释放的技能
	{
		//判断怪物是否存在
		if (!mons)
		{
			return;
		}
		//判断技能是否可用
		if (sk->GetLevel() < 1
			|| !sk->BCoolDown()
			|| Player::GetCurrMp() < sk->GetExpendMp())
		{
			return;
		}

		//如果怪物死亡 || 怪物不可见
		if (Monster::BDead(mons) || !Monster::BVisable(mons))
		{
			return;
		}

		SKILL_TO_MONS sk2mons = { 0 };
		sk2mons.targetId = GetObjectForSkill();
		memcpy(&sk2mons.pCurrPnt, Player::GetCurrPostion(), sizeof(EP_POINT_3D));
		memcpy(&sk2mons.pDestPnt, Player::GetCurrPostion(), sizeof(EP_POINT_3D));
		sk2mons.skillIndex = sk->GetIndex();
		sk2mons.skillBase = sk->GetNodeBase();
		GameCall::UseSkill(sk2mons.targetId, &sk2mons.pAskPnt, &sk2mons.pDestPnt, sk2mons.skillIndex, sk2mons.skillBase);
		//CHookToMainThread::GetInstance()->SendMessageToMainThread(MSG_USESKILL, (LPARAM)(&sk2mons));
		break;
	}
	case SK_SKILL_VIKTOR: //维克托的E
	{
		//判断怪物是否存在
		if (!mons)
		{
			return;
		}
		//判断技能是否可用
		if (sk->GetLevel() < 1
			|| !sk->BCoolDown()
			|| Player::GetCurrMp() < sk->GetExpendMp())
		{
			return;
		}

		//如果怪物死亡 || 怪物不可见
		if (Monster::BDead(mons) || !Monster::BVisable(mons))
		{
			return;
		}

		//如果怪物和人物之间的距离 > 技能范围
		auto dis = sk->GetDistance();
		if (utils::GetDistance(Player::GetCurrPostion(), Monster::GetCurrentPos(mons)) < dis * dis)
		{
			SKILL_TO_MONS sk2mons = { 0 };
			memcpy(&sk2mons.pCurrPnt, Player::GetCurrPostion(), sizeof(EP_POINT_3D));

			//计算预判坐标
			if (Monster::BMoving(mons))
			{
				//获取到延迟后的坐标
				EP_POINT_3D pnt;
				float delay = 0.0f;
				if (MissileSpeed > 0)
				{
					delay = skDelay + sqrt(utils::GetDistance(Player::GetCurrPostion(), Monster::GetCurrentPos(mons)) / MissileSpeed);
				}
				else
				{
					delay = skDelay;
				}

				//计算预判坐标
				auto disMonsMove = Monster::GetMoveSpeed(mons) * delay;
				auto angles = atan2(Monster::GetOrientation(mons)->y, Monster::GetOrientation(mons)->x);
				pnt.x = Monster::GetCurrentPos(mons)->x + cos(angles) * disMonsMove;
				pnt.z = Monster::GetCurrentPos(mons)->z;
				pnt.y = Monster::GetCurrentPos(mons)->y + sin(angles) * disMonsMove;
				memcpy(&sk2mons.pAskPnt, &pnt, sizeof(EP_POINT_3D));
			}
			else
			{
				memcpy(&sk2mons.pAskPnt, Monster::GetCurrentPos(mons), sizeof(EP_POINT_3D));
			}
			memcpy(&sk2mons.pDestPnt, Player::GetCurrPostion(), sizeof(EP_POINT_3D));
			sk2mons.targetId = 0;
			sk2mons.skillIndex = skIndex;
			sk2mons.skillBase = sk->GetNodeBase();
			//发包
			GameCall::UseSkill(sk2mons.targetId, &sk2mons.pAskPnt, &sk2mons.pDestPnt, sk2mons.skillIndex, sk2mons.skillBase);
			//CHookToMainThread::GetInstance()->SendMessageToMainThread(MSG_USESKILL, (LPARAM)(&sk2mons));
			if (bAttack)
			{
				auto dis = Player::GetAttackDistance() + Monster::GetModelWidth(mons);
				if (utils::GetDistance(Player::GetCurrPostion(), Monster::GetCurrentPos(mons)) < dis * dis)
				{
					Sleep(300);
					ZouA2Mons(mons, true);
				}
			}
		}
		else 
		{
			SKILL_TO_MONS sk2mons = { 0 };
			memcpy(&sk2mons.pCurrPnt, Player::GetCurrPostion(), sizeof(EP_POINT_3D));

			//计算预判坐标
			if (Monster::BMoving(mons))
			{
				//获取到延迟后的坐标
				EP_POINT_3D pnt;
				float delay = 0.0f;
				if (MissileSpeed > 0)
				{
					delay = skDelay + sqrt(utils::GetDistance(Player::GetCurrPostion(), Monster::GetCurrentPos(mons)) / MissileSpeed);
				}
				else
				{
					delay = skDelay;
				}

				//计算预判坐标
				auto disMonsMove = Monster::GetMoveSpeed(mons) * delay;
				auto angles = atan2(Monster::GetOrientation(mons)->y, Monster::GetOrientation(mons)->x);
				pnt.x = Monster::GetCurrentPos(mons)->x + cos(angles) * disMonsMove;
				pnt.z = Monster::GetCurrentPos(mons)->z;
				pnt.y = Monster::GetCurrentPos(mons)->y + sin(angles) * disMonsMove;
				memcpy(&sk2mons.pAskPnt, &pnt, sizeof(EP_POINT_3D));
			}
			else
			{
				memcpy(&sk2mons.pAskPnt, Monster::GetCurrentPos(mons), sizeof(EP_POINT_3D));
			}

			float angles = atan2(Monster::GetCurrentPos(mons)->y - Player::GetCurrPostion()->y, Monster::GetCurrentPos(mons)->x - Player::GetCurrPostion()->x);
			sk2mons.pDestPnt.x = cos(angles) * sk->GetDistance() + Player::GetCurrPostion()->x;
			sk2mons.pDestPnt.y = sin(angles) * sk->GetDistance() + Player::GetCurrPostion()->y;
			sk2mons.pDestPnt.z = Monster::GetCurrentPos(mons)->z;

			sk2mons.targetId = 0;
			sk2mons.skillIndex = skIndex;
			sk2mons.skillBase = sk->GetNodeBase();
			//发包		
			GameCall::UseSkill(sk2mons.targetId, &sk2mons.pAskPnt, &sk2mons.pDestPnt, sk2mons.skillIndex, sk2mons.skillBase);
			//CHookToMainThread::GetInstance()->SendMessageToMainThread(MSG_USESKILL, (LPARAM)(&sk2mons));
			if (bAttack)
			{
				auto dis = Player::GetAttackDistance() + Monster::GetModelWidth(mons);
				if (utils::GetDistance(Player::GetCurrPostion(), Monster::GetCurrentPos(mons)) < dis * dis)
				{
					Sleep(300);
					ZouA2Mons(mons, true);
				}
			}
		}
		break;
	}
	default:
		break;
	}
}

void Player::UseSkillToMons2(DWORD mons, DWORD skIndex, float MissileSpeed, float skDelay)
{
	//判断怪物是否存在
	if (!mons)
	{
		return;
	}

	auto sk = GetSkillByIndex(skIndex);
	if (!sk)
	{
		return;
	}

	//判断技能是否可用
	if (sk->GetLevel() < 1 || !sk->BCoolDown() /*|| Player::GetCurrMp() < sk->GetExpendMp()*/)
	{
		return;
	}

	//如果怪物死亡 || 怪物不可见
	if (Monster::BDead(mons) || !Monster::BVisable(mons))
	{
		return;
	}

	//如果怪物和人物之间的距离 > 技能范围
	auto dis = sk->GetDistance() + Monster::GetModelWidth(mons);
	if (utils::GetDistance(Player::GetCurrPostion(), Monster::GetCurrentPos(mons)) > dis * dis)
	{
		return;
	}

	SKILL_TO_MONS sk2mons = { 0 };
	memcpy(&sk2mons.pCurrPnt, Player::GetCurrPostion(), sizeof(EP_POINT_3D));
	//计算预判坐标
	if (Monster::BMoving(mons))
	{
		//获取到延迟后的坐标
		EP_POINT_3D pnt;
		pnt.x = Monster::GetCurrentPos(mons)->x + Monster::GetOrientation(mons)->x * Monster::GetMoveSpeed(mons) * skDelay;
		pnt.z = Monster::GetCurrentPos(mons)->z;
		pnt.y = Monster::GetCurrentPos(mons)->y + Monster::GetOrientation(mons)->y * Monster::GetMoveSpeed(mons) * skDelay;

		memcpy(&sk2mons.pAskPnt, &pnt, sizeof(EP_POINT_3D));
	}
	else 
	{
		memcpy(&sk2mons.pAskPnt, Monster::GetCurrentPos(mons), sizeof(EP_POINT_3D));
	}

	sk2mons.targetId = 0;
	sk2mons.skillIndex = skIndex;
	sk2mons.skillBase = sk->GetNodeBase();
	//组包完成，开始发包
	CHookToMainThread::GetInstance()->SendMessageToMainThread(MSG_USESKILL2, (LPARAM)(&sk2mons));
}

void Player::KalistaAutoE2Object(DWORD mons)
{
	if (!mons)
	{
		return;
	}

	if (Monster::BDead(mons) || !Monster::BVisable(mons) || Monster::GetCurrentHealth(mons) < 4.0f)
	{
		return;
	}

	CBufferService tempBufferService(mons);
	tempBufferService.travse();

	for (auto buff : tempBufferService.GetBufferList())
	{
		if (nullptr == Buffer::GetName(buff))
		{
			continue;
		}

		if (strstr(Buffer::GetName(buff), "gemarker") == NULL)
		{
			continue;
		}

		auto BufferCount = Buffer::GetBufferCount(buff);
		if (0 == BufferCount)
		{
			BufferCount = 1;
		}

		auto ske = Player::GetSkillByIndex(2);
		if (!ske)
		{
			return;
		}

		if (!ske->BCoolDown() || ske->GetLevel() < 1 || Player::GetCurrMp() < ske->GetExpendMp())
		{
			return;
		}

		//计算护甲
		auto defense = Monster::GetDefensePower(mons) - Player::GetArmour();
		if (defense < 0)
		{
			defense = 0;
		}
		//伤害计算公式 (技能伤害 + 攻击力加成比例) * （100.0f / (100.0f + 防御力*（1.0 - 穿透率）)）；
		auto hurt = (ske->GetSkillAggressivity() + 0.55f * Player::GetAttackPower()) * (100.0f / (100.0f + (defense * (1.0f - Player::GetArmourPenetration() / 100.0f)))); //减伤比例
		auto destHurt = hurt * (1.0f + (float)(BufferCount - 1) * (0.3f + (float)ske->GetLevel() * 0.05f));
		if (destHurt > Monster::GetCurrentHealth(mons) &&
			Monster::GetCurrentHealth(mons) > 3.0f)
		{
			static DWORD timesec = 0;
			if (GetTickCount() - timesec > 500) {
				UseSkillToMons(mons, 2, 0 , 0,false);
				timesec = GetTickCount();
			}

		}
		
	}
}



//Player::Player()
//{
//	m_playerObj = utils::read<DWORD>(Base_RoleSelfAddr);
//	if (m_playerObj)
//	{
//		m_skService = std::shared_ptr<CSkillService>(new CSkillService(m_playerObj));
//		m_skService->travse();
//		m_buffService = std::shared_ptr<CBufferService>(new CBufferService(m_playerObj));
//		m_buffService->travse();
//	}
//
//	m_ZouKanTimeSec[0] = 0.0f;
//	m_ZouKanTimeSec[1] = 0.0f;
//}



char * Player::GetName()
{
	
	auto m_playerObj = utils::read<DWORD>(Base_RoleSelfAddr);
	if (m_playerObj)
	{
		return Monster::GetName(m_playerObj);
	}
	return nullptr;
}

char * Player::GetHeroName()
{
	auto m_playerObj = utils::read<DWORD>(Base_RoleSelfAddr);
	if (m_playerObj)
	{
		return Monster::GetHeroName(m_playerObj);
	}
	return nullptr;
}

float Player::GetCurrHp()
{
	auto m_playerObj = utils::read<DWORD>(Base_RoleSelfAddr);
	if (m_playerObj)
	{
		return Monster::GetCurrentHealth(m_playerObj);
	}
	return 0.0f;
}

float Player::GetMaxHp()
{
	auto m_playerObj = utils::read<DWORD>(Base_RoleSelfAddr);
	if (m_playerObj)
	{
		return Monster::GetMaxHealth(m_playerObj);
	}
	return 0.0f;
}

float Player::GetCurrMp()
{
	auto m_playerObj = utils::read<DWORD>(Base_RoleSelfAddr);
	if (m_playerObj)
	{
		return Monster::GetCurrentMagic(m_playerObj);
	}
	return 0.0f;
}

float Player::GetMaxMp()
{
	auto m_playerObj = utils::read<DWORD>(Base_RoleSelfAddr);
	if (m_playerObj)
	{
		return Monster::GetMaxMagic(m_playerObj);
	}
	return 0.0f;
}

PEP_POINT_3D Player::GetCurrPostion() 
{
	auto m_playerObj = utils::read<DWORD>(Base_RoleSelfAddr);
	if (m_playerObj)
	{
		return Monster::GetCurrentPos(m_playerObj);
	}
	return nullptr;
}

bool Player::BDead() 
{
	auto m_playerObj = utils::read<DWORD>(Base_RoleSelfAddr);
	if (m_playerObj)
	{
		return Monster::BDead(m_playerObj);
	}
	return true;
}

bool Player::BVisableSee() 
{
	auto m_playerObj = utils::read<DWORD>(Base_RoleSelfAddr);
	if (m_playerObj)
	{
		return Monster::BVisable(m_playerObj);
	}
	return false;
}

bool Player::BMoving() 
{
	auto m_playerObj = utils::read<DWORD>(Base_RoleSelfAddr);
	if (m_playerObj)
	{
		return Monster::BMoving(m_playerObj);
	}
	return false;
}

PEP_POINT_3D Player::GetOrientation() 
{
	auto m_playerObj = utils::read<DWORD>(Base_RoleSelfAddr);
	if (m_playerObj)
	{
		return Monster::GetOrientation(m_playerObj);
	}
	return nullptr;
}

DWORD Player::GetCamp() 
{
	auto m_playerObj = utils::read<DWORD>(Base_RoleSelfAddr);
	if (m_playerObj)
	{
		return Monster::GetCamp(m_playerObj);
	}
	return 0;
}

DWORD Player::GetType() 
{
	auto m_playerObj = utils::read<DWORD>(Base_RoleSelfAddr);
	if (m_playerObj)
	{
		return Monster::GetType(m_playerObj);
	}
	return 0;
}

DWORD Player::GetLevel() 
{
	return 0;
}

float Player::GetAttackPower() 
{
	auto dwBase = utils::read<DWORD>(Base_UIAddr);
	if (dwBase)
	{
		auto dwOffset1 = utils::read<DWORD>(dwBase + 0x28);
		if (dwOffset1)
		{
			auto dwOffset2 = utils::read<DWORD>(dwOffset1 + 0x4);
			if (dwOffset2)
			{
				auto dwOffset3 = utils::read<DWORD>(dwOffset2 + 0x10);
				if (dwOffset3)
				{
					return  utils::read<float>(dwOffset3 + 0x10);
				}
			}
		}
	}
	return 0.0f;
}

float Player::GetMoveSpeed() 
{
	auto m_playerObj = utils::read<DWORD>(Base_RoleSelfAddr);
	if (m_playerObj)
	{
		return Monster::GetMoveSpeed(m_playerObj);
	}
	return 0;
}

float Player::GetAttackSpeed() 
{
	auto dwBase = utils::read<DWORD>(Base_UIAddr);
	if (dwBase)
	{
		auto dwOffset1 = utils::read<DWORD>(dwBase + 0x28);
		if (dwOffset1)
		{
			auto dwOffset2 = utils::read<DWORD>(dwOffset1 + 0x4);
			if (dwOffset2)
			{
				auto dwOffset3 = utils::read<DWORD>(dwOffset2 + 0x10);
				if (dwOffset3)
				{
					auto Speed = utils::read<float>(dwOffset3 + 0x18);
					if (Speed)
					{
						return (float)(1.0 / Speed);
					}
				}
			}
		}
	}
	return 0.0f;
}

float Player::GetAttackDistance() 
{
	auto dwBase = utils::read<DWORD>(Base_UIAddr);
	if (dwBase)
	{
		auto dwOffset1 = utils::read<DWORD>(dwBase + 0x28);
		if (dwOffset1)
		{
			auto dwOffset2 = utils::read<DWORD>(dwOffset1 + 0x4);
			if (dwOffset2)
			{
				auto dwOffset3 = utils::read<DWORD>(dwOffset2 + 0x10);
				if (dwOffset3)
				{
					return utils::read<float>(dwOffset3 + 0x68);

				}
			}
		}
	}
	return 0.0f;
}

Skill* Player::GetSkillByIndex(DWORD index)
{
	if (!m_skService)
	{
		auto m_playerObj = utils::read<DWORD>(Base_RoleSelfAddr);
		if (m_playerObj)
		{
			m_skService = std::shared_ptr<CSkillService>(new CSkillService(m_playerObj));
			m_skService->travse();
		}
	}
	return m_skService->GetSkillByIndex(index);
}



float Player::GetArmour() 
{
	auto dwBase = utils::read<DWORD>(Base_UIAddr);
	if (dwBase)
	{
		auto dwOffset1 = utils::read<DWORD>(dwBase + 0x28);
		if (dwOffset1)
		{
			auto dwOffset2 = utils::read<DWORD>(dwOffset1 + 0x4);
			if (dwOffset2)
			{
				auto dwOffset3 = utils::read<DWORD>(dwOffset2 + 0x10);
				if (dwOffset3)
				{
					return utils::read<float>(dwOffset3 + 0x58);
				}
			}
		}
	}
	return 0.0f;
}

float Player::GetArmourPenetration() 
{
	auto dwBase = utils::read<DWORD>(Base_UIAddr);
	if (dwBase)
	{
		auto dwOffset1 = utils::read<DWORD>(dwBase + 0x28);
		if (dwOffset1)
		{
			auto dwOffset2 = utils::read<DWORD>(dwOffset1 + 0x4);
			if (dwOffset2)
			{
				auto dwOffset3 = utils::read<DWORD>(dwOffset2 + 0x10);
				if (dwOffset3)
				{
					return utils::read<float>(dwOffset3 + 0x5c);
				}
			}
		}
	}
	return 0.0f;
}

float Player::GetMonsModelSize() 
{
	return 0.0f;
}

float Player::GetSpellPower() 
{
	auto dwBase = utils::read<DWORD>(Base_UIAddr);
	if (dwBase)
	{
		auto dwOffset1 = utils::read<DWORD>(dwBase + 0x28);
		if (dwOffset1)
		{
			auto dwOffset2 = utils::read<DWORD>(dwOffset1 + 0x4);
			if (dwOffset2)
			{
				auto dwOffset3 = utils::read<DWORD>(dwOffset2 + 0x10);
				if (dwOffset3)
				{
					return utils::read<float>(dwOffset3 + 0x0);
				}
			}
		}
	}
	return 0.0f;
}

float Player::GetXerathPowerTime() 
{
	auto skObj = GetSkillByIndex(0);
	if (skObj)
	{
		return skObj->GetXerathPowerTime();
	}
	return 0.0f;
}

float Player::GetXerathSkillCurrDistance()
{
	float dis = 0.0f;
	auto time = Player::GetXerathPowerTime();
	if (time > 0) {
		auto tempTime = GameCall::GetGameTickTime() - time;
		dis = tempTime * 450.0f - 100.0f;

		if (dis > 700.0f) {
			dis = 700.0f;
		}
	}

	return (700.0f + dis);
}

DWORD Player::GetObjectForSkill()
{
	auto m_playerObj = utils::read<DWORD>(Base_RoleSelfAddr);
	if (m_playerObj)
	{
		return Monster::GetObjectForSkill(m_playerObj);
	}
	return 0;
}
