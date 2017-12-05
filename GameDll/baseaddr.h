#pragma once
#include "stdafx.h"


const DWORD Base_RoleSelfAddr = 0x01b016c0;    //玩家基地址     偏移:2    //OD地址:0x00B02AB2
const DWORD Base_MonsterCurrentHpOffset = 0x0000064C;    //当前血量偏移     偏移:5    //OD地址:0x00AA59DF
const DWORD Base_MonsterCurrentMpOffset = 0x000002D8;    //当前蓝量偏移     偏移:5    //OD地址:0x00AA59DF
const DWORD Base_MonsterMoveSpeedOffset = 0xA74;  //移动速度
const DWORD Base_MonsterAttackSpeedOffset = 0xA78; //攻击距离
const DWORD Base_MonsterDefenseOffset = 0xA5C; //防御力
const DWORD Base_MonsterFixedAttackPowerOffset = 0xA34; //固定攻击力
const DWORD Base_MonsterDynamicAttackPowerOffset = 0x9BC; //动态攻击力
const DWORD Base_MonsterDeadTagOffset = 0x100; //是否死亡标志位
const DWORD Base_GameWndHwndAddr = 0x1AFFA2C;    //游戏窗口基地址     偏移:2    //OD地址:0x0081AD1E
const DWORD Base_SkillTravseOffset1 = 0x00001F10;    //技能数组第一层偏移     偏移:1    //OD地址:0x006C141D
const DWORD Base_SkillTravseOffset2 = 0x00000590;    //技能数组第二层偏移     偏移:2    //OD地址:0x006BAD50
const DWORD Base_SkillOffset_Object = 0x000000FC;    //技能对象偏移     偏移:2    //OD地址:0x006C1434
const DWORD Base_SkillAggressivityOffset = 0x0c4;//技能基础攻击力
const DWORD Base_SkillOffset_Range = 0x37c; //技能范围偏移
const DWORD Base_SkillMpExpendOffset = 0x568; //技能蓝耗
const DWORD Base_BufferOffset = 0x1630;    //buff偏移
const DWORD Base_BufferCountOffset = 0x70; //BUFF层数
const DWORD Base_GameStartTime = 0x1B020A4;    //游戏开局时间     偏移:1    //OD地址:0x00836C62
const DWORD Base_MonsterArrayAddr = 0x1B04134;    //怪物数组基地址     偏移:2    //OD地址:0x006A2502
const DWORD Base_SkillCallAddr2 = 0x039FED43; //技能CALL2
const DWORD Base_SkillCallAddr = 0x00DC90F0; //技能CALL //039C2384 
const DWORD Base_FindWayCallAddr = 0x00C41230;    //寻路call     偏移:1    //OD地址:0x00AE5662
const DWORD Base_MousePointAddr = 0x1AFE168;      //鼠标位置
const DWORD Base_MonsterBMovingOffset = 0x54A0;  //是否移动 00C0DB87    8A8F E0400000   MOV CL, BYTE PTR DS : [EDI + 40E0]
const DWORD Base_MonsterOrientationXOffset = 0x1774;    //朝向偏移     偏移:5    //OD地址:0x00AA59DF 
const DWORD Base_UIAddr = 0x1B00584;  //UI界面基地址
const DWORD Base_MonsterBVisableOffset = 0x1e8; //是否可见偏移1
const DWORD Base_MonsterBVisableOffset2 = 0x0d8; //是否可见偏移1
const DWORD Base_CassiopeiaAggressivity = 0x60; //蛇女E技能基础伤害便宜


