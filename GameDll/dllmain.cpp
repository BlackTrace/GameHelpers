// dllmain.cpp : ���� DLL Ӧ�ó������ڵ㡣
#include "stdafx.h"
#include "dllmain.h"
#include "TargetSelecter.h"
#include "utils.h"
#include "SharedMemoryClient.h"
#include "Player.h"
#include "HookToMainThread.h"
//�߳̾��
HANDLE g_tarselecterHandle = { 0 };
HANDLE g_generalHandle = { 0 };

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		DisableThreadLibraryCalls(hModule);

		//�����ڴ��ʧ��
		if (!CSharedMemoryClient::GetInstance()->Init())
		{
			break;
		}

		//����Ŀ��ѡ�����߳�
		g_tarselecterHandle = ::CreateThread(NULL, NULL, LPTHREAD_START_ROUTINE(TargetSelecterProc), NULL, NULL, NULL);

		//�����߳�
		std::string hero = Player::GetHeroName();
		if (hero.compare("Cassiopeia") == 0)
		{
			//��Ů
			g_generalHandle = ::CreateThread(NULL, NULL, LPTHREAD_START_ROUTINE(CassiopeiaDelProc), NULL, NULL, NULL);
		}
		else if (hero.compare("Xerath") == 0)
		{
			//����˹
			g_generalHandle = ::CreateThread(NULL, NULL, LPTHREAD_START_ROUTINE(XerathDelProc), NULL, NULL, NULL);
		}
		else if (hero.compare("Kalista") == 0)
		{
			//����Ь
			g_generalHandle = ::CreateThread(NULL, NULL, LPTHREAD_START_ROUTINE(KalistaDelProc), NULL, NULL, NULL);
		}
		else if (hero.compare("KogMaw") == 0)
		{
			//����Ь
			g_generalHandle = ::CreateThread(NULL, NULL, LPTHREAD_START_ROUTINE(KogMawDelProc), NULL, NULL, NULL);
		}
		else if (hero.compare("Ezreal") == 0)
		{
			//ez
			g_generalHandle = ::CreateThread(NULL, NULL, LPTHREAD_START_ROUTINE(EzrealDelProc), NULL, NULL, NULL);
		}
		else if (hero.compare("Varus") == 0)
		{
			//ά³˹
			g_generalHandle = ::CreateThread(NULL, NULL, LPTHREAD_START_ROUTINE(VarusDelProc), NULL, NULL, NULL);
		}
		else 
		{
			g_generalHandle = ::CreateThread(NULL, NULL, LPTHREAD_START_ROUTINE(GeneralDelProc), NULL, NULL, NULL);
		}
		
		break;
	}
	case DLL_THREAD_ATTACH:
	{
		break;
	}
	case DLL_THREAD_DETACH:
	{
		break;
	}
	case DLL_PROCESS_DETACH:
	{
		if (g_tarselecterHandle)
		{
			TerminateThread(g_tarselecterHandle, 1);
			CloseHandle(g_tarselecterHandle);
		}

		if (g_generalHandle)
		{
			TerminateThread(g_generalHandle, 1);
			CloseHandle(g_generalHandle);
		}
		break;
	}
	}
	return TRUE;
}

DWORD WINAPI TargetSelecterProc(LPVOID lpParameter)
{

	//���ص����߳�
	if (!CHookToMainThread::GetInstance()->Hook())
	{
		return 1;
	}


	while (true)
	{
		if (!CTargetSelecter::GetInstance()->travse())
		{
			break;
		}
		Sleep(10);
	}

	return 0;
}

float GetSkillDelayByIndex(DWORD index)
{
	auto p = CSharedMemoryClient::GetInstance()->GetSharedMemoryPointer();
	switch (index)
	{
	case 0:
	{
		return p->f_Sk1Delay;
	}
	case 1:
	{
		return p->f_Sk2Delay;
	}
	case 2:
	{
		return p->f_Sk3Delay;
	}
	case 3:
	{
		return p->f_Sk4Delay;
	}
	default:
		break;
	}
	return 0.0f;
}

float GetSkillSpeedByIndex(DWORD index)
{
		auto p = CSharedMemoryClient::GetInstance()->GetSharedMemoryPointer();
		switch (index)
		{
		case 0:
		{
			return p->f_sk1Speed;
		}
		case 1:
		{
			return p->f_sk2Speed;
		}
		case 2:
		{
			return p->f_sk3Speed;
		}
		case 3:
		{
			return p->f_sk4Speed;
		}
		default:
			break;
		}
		return 0.0f;
}

