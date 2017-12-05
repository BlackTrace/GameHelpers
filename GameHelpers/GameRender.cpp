#include "GameRender.h"
#include "imgui\imgui.h"
#include "imgui\imgui_impl_dx9.h"
#include "VMProtectSDK.h"
#include "NetworkService.h"
#include "ShareMemoryService.h"
#include "virtualKey.h"
#include "resource.h"
#include <sstream>
#include "..\GameDll\HookToMainThread.h"
std::mutex CGameRender::m_mutex;
CGameRender* CGameRender::m_pInstance = nullptr;

#define WM_TRAY (WM_USER + 100)  
#define WM_TASKBAR_CREATED RegisterWindowMessage(TEXT("TaskbarCreated"))  
NOTIFYICONDATA CGameRender::m_nid = {0}; //初始化托盘相关
HMENU CGameRender::m_hMenu = {0}; //托盘菜单
CGameRender::CGameRender()
{
	m_className = "dsaas";
	m_WndHwnd = 0;
	m_width = 480;
	m_height = 360;
	m_wndClassEx = {0};
	m_pDirect3d = nullptr;
	m_pD3DDevice = nullptr;
	m_index = EM_RENDER_LOIN;
	memset(m_username, 0, 32);
	memset(m_password, 0, 32);
	memset(m_key, 0, 64);
	memset(m_recommend, 0, 32);
	m_bAutoLogin = true;
}


CGameRender::~CGameRender()
{

	ImGui_ImplDX9_Shutdown();

	if (m_pD3DDevice)
	{
		m_pD3DDevice->Release();
	}

	if (m_pDirect3d)
	{
		m_pDirect3d->Release();
	}

	UnregisterClass(m_className.c_str(), m_wndClassEx.hInstance);
}

CGameRender * CGameRender::GetInstance()
{
	if (nullptr == m_pInstance)
	{
		m_mutex.lock();
		if (nullptr == m_pInstance)
		{
			m_pInstance = new CGameRender();
		}
		m_mutex.unlock();
	}
	return m_pInstance;
}

bool CGameRender::Init(HINSTANCE hInstance)
{
	if (!InitWindow())
	{
		return false;
	}

	InitTray(hInstance);

	//初始化d3d失败
	if (!InitD3d())
	{
		return false;
	}

	//初始化imgui for dx9
	if (!ImGui_ImplDX9_Init(m_WndHwnd, m_pD3DDevice))
	{
		return false;
	}

	//自动登录逻辑
	GetPrivateProfileString("Account", "username", "", m_username, 32, ".\\config.ini");
	GetPrivateProfileString("Account", "password", "", m_password, 32, ".\\config.ini");
	if (CNetworkService::GetInstance()->Login(m_username, m_password) == 0)
	{
		if (CShareMemoryService::GetInstance()->Init())
		{
			m_index = EM_RENDER_CONFIG;
			InitSharedMemory();
		}
	}

	//显示并更新窗口
	ShowWindow(m_WndHwnd, SW_SHOWDEFAULT);
	UpdateWindow(m_WndHwnd);

	return true;
}

void CGameRender::MessageLoop()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			Render();
		}
	}
}

bool CGameRender::InitWindow()
{
	//初始化窗口风格
	m_wndClassEx = { sizeof(WNDCLASSEX), CS_CLASSDC, WindowProc, 0L, 0L,
		GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
		m_className.c_str(), NULL };
	RegisterClassEx(&m_wndClassEx);

	//创建一个应用程序窗口
	m_WndHwnd = CreateWindow(m_className.c_str(), "", 
		WS_EX_TOOLWINDOW | WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
		100, 100, m_width, m_height, GetDesktopWindow(), NULL,
		m_wndClassEx.hInstance, NULL);
	if (!m_WndHwnd)
	{
		return false;
	}

	return true;
}

