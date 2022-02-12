#pragma once
#include "Character.h"
class CMonster :
    public CCharacter
{
	friend class CScene;

protected:
	CMonster();
	CMonster(const CMonster& obj);
	virtual ~CMonster();

private:
	EMonsterAI	m_AI;
	Vector2	m_Dir;
	float	m_FireTime;
	float	m_FireTimeMax;
	int		m_Count;
	float	m_DetectDistance;
	float	m_AttackDistance;
	bool	m_AttackEnable;
	bool	m_Skill1Enable;

public:
	virtual void Start();
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void PostUpdate(float DeltaTime);
	virtual void Collision(float DeltaTime);
	virtual void Render(HDC hDC);
	virtual CMonster* Clone();
	virtual float SetDamage(float Damage);

protected:
	void AIIdle(float DeltaTime);
	void AITrace(float DeltaTime);
	void AIAttack(float DeltaTime);
	void AIDeath(float DeltaTime);

public:
	void AttackEnd();
	void Fire();
};