DWORD WINAPI GeneralDelProc(LPVOID lpParameter)
{
	std::vector<DWORD> m_personList;
	std::vector<DWORD> m_monsterList;
	auto pSharedPointer = CSharedMemoryClient::GetInstance()->GetSharedMemoryPointer();
	while (true)
	{

		if (WAIT_OBJECT_0 == WaitForSingleObject(g_evt, 1))
		{
			g_mutex.lock();
			m_personList.swap(g_personList);
			m_monsterList.swap(g_monsterList);
			g_mutex.unlock();
		}

		//������������ȼ�
		if (GetAsyncKeyState(pSharedPointer->dwLianZhaoHotKey) & 0x8000)
		{

			if (pSharedPointer->b_Sk1ToUse)
			{
				auto sk = Player::GetSkillByIndex(pSharedPointer->i_Sk1ToUse);
				if (sk)
				{
					auto dis = sk->GetDistance();
					if (dis < 1)
					{
						dis = Player::GetAttackDistance();
					}

					auto obj = GetHealthLowerestPerson(m_personList, dis);

					static DWORD timesec = 0;
					if (GetTickCount() - timesec > 100)
					{
						Player::UseSkillToMons(obj, sk->GetIndex(), GetSkillDelayByIndex(sk->GetIndex()), GetSkillSpeedByIndex(sk->GetIndex()), pSharedPointer->b_AttackAfterSkill);
						timesec = GetTickCount();
					}
				}
			}


			if (pSharedPointer->b_Sk2ToUse)
			{
				auto sk = Player::GetSkillByIndex(pSharedPointer->i_Sk2ToUse);
				if (sk)
				{
					auto dis = sk->GetDistance();
					if (dis < 1)
					{
						dis = Player::GetAttackDistance();
					}

					auto obj = GetHealthLowerestPerson(m_personList, dis);

					static DWORD timesec = 0;
					if (GetTickCount() - timesec > 100)
					{
						Player::UseSkillToMons(obj, sk->GetIndex(), GetSkillDelayByIndex(sk->GetIndex()), GetSkillSpeedByIndex(sk->GetIndex()),pSharedPointer->b_AttackAfterSkill);
						timesec = GetTickCount();
					}
				}
			}


			if (pSharedPointer->b_Sk3ToUse)
			{
				auto sk = Player::GetSkillByIndex(pSharedPointer->i_Sk3ToUse);
				if (sk)
				{
					auto dis = sk->GetDistance();
					if (dis < 1)
					{
						dis = Player::GetAttackDistance();
					}

					auto obj = GetHealthLowerestPerson(m_personList, dis);

					static DWORD timesec = 0;
					if (GetTickCount() - timesec > 100)
					{
						Player::UseSkillToMons(obj, sk->GetIndex(), GetSkillDelayByIndex(sk->GetIndex()), GetSkillSpeedByIndex(sk->GetIndex()),pSharedPointer->b_AttackAfterSkill);
						timesec = GetTickCount();
					}
				}
			}


			if (pSharedPointer->b_Sk4ToUse)
			{
				auto sk = Player::GetSkillByIndex(pSharedPointer->i_Sk4ToUse);
				if (sk)
				{
					auto dis = sk->GetDistance();
					if (dis < 1)
					{
						dis = Player::GetAttackDistance();
					}

					auto obj = GetHealthLowerestPerson(m_personList, dis);

					static DWORD timesec = 0;
					if (GetTickCount() - timesec > 100)
					{
						Player::UseSkillToMons(obj, sk->GetIndex(), GetSkillDelayByIndex(sk->GetIndex()), GetSkillSpeedByIndex(sk->GetIndex()),pSharedPointer->b_AttackAfterSkill);
						timesec = GetTickCount();
					}
				}
			}

			if (pSharedPointer->b_OpenMove)
			{
				static DWORD timesec = 0;
				if (GetTickCount() - timesec > 15)
				{
					auto obj = GetHealthLowerestPerson(m_personList, Player::GetAttackDistance());
					Player::ZouA2Mons(obj, pSharedPointer->b_OpenAttack);
					timesec = GetTickCount();
				}

			}
		}

		//���
		if (GetAsyncKeyState(pSharedPointer->dwQingArmyHotKey) & 0x8000)
		{
			if (pSharedPointer->b_OpenQingArmy)
			{
				static DWORD timesec = 0;
				if (GetTickCount() - timesec > 15)
				{
					auto obj = GetHealthLowerestPerson(m_monsterList, Player::GetAttackDistance());
					Player::ZouA2Mons(obj, true);
					timesec = GetTickCount();
				}

			}
		}

		//����
		if (GetAsyncKeyState(pSharedPointer->dwFarmingHotKey) & 0x8000)
		{
			if (pSharedPointer->b_OpenFarming)
			{
				static DWORD timesec = 0;
				if (GetTickCount() - timesec > 15)
				{
					auto obj = GetHealthLowerestPerson(m_monsterList, Player::GetAttackDistance());
					if (Monster::GetCurrentHealth(obj) < Player::GetAttackPower())
					{
						Player::ZouA2Mons(obj, true);
						timesec = GetTickCount();
					}
					else {
						Player::ZouA2Mons(obj, false);
					}
	
				}

			}
		}

	}

	return 0;
}

