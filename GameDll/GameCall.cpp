#include "stdafx.h"
#include "GameCall.h"
#include "utils.h"

//处理技能校验



float GameCall::GetGameTickTime()
{
	return utils::read<float>(Base_GameStartTime + 0x30);
}

PEP_POINT_3D GameCall::GetMousePoint()
{
	auto obj = utils::read<DWORD>(Base_MousePointAddr);
	if (obj)
	{
		auto offset = utils::read<DWORD>(obj + 0x14);
		if (offset)
		{
			return (PEP_POINT_3D)(offset + 0x10);
		}
	}

	return nullptr;
}

void GameCall::FindWay(PEP_POINT_3D pt)
{

	VMProtectBegin(__FUNCTION__);
	__asm
	{
		PUSH 0;
		PUSH 0;
		PUSH 0;
		PUSH 0;
		MOV EDI, pt;
		PUSH EDI;
		PUSH 2;
		MOV ECX, Base_RoleSelfAddr;
		MOV ECX, [ECX];
		MOV EAX, Base_FindWayCallAddr;
		CALL EAX;
	}
	VMProtectEnd();

}

void GameCall::Attack(DWORD dwNodeBase)
{
	VMProtectBegin(__FUNCTION__);
	__asm
	{

		PUSH 0;
		PUSH 0;
		PUSH 0;
		MOV EAX, Base_MousePointAddr;
		MOV EAX, [EAX];
		MOV ESI, DWORD PTR DS : [EAX + 30];
		MOV EBX, dwNodeBase;
		PUSH EBX;
		LEA EAX, [EBX + 0x104];
		PUSH EAX;
		PUSH 3;
		MOV EDX, Base_RoleSelfAddr;
		MOV EDX, [EDX];
		MOV ECX, EDX;
		MOV EAX, Base_FindWayCallAddr;
		CALL EAX;
	}
	VMProtectEnd();
}

void GameCall::UseSkill(DWORD dwtargetId, PEP_POINT_3D pCurrPnt, PEP_POINT_3D pDestPnt, DWORD dwSkillIndex, DWORD dwSkillBase)
{
	VMProtectBegin(__FUNCTION__);
	__asm
	{
		MOV ECX, Base_RoleSelfAddr;
		MOV ECX, [ECX];
		ADD ECX, Base_SkillTravseOffset1;
		MOV EBX, dwtargetId;
		PUSH EBX;
		PUSH pCurrPnt;
		PUSH pDestPnt;
		MOV EAX, dwSkillIndex;
		PUSH EAX;
		MOV EDI, dwSkillBase;
		PUSH EDI;
		MOV EAX, Base_SkillCallAddr;
		CALL EAX;
	}
	VMProtectEnd();
}

void GameCall::useSkill2(PEP_POINT_3D pnt, DWORD index, DWORD skObj)
{
	VMProtectBegin(__FUNCTION__);
	__asm {
		MOV ESI, Base_RoleSelfAddr;
		MOV ESI, [ESI];
		PUSH 1;
		PUSH pnt;
		PUSH index;
		PUSH skObj;
		MOV ECX, ESI;
		ADD ECX, Base_SkillTravseOffset1;
		MOV EAX, Base_SkillCallAddr2;
		CALL EAX;
	}
	VMProtectEnd();
}

bool GameCall::Defense()
{
	VMProtectBegin(__FUNCTION__);
	__asm
	{
		PUSH 1;
		PUSH 0;
		PUSH 0;
		PUSH 0;
		MOV ECX, Base_RoleSelfAddr;
		MOV ECX, [ECX];
		LEA EAX, DWORD PTR DS : [ECX + 104];
		PUSH EAX;
		PUSH 0x0A;
		MOV EAX, Base_FindWayCallAddr;
		CALL EAX;
	}
	VMProtectEnd();
}



