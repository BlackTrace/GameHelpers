#include "main.h"
#include "NetworkService.h"
#include "GameRender.h"
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE prevhInst, LPSTR cmdLine, int show)
{
	//��ʼ�������
	if (!CNetworkService::GetInstance()->Init())
	{
		return 1;
	}

	//��ʼ������
	if (!CGameRender::GetInstance()->Init(hInst))
	{
		return 1;
	}

	CGameRender::GetInstance()->MessageLoop();

	return 0;
}
