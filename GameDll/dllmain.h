#pragma once

//目标选择器线程
DWORD WINAPI TargetSelecterProc(
	_In_ LPVOID lpParameter
);

float GetSkillDelayByIndex(DWORD index);
float GetSkillSpeedByIndex(DWORD index);
//通用连招进程
DWORD WINAPI GeneralDelProc(
	_In_ LPVOID lpParameter
);

//蛇女连招 (有默认配置)
DWORD WINAPI CassiopeiaDelProc(
	_In_ LPVOID lpParameter
);

//泽拉斯连招 （有默认连招）
DWORD WINAPI XerathDelProc(
	_In_ LPVOID lpParameter
);

//滑板鞋连招 (有默认连招)
DWORD WINAPI KalistaDelProc(
	_In_ LPVOID lpParameter
);

//大嘴脚本
//159C8020  4B 6F 67 4D 61 77                                KogMaw
DWORD WINAPI KogMawDelProc(
	_In_ LPVOID lpParameter
);

//伊泽瑞尔脚本
 //141DB7A0  45 7A 72 65 61 6C                                Ezreal

DWORD WINAPI EzrealDelProc(
	_In_ LPVOID lpParameter
);

//维鲁斯脚本
//141FCCA0  56 61 72 75 73                                   Varus

DWORD WINAPI VarusDelProc(
	_In_ LPVOID lpParameter
);

//维克托脚本


//重置共享内存
void InitSharedMemory();