DWORD WINAPI CassiopeiaDelProc(LPVOID lpParameter)
{
	std::vector<DWORD> m_personList;
	std::vector<DWORD> m_monsterList;
	auto pSharedPointer = CSharedMemoryClient::GetInstance()->GetSharedMemoryPointer();
	InitSharedMemory();
	pSharedPointer->b_LockW = true;
	pSharedPointer->b_LockR = true;
	pSharedPointer->b_Sk1ToUse = true;
	pSharedPointer->i_Sk1ToUse = 0;
	pSharedPointer->b_Sk2ToUse = true;
	pSharedPointer->i_Sk2ToUse = 2;
	pSharedPointer->b_openCassiopeiaFarming = true;
	//��ȡ��Ů��������
	while (true)
	{

		if (WAIT_OBJECT_0 == WaitForSingleObject(g_evt, 1))
		{
			g_mutex.lock();
			m_personList.swap(g_personList);
			m_monsterList.swap(g_monsterList);
			g_mutex.unlock();
		}

		if (GetAsyncKeyState(pSharedPointer->dwLianZhaoHotKey) & 0x8000)
		{

			{
				auto sk = Player::GetSkillByIndex(0);
				if (sk)
				{
					auto dis = sk->GetDistance();
					if (dis < 1)
					{
						dis = Player::GetAttackDistance();
					}

					auto obj = GetHealthLowerestPerson(m_personList, dis);
					static DWORD timesec = 0;
					if (GetTickCount() - timesec > 150)
					{
						Player::UseSkillToMons(obj, sk->GetIndex(), GetSkillDelayByIndex(0), GetSkillSpeedByIndex(0), pSharedPointer->b_AttackAfterSkill);
						timesec = GetTickCount();
					}
				}
			}


			{
				auto sk = Player::GetSkillByIndex(2);
				if (sk)
				{
					auto dis = sk->GetDistance();
					if (dis < 1)
					{
						dis = Player::GetAttackDistance();
					}

					auto obj = GetCassiopeiaPoisonPerson(m_personList, dis);
					static DWORD timesec = 0;
					if (GetTickCount() - timesec > 150)
					{
						Player::UseSkillToMons(obj, sk->GetIndex(), GetSkillDelayByIndex(2), GetSkillSpeedByIndex(2), pSharedPointer->b_AttackAfterSkill);
						timesec = GetTickCount();
					}
				}
			}

			if (pSharedPointer->b_OpenMove)
			{
				static DWORD timesec = 0;
				if (GetTickCount() - timesec > 15)
				{
					auto obj = GetHealthLowerestPerson(m_personList, Player::GetAttackDistance());
					Player::ZouA2Mons(obj, pSharedPointer->b_OpenAttack);
					timesec = GetTickCount();
				}

			}
		}
		else 
		{
			//��Ů����
			if (pSharedPointer->b_openCassiopeiaFarming)
			{
				auto ske = Player::GetSkillByIndex(2);
				if (ske)
				{
					//��ȡ���ܷ�Χ�ڵĹ���
					auto mons = GetHealthLowerestPerson(m_monsterList, ske->GetDistance());
					if (mons)
					{
						//�ȽϹ���Ѫ�� < �����˺� + ����ǿ�� * 0.1f
						if (Monster::GetCurrentHealth(mons) < ske->GetBaseAggressivity() + Player::GetSpellPower() * 0.1f)
						{
							static DWORD timesec = 0;
							if (GetTickCount() - timesec > 100)
							{
								Player::UseSkillToMons(mons, 2, 0,0, false);
								timesec = GetTickCount();
							}
						}
					}
				}
			}

		}

	}

	return 0;
}

