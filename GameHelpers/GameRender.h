#pragma once
/*
//1.���𴰿ڵĴ���
//2.���𴰿ڵ���Ⱦ
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
	//��ȡ��ʵ������
	static CGameRender* GetInstance();
	//��ʼ��
	bool Init(HINSTANCE hInstance);
	void MessageLoop();
	//����������Ϣ
	void SendTrayMessage(std::string uMsg);
private:
	//��ʼ�����򴰿�
	bool InitWindow();
	//��ʼ��D3D
	bool InitD3d();
	//��Ⱦ
	void Render();
	//��¼
	void Login();
	//ע��
	void Register();
	//��ֵ
	void Addtime();
	//������
	void MainPage();
	//��ʼ�������ڴ�
	void InitSharedMemory();
	//��ʼ��ϵͳ����
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
	char m_username[32]; //�û���
	char m_password[32]; //����
	char m_key[64];      //����
	char m_recommend[32];
	std::string m_log;
	bool m_bAutoLogin;

public:
	static NOTIFYICONDATA m_nid; //��ʼ���������
	static HMENU m_hMenu; //���̲˵�
};

//��Ϸ���ڵĻص�����
LRESULT CALLBACK WindowProc(
	_In_ HWND   hwnd,
	_In_ UINT   uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
);

//��Ϸע���߳�
DWORD WINAPI InjectThreadProc(
	_In_ LPVOID lpParameter
);