bool CGameRender::InitD3d()
{
	D3DDISPLAYMODE displayMode;

	//创建一个D3D对象
	m_pDirect3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (m_pDirect3d == NULL)
	{
		return false;
	}

	//获取桌面显示模式
	if (FAILED(m_pDirect3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displayMode)))
	{
		return false;
	}


	//使用下面的结构来初始化D3D设备对象
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));

	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = displayMode.Format;

	// Create the D3DDevice
	if (FAILED(m_pDirect3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_WndHwnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &m_pD3DDevice)))
	{
		return false;
	}

	return true;
}
void CGameRender::Render()
{
	// 清空缓冲区
	m_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(20, 20, 20), 1.0f, 0);

	// 开始渲染
	m_pD3DDevice->BeginScene();

	switch (m_index)
	{
	case EM_RENDER_LOIN:
	{ //登录
		Login();
		break;
	}
	case EM_RENDER_REGISTER:
	{ //注册
		Register();
		break;
	}
	case EM_RENDER_ADDTIME:
	{
		Addtime();
		break;
	}
	case EM_RENDER_CONFIG:
	{
		MainPage();
		break;
	}
	default: 
		break;
	}

	// 结束渲染
	m_pD3DDevice->EndScene();

	// 缓冲区翻转
	m_pD3DDevice->Present(NULL, NULL, NULL, NULL);
}
void CGameRender::Login()
{
	ImGui_ImplDX9_NewFrame();
	//设置窗口风格
	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoTitleBar;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_AlwaysAutoResize;


	if (!ImGui::Begin("", NULL, ImVec2(0, 0), 0.0f, window_flags))
	{
		ImGui::End();
		return;
	}
	ImGui::SetWindowPos(ImVec2(0, 0));

	ImGui::NewLine();
	ImGui::NewLine();
	ImGui::NewLine();
	ImGui::NewLine();
	
	ImGui::Text("Username:"); ImGui::SameLine();
	ImGui::PushItemWidth(300);
	ImGui::InputText("##username", m_username, 32);
	ImGui::PopItemWidth();
	ImGui::Text("Password:"); ImGui::SameLine();
	ImGui::PushItemWidth(300);
	ImGui::InputText("##password", m_password, 32);
	ImGui::PopItemWidth();
	ImGui::PushItemWidth(150);
	ImGui::LabelText("","%s", m_log.c_str()); ImGui::SameLine();
	ImGui::PopItemWidth();
	ImGui::Checkbox("Auto Logon", &m_bAutoLogin); ImGui::SameLine();
	if (ImGui::Button("LOGIN", ImVec2(100, 0)))
	{
		//处理登录逻辑

		auto ret =  CNetworkService::GetInstance()->Login(m_username, m_password);
		switch (ret)
		{
		case 0: 
		{
			if (CShareMemoryService::GetInstance()->Init())
			{
				m_index = EM_RENDER_CONFIG;
				InitSharedMemory();
				std::stringstream ss;
				ss << m_bAutoLogin;
				WritePrivateProfileString("Account", "username", m_username, ".\\config.ini");
				WritePrivateProfileString("Account", "password", m_password, ".\\config.ini");
				WritePrivateProfileString("Account", "bRemberPassword", ss.str().c_str(), ".\\config.ini");
			}
			break;
		}
		case -1: {
			m_log = "login failed!";
			break;
		}
		case -2: {
			m_log = "Registration code is disabled!";
			break;
		}
		case -3: {
			m_log = "Bind the machine to the limit!";
			break;
		}
		case -4: {
			m_log = "The registration code is online!";
			break;
		}
		case -5: {
			m_log = "expired!";
			break;
		}
		case -6: {
			m_log = "Insufficient user balance!";
			break;
		}
		case -7: {
			m_log = "The user is invalid!";
			break;
		}
		default:
			m_log = "Unknown logon error!";
			break;
		}
	}
	ImGui::NewLine();

	ImGui::BeginGroup();
	if (ImGui::Button("Register", ImVec2(150, 0)))
	{
		m_log = "";
		m_index = EM_RENDER_REGISTER;
	}
	ImGui::SameLine();
	if (ImGui::Button("Addtime", ImVec2(150, 0)))
	{
		m_log = "";
		m_index = EM_RENDER_ADDTIME;
	}
	ImGui::SameLine();
	ImGui::EndGroup();

	ImGui::End();
	ImGui::Render();

}
void CGameRender::Register()
{
	ImGui_ImplDX9_NewFrame();
	//设置窗口风格
	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoTitleBar;
	//window_flags |= ImGuiWindowFlags_ShowBorders;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoMove;
	// window_flags |= ImGuiWindowFlags_NoScrollbar;
	// window_flags |= ImGuiWindowFlags_NoCollapse;
	// window_flags |= ImGuiWindowFlags_MenuBar;
	window_flags |= ImGuiWindowFlags_AlwaysAutoResize;

	if (!ImGui::Begin("", NULL, ImVec2(0, 0), 0.0f, window_flags))
	{
		ImGui::End();
		return;
	}
	ImGui::SetWindowPos(ImVec2(0, 0));

	ImGui::NewLine();
	ImGui::NewLine();
	ImGui::NewLine();
	ImGui::NewLine();

	ImGui::Text("Username:"); ImGui::SameLine();
	ImGui::PushItemWidth(300);
	ImGui::InputText("##username", m_username, 32);
	ImGui::PopItemWidth();
	ImGui::Text("Password:"); ImGui::SameLine();
	ImGui::PushItemWidth(300);
	ImGui::InputText("##password", m_password, 32);
	ImGui::PopItemWidth();
	ImGui::PushItemWidth(220);
	ImGui::LabelText("", "%s", m_log.c_str()); ImGui::SameLine();
	ImGui::PopItemWidth();
	if (ImGui::Button("REGISTER", ImVec2(100, 0)))
	{

		auto ret =  CNetworkService::GetInstance()->Register(m_username, m_password);
		switch (ret)
		{
		case 0:
		{
			m_log = "Register success!";
			break;
		}
		case -8:
		{
			m_log = "Username has already exists!";
			break;
		}
		default:
			m_log = "Register failed!";
			break;
		}
	}


	ImGui::NewLine();

	ImGui::BeginGroup();
	if (ImGui::Button("Login", ImVec2(150, 0)))
	{
		m_log = "";
		m_index = EM_RENDER_LOIN;
	}
	ImGui::SameLine();
	if (ImGui::Button("Addtime", ImVec2(150, 0)))
	{
		m_log = "";
		m_index = EM_RENDER_ADDTIME;
	}
	ImGui::SameLine();
	ImGui::EndGroup();

	ImGui::End();
	ImGui::Render();
}

