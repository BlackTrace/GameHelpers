#pragma once
#include <Windows.h>
#define GAME_MAP_NAME "GameHelpers_1.1"

typedef struct _SHARED_MEMORY_ST
{
	//技能延迟 
	float f_Sk1Delay;
	float f_Sk2Delay;
	float f_Sk3Delay;
	float f_Sk4Delay;
	//技能速度
	float f_sk1Speed;
	float f_sk2Speed;
	float f_sk3Speed;
	float f_sk4Speed;
	//目标选择器
	int i_targetselecter;

	//走砍
	bool b_OpenMove;
	bool b_OpenAttack;
	//连招热键
	DWORD dwLianZhaoHotKey;


	//锁定技能
	bool b_LockQ;
	bool b_LockW;
	bool b_LockE;
	bool b_LockR;

	//连招
	bool b_AttackAfterSkill; //是否在技能后接平A
	int  i_Sk1ToUse; //连招技能1
	bool b_Sk1ToUse; //是否技能技能1
	int  i_Sk2ToUse; //连招技能2
	bool b_Sk2ToUse; //是否技能技能1
	int  i_Sk3ToUse; //连招技能3
	bool b_Sk3ToUse; //是否技能技能1
	int  i_Sk4ToUse; //连招技能4
	bool b_Sk4ToUse; //是否技能技能1

	//补刀
	bool  b_OpenFarming;
	DWORD dwFarmingHotKey;

	//清兵热键
	bool b_OpenQingArmy;
	DWORD dwQingArmyHotKey;

	//活化剂

	//躲避
	bool b_Evade;


	bool b_openCassiopeiaFarming; //蛇女补刀
	bool b_openKalistaFarmingToPerson; //滑板鞋E人
	bool b_openKalistaFarmingToMonster; //滑板鞋E兵
}SHARED_MEMORY_ST , *PSHARED_MEMORY_ST;