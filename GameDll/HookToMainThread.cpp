#include "stdafx.h"
#include "HookToMainThread.h"
#include "baseaddr.h"
#include "utils.h"
#include "GameCall.h"
#include "Player.h"
#include "TargetSelecter.h"
#include "SharedMemoryClient.h"
std::mutex CHookToMainThread::m_mutex;
CHookToMainThread* CHookToMainThread::m_pInstance = nullptr;
HHOOK CHookToMainThread::m_hHook = { 0 };
HHOOK CHookToMainThread::m_msgHook = { 0 };
DWORD CHookToMainThread::m_msgCode = RegisterWindowMessage("MonkeyToy");
LONG  CHookToMainThread::m_msgHookEX = { 0 };

bool UseSkillForKeyDown(UINT uMsg, WPARAM wParam);


CHookToMainThread::CHookToMainThread()
{
}

CHookToMainThread * CHookToMainThread::GetInstance()
{
	if (nullptr == m_pInstance)
	{
		m_mutex.lock();
		if (nullptr == m_pInstance)
		{
			m_pInstance = new CHookToMainThread();
		}
		m_mutex.unlock();
	}
	return m_pInstance;
}

CHookToMainThread::~CHookToMainThread()
{
	UnHook();
}

bool CHookToMainThread::Hook()
{
	auto hwnd = GetGameHwnd();
	if (!hwnd) {
		return false;
	}
	utils::log("HXL:- hwnd = %x", hwnd);

	auto threadId = GetWindowThreadProcessId(hwnd, NULL);
	if (!threadId) {
		return false;
	}
	utils::log("HXL:- threadId = %x", threadId);
	//挂载到主线程
	m_hHook = SetWindowsHookEx(WH_CALLWNDPROC, CallWndProc, NULL, threadId);
	if (!m_hHook) {
		return false;
	}

	utils::log("HXL:- m_hHook = %x", m_hHook);
	//m_msgHook = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, NULL, threadId);
	//if (!m_msgHook) {
	//	return false;
	//}

	m_msgHookEX =  SetWindowLongW(hwnd, GWL_WNDPROC, (LONG)WindowProc);
	if (0 == m_msgHookEX) {
		return false;
	}

	return true;
}

bool CHookToMainThread::UnHook()
{
	if (m_hHook) {
		UnhookWindowsHookEx(m_hHook);
	}

	if (m_msgHook) {
		UnhookWindowsHookEx(m_msgHook);
	}
	return true;
}

void CHookToMainThread::SendMessageToMainThread(MESSAGE msg, LPARAM lparam)
{
	::SendMessage(GetGameHwnd(), m_msgCode, msg, lparam);
}

HWND CHookToMainThread::GetGameHwnd() const
{
	return (HWND)(utils::read<DWORD>(Base_GameWndHwndAddr));
}

LRESULT CALLBACK CallWndProc(
	_In_ int    nCode,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
)
{
	CWPSTRUCT *pArg = (PCWPSTRUCT)(lParam);
	if (nCode == HC_ACTION)
	{
		if (pArg->message == CHookToMainThread::m_msgCode)
		{
			switch (pArg->wParam)
			{
			case MESSAGE::MSG_FINDWAY:
			{
				auto mousePnt = GameCall::GetMousePoint();
				auto playerpnt = Player::GetCurrPostion();
				if (mousePnt && playerpnt) {
					if (utils::GetDistance(mousePnt, playerpnt) < 2000.0f * 2000.0f) {
						GameCall::FindWay(mousePnt);
					}
				}
				return 1;
			}
			case MESSAGE::MSG_ATTACKCALL:
			{
				DWORD monObj = *(DWORD*)(pArg->lParam);
				GameCall::Attack(monObj);
				return 1;
			}
			case MESSAGE::MSG_USESKILL:
			{
				SKILL_TO_MONS sk2mons = *(SKILL_TO_MONS*)(pArg->lParam);
				GameCall::UseSkill(sk2mons.targetId, &sk2mons.pAskPnt, &sk2mons.pDestPnt, sk2mons.skillIndex, sk2mons.skillBase);
				//GameCall::Defense();
				return 1;
			}
			case MSG_USESKILL2:
			{
				SKILL_TO_MONS sk2mons = *(SKILL_TO_MONS*)(pArg->lParam);
				GameCall::useSkill2(&sk2mons.pAskPnt, sk2mons.skillIndex, sk2mons.skillBase);
				return 1;
			}
			case  MESSAGE::MSG_DEFENSE:
			{
				GameCall::Defense();
				return 1;
			}
			default:
				break;
			}
		}
	}
	return CallNextHookEx(CHookToMainThread::m_hHook, nCode, wParam, lParam);
}

