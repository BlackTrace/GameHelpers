#pragma once

//Ŀ��ѡ�����߳�
DWORD WINAPI TargetSelecterProc(
	_In_ LPVOID lpParameter
);

float GetSkillDelayByIndex(DWORD index);
float GetSkillSpeedByIndex(DWORD index);
//ͨ�����н���
DWORD WINAPI GeneralDelProc(
	_In_ LPVOID lpParameter
);

//��Ů���� (��Ĭ������)
DWORD WINAPI CassiopeiaDelProc(
	_In_ LPVOID lpParameter
);

//����˹���� ����Ĭ�����У�
DWORD WINAPI XerathDelProc(
	_In_ LPVOID lpParameter
);

//����Ь���� (��Ĭ������)
DWORD WINAPI KalistaDelProc(
	_In_ LPVOID lpParameter
);

//����ű�
//159C8020  4B 6F 67 4D 61 77                                KogMaw
DWORD WINAPI KogMawDelProc(
	_In_ LPVOID lpParameter
);

//��������ű�
 //141DB7A0  45 7A 72 65 61 6C                                Ezreal

DWORD WINAPI EzrealDelProc(
	_In_ LPVOID lpParameter
);

//ά³˹�ű�
//141FCCA0  56 61 72 75 73                                   Varus

DWORD WINAPI VarusDelProc(
	_In_ LPVOID lpParameter
);

//ά���нű�


//���ù����ڴ�
void InitSharedMemory();