#pragma once
#include "stdafx.h"
#include "Monster.h"
namespace utils
{
	//���ڴ溯��
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

	//��־����
	void log(char* _format, ...);

	//����ֵΪ�����ƽ��
	float GetDistance(PEP_POINT_3D pt1, PEP_POINT_3D pt2);
};

