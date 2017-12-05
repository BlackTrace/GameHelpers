#include "stdafx.h"
#include "Player.h"
#include "baseaddr.h"
#include "utils.h"
#include "GameCall.h"
#include "HookToMainThread.h"

//ȫ�ֱ���
std::shared_ptr<CSkillService> m_skService = nullptr;
std::shared_ptr<CBufferService> m_buffService = nullptr;
float m_ZouKanTimeSec[2] = {0};


void Player::ZouA2Mons(DWORD target, bool tag)
{
	static bool bDefense = false;
	//timeSec[0]������ҡ  timeSec[1]����ǰҡ
	if (GameCall::GetGameTickTime() > m_ZouKanTimeSec[1])
	{
		if (target && tag)
		{
			//�߿������㹥��ǰ��ҡ
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
		//����ﵽ������ҡʱ��
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

	//���ʱ��С�ڹ�����ҡʱ�䣬��ʹ�ü���
	if (GameCall::GetGameTickTime() < m_ZouKanTimeSec[0])
	{
		return;
	}

	auto sk = GetSkillByIndex(skIndex);
	if (!sk)
	{
		return;
	}

	//���ݼ�������ȷ�����ܲ���
	switch (sk->GetSkillType())
	{
	case SK_SKILL_LINE :  //ָ���Լ��ܣ�����Q
	{
		//�жϹ����Ƿ����
		if (!mons)
		{
			return;
		}
		//�жϼ����Ƿ����
		if (sk->GetLevel() < 1 || !sk->BCoolDown() || Player::GetCurrMp() < sk->GetExpendMp())
		{
			return;
		}

		//����������� || ���ﲻ�ɼ�
		if (Monster::BDead(mons) || !Monster::BVisable(mons))
		{
			return;
		}
		//������������֮��ľ��� > ���ܷ�Χ
		auto dis = sk->GetDistance();
		if (utils::GetDistance(Player::GetCurrPostion(), Monster::GetCurrentPos(mons)) > dis * dis)
		{
			return;
		}
		SKILL_TO_MONS sk2mons = { 0 };
		memcpy(&sk2mons.pCurrPnt, Player::GetCurrPostion(), sizeof(EP_POINT_3D));
		//����Ԥ������
		if (Monster::BMoving(mons))
		{
			//��ȡ���ӳٺ������
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

			//����Ԥ������
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

		//Ȼ��Ѽ��������Բ��ֱ�ߵĽ�����Ϊ�ڶ�������
		float angles = atan2(Monster::GetCurrentPos(mons)->y - Player::GetCurrPostion()->y, Monster::GetCurrentPos(mons)->x - Player::GetCurrPostion()->x);
		sk2mons.pDestPnt.x = cos(angles) * sk->GetDistance() + Player::GetCurrPostion()->x;
		sk2mons.pDestPnt.y = sin(angles) * sk->GetDistance() + Player::GetCurrPostion()->y;
		sk2mons.pDestPnt.z = Monster::GetCurrentPos(mons)->z;

		sk2mons.targetId = 0;
		sk2mons.skillIndex = skIndex;
		sk2mons.skillBase = sk->GetNodeBase();
		//�����ɣ���ʼ����
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
	case SK_SKILL_LOCK :   //�������ܣ����簲��Q
	{
		//�жϹ����Ƿ����
		if (!mons)
		{
			return;
		}
		//�жϼ����Ƿ����
		if (sk->GetLevel() < 1
			|| !sk->BCoolDown()
			|| Player::GetCurrMp() < sk->GetExpendMp())
		{
			return;
		}

		//����������� || ���ﲻ�ɼ�
		if (Monster::BDead(mons) || !Monster::BVisable(mons))
		{
			return;
		}

		//������������֮��ľ��� > ���ܷ�Χ
		auto dis = sk->GetDistance();
		if (utils::GetDistance(Player::GetCurrPostion(), Monster::GetCurrentPos(mons)) > dis * dis)
		{
			return;
		}

		//���
		SKILL_TO_MONS sk2mons = { 0 };
		memcpy(&sk2mons.pCurrPnt, Player::GetCurrPostion(), sizeof(EP_POINT_3D));
		memcpy(&sk2mons.pDestPnt, Monster::GetCurrentPos(mons), sizeof(EP_POINT_3D));
		sk2mons.targetId = Monster::GetObjectForSkill(mons);
		sk2mons.skillIndex = skIndex;
		sk2mons.skillBase = sk->GetNodeBase();

		//����
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
	case SK_SKILL_SECTOR :  //���μ��ܣ����簲��W
	{
		//�жϹ����Ƿ����
		if (!mons)
		{
			return;
		}
		//�жϼ����Ƿ����
		if (sk->GetLevel() < 1
			|| !sk->BCoolDown()
			|| Player::GetCurrMp() < sk->GetExpendMp())
		{
			return;
		}

		//����������� || ���ﲻ�ɼ�
		if (Monster::BDead(mons) || !Monster::BVisable(mons))
		{
			return;
		}

		//������������֮��ľ��� > ���ܷ�Χ
		auto dis = sk->GetDistance();
		if (utils::GetDistance(Player::GetCurrPostion(), Monster::GetCurrentPos(mons)) > dis * dis)
		{
			return;
		}

		//���
		SKILL_TO_MONS sk2mons = { 0 };
		memcpy(&sk2mons.pCurrPnt, Player::GetCurrPostion(), sizeof(EP_POINT_3D));
		//����Ԥ������
		if (Monster::BMoving(mons)) 
		{
			//��ȡ���ӳٺ������
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

			//����Ԥ������
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
		//����
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
	case SK_SKILL_ROUND : //Բ�μ��ܣ�����R ,����R
	case SK_SKILL_ROUND2: //����˹R , ����Q
	{
		//�жϹ����Ƿ����
		if (!mons)
		{
			return;
		}
		//�жϼ����Ƿ����
		if (sk->GetLevel() < 1
			|| !sk->BCoolDown()
			|| Player::GetCurrMp() < sk->GetExpendMp())
		{
			return;
		}

		//����������� || ���ﲻ�ɼ�
		if (Monster::BDead(mons) || !Monster::BVisable(mons))
		{
			return;
		}

		//������������֮��ľ��� > ���ܷ�Χ
		auto dis = sk->GetDistance();
		if (utils::GetDistance(Player::GetCurrPostion(), Monster::GetCurrentPos(mons)) > dis * dis)
		{
			return;
		}

		//���
		SKILL_TO_MONS sk2mons = { 0 };
		memcpy(&sk2mons.pCurrPnt, Player::GetCurrPostion(), sizeof(EP_POINT_3D));
		//����Ԥ������
		if (Monster::BMoving(mons))
		{
			//��ȡ���ӳٺ������
			//��ȡ���ӳٺ������
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

			//����Ԥ������
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
		//����
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
	case SK_SKILL_TOSELF: //���Լ��ͷŵļ���
	{
		//�жϹ����Ƿ����
		if (!mons)
		{
			return;
		}
		//�жϼ����Ƿ����
		if (sk->GetLevel() < 1
			|| !sk->BCoolDown()
			|| Player::GetCurrMp() < sk->GetExpendMp())
		{
			return;
		}

		//����������� || ���ﲻ�ɼ�
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
	case SK_SKILL_VIKTOR: //ά���е�E
	{
		//�жϹ����Ƿ����
		if (!mons)
		{
			return;
		}
		//�жϼ����Ƿ����
		if (sk->GetLevel() < 1
			|| !sk->BCoolDown()
			|| Player::GetCurrMp() < sk->GetExpendMp())
		{
			return;
		}

		//����������� || ���ﲻ�ɼ�
		if (Monster::BDead(mons) || !Monster::BVisable(mons))
		{
			return;
		}

		//������������֮��ľ��� > ���ܷ�Χ
		auto dis = sk->GetDistance();
		if (utils::GetDistance(Player::GetCurrPostion(), Monster::GetCurrentPos(mons)) < dis * dis)
		{
			SKILL_TO_MONS sk2mons = { 0 };
			memcpy(&sk2mons.pCurrPnt, Player::GetCurrPostion(), sizeof(EP_POINT_3D));

			//����Ԥ������
			if (Monster::BMoving(mons))
			{
				//��ȡ���ӳٺ������
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

				//����Ԥ������
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
			//����
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

			//����Ԥ������
			if (Monster::BMoving(mons))
			{
				//��ȡ���ӳٺ������
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

				//����Ԥ������
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
			//����		
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
	//�жϹ����Ƿ����
	if (!mons)
	{
		return;
	}

	auto sk = GetSkillByIndex(skIndex);
	if (!sk)
	{
		return;
	}

	//�жϼ����Ƿ����
	if (sk->GetLevel() < 1 || !sk->BCoolDown() /*|| Player::GetCurrMp() < sk->GetExpendMp()*/)
	{
		return;
	}

	//����������� || ���ﲻ�ɼ�
	if (Monster::BDead(mons) || !Monster::BVisable(mons))
	{
		return;
	}

	//������������֮��ľ��� > ���ܷ�Χ
	auto dis = sk->GetDistance() + Monster::GetModelWidth(mons);
	if (utils::GetDistance(Player::GetCurrPostion(), Monster::GetCurrentPos(mons)) > dis * dis)
	{
		return;
	}

	SKILL_TO_MONS sk2mons = { 0 };
	memcpy(&sk2mons.pCurrPnt, Player::GetCurrPostion(), sizeof(EP_POINT_3D));
	//����Ԥ������
	if (Monster::BMoving(mons))
	{
		//��ȡ���ӳٺ������
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
	//�����ɣ���ʼ����
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

		//���㻤��
		auto defense = Monster::GetDefensePower(mons) - Player::GetArmour();
		if (defense < 0)
		{
			defense = 0;
		}
		//�˺����㹫ʽ (�����˺� + �������ӳɱ���) * ��100.0f / (100.0f + ������*��1.0 - ��͸�ʣ�)����
		auto hurt = (ske->GetSkillAggressivity() + 0.55f * Player::GetAttackPower()) * (100.0f / (100.0f + (defense * (1.0f - Player::GetArmourPenetration() / 100.0f)))); //���˱���
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