void CGameRender::Addtime()
{
	ImGui_ImplDX9_NewFrame();
	//设置窗口风格
	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoTitleBar;
	//window_flags |= ImGuiWindowFlags_ShowBorders;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoMove;
	// window_flags |= ImGuiWindowFlags_NoScrollbar;
	// window_flags |= ImGuiWindowFlags_NoCollapse;
	// window_flags |= ImGuiWindowFlags_MenuBar;
	window_flags |= ImGuiWindowFlags_AlwaysAutoResize;

	if (!ImGui::Begin("", NULL, ImVec2(0, 0), 0.0f, window_flags))
	{
		ImGui::End();
		return;
	}
	ImGui::SetWindowPos(ImVec2(0, 0));

	ImGui::NewLine();
	ImGui::NewLine();
	ImGui::NewLine();

	ImGui::Text("Username :"); ImGui::SameLine();
	ImGui::PushItemWidth(300);
	ImGui::InputText("##Username", m_username, 32);
	ImGui::PopItemWidth();
	ImGui::Text("Key      :"); ImGui::SameLine();
	ImGui::PushItemWidth(300);
	ImGui::InputText("##key", m_key, 64);
	ImGui::PopItemWidth();
	ImGui::Text("Recommend:"); ImGui::SameLine();
	ImGui::PushItemWidth(300);
	ImGui::InputText("##Recommend", m_recommend, 64);
	ImGui::PopItemWidth();
	ImGui::PushItemWidth(220);
	ImGui::LabelText("", "%s", m_log.c_str()); ImGui::SameLine();
	ImGui::PopItemWidth();
	static SHORT days = 0, points = 0;
	if (ImGui::Button("ADDTIME", ImVec2(100, 0)))
	{
		//充值操作

		auto ret = CNetworkService::GetInstance()->UserAddTime(m_username, m_key, m_recommend, days, points);
		switch (ret)
		{
		case 0:
		{
			std::stringstream ss;
			ss << "AddTime success , " << days << " days";
			m_log = ss.str();
			break;
		}
		default:
			m_log = "AddTime Failed!";
			break;
		}
	}

	ImGui::NewLine();

	ImGui::BeginGroup();
	if (ImGui::Button("Login", ImVec2(150, 0)))
	{
		m_log = "";
		m_index = EM_RENDER_LOIN;
	}
	ImGui::SameLine();
	if (ImGui::Button("Register", ImVec2(150, 0)))
	{
		m_log = "";
		m_index = EM_RENDER_REGISTER;
	}
	ImGui::SameLine();
	ImGui::EndGroup();

	ImGui::End();
	ImGui::Render();
}