DWORD WINAPI XerathDelProc(LPVOID lpParameter)
{
	std::vector<DWORD> m_personList;
	std::vector<DWORD> m_monsterList;
	auto pSharedPointer = CSharedMemoryClient::GetInstance()->GetSharedMemoryPointer();

	InitSharedMemory();
	pSharedPointer->f_Sk1Delay = 0.35f;
	pSharedPointer->f_Sk2Delay = 0.35f;
	pSharedPointer->f_Sk3Delay = 0.35f;
	pSharedPointer->b_LockW = true;
	pSharedPointer->b_LockE = true;
	auto skq = Player::GetSkillByIndex(0);
	if (!skq)
	{
		return 1;
	}
	//���ü��ܷ�Χ
	skq->SetDistance(1500.0f);
	while (true)
	{

		if (WAIT_OBJECT_0 == WaitForSingleObject(g_evt, 1))
		{
			g_mutex.lock();
			m_personList.swap(g_personList);
			m_monsterList.swap(g_monsterList);
			g_mutex.unlock();
		}

		if (GetAsyncKeyState(pSharedPointer->dwLianZhaoHotKey) & 0x8000)
		{
			auto mons = GetHealthLowerestPerson(m_personList, skq->GetDistance());
			if (mons)
			{
				//û�л�û�п�ʼ������������
				if (Player::GetXerathPowerTime() < 0.0001)
				{

					static DWORD timeSec = 0;
					if (GetTickCount() - timeSec > 100)
					{
						Player::UseSkillToMons(mons, 0, pSharedPointer->f_Sk1Delay, 0, false);
						timeSec = GetTickCount();
					}
				}
				else
				{
					//�жϼ��ܵľ���֪��ﵽ���ͷž��룬����Ǿ��ͷż���
					auto currDis = Player::GetXerathSkillCurrDistance();
					if (utils::GetDistance(Player::GetCurrPostion(), Monster::GetCurrentPos(mons)) < currDis * currDis)
					{
						static DWORD timeSec = 0;
						if (GetTickCount() - timeSec > 100)
						{
							Player::UseSkillToMons2(mons, 0, pSharedPointer->f_Sk1Delay , pSharedPointer->f_sk1Speed);
							timeSec = GetTickCount();
						}
					}
				}
			}

			if (pSharedPointer->b_OpenMove)
			{
				static DWORD timesec = 0;
				if (GetTickCount() - timesec > 15)
				{
					Player::ZouA2Mons(0,false);
					timesec = GetTickCount();
				}

			}
		}

		//����˹����
		if (GetAsyncKeyState('T') & 0x8000)
		{
			auto skr = Player::GetSkillByIndex(3);
			if (skr)
			{
				auto mons = GetHealthLowerestPerson(m_personList, skr->GetDistance());
				if (mons)
				{
					static DWORD timeSec = 0;
					if (GetTickCount() - timeSec > 100)
					{
						Player::UseSkillToMons(mons, 3, pSharedPointer->f_Sk4Delay, GetSkillSpeedByIndex(skr->GetIndex()),false);
					}
				}
			}
		}
	}

	return 0;
}

