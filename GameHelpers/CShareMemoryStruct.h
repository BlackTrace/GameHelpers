#pragma once
#include <Windows.h>
#define GAME_MAP_NAME "GameHelpers_1.1"

typedef struct _SHARED_MEMORY_ST
{
	//�����ӳ� 
	float f_Sk1Delay;
	float f_Sk2Delay;
	float f_Sk3Delay;
	float f_Sk4Delay;
	//�����ٶ�
	float f_sk1Speed;
	float f_sk2Speed;
	float f_sk3Speed;
	float f_sk4Speed;
	//Ŀ��ѡ����
	int i_targetselecter;

	//�߿�
	bool b_OpenMove;
	bool b_OpenAttack;
	//�����ȼ�
	DWORD dwLianZhaoHotKey;


	//��������
	bool b_LockQ;
	bool b_LockW;
	bool b_LockE;
	bool b_LockR;

	//����
	bool b_AttackAfterSkill; //�Ƿ��ڼ��ܺ��ƽA
	int  i_Sk1ToUse; //���м���1
	bool b_Sk1ToUse; //�Ƿ��ܼ���1
	int  i_Sk2ToUse; //���м���2
	bool b_Sk2ToUse; //�Ƿ��ܼ���1
	int  i_Sk3ToUse; //���м���3
	bool b_Sk3ToUse; //�Ƿ��ܼ���1
	int  i_Sk4ToUse; //���м���4
	bool b_Sk4ToUse; //�Ƿ��ܼ���1

	//����
	bool  b_OpenFarming;
	DWORD dwFarmingHotKey;

	//����ȼ�
	bool b_OpenQingArmy;
	DWORD dwQingArmyHotKey;

	//���

	//���
	bool b_Evade;


	bool b_openCassiopeiaFarming; //��Ů����
	bool b_openKalistaFarmingToPerson; //����ЬE��
	bool b_openKalistaFarmingToMonster; //����ЬE��
}SHARED_MEMORY_ST , *PSHARED_MEMORY_ST;