void CGameRender::MainPage()
{
	ImGui_ImplDX9_NewFrame();

	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoTitleBar;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoMove;
	//window_flags |= ImGuiWindowFlags_AlwaysAutoResize;

	ImGui::SetNextWindowSize(ImVec2(m_width -30.0f, m_height-50.0f));
	if (!ImGui::Begin("", NULL, ImVec2(0, 0), 0.0f, window_flags))
	{
		ImGui::End();
		return;
	}

	ImGui::SetWindowPos(ImVec2(0, 0));
	ImGui::TextColored( ImVec4(1.0f , 0.0f , 1.0f , 1.0f), "Welcome to MT!");
	if (ImGui::CollapsingHeader("Basic"))
	{
		auto pSharedPointer = CShareMemoryService::GetInstance()->GetSharedMemoryPointer();
		if (pSharedPointer) {

			ImGui::SliderFloat("Q Delay Time", &pSharedPointer->f_Sk1Delay, 0, 1);
			ImGui::SliderFloat("Q MissileSpeed", &pSharedPointer->f_sk1Speed, 0, 2000);

			ImGui::SliderFloat("W Delay Time", &pSharedPointer->f_Sk2Delay, 0, 1);
			ImGui::SliderFloat("W MissileSpeed", &pSharedPointer->f_sk2Speed, 0, 2000);

			ImGui::SliderFloat("E Delay Time", &pSharedPointer->f_Sk3Delay, 0, 1);
			ImGui::SliderFloat("E MissileSpeed", &pSharedPointer->f_sk3Speed, 0, 2000);

			ImGui::SliderFloat("R Delay Time", &pSharedPointer->f_Sk4Delay, 0, 1);
			ImGui::SliderFloat("R MissileSpeed", &pSharedPointer->f_sk4Speed, 0, 2000);
		}
	}

	if (ImGui::CollapsingHeader("target selecter"))
	{
		auto pSharedPointer = CShareMemoryService::GetInstance()->GetSharedMemoryPointer();
		if (pSharedPointer) {
			ImGui::Combo("##targetselecter", &pSharedPointer->i_targetselecter, "AutoPriority\0LowHP\0MostAD\0MostAP\0Closest\0NearMouse\0LessAttack\0LessCast\0MostStack\0\0");
		}
		
	}

	if (ImGui::CollapsingHeader("Move & Attack"))
	{
		auto pSharedPointer = CShareMemoryService::GetInstance()->GetSharedMemoryPointer();
		if (pSharedPointer)
		{
			static char* Move[2] = { "True##Move" , "False##Move" };
			ImGui::LabelText("", "Move"); ImGui::SameLine();
			if (ImGui::Button(pSharedPointer->b_OpenMove ? Move[0] : Move[1]))
			{
				pSharedPointer->b_OpenMove = pSharedPointer->b_OpenMove ^ 1;
			}
			static char* Attack[2] = { "True##Attack"  , "False##Attack" };
			ImGui::LabelText("", "Attack"); ImGui::SameLine();
			if (ImGui::Button(pSharedPointer->b_OpenAttack ? Attack[0] : Attack[1]))
			{
				pSharedPointer->b_OpenAttack = pSharedPointer->b_OpenAttack ^ 1;
			}


			ImGuiIO& io = ImGui::GetIO();
			for (int i = 0; i < 100; i++)
			{
				if (io.KeysDown[i] == 1)
				{
					pSharedPointer->dwLianZhaoHotKey = i;
				}
			}
			ImGui::LabelText("", "HotKey##QingArmy"); ImGui::SameLine();
			ImGui::Button(KeyNames[pSharedPointer->dwLianZhaoHotKey].text, ImVec2(70, 0));
		}
	}

	if (ImGui::CollapsingHeader("Lock Skill"))
	{
		auto pSharedPointer = CShareMemoryService::GetInstance()->GetSharedMemoryPointer();
		if (pSharedPointer)
		{
			//Q
			static char* LockSkillQ[2] = { "True##LockSkillQ" , "False##LockSkillQ" };
			ImGui::LabelText("","Lock Skill Q"); ImGui::SameLine();
			if (ImGui::Button(pSharedPointer->b_LockQ ? LockSkillQ[0] : LockSkillQ[1]))
			{
				pSharedPointer->b_LockQ = pSharedPointer->b_LockQ ^ 1;
			}

			//W
			static char* LockSkillW[2] = { "True##LockSkillW" , "False##LockSkillW" };
			ImGui::LabelText("", "Lock Skill W"); ImGui::SameLine();
			if (ImGui::Button(pSharedPointer->b_LockW ? LockSkillW[0] : LockSkillW[1]))
			{
				pSharedPointer->b_LockW = pSharedPointer->b_LockW ^ 1;
			}

			//E
			static char* LockSkillE[2] = { "True##LockSkillE" , "False##LockSkillE" };
			ImGui::LabelText("", "Lock Skill E"); ImGui::SameLine();
			if (ImGui::Button(pSharedPointer->b_LockE ? LockSkillE[0] : LockSkillE[1]))
			{
				pSharedPointer->b_LockE = pSharedPointer->b_LockE ^ 1;
			}

			//R
			static char* LockSkillR[2] = { "True##LockSkillR" , "False##LockSkillR" };
			ImGui::LabelText("", "Lock Skill R"); ImGui::SameLine();
			if (ImGui::Button(pSharedPointer->b_LockR ? LockSkillR[0] : LockSkillR[1]))
			{
				pSharedPointer->b_LockR = pSharedPointer->b_LockR^ 1;
			}
		}
	}

	if (ImGui::CollapsingHeader("Combo"))
	{
		auto pSharedPointer = CShareMemoryService::GetInstance()->GetSharedMemoryPointer();
		if (pSharedPointer)
		{
			{
				static char* AttackAfSkill[2] = { "True##AttackAfSkill" , "False##AttackAfSkill" };
				ImGui::LabelText("", "Attack after skill"); ImGui::SameLine();
				if (ImGui::Button(pSharedPointer->b_AttackAfterSkill ? AttackAfSkill[0] : AttackAfSkill[1])) {
					pSharedPointer->b_AttackAfterSkill = pSharedPointer->b_AttackAfterSkill ^ 1;
				}
			}
			


			{
				ImGui::Combo("1->", &pSharedPointer->i_Sk1ToUse, "Q\0W\0E\0R\0\0"); ImGui::SameLine();
				static char* Sk1ToUse[2] = { "True##Sk1ToUse", "False##Sk1ToUse" };
				if (ImGui::Button(pSharedPointer->b_Sk1ToUse ? Sk1ToUse[0] : Sk1ToUse[1])) {
					pSharedPointer->b_Sk1ToUse = pSharedPointer->b_Sk1ToUse ^ 1;
				}
			}

			{
				ImGui::Combo("2->", &pSharedPointer->i_Sk2ToUse, "Q\0W\0E\0R\0\0"); ImGui::SameLine();
				static char* Sk2ToUse[] = { "True##Sk2ToUse", "False##Sk2ToUse" };
				if (ImGui::Button(pSharedPointer->b_Sk2ToUse ? Sk2ToUse[0] : Sk2ToUse[1])) {
					pSharedPointer->b_Sk2ToUse = pSharedPointer->b_Sk2ToUse ^ 1;
				}
			}

			{
				ImGui::Combo("3->", &pSharedPointer->i_Sk3ToUse, "Q\0W\0E\0R\0\0"); ImGui::SameLine();
				static char* Sk3ToUse[] = { "True##Sk3ToUse", "False##Sk3ToUse" };
				if (ImGui::Button(pSharedPointer->b_Sk3ToUse ? Sk3ToUse[0] : Sk3ToUse[1])) {
					pSharedPointer->b_Sk3ToUse = pSharedPointer->b_Sk3ToUse ^ 1;
				}
			}

			{
				ImGui::Combo("4->", &pSharedPointer->i_Sk4ToUse, "Q\0W\0E\0R\0\0"); ImGui::SameLine();
				static char* Sk4ToUse[] = { "True##Sk4ToUse", "False##Sk4ToUse" };
				if (ImGui::Button(pSharedPointer->b_Sk4ToUse ? Sk4ToUse[0] : Sk4ToUse[1])) {
					pSharedPointer->b_Sk4ToUse = pSharedPointer->b_Sk4ToUse ^ 1;
				}
			}

		}
	}
	if (ImGui::CollapsingHeader("Farming"))
	{
		//补刀
		auto pSharedPointer = CShareMemoryService::GetInstance()->GetSharedMemoryPointer();
		if (pSharedPointer)
		{
			static char* Farming[2] = { "True##Farming", "False##Farming" };
			ImGui::LabelText("", "Farming->"); ImGui::SameLine();
			if (ImGui::Button(pSharedPointer->b_OpenFarming ? Farming[0] : Farming[1]))
			{
				pSharedPointer->b_OpenFarming = pSharedPointer->b_OpenFarming ^ 1;
			}

			ImGuiIO& io = ImGui::GetIO();
			for (int i = 0; i < 100; i++)
			{
				if (io.KeysDown[i] == 1)
				{
					pSharedPointer->dwFarmingHotKey = i;
				}
			}
			ImGui::LabelText("", "HotKey##Farming"); ImGui::SameLine();
			ImGui::Button(KeyNames[pSharedPointer->dwFarmingHotKey].text, ImVec2(70,0));
		}

	}


	if (ImGui::CollapsingHeader("QingArmy"))
	{
		//清兵
		auto pSharedPointer = CShareMemoryService::GetInstance()->GetSharedMemoryPointer();
		if (pSharedPointer)
		{
			static char* QingArmy[2] = { "True##QingArmy", "False##QingArmy" };
			ImGui::LabelText("", "QingArmy->"); ImGui::SameLine();
			if (ImGui::Button(pSharedPointer->b_OpenQingArmy ? QingArmy[0] : QingArmy[1]))
			{
				pSharedPointer->b_OpenQingArmy = pSharedPointer->b_OpenQingArmy ^ 1;
			}

			ImGuiIO& io = ImGui::GetIO();
			for (int i = 0; i < 100; i++)
			{
				if (io.KeysDown[i] == 1)
				{
					pSharedPointer->dwQingArmyHotKey = i;
				}
			}
			ImGui::LabelText("", "HotKey##QingArmy"); ImGui::SameLine();
			ImGui::Button(KeyNames[pSharedPointer->dwQingArmyHotKey].text, ImVec2(70, 0));
		}
	}

	//if (ImGui::CollapsingHeader("Activator"))
	//{
	//	//活化剂
	//	ImGui::Text("plz wait ....");
	//}

	//if (ImGui::CollapsingHeader("Evade"))
	//{
	//	//躲避
	//	ImGui::Text("plz wait ....");
	//}
	if (ImGui::CollapsingHeader("Others"))
	{
		auto pSharedPointer = CShareMemoryService::GetInstance()->GetSharedMemoryPointer();
		if (pSharedPointer)
		{
			//蛇女E 补刀
			{
				ImGui::LabelText("", "Cassiopeia E Monster"); ImGui::SameLine();
				static char* Cassiopeia[2] = { "True##Cassiopeia" , "False##Cassiopeia" };
				if(ImGui::Button(pSharedPointer->b_openCassiopeiaFarming ? Cassiopeia[0] : Cassiopeia[1]))
				{
					pSharedPointer->b_openCassiopeiaFarming = pSharedPointer->b_openCassiopeiaFarming ^ 1;
				}
				
			}
			//滑板鞋E
			{
				{
					ImGui::LabelText("", "Kalista E Monster"); ImGui::SameLine();
					static char* KalistaEm[2] = { "True##KalistaEm" , "False##KalistaEm" };
					if (ImGui::Button(pSharedPointer->b_openKalistaFarmingToMonster ? KalistaEm[0] : KalistaEm[1]))
					{
						pSharedPointer->b_openKalistaFarmingToMonster = pSharedPointer->b_openKalistaFarmingToMonster ^ 1;
					}
				}

				{
					ImGui::LabelText("", "Kalista E Person"); ImGui::SameLine();
					static char* KalistaEp[2] = { "True##KalistaEp" , "False##KalistaEp" };
					if (ImGui::Button(pSharedPointer->b_openKalistaFarmingToPerson ? KalistaEp[0] : KalistaEp[1]))
					{
						pSharedPointer->b_openKalistaFarmingToPerson = pSharedPointer->b_openKalistaFarmingToPerson ^ 1;
					}
				}

			}
		}

	}

	ImGui::Text("plz click btn in the game ! ->"); ImGui::SameLine();
	if (ImGui::Button("Click Me!"))
	{
		auto handle = ::CreateThread(NULL, NULL, LPTHREAD_START_ROUTINE(InjectThreadProc), NULL, NULL, NULL);
		CloseHandle(handle);
	}

	ImGui::SameLine();
	if (ImGui::Button("Reset All"))
	{
		InitSharedMemory();
		SendTrayMessage("重置配置完成");
	}

	ImGui::End();
	ImGui::Render();
}

