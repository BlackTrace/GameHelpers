#pragma once
#include "stdafx.h"
#include <mutex>
#include "Monster.h"

enum MESSAGE
{
	MSG_FINDWAY = 100,
	MSG_ATTACKCALL,
	MSG_USESKILL,
	MSG_USESKILL2,
	MSG_DEFENSE
};

struct SKILL_TO_MONS
{
	DWORD targetId;
	EP_POINT_3D pCurrPnt;
	EP_POINT_3D pDestPnt;
	EP_POINT_3D pAskPnt;
	DWORD skillIndex;
	DWORD skillBase;
};


class CHookToMainThread
{
	CHookToMainThread();
	CHookToMainThread(const CHookToMainThread&) = delete;
	void operator= (const CHookToMainThread&) = delete;
	static std::mutex m_mutex;
	static CHookToMainThread* m_pInstance;
public:
	static CHookToMainThread* GetInstance();
	~CHookToMainThread();

	//挂载到主线程
	bool Hook();
	//从主线程卸载
	bool UnHook();
	//向主线程发送消息
	void SendMessageToMainThread(MESSAGE msg, LPARAM lparam);
public:
	static HHOOK m_hHook;
	static DWORD m_msgCode;
	static HHOOK m_msgHook;
	static LONG m_msgHookEX;
private:
	HWND GetGameHwnd() const;
};

LRESULT CALLBACK CallWndProc(
	_In_ int    nCode,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
);

LRESULT CALLBACK WindowProc(
	_In_ HWND   hwnd,
	_In_ UINT   uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
);

LRESULT CALLBACK GetMsgProc(_In_ int code, _In_ WPARAM wParam, _In_ LPARAM lParam);