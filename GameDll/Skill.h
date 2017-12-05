#pragma once

enum SK_SKILL_TYPE
{
	SK_SKILL_LINE = 0xC5F0,   //ָ���Լ��ܣ�����Q
	SK_SKILL_LOCK = 0xB100,   //�������ܣ����簲��Q
	SK_SKILL_SECTOR = 0x8A60, //���μ��ܣ����簲��W
	SK_SKILL_ROUND = 0xACF0, //Բ�μ��ܣ�����R ,����R
	SK_SKILL_ROUND2 = 0xDE00, //����˹R , ����Q
	SK_SKILL_TOSELF = 0xF3A0, //���Լ��ͷŵļ���
	SK_SKILL_VIKTOR = 0xD9C0, //ά���е�E
};

class Skill
{
public:
	Skill(DWORD dwNodeBase, DWORD _index);
	~Skill();
	//��ȡ��������
	DWORD GetIndex() const;
	//��ȡ��������
	char* GetName() const;
	//��ȡ���ܵȼ�
	DWORD GetLevel() const;
	//�����Ƿ���ȴ
	bool BCoolDown() const;
	//��ȡ�������ĵ�����
	float GetExpendMp() const;
	//��ȡ���ܷ�Χ
	float GetDistance() ;
	//��ȡ���ܷ�Χ2
	float GetDistance1() ;
	//��ȡ���ܷ�Χ2
	float GetDistance2() ;
	//��ȡ���ܷ�Χ3
	float GetDistance3() ;
	//��ȡ���ܹ�����
	float GetSkillAggressivity() const;
	//��ȡ���ܹ������ӳɱ���
	//��ȡ��������
	WORD GetSkillType();
	//��ȡ��ŮE���ܻ����˺�
	float GetBaseAggressivity() const;
	//��ȡ����˹����ʱ��
	float GetXerathPowerTime();
	DWORD GetNodeBase();

	//���ü��ܷ�Χ
	void SetDistance(float dis);
	//���ü��ܿ��
	void SetWidth(float width);
	float GetWidth();
private:
	DWORD m_index;
	WORD m_skillType;
	DWORD m_dwNodeBase;
	float m_Distance;
	//���ܷ�Χ
	float m_skDistance1[7];
	float m_skDistance2[7];
	float m_skDistance3[7];
	//���ܹ�����
	float m_skAggressivity[7];
	//���ܿ��
	float m_skWidth;
};