void CGameRender::InitSharedMemory()
{
	auto pSharedPointer = CShareMemoryService::GetInstance()->GetSharedMemoryPointer();
	if (pSharedPointer)
	{
		pSharedPointer->f_Sk1Delay = 0.4f;
		pSharedPointer->f_Sk2Delay = 0.4f;
		pSharedPointer->f_Sk3Delay = 0.4f;
		pSharedPointer->f_Sk4Delay = 0.4f;
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

void CGameRender::InitTray(HINSTANCE hInstance)
{
	m_nid.cbSize = sizeof(NOTIFYICONDATA);
	m_nid.hWnd = m_WndHwnd;
	m_nid.uID = IDI_ICON1;
	m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
	m_nid.uCallbackMessage = WM_TRAY;
	m_nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	lstrcpy(m_nid.szTip, "MonkeyToy");

	m_hMenu = CreatePopupMenu();//生成托盘菜单  
							  //为托盘菜单添加两个选项  
	AppendMenu(m_hMenu, MF_STRING, ID_SHOW, TEXT("显示"));
	AppendMenu(m_hMenu, MF_STRING, ID_EXIT, TEXT("退出"));

	Shell_NotifyIcon(NIM_ADD, &m_nid);
}

void CGameRender::SendTrayMessage(std::string uMsg)
{
	lstrcpy(m_nid.szInfoTitle, "MonkeyToy");
	lstrcpy(m_nid.szInfo, uMsg.c_str());
	m_nid.uTimeout = 1000;
	Shell_NotifyIcon(NIM_MODIFY, &m_nid);
}


extern LRESULT ImGui_ImplDX9_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//如果IMGU处理了消息，直接返回
	if (ImGui_ImplDX9_WndProcHandler(hwnd, uMsg, wParam, lParam))
	{
		return true;
	}

	switch (uMsg)
	{
	case WM_SYSCOMMAND:
	{
		if (wParam == SC_MINIMIZE) {
			CGameRender::GetInstance()->SendTrayMessage("点击打开MokeyToy");
			ShowWindow(hwnd, SW_HIDE);
			return 1;
		}
		break;
	}
	case WM_TRAY:
		switch (lParam)
		{
		case WM_RBUTTONDOWN:
		{
			//获取鼠标坐标  
			POINT pt; GetCursorPos(&pt);

			//解决在菜单外单击左键菜单不消失的问题  
			SetForegroundWindow(hwnd);

			//使菜单某项变灰  
			//EnableMenuItem(hMenu, ID_SHOW, MF_GRAYED);      

			//显示并获取选中的菜单  
			int cmd = TrackPopupMenu(CGameRender::m_hMenu, TPM_RETURNCMD, pt.x, pt.y, NULL, hwnd,NULL);
			if (cmd == ID_SHOW)
			{
				ShowWindow(hwnd, SW_SHOW);
				return true;
			}

			if (cmd == ID_EXIT)
			{
				PostMessage(hwnd, WM_DESTROY, NULL, NULL);
				return true;
			}
			break;
		}

		case WM_LBUTTONDOWN:
		{
			ShowWindow(hwnd, SW_SHOW);
			return true;
		}
		case WM_LBUTTONDBLCLK:
			break;
		}
		break;
	case WM_DESTROY:
	{
		//窗口退出消息
		Shell_NotifyIcon(NIM_DELETE, &CGameRender::m_nid);
		PostQuitMessage(0);
		return true;
	}

	if (uMsg == WM_TASKBAR_CREATED)
	{
		//系统Explorer崩溃重启时，重新加载托盘  
		Shell_NotifyIcon(NIM_ADD, &CGameRender::m_nid);
		return true;
	}
	default:
		break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}



DWORD WINAPI InjectThreadProc(LPVOID lpParameter)
{

	DWORD pid = 0;
	VMProtectBegin(__FUNCTION__);
	VMProtectEnd();
	return 1;
}
