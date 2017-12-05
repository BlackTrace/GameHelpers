#pragma once
#include "stdafx.h"
#include "Monster.h"
namespace utils
{
	//读内存函数
	template<class T>
	T read(const DWORD & obj)
	{
		T value = {0};
		__try {
			value = *(T*)(obj);
		}
		__except (1) {
		}

		return value;
	}

	//日志函数
	void log(char* _format, ...);

	//返回值为距离的平方
	float GetDistance(PEP_POINT_3D pt1, PEP_POINT_3D pt2);
};