LRESULT CALLBACK WindowProc(
	_In_ HWND   hwnd,
	_In_ UINT   uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
)
{
	if (uMsg >= 0x50 && uMsg < 0x100)
	{
		return true;
	}


	if (UseSkillForKeyDown(uMsg, wParam))
	{
		return true;
	}


	return CallWindowProc((WNDPROC)CHookToMainThread::m_msgHookEX, hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK GetMsgProc(_In_ int code, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	return CallNextHookEx(CHookToMainThread::m_msgHook, code, wParam, lParam);
}
std::vector<DWORD> m_personList;
bool UseSkillForKeyDown(UINT uMsg, WPARAM wParam)
{
	if (m_personList.empty())
	{
		if (WAIT_OBJECT_0 == WaitForSingleObject(g_evt, 1))
		{
			g_mutex.lock();
			m_personList.swap(g_personList2);
			g_mutex.unlock();
		}
	}

	if (uMsg == WM_KEYDOWN || uMsg == WM_KEYFIRST || uMsg == WM_SYSCHAR || uMsg == WM_CHAR || uMsg == WM_SYSKEYDOWN)
	//if(uMsg == WM_KEYDOWN || uMsg == WM_KEYUP || uMsg == WM_CHAR)
	{
		auto pSharedPnter = CSharedMemoryClient::GetInstance()->GetSharedMemoryPointer();
		switch (wParam)
		{
		case 'Q':
		case 'q':
		{
			if (pSharedPnter->b_LockQ)
			{
				auto sk = Player::GetSkillByIndex(0);
				if (sk)
				{
					auto mons = GetHealthLowerestPerson(m_personList, sk->GetDistance());
					if (mons)
					{
						static DWORD timeSec = 0;
						if (GetTickCount() - timeSec > 300) 
						{
							Player::UseSkillToMons(mons, 0, pSharedPnter->f_Sk1Delay, pSharedPnter->f_sk1Speed,false);
							timeSec = GetTickCount();
						}

					}			

				}

			}
			return true;
		}
		case 'W':
		case 'w':
		{			
			if (pSharedPnter->b_LockW)
			{
				auto sk = Player::GetSkillByIndex(1);
				if (sk)
				{
					auto mons = GetHealthLowerestPerson(m_personList, sk->GetDistance());
					if (mons)
					{
						static DWORD timeSec = 0;
						if (GetTickCount() - timeSec > 300)
						{
							Player::UseSkillToMons(mons, 1, pSharedPnter->f_Sk2Delay, pSharedPnter->f_sk2Speed, false);
							timeSec = GetTickCount();
							
						}
					}
				}

			}
			return true;
		}
		case 'E':
		case 'e':
		{
			if (pSharedPnter->b_LockE)
			{
				auto sk = Player::GetSkillByIndex(2);
				if (sk)
				{
					auto mons = GetHealthLowerestPerson(m_personList, sk->GetDistance());
					if (mons)
					{
						static DWORD timeSec = 0;
						if (GetTickCount() - timeSec > 300)
						{
							Player::UseSkillToMons(mons, 2, pSharedPnter->f_Sk3Delay, pSharedPnter->f_sk3Speed, false);
							timeSec = GetTickCount();
							
						}

					}
				}
				
			}
			return true;
		}
		case 'R':
		case 'r':
		{
			if (pSharedPnter->b_LockR)
			{
				auto sk = Player::GetSkillByIndex(3);
				if (sk)
				{
					auto mons = GetHealthLowerestPerson(m_personList, sk->GetDistance());
					if (mons)
					{
						static DWORD timeSec = 0;
						if (GetTickCount() - timeSec > 300)
						{
							Player::UseSkillToMons(mons, 3, pSharedPnter->f_Sk4Delay, pSharedPnter->f_sk4Speed, false);
							timeSec = GetTickCount();
							
						}
						
					}
				}
			}
			return true;
		}
		}
	}

	return false;
}