DWORD WINAPI KalistaDelProc(LPVOID lpParameter)
{
	std::vector<DWORD> m_personList;
	std::vector<DWORD> m_monsterList;
	auto pSharedPointer = CSharedMemoryClient::GetInstance()->GetSharedMemoryPointer();
	//���ù����ڴ�
	InitSharedMemory();
	//��ȡ�����ļ�
	pSharedPointer->b_LockQ = true;
	pSharedPointer->b_OpenAttack = true;
	pSharedPointer->b_OpenFarming = true;
	pSharedPointer->b_OpenQingArmy = true;
	pSharedPointer->b_openKalistaFarmingToPerson = true;
	pSharedPointer->b_openKalistaFarmingToMonster = true;
	auto skq = Player::GetSkillByIndex(0);
	if (!skq)
	{
		return 1;
	}

	skq->SetWidth(40.0f);

	while (true)
	{

		if (WAIT_OBJECT_0 == WaitForSingleObject(g_evt, 1))
		{
			g_mutex.lock();
			m_personList.swap(g_personList);
			m_monsterList.swap(g_monsterList);
			g_mutex.unlock();
		}

		//������������ȼ�
		if (GetAsyncKeyState(pSharedPointer->dwLianZhaoHotKey) & 0x8000)
		{

			{

				if (skq)
				{
					auto dis = skq->GetDistance();
					auto obj = GetHealthLowerestPerson(m_personList, dis);
					if (obj)
					{
						static DWORD timesec = 0;
						if (GetTickCount() - timesec > 100)
						{
							//��ȡ��Ԥ�е�����
							EP_POINT_3D pnt = { 0 };
							if (Monster::BMoving(obj))
							{
								auto delay = pSharedPointer->f_Sk1Delay + pSharedPointer->f_sk1Speed / sqrt(utils::GetDistance(Player::GetCurrPostion(), Monster::GetCurrentPos(obj)));
								pnt.x = Monster::GetCurrentPos(obj)->x + Monster::GetOrientation(obj)->x * Monster::GetMoveSpeed(obj) * delay;
								pnt.z = Monster::GetCurrentPos(obj)->z;
								pnt.y = Monster::GetCurrentPos(obj)->y + Monster::GetOrientation(obj)->y * Monster::GetMoveSpeed(obj) * delay;
							}
							else {
								memcpy(&pnt, Monster::GetCurrentPos(obj), sizeof(EP_POINT_3D));
							}

							if (!IsCoveredByMonster(m_monsterList, &pnt, skq->GetDistance() , skq->GetWidth()))
							{
								Player::UseSkillToMons(obj, skq->GetIndex(), GetSkillDelayByIndex(0), GetSkillSpeedByIndex(0), pSharedPointer->b_AttackAfterSkill);
								timesec = GetTickCount();
							}
						}
					}
				}
			}

			if (pSharedPointer->b_OpenMove)
			{
				static DWORD timesec = 0;
				if (GetTickCount() - timesec > 15)
				{
					auto obj = GetHealthLowerestPerson(m_personList, Player::GetAttackDistance());
					Player::ZouA2Mons(obj, pSharedPointer->b_OpenAttack);
					timesec = GetTickCount();
				}
			}
		}

		//���
		if (GetAsyncKeyState(pSharedPointer->dwQingArmyHotKey) & 0x8000)
		{
			if (pSharedPointer->b_OpenQingArmy)
			{
				static DWORD timesec = 0;
				if (GetTickCount() - timesec > 15)
				{
					auto obj = GetHealthLowerestPerson(m_monsterList, Player::GetAttackDistance());
					Player::ZouA2Mons(obj, true);
					timesec = GetTickCount();
				}

			}
		}

		//����
		if (GetAsyncKeyState(pSharedPointer->dwFarmingHotKey) & 0x8000)
		{
			if (pSharedPointer->b_OpenFarming)
			{
				static DWORD timesec = 0;
				if (GetTickCount() - timesec > 15)
				{
					auto obj = GetHealthLowerestPerson(m_monsterList, Player::GetAttackDistance());
					if (Monster::GetCurrentHealth(obj) < Player::GetAttackPower())
					{
						Player::ZouA2Mons(obj, true);
						timesec = GetTickCount();
					}
					else {
						Player::ZouA2Mons(obj, false);
					}

				}

			}
		}

		//����Ь�Զ�E��
		if (pSharedPointer->b_openKalistaFarmingToPerson)
		{
			auto ske = Player::GetSkillByIndex(2);
			if (ske)
			{
				auto Mons = GetHealthLowerestPerson(m_personList, ske->GetDistance());
				if (Mons)
				{
					Player::KalistaAutoE2Object(Mons);
				}

				auto Mons2Att = GetHealthLowerestPerson(m_personList, Player::GetAttackDistance());
				if (Mons2Att && Mons2Att != Mons)
				{
					Player::KalistaAutoE2Object(Mons2Att);
				}
			}
		}

		if (pSharedPointer->b_openKalistaFarmingToMonster)
		{
			auto ske = Player::GetSkillByIndex(2);
			if (ske)
			{
				auto Mons = GetHealthLowerestPerson(m_monsterList, ske->GetDistance());
				if (Mons)
				{
					Player::KalistaAutoE2Object(Mons);
				}
			}
		}
	}

	return 0;
}

