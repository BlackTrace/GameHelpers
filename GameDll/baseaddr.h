#pragma once
#include "stdafx.h"


const DWORD Base_RoleSelfAddr = 0x01b016c0;    //��һ���ַ     ƫ��:2    //OD��ַ:0x00B02AB2
const DWORD Base_MonsterCurrentHpOffset = 0x0000064C;    //��ǰѪ��ƫ��     ƫ��:5    //OD��ַ:0x00AA59DF
const DWORD Base_MonsterCurrentMpOffset = 0x000002D8;    //��ǰ����ƫ��     ƫ��:5    //OD��ַ:0x00AA59DF
const DWORD Base_MonsterMoveSpeedOffset = 0xA74;  //�ƶ��ٶ�
const DWORD Base_MonsterAttackSpeedOffset = 0xA78; //��������
const DWORD Base_MonsterDefenseOffset = 0xA5C; //������
const DWORD Base_MonsterFixedAttackPowerOffset = 0xA34; //�̶�������
const DWORD Base_MonsterDynamicAttackPowerOffset = 0x9BC; //��̬������
const DWORD Base_MonsterDeadTagOffset = 0x100; //�Ƿ�������־λ
const DWORD Base_GameWndHwndAddr = 0x1AFFA2C;    //��Ϸ���ڻ���ַ     ƫ��:2    //OD��ַ:0x0081AD1E
const DWORD Base_SkillTravseOffset1 = 0x00001F10;    //���������һ��ƫ��     ƫ��:1    //OD��ַ:0x006C141D
const DWORD Base_SkillTravseOffset2 = 0x00000590;    //��������ڶ���ƫ��     ƫ��:2    //OD��ַ:0x006BAD50
const DWORD Base_SkillOffset_Object = 0x000000FC;    //���ܶ���ƫ��     ƫ��:2    //OD��ַ:0x006C1434
const DWORD Base_SkillAggressivityOffset = 0x0c4;//���ܻ���������
const DWORD Base_SkillOffset_Range = 0x37c; //���ܷ�Χƫ��
const DWORD Base_SkillMpExpendOffset = 0x568; //��������
const DWORD Base_BufferOffset = 0x1630;    //buffƫ��
const DWORD Base_BufferCountOffset = 0x70; //BUFF����
const DWORD Base_GameStartTime = 0x1B020A4;    //��Ϸ����ʱ��     ƫ��:1    //OD��ַ:0x00836C62
const DWORD Base_MonsterArrayAddr = 0x1B04134;    //�����������ַ     ƫ��:2    //OD��ַ:0x006A2502
const DWORD Base_SkillCallAddr2 = 0x039FED43; //����CALL2
const DWORD Base_SkillCallAddr = 0x00DC90F0; //����CALL //039C2384 
const DWORD Base_FindWayCallAddr = 0x00C41230;    //Ѱ·call     ƫ��:1    //OD��ַ:0x00AE5662
const DWORD Base_MousePointAddr = 0x1AFE168;      //���λ��
const DWORD Base_MonsterBMovingOffset = 0x54A0;  //�Ƿ��ƶ� 00C0DB87    8A8F E0400000   MOV CL, BYTE PTR DS : [EDI + 40E0]
const DWORD Base_MonsterOrientationXOffset = 0x1774;    //����ƫ��     ƫ��:5    //OD��ַ:0x00AA59DF 
const DWORD Base_UIAddr = 0x1B00584;  //UI�������ַ
const DWORD Base_MonsterBVisableOffset = 0x1e8; //�Ƿ�ɼ�ƫ��1
const DWORD Base_MonsterBVisableOffset2 = 0x0d8; //�Ƿ�ɼ�ƫ��1
const DWORD Base_CassiopeiaAggressivity = 0x60; //��ŮE���ܻ����˺�����


