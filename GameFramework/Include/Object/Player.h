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
private : // LeverMove, DashMove
	// 레버 속도 프로그램
	float m_MoveVelocity;
	float m_MoveMaxVelocity;

	float m_LeverMoveAccel;
	float m_LeverVelocity;
	float m_LeverMaxMoveVelocity;

	float m_ButtonMoveAccel;
	float m_ButtonVelocity;
	float m_ButtonMaxMoveVelocity;
	float m_DashTwiceLimitTime;

	bool m_RightMove;
	bool m_ToLeftWhenRightMove;
	bool m_RightMovePush;

	bool m_LeftMove;
	bool m_ToRightWhenLeftMove;
	bool m_LeftMovePush;

	bool m_IsLeverMoving;
	bool m_IsButtonMoving;

private : // Swim
	bool m_IsSwimmingUp;
	float m_SwimSpeed;
	float m_NoSwimDownSpeed;


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
	void MoveLeft(float DeltaTime);
	void MoveDashLeft(float DeltaTime);
	void MoveRight(float DeltaTime);
	void MoveDashRight(float DeltaTime);
	void RightLeverMoveEnd(float DeltaTime);
	void RightDashMoveEnd(float DeltaTime);
	void LeftLeverMoveEnd(float DeltaTime);
	void LeftDashMoveEnd(float DeltaTime);
	void PlayerMoveUpdate(float DeltaTime);
private : // Swim;
	void SwimMoveUp(float DeltaTime);
	void NoSwimGoDown(float DeltaTime);
private :
	void MoveUp(float DeltaTime);
	void MoveDown(float DeltaTime);
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
	void MoveInfoReset();
public:
	void AttackEnd();
	void Fire();

	void Skill1End();
	void Skill1Enable();
};