DWORD WINAPI KogMawDelProc(LPVOID lpParameter)
{
	std::vector<DWORD> m_personList;
	std::vector<DWORD> m_monsterList;
	auto pSharedPointer = CSharedMemoryClient::GetInstance()->GetSharedMemoryPointer();
	//���ù����ڴ�
	InitSharedMemory();

	//��ȡ�����ļ�
	pSharedPointer->b_LockQ = true;
	pSharedPointer->b_LockE = true;
	pSharedPointer->b_LockR = true;
	pSharedPointer->b_OpenAttack = true;
	pSharedPointer->b_OpenFarming = true;
	pSharedPointer->b_OpenQingArmy = true;
	pSharedPointer->f_Sk4Delay = 0.8f;

	auto skq = Player::GetSkillByIndex(0);
	if (!skq)
	{
		return 1;
	}
	skq->SetWidth(70.0f);

	while (true)
	{


		if (WAIT_OBJECT_0 == WaitForSingleObject(g_evt, 1))
		{
			g_mutex.lock();
			m_personList.swap(g_personList);
			m_monsterList.swap(g_monsterList);
			g_mutex.unlock();
		}

		//������������ȼ�
		if (GetAsyncKeyState(pSharedPointer->dwLianZhaoHotKey) & 0x8000)
		{
			//�Զ��ڵ��жϵ�Q
			{
				if (skq)
				{
					auto dis = skq->GetDistance();
					auto obj = GetHealthLowerestPerson(m_personList, dis);
					if (obj)
					{
						static DWORD timesec = 0;
						if (GetTickCount() - timesec > 100)
						{
							//��ȡ��Ԥ�е�����
							EP_POINT_3D pnt = { 0 };
							if (Monster::BMoving(obj))
							{
								auto delay = pSharedPointer->f_Sk1Delay + pSharedPointer->f_sk1Speed / sqrt(utils::GetDistance(Player::GetCurrPostion(), Monster::GetCurrentPos(obj)));
								pnt.x = Monster::GetCurrentPos(obj)->x + Monster::GetOrientation(obj)->x * Monster::GetMoveSpeed(obj) * delay;
								pnt.z = Monster::GetCurrentPos(obj)->z;
								pnt.y = Monster::GetCurrentPos(obj)->y + Monster::GetOrientation(obj)->y * Monster::GetMoveSpeed(obj) * delay;
							}
							else {
								memcpy(&pnt, Monster::GetCurrentPos(obj), sizeof(EP_POINT_3D));
							}

							if (!IsCoveredByMonster(m_monsterList, &pnt, skq->GetDistance(), skq->GetWidth()))
							{
								Player::UseSkillToMons(obj, skq->GetIndex(), GetSkillDelayByIndex(0), GetSkillSpeedByIndex(0), pSharedPointer->b_AttackAfterSkill);
								timesec = GetTickCount();
							}
						}
					}
				}
			}//�Զ��ڵ��жϵ�Q

			 //�Զ��ͷ�W
			{
				auto skw = Player::GetSkillByIndex(1);
				if (skw)
				{
					auto dis = skw->GetDistance() + skw->GetSkillAggressivity();
					auto obj = GetHealthLowerestPerson(m_personList, dis);
					if (obj)
					{
						static DWORD timesec = 0;
						if (GetTickCount() - timesec > 100) {
							Player::UseSkillToMons(obj, skw->GetIndex(), 0, 0, 0);
							timesec = GetTickCount();
						}
					}
				}
			}//�Զ��ͷ�W


			{
				//������Ѫ��С�ڰٷ�֮30��ʱ���Զ�ʹ��R
				auto skr = Player::GetSkillByIndex(3);
				if (skr)
				{
					auto mons = GetHealthLowerestPerson(m_personList, skr->GetDistance());
					if (mons)
					{
						auto percent = Monster::GetCurrentHealth(mons) / Monster::GetMaxHealth(mons);
						if (percent < 0.30f)
						{
							static DWORD timesec = 0;
							if (GetTickCount() - timesec > 100)
							{
								Player::UseSkillToMons(mons, skr->GetIndex(), pSharedPointer->f_Sk4Delay , pSharedPointer->f_sk4Speed, false);
								timesec = GetTickCount();
							}
						}
	

					}
				}
			}

			if (pSharedPointer->b_OpenMove)
			{//�߿�
				static DWORD timesec = 0;
				if (GetTickCount() - timesec > 15)
				{
					auto obj = GetHealthLowerestPerson(m_personList, Player::GetAttackDistance());
					Player::ZouA2Mons(obj, pSharedPointer->b_OpenAttack);
					timesec = GetTickCount();
				}
			}

			//�Զ�R
		}

		//���
		if (GetAsyncKeyState(pSharedPointer->dwQingArmyHotKey) & 0x8000)
		{
			if (pSharedPointer->b_OpenQingArmy)
			{
				static DWORD timesec = 0;
				if (GetTickCount() - timesec > 15)
				{
					auto obj = GetHealthLowerestPerson(m_monsterList, Player::GetAttackDistance());
					Player::ZouA2Mons(obj, true);
					timesec = GetTickCount();
				}

			}
		}

		//����
		if (GetAsyncKeyState(pSharedPointer->dwFarmingHotKey) & 0x8000)
		{
			if (pSharedPointer->b_OpenFarming)
			{
				static DWORD timesec = 0;
				if (GetTickCount() - timesec > 15)
				{
					auto obj = GetHealthLowerestPerson(m_monsterList, Player::GetAttackDistance());
					if (Monster::GetCurrentHealth(obj) < Player::GetAttackPower())
					{
						Player::ZouA2Mons(obj, true);
						timesec = GetTickCount();
					}
					else {
						Player::ZouA2Mons(obj, false);
					}

				}

			}
		}
	}
	return 0;
}

