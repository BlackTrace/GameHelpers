#include "main.h"
#include "NetworkService.h"
#include "GameRender.h"
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE prevhInst, LPSTR cmdLine, int show)
{
	//初始化网络库
	if (!CNetworkService::GetInstance()->Init())
	{
		return 1;
	}

	//初始化窗口
	if (!CGameRender::GetInstance()->Init(hInst))
	{
		return 1;
	}

	CGameRender::GetInstance()->MessageLoop();

	return 0;
}
