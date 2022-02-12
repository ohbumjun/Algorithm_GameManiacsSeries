#pragma once
#include "Character.h"
#include "Bullet.h"
class CPlayer :
    public CCharacter
{
	friend class CScene;

protected:
	CPlayer();
	CPlayer(const CPlayer& obj);
	virtual ~CPlayer();

protected:
	bool	m_Skill1Enable;
	float	m_Skill1Time;
	std::list<CSharedPtr<CBullet>>	m_Skill1BulletList;
	CSharedPtr<CWidgetComponent>	m_HPBarWidget;
	CSharedPtr<CWidgetComponent>	m_NameWidget;

	// 레버 속도 프로그램
	float m_MoveVelocity;
	float m_MoveMaxVelocity;

	float m_LeverMoveAccel;
	float m_LeverVelocity;
	float m_LeverMaxMoveVelocity;

	float m_ButtonMoveAccel;
	float m_ButtonVelocity;
	float m_ButtonMaxMoveVelocity;

	bool m_RightMove;
	bool m_ToLeftWhenRightMove;
	bool m_RightMovePush;

	bool m_LeftMove;
	bool m_ToRightWhenLeftMove;
	bool m_LeftMovePush;

	bool m_IsMoving;

public:
	virtual void SetAttackSpeed(float Speed)
	{
		CCharacter::SetAttackSpeed(Speed);

		SetAnimationPlayScale("LucidNunNaRightAttack", Speed);
	}

public:
	virtual void Start();
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void PostUpdate(float DeltaTime);
	virtual void Collision(float DeltaTime);
	virtual void Render(HDC hDC);
	virtual CPlayer* Clone();
	virtual float SetDamage(float Damage);

private:
	bool CalculateLeftDir();
	bool CalculateRightDir();
private :
	void MoveUp(float DeltaTime);
	void MoveDown(float DeltaTime);
	void MoveLeft(float DeltaTime);
	void MoveDashLeft(float DeltaTime);
	void MoveRight(float DeltaTime);
	void MoveDashRight(float DeltaTime);
private :
	void BulletFire(float DeltaTime);
	void Pause(float DeltaTime);
	void Resume(float DeltaTime);
	void Skill1(float DeltaTime);
	void JumpKey(float DeltaTime);
private :
	float CalculateLeverMoveSpeed(float DeltaTime);
	float CalculateButtonMoveSpeed(float DeltaTime);
	float CalculateTotalMoveSpeed();
	void RightMoveButtonUp(float DeltaTime);
	void LeftMoveButtonUp(float DeltaTime);
	void MoveInfoReset();
public:
	void AttackEnd();
	void Fire();

	void Skill1End();
	void Skill1Enable();
};