DWORD WINAPI EzrealDelProc(LPVOID lpParameter)
{
	std::vector<DWORD> m_personList;
	std::vector<DWORD> m_monsterList;
	auto pSharedPointer = CSharedMemoryClient::GetInstance()->GetSharedMemoryPointer();
	//���ù����ڴ�
	InitSharedMemory();
	//��ȡ�����ļ�
	pSharedPointer->f_Sk1Delay = 0.3f;
	pSharedPointer->f_Sk2Delay = 0.3f;
	pSharedPointer->f_Sk4Delay = 0.35f;

	pSharedPointer->b_OpenAttack = true;
	pSharedPointer->b_OpenFarming = true;
	pSharedPointer->b_OpenQingArmy = true;

	auto skq = Player::GetSkillByIndex(0);
	if (!skq)
	{
		return 1;
	}
	skq->SetWidth(60.0f);
	
	while (true)
	{

		if (WAIT_OBJECT_0 == WaitForSingleObject(g_evt, 1))
		{
			g_mutex.lock();
			m_personList.swap(g_personList);
			m_monsterList.swap(g_monsterList);
			g_mutex.unlock();
		}


		//����
		if (GetAsyncKeyState(pSharedPointer->dwLianZhaoHotKey) & 0x8000)
		{
			//�Զ��ڵ��жϵ�Q
			{
				if (skq)
				{
					auto dis = skq->GetDistance();
					auto obj = GetHealthLowerestPerson(m_personList, dis);
					if (obj)
					{
						static DWORD timesec = 0;
						if (GetTickCount() - timesec > 100)
						{
							//��ȡ��Ԥ�е�����
							EP_POINT_3D pnt = { 0 };
							if (Monster::BMoving(obj))
							{
								auto delay = pSharedPointer->f_Sk1Delay + pSharedPointer->f_sk1Speed / sqrt(utils::GetDistance(Player::GetCurrPostion(), Monster::GetCurrentPos(obj)));
								pnt.x = Monster::GetCurrentPos(obj)->x + Monster::GetOrientation(obj)->x * Monster::GetMoveSpeed(obj) * delay;
								pnt.z = Monster::GetCurrentPos(obj)->z;
								pnt.y = Monster::GetCurrentPos(obj)->y + Monster::GetOrientation(obj)->y * Monster::GetMoveSpeed(obj) * delay;
							}
							else {
								memcpy(&pnt, Monster::GetCurrentPos(obj), sizeof(EP_POINT_3D));
							}

							if (!IsCoveredByMonster(m_monsterList, &pnt, skq->GetDistance(), skq->GetWidth()))
							{
								Player::UseSkillToMons(obj, skq->GetIndex(), GetSkillDelayByIndex(0), GetSkillSpeedByIndex(0), pSharedPointer->b_AttackAfterSkill);
								timesec = GetTickCount();
							}
						}
					}
				}
			}//�Զ��ڵ��жϵ�Q


			 //�Զ��ڵ��жϵ�W
			{
				auto skw = Player::GetSkillByIndex(1);
				if (skw)
				{
					auto dis = skw->GetDistance();
					auto obj = GetHealthLowerestPerson(m_personList, dis);
					if (obj)
					{
						static DWORD timesec = 0;
						if (GetTickCount() - timesec > 100)
						{
							Player::UseSkillToMons(obj, skw->GetIndex(), GetSkillDelayByIndex(1), GetSkillSpeedByIndex(1), pSharedPointer->b_AttackAfterSkill);
							timesec = GetTickCount();
						}
					}
				}
			}//�Զ��ڵ��жϵ�w


			if (pSharedPointer->b_OpenMove)
			{//�߿�
				static DWORD timesec = 0;
				if (GetTickCount() - timesec > 15)
				{
					auto obj = GetHealthLowerestPerson(m_personList, Player::GetAttackDistance());
					Player::ZouA2Mons(obj, pSharedPointer->b_OpenAttack);
					timesec = GetTickCount();
				}
			}
		}


		//���
		if (GetAsyncKeyState(pSharedPointer->dwQingArmyHotKey) & 0x8000)
		{
			if (pSharedPointer->b_OpenQingArmy)
			{
				static DWORD timesec = 0;
				if (GetTickCount() - timesec > 15)
				{
					auto obj = GetHealthLowerestPerson(m_monsterList, Player::GetAttackDistance());
					Player::ZouA2Mons(obj, true);
					timesec = GetTickCount();
				}

			}
		}

		//����
		if (GetAsyncKeyState(pSharedPointer->dwFarmingHotKey) & 0x8000)
		{
			if (pSharedPointer->b_OpenFarming)
			{
				static DWORD timesec = 0;
				if (GetTickCount() - timesec > 15)
				{
					auto obj = GetHealthLowerestPerson(m_monsterList, Player::GetAttackDistance());
					if (Monster::GetCurrentHealth(obj) < Player::GetAttackPower())
					{
						Player::ZouA2Mons(obj, true);
						timesec = GetTickCount();
					}
					else {
						Player::ZouA2Mons(obj, false);
					}

				}

			}
		}
	}

	return 0;
}

