#pragma once
/*
//1.负责窗口的创建
//2.负责窗口的渲染
*/

#include <mutex>
#include <Windows.h>
#include <string>
#include "d3d9.h"

enum EM_RENDER_MSG
{
	EM_RENDER_LOIN = 100,
	EM_RENDER_REGISTER,
	EM_RENDER_ADDTIME,
	EM_RENDER_CONFIG
};


class CGameRender
{
	CGameRender();
	CGameRender(const CGameRender&) = delete;
	void operator=(const CGameRender&) = delete;
public:
	~CGameRender();
	//获取单实例对象
	static CGameRender* GetInstance();
	//初始化
	bool Init(HINSTANCE hInstance);
	void MessageLoop();
	//发送托盘消息
	void SendTrayMessage(std::string uMsg);
private:
	//初始化程序窗口
	bool InitWindow();
	//初始化D3D
	bool InitD3d();
	//渲染
	void Render();
	//登录
	void Login();
	//注册
	void Register();
	//充值
	void Addtime();
	//主界面
	void MainPage();
	//初始化共享内存
	void InitSharedMemory();
	//初始化系统托盘
	void InitTray(HINSTANCE hInstance);

private:
	static std::mutex m_mutex;
	static CGameRender* m_pInstance;
	std::string m_className;
	HWND m_WndHwnd;
	int m_width;
	int m_height;
	WNDCLASSEX m_wndClassEx;
	LPDIRECT3D9 m_pDirect3d;
	LPDIRECT3DDEVICE9 m_pD3DDevice;
	EM_RENDER_MSG m_index;
	char m_username[32]; //用户名
	char m_password[32]; //密码
	char m_key[64];      //卡密
	char m_recommend[32];
	std::string m_log;
	bool m_bAutoLogin;

public:
	static NOTIFYICONDATA m_nid; //初始化托盘相关
	static HMENU m_hMenu; //托盘菜单
};

//游戏窗口的回调函数
LRESULT CALLBACK WindowProc(
	_In_ HWND   hwnd,
	_In_ UINT   uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
);

//游戏注入线程
DWORD WINAPI InjectThreadProc(
	_In_ LPVOID lpParameter
);