DWORD WINAPI VarusDelProc(LPVOID lpParameter)
{
	std::vector<DWORD> m_personList;
	std::vector<DWORD> m_monsterList;
	auto pSharedPointer = CSharedMemoryClient::GetInstance()->GetSharedMemoryPointer();

	InitSharedMemory();
	pSharedPointer->b_LockE = true;
	pSharedPointer->b_LockR = true;
	pSharedPointer->b_OpenFarming = true;
	pSharedPointer->b_OpenQingArmy = true;
	pSharedPointer->b_OpenAttack = true;
	auto skq = Player::GetSkillByIndex(0);
	if (!skq)
	{
		return 1;
	}
	//���ü��ܷ�Χ
	skq->SetDistance(1800.0f);
	while (true)
	{

		if (WAIT_OBJECT_0 == WaitForSingleObject(g_evt, 1))
		{
			g_mutex.lock();
			m_personList.swap(g_personList);
			m_monsterList.swap(g_monsterList);
			g_mutex.unlock();
		}

		if (GetAsyncKeyState(pSharedPointer->dwLianZhaoHotKey) & 0x8000)
		{
			if (pSharedPointer->b_OpenMove)
			{//�߿�
				static DWORD timesec = 0;
				if (GetTickCount() - timesec > 15)
				{
					auto obj = GetHealthLowerestPerson(m_personList, Player::GetAttackDistance());
					Player::ZouA2Mons(obj, pSharedPointer->b_OpenAttack);
					timesec = GetTickCount();
				}
			}
		}



		//���
		if (GetAsyncKeyState(pSharedPointer->dwQingArmyHotKey) & 0x8000)
		{
			if (pSharedPointer->b_OpenQingArmy)
			{
				static DWORD timesec = 0;
				if (GetTickCount() - timesec > 15)
				{
					auto obj = GetHealthLowerestPerson(m_monsterList, Player::GetAttackDistance());
					Player::ZouA2Mons(obj, true);
					timesec = GetTickCount();
				}

			}
		}

		//����
		if (GetAsyncKeyState(pSharedPointer->dwFarmingHotKey) & 0x8000)
		{
			if (pSharedPointer->b_OpenFarming)
			{
				static DWORD timesec = 0;
				if (GetTickCount() - timesec > 15)
				{
					auto obj = GetHealthLowerestPerson(m_monsterList, Player::GetAttackDistance());
					if (Monster::GetCurrentHealth(obj) < Player::GetAttackPower())
					{
						Player::ZouA2Mons(obj, true);
						timesec = GetTickCount();
					}
					else {
						Player::ZouA2Mons(obj, false);
					}

				}

			}
		}
	}

	return 0;
}


void InitSharedMemory()
{
	auto pSharedPointer = CSharedMemoryClient::GetInstance()->GetSharedMemoryPointer();
	if (pSharedPointer)
	{
		pSharedPointer->f_Sk1Delay = 0.35f;
		pSharedPointer->f_Sk2Delay = 0.35f;
		pSharedPointer->f_Sk3Delay = 0.35f;
		pSharedPointer->f_Sk4Delay = 0.35f;
		pSharedPointer->f_sk1Speed = 0;
		pSharedPointer->f_sk2Speed = 0;
		pSharedPointer->f_sk3Speed = 0;
		pSharedPointer->f_sk4Speed = 0;
		pSharedPointer->i_targetselecter = 0;
		pSharedPointer->b_OpenMove = true;
		pSharedPointer->b_OpenAttack = false;
		pSharedPointer->dwLianZhaoHotKey = VK_SPACE;
		pSharedPointer->b_LockQ = false;
		pSharedPointer->b_LockW = false;
		pSharedPointer->b_LockE = false;
		pSharedPointer->b_LockR = false;
		pSharedPointer->b_AttackAfterSkill = false;
		pSharedPointer->i_Sk1ToUse = 0;
		pSharedPointer->b_Sk1ToUse = false;
		pSharedPointer->i_Sk2ToUse = 1;
		pSharedPointer->b_Sk2ToUse = false;
		pSharedPointer->i_Sk3ToUse = 2;
		pSharedPointer->b_Sk3ToUse = false;
		pSharedPointer->i_Sk4ToUse = 3;
		pSharedPointer->b_Sk4ToUse = false;
		pSharedPointer->b_OpenFarming = false;
		pSharedPointer->dwFarmingHotKey = 'C';
		pSharedPointer->b_OpenQingArmy = false;
		pSharedPointer->dwQingArmyHotKey = 'V';
		pSharedPointer->b_openCassiopeiaFarming = false;
		pSharedPointer->b_openKalistaFarmingToPerson = false;
		pSharedPointer->b_openKalistaFarmingToMonster = false;
	}
}