
#include "Player.h"
#include "Monster.h"
#include "EffectHit.h"
#include "DamageFont.h"
#include "../Scene/Scene.h"
#include "../Scene/SceneResource.h"
#include "../Input.h"
#include "../GameManager.h"
#include "../Collision/ColliderBox.h"
#include "../Collision/ColliderSphere.h"
#include "../UI/UICharacterStateHUD.h"
#include "../UI/ProgressBar.h"
#include "../UI/UIText.h"

CPlayer::CPlayer()	:
	m_Skill1Enable(false),
	m_Skill1Time(0.f),
	m_RightMove(false),
	m_ToLeftWhenRightMove(false),
	m_RightMovePush(false),
	m_LeftMove(false),
	m_ToRightWhenLeftMove(false),
	m_LeftMovePush(false),
	m_TriangleJump(false),
	m_JumpDown(false),
	m_IsFlying(false),
	m_JumpDownDist(0.f)
{
	m_LeverMoveAccel = 1.f;
	m_ButtonMoveAccel = 1.5f;
	m_MoveVelocity = 0.f;

	m_LeverMaxMoveVelocity  = 250.f;
	m_ButtonMaxMoveVelocity = 200.f;
	m_MoveMaxVelocity = m_LeverMaxMoveVelocity + m_ButtonMaxMoveVelocity;

	// 이단 대쉬
	// m_DashTwiceLimitTime = 0.2f;

	m_IsLeverMoving = false;
	m_IsButtonMoving = false;
	
	m_IsSwimmingUp = false;
	m_SwimSpeed = 200.f;
	m_NoSwimDownSpeed = 100.;

	// Fly 속도
	m_FlySpeed = 200.f;

	// 점프에 붙는 가속도
	m_JumpAccel = 1.1f;
}

CPlayer::CPlayer(const CPlayer& obj)	:
	CCharacter(obj)
{
	m_Skill1Time = obj.m_Skill1Time;
	m_Skill1Enable = false;
}

CPlayer::~CPlayer()
{
}

void CPlayer::Start()
{
	CCharacter::Start();

	/*
	CInput::GetInst()->SetCallback<CPlayer>("MoveUp", KeyState_Push,
		this, &CPlayer::MoveUp);
		*/
	// CInput::GetInst()->SetCallback<CPlayer>("MoveUp", KeyState_Push,
	//	this, &CPlayer::SwimMoveUp);
	CInput::GetInst()->SetCallback<CPlayer>("MoveUp", KeyState_Push,
		this, &CPlayer::MoveUp);

	CInput::GetInst()->SetCallback<CPlayer>("MoveUp", KeyState_Up,
		this, &CPlayer::MoveUpEnd);
	

	// CInput::GetInst()->SetCallback<CPlayer>("MoveDown", KeyState_Push,
		// this, &CPlayer::MoveDown);
	CInput::GetInst()->SetCallback<CPlayer>("MoveDown", KeyState_Push,
		this, &CPlayer::JumpDown);

	CInput::GetInst()->SetCallback<CPlayer>("MoveLeft", KeyState_Push,
		this, &CPlayer::MoveLeft);

	CInput::GetInst()->SetCallback<CPlayer>("MoveDashLeft", KeyState_Push,
		this, &CPlayer::MoveDashLeft);

	CInput::GetInst()->SetCallback<CPlayer>("MoveLeft", KeyState_Up,
		this, &CPlayer::LeftLeverMoveEnd);

	CInput::GetInst()->SetCallback<CPlayer>("MoveDashLeft", KeyState_Up,
			this, &CPlayer::LeftDashMoveEnd);

	// Right 
	CInput::GetInst()->SetCallback<CPlayer>("MoveRight", KeyState_Push,
		this, &CPlayer::MoveRight);

	CInput::GetInst()->SetCallback<CPlayer>("MoveDashRight", KeyState_Push,
		this, &CPlayer::MoveDashRight);

	CInput::GetInst()->SetCallback<CPlayer>("MoveRight", KeyState_Up,
		this, &CPlayer::RightLeverMoveEnd);

	CInput::GetInst()->SetCallback<CPlayer>("MoveDashRight", KeyState_Up, //
		this, &CPlayer::RightDashMoveEnd);

	CInput::GetInst()->SetCallback<CPlayer>("Fire", KeyState_Down,
		this, &CPlayer::BulletFire);

	CInput::GetInst()->SetCallback<CPlayer>("Pause", KeyState_Down,
		this, &CPlayer::Pause);
	CInput::GetInst()->SetCallback<CPlayer>("Resume", KeyState_Down,
		this, &CPlayer::Resume);

	CInput::GetInst()->SetCallback<CPlayer>("Skill1", KeyState_Down,
		this, &CPlayer::Skill1);

	CInput::GetInst()->SetCallback<CPlayer>("Jump", KeyState_Down,
		this, &CPlayer::JumpKey);
	CInput::GetInst()->SetCallback<CPlayer>("JumpWhileDash", KeyState_Down,
		this, &CPlayer::JumpKey);
}

bool CPlayer::Init()
{
	if (!CCharacter::Init())
		return false;

	SetPivot(0.5f, 1.f);

	//SetTexture("Teemo", TEXT("teemo.bmp"));
	CreateAnimation();
	AddAnimation("LucidNunNaRightIdle");
	AddAnimation("LucidNunNaRightWalk", true, 0.6f);
	AddAnimation("LucidNunNaRightAttack", false, 0.5f);
	AddAnimation("LucidNunNaRightSkill1", false, 0.5f);

	AddAnimationNotify<CPlayer>("LucidNunNaRightAttack", 2, this, &CPlayer::Fire);
	SetAnimationEndNotify<CPlayer>("LucidNunNaRightAttack", this, &CPlayer::AttackEnd);

	AddAnimationNotify<CPlayer>("LucidNunNaRightSkill1", 2, this, &CPlayer::Skill1Enable);
	SetAnimationEndNotify<CPlayer>("LucidNunNaRightSkill1", this, &CPlayer::Skill1End);

	/*CColliderBox* Body = AddCollider<CColliderBox>("Body");
	Body->SetExtent(80.f, 75.f);
	Body->SetOffset(0.f, -37.5f);*/

	CColliderSphere* Head = AddCollider<CColliderSphere>("Head");
	//Head->SetExtent(40.f, 30.f);
	Head->SetRadius(20.f);
	Head->SetOffset(0.f, -60.f);
	Head->SetCollisionProfile("Player");

	CColliderBox* Body = AddCollider<CColliderBox>("Body");
	Body->SetExtent(80.f, 45.f);
	Body->SetOffset(0.f, -22.5f);
	Body->SetCollisionProfile("Player");
	Body->SetCollisionBeginFunction(this, &CPlayer::JumpDownAttack);

	m_HPBarWidget = CreateWidgetComponent("HPBarWidget");

	CProgressBar* HPBar = m_HPBarWidget->CreateWidget<CProgressBar>("HPBar");

	HPBar->SetTexture("WorldHPBar", TEXT("CharacterHPBar.bmp"));	

	m_HPBarWidget->SetPos(-25.f, -95.f);

	CWidgetComponent* NameWidget = CreateWidgetComponent("NameWidget");

	CUIText* NameText = NameWidget->CreateWidget<CUIText>("NameText");

	NameText->SetText(TEXT("루시드눈나"));
	NameText->SetTextColor(255, 0, 0);

	NameWidget->SetPos(-25.f, -115.f);

	m_CharacterInfo.HP = 1000;
	m_CharacterInfo.HPMax = 1000;

	// SetGravityAccel(30.f);
	// SetPhysicsSimulate(false);
	SetPhysicsSimulate(true);
	SetJumpVelocity(70.f);
	SetSideWallCheck(true);

	return true;
}

void CPlayer::Update(float DeltaTime)
{
	CCharacter::Update(DeltaTime);

	if (GetAsyncKeyState(VK_F1) & 0x8000)
		SetAttackSpeed(0.5f);

	if (m_Skill1Enable)
	{
		m_Skill1Time += DeltaTime;

		if (m_Skill1Time >= 3.f)
		{
			m_Skill1Enable = false;
			m_Skill1Time = 0.f;

			SetTimeScale(1.f);
			CGameManager::GetInst()->SetTimeScale(1.f);

			/*auto	iter = m_Skill1BulletList.begin();
			auto	iterEnd = m_Skill1BulletList.end();

			for (; iter != iterEnd; ++iter)
			{
				(*iter)->SetTimeScale(1.f);
			}

			m_Skill1BulletList.clear();*/
		}
	}

	if (CheckCurrentAnimation("LucidNunNaRightAttack"))
		SetOffset(0.f, 20.f);

	else
		SetOffset(0.f, 0.f);

	PlayerMoveUpdate(DeltaTime);
	// SwimMoveUpdate(DeltaTime);

	JumpDownDistUpdate(DeltaTime);

}

void CPlayer::PostUpdate(float DeltaTime)
{
	CCharacter::PostUpdate(DeltaTime);

	// 현재 애니메이션이 Walk 상태에서 속도가 0이 되었다면 이전까지 움직이다가
	// 지금 멈췄다는 것이다.
	if (CheckCurrentAnimation("LucidNunNaRightWalk") &&
		m_Velocity.Length() == 0.f)
	{
		ChangeAnimation("LucidNunNaRightIdle");
	}
}

void CPlayer::Collision(float DeltaTime)
{
	CCharacter::Collision(DeltaTime);
}

void CPlayer::Render(HDC hDC)
{
	CCharacter::Render(hDC);
}

CPlayer* CPlayer::Clone()
{
	return new CPlayer(*this);
}

float CPlayer::SetDamage(float Damage)
{
	Damage = CCharacter::SetDamage(Damage);

	CUICharacterStateHUD* State = m_Scene->FindUIWindow<CUICharacterStateHUD>("CharacterStateHUD");

	if (State)
		State->SetHPPercent(m_CharacterInfo.HP / (float)m_CharacterInfo.HPMax);

	CProgressBar* HPBar = (CProgressBar*)m_HPBarWidget->GetWidget();

	HPBar->SetPercent(m_CharacterInfo.HP / (float)m_CharacterInfo.HPMax);

	return Damage;
}


void CPlayer::MoveUp(float DeltaTime)
{
	//m_Pos.y -= 200.f * DeltaTime;
	// Move(Vector2(0.f, -1.f));
	// ChangeAnimation("LucidNunNaRightWalk");


	// 1) FlyAfter Jump
	FlyAfterJump(DeltaTime);

	// 2) 혹은 SwimMoveUp
	// SwimMoveUp(DeltaTime);
}

void CPlayer::MoveUpEnd(float DeltaTime)
{
	if (m_IsFlying)
	{
		// 땅에 닿은 상태라면 m_IsFlying을 False로 해서
		// 다시 Jump 이후 날게 세팅한다.
		if (m_IsGround)
			m_IsFlying = false;
	}
}

void CPlayer::MoveDown(float DeltaTime)
{
	Move(Vector2(0.f, 1.f));
	ChangeAnimation("LucidNunNaRightWalk");
}


void CPlayer::SwimMoveUp(float DeltaTime)
{
	//m_Pos.y -= 200.f * DeltaTime;
	
	m_IsSwimmingUp = true;;
	Move(Vector2(0.f, -1.f), m_SwimSpeed);
	ChangeAnimation("LucidNunNaRightWalk");
}

void CPlayer::SwimEnd(float DeltaTime)
{
	m_IsSwimmingUp = false;
}

void CPlayer::NoSwimGoDown(float DeltaTime)
{
	Move(Vector2(0.f, 1.f), m_NoSwimDownSpeed);
	ChangeAnimation("LucidNunNaRightWalk");
}

void CPlayer::SwimMoveUpdate(float DeltaTime)
{
	if (!m_IsSwimmingUp)
	{
		Move(Vector2(0.f, 1.f), m_NoSwimDownSpeed);
	}
}

void CPlayer::FlyAfterJump(float DeltaTime)
{

	// 일단 한 번 뛴 상태에서 날아야 한다.
	if (m_Jump)
	{
		m_IsFlying = true;

		// 중력 적용 방지 
		m_Jump = false;
		m_IsGround = true;
	}

	// 나는 중이라면 계속 날게 세팅한다.
	if (m_IsFlying)
	{
		Move(Vector2(0.f, -1.f),m_FlySpeed);
		ChangeAnimation("LucidNunNaRightWalk");

		// 중력 적용 방지 
		m_Jump = false;
		m_IsGround = true;
	}
}

void CPlayer::TriangleJumpLeft(float DeltaTime)
{
	m_Jump = true;
	m_IsGround = false;

	// 점프 시작 높이 세팅
	m_FallStartY = m_Pos.y;

	// FallTime 초기화 
	m_FallTime = 0.f;

	// 점프에 적용되는 누적가속도 시간도 세팅해둔다.
	m_JumpAccelAccTime = 0.f;

	m_TriangleJump = true;

	m_LeftMove = true;
	m_RightMove = false;

	m_ToRightWhenLeftMove = false;
	m_ToLeftWhenRightMove = false;
}

void CPlayer::TriangleJumpRight(float DeltaTime)
{
	m_Jump = true;
	m_IsGround = false;

	// 점프 시작 높이 세팅
	m_FallStartY = m_Pos.y;

	// FallTime 초기화 
	m_FallTime = 0.f;

	// 점프에 적용되는 누적가속도 시간도 세팅해둔다.
	m_JumpAccelAccTime = 0.f;

	m_TriangleJump = true;

	m_LeftMove = false;
	m_RightMove = true;

	m_ToRightWhenLeftMove = false;
	m_ToLeftWhenRightMove = false;
}

void CPlayer::SetObjectLand()
{
	CCharacter::SetObjectLand();

	m_TriangleJump = false;
}

void CPlayer::JumpDown(float DeltaTime)
{
	// 공중에서는 X
	if (m_IsGround)
	{
		// 떨어지게 하기 
		m_IsGround = false;

		m_FallTime = 0.f;
		m_FallStartY = m_Pos.y;

		// Down Jump 중임 표시
		m_JumpDown = true;
	}

}

void CPlayer::JumpDownDistUpdate(float DeltaTime)
{
	if (m_JumpDown)
	{
		m_JumpDownDist = m_Pos.y - m_FallStartY;

		if (m_JumpDownDist > m_Size.y)
			m_JumpDown = false;
	}
}

void CPlayer::JumpDownAttack(CCollider* Src, CCollider* Dest, float DeltaTime)
{
	// 낙하중 
	if (m_Pos.y > m_PrevPos.y && !m_IsGround)
	{
		try {
			CMonster* Owner = dynamic_cast<CMonster*>(Dest->GetOwner());

			if (Owner) // Good Cast
			{
 				CEffectHit* Hit = m_Scene->CreateObject<CEffectHit>("HitEffect", "HitEffect",
					m_Pos, Vector2(178.f, 164.f));

				m_Scene->GetSceneResource()->SoundPlay("Fire");

				CDamageFont* DamageFont = m_Scene->CreateObject<CDamageFont>("DamageFont",
					m_Pos);

				// 해당 방향으로 튕겨나가게 하기
				float DirX = m_Pos.x - m_PrevPos.x;

				// 왼쪽으로 가고 있었음
				if (DirX <= 0)
				{
					Owner->SetHitDir(Vector2(-1.f, 0.f));
				}
				else
				{
					Owner->SetHitDir(Vector2(1.f, 0.f));
				}
			}
			else // bad_cast
			{
				CMonster* Owner = (CMonster*)Dest->GetOwner();
			}
		}
		catch (std::bad_cast b) {
		}
		
	}
}

bool CPlayer::CheckBottomCollision()
{
	if (m_JumpDown)
		return false;

	bool BottomCollision = CCharacter::CheckBottomCollision();

	if (BottomCollision)
	{
		// m_TriangleJump = false;
		// m_IsFlying = false;
	}

	return true;
}


void CPlayer::BulletFire(float DeltaTime)
{
	ChangeAnimation("LucidNunNaRightAttack");

	/*if (m_Skill1Enable)
	{
		Bullet->SetTimeScale(0.f);
		m_Skill1BulletList.push_back(Bullet);
	}*/
}

void CPlayer::Pause(float DeltaTime)
{
	CGameManager::GetInst()->SetTimeScale(0.f);
}

void CPlayer::Resume(float DeltaTime)
{
	CGameManager::GetInst()->SetTimeScale(1.f);
}

void CPlayer::Skill1(float DeltaTime)
{
	ChangeAnimation("LucidNunNaRightSkill1");
}

void CPlayer::JumpKey(float DeltaTime)
{
	// 삼각 충돌 적용하기 
	bool SideCollision = false;

	CTileMap* TileMap = m_Scene->GetTileMap();

	Vector2 LT  = m_Pos - m_Pivot * m_Size;
	Vector2 RB = LT + m_Size;

	// 오른쪽이면, 오른쪽 한칸 타일
	if (m_RightMove)
	{
		int IndexX         = TileMap->GetOriginTileIndexX(RB.x);
		int TopIndexY         = TileMap->GetOriginTileIndexY(LT.y);
		int BottomIndexY         = TileMap->GetOriginTileIndexY(RB.y);
		int IndexF         = -1;

		int IndexXRight = IndexX + 1;
		int IndexXLeft = IndexX - 1;

		if (IndexX > TileMap->GetTileCountX() - 1)
			IndexX = TileMap->GetTileCountX() - 1;
		if (IndexXRight > TileMap->GetTileCountX() - 1)
			IndexXRight = TileMap->GetTileCountX() - 1;
		if (IndexXLeft < 0)
			IndexXLeft = 0;

		for (int y = TopIndexY; y <= BottomIndexY; y++)
		{
			// 오른쪽 2개의 타일을 조사한다.
			for (int x = IndexX; x <= IndexXRight; x++)
			{
				IndexF = y * TileMap->GetTileCountX() + x;

				if (TileMap->GetTile(IndexF)->GetTileOption() !=
					ETileOption::Wall)
					continue;

				//if (!TileMap->GetTile(IndexF)->GetSideCollision())
				//	continue;

				Vector2	TilePos = TileMap->GetTile(IndexF)->GetPos();
				Vector2	TileSize = TileMap->GetTile(IndexF)->GetSize();

				// 화면에 딱 붙어있을 때는 무시한다.
				if (TilePos.y - 0.001f <= RB.y && RB.y <= TilePos.y + 0.001f)
					continue;

				// 현재 위치 Tile 도 Wall 이고, 오른쪽도 Wall 이라면,
				// 그것은, 삼각 점프가 아니게 된다. ( 그냥 Tile이 연속적으로 있는 곳에 대해서는 삼각점프 X)
				if (x == IndexX)
				{
					if (TileMap->GetTile(y * TileMap->GetTileCountX() + IndexXLeft)->GetTileOption() ==
						ETileOption::Wall)
						continue;
				}

				if (x == IndexXRight)
				{
					if (TileMap->GetTile(y * TileMap->GetTileCountX() + IndexX)->GetTileOption() ==
						ETileOption::Wall)
						continue;
				}

				SideCollision = true;
				break;
			}
			if (SideCollision)
				break;
		}
	}

	// 왼쪽 이면, 왼쪽 한칸 타일
	else if (m_LeftMove)
	{
		int IndexX = TileMap->GetOriginTileIndexX(LT.x);
		int TopIndexY = TileMap->GetOriginTileIndexY(LT.y);
		int BottomIndexY = TileMap->GetOriginTileIndexY(RB.y);
		int IndexF = -1;

		int IndexXLeft = IndexX - 1;
		int IndexXRight = IndexX +1;

		if (IndexX < 0)
			IndexX = 0;
		if (IndexXLeft < 0)
			IndexXLeft = 0;
		if (IndexXRight >= TileMap->GetTileCountX() - 1)
			IndexXRight = TileMap->GetTileCountX() - 1;

		// 오른쪽 2개의 타일을 조사한다.
		for (int y = TopIndexY; y <= BottomIndexY; y++)
		{
			for (int x = IndexX; x >= IndexXLeft; x--)
			{
				IndexF = y * TileMap->GetTileCountX() + x;

				if (TileMap->GetTile(IndexF)->GetTileOption() !=
					ETileOption::Wall)
					continue;

				//if (!TileMap->GetTile(IndexF)->GetSideCollision())
				//	continue;

				Vector2	TilePos  = TileMap->GetTile(IndexF)->GetPos();
				Vector2	TileSize = TileMap->GetTile(IndexF)->GetSize();

				// 화면에 딱 붙어있을 때는 무시한다.
				if (TilePos.y - 0.001f <= RB.y && RB.y <= TilePos.y + 0.001f)
					continue;

				// 현재 위치 Tile 도 Wall 이고, 왼쪽도 Wall 이라면,
				// 그것은, 삼각 점프가 아니게 된다. ( 그냥 Tile이 연속적으로 있는 곳에 대해서는 삼각점프 X)
				if (x == IndexX)
				{
					if (TileMap->GetTile(y * TileMap->GetTileCountX() + IndexXRight)->GetTileOption() ==
						ETileOption::Wall)
						continue;
				}

				if (x == IndexXLeft)
				{
					if (TileMap->GetTile(y * TileMap->GetTileCountX() + IndexX)->GetTileOption() ==
						ETileOption::Wall)
						continue;
				}

				SideCollision = true;
				break;
			}
			if (SideCollision)
				break;
		}
	}

	// SideCollision이 있었다면 단순 Jump 한다.
	if (m_IsGround)
	{
		Jump();
	}
	// 그게 아니라면 삼각 점프를 한다.
	else if (SideCollision)
	{
		// 현재 움직이는 방향 반대로 움직여야 한다.
		// 오른쪽
		if (m_RightMove)
		{
			TriangleJumpLeft(DeltaTime);
		}
		// 왼쪽
		else if (m_LeftMove)
		{
			TriangleJumpRight(DeltaTime);
		}
	}
}

float CPlayer::CalculateLeverMoveSpeed(float DeltaTime)
{
	// 점프 중이어도 적용 
	// if (!m_IsGround)
	//	return m_LeverVelocity;
		
	// 오른쪽 이동 중이라면
	if (m_RightMove)
	{
		// 계속 오른쪽 이동 중이라면
		if (!m_ToLeftWhenRightMove)
		{
			m_LeverVelocity += m_LeverMoveAccel;
		}
		// 그게 아니라, 이제 왼쪽 이동 중이라면
		else
		{
			m_LeverVelocity -= m_LeverMoveAccel * 2.f;
		}
	}

	// 만약 왼쪽 이동 중이라면 
	else if (m_LeftMove)
	{
		// 계속 왼쪽 이동 중이라면
		if (!m_ToRightWhenLeftMove)
		{
			m_LeverVelocity += m_LeverMoveAccel;
		}
		// 그게 아니라, 이제 오른쪽 이동 중이라면
		else
		{
			m_LeverVelocity -= m_LeverMoveAccel * 2.f;
		}
	}

	// 최대 치 조절
	if (m_LeverVelocity >= m_LeverMaxMoveVelocity)
	{
		m_LeverVelocity = m_LeverMaxMoveVelocity;
	}

	if (m_LeverVelocity < 0.f)
		m_LeverVelocity = 0.f;
	
	return m_LeverVelocity;

}

float CPlayer::CalculateButtonMoveSpeed(float DeltaTime)
{
	if (!m_IsGround)
		return m_LeverVelocity;

	// 오른쪽 이동 중이라면
	if (m_RightMove)
	{
		// 계속 오른쪽 이동 중이라면
		if (!m_ToLeftWhenRightMove)
		{
			m_ButtonVelocity += m_ButtonMoveAccel;
		}
		// 그게 아니라, 이제 왼쪽 이동 중이라면
		else
		{
			m_ButtonVelocity -= m_ButtonMoveAccel * 2.f;
		}
	}

	// 만약 왼쪽 이동 중이라면 
	else if (m_LeftMove)
	{
		// 계속 왼쪽 이동 중이라면
		if (!m_ToRightWhenLeftMove)
		{
			m_ButtonVelocity += m_ButtonMoveAccel;
		}
		// 그게 아니라, 이제 오른쪽 이동 중이라면
		else
		{
			m_ButtonVelocity -= m_ButtonMoveAccel * 2.f;
		}
	}

	// 최대 치 조절
	if (m_ButtonVelocity >= m_ButtonMaxMoveVelocity)
	{
		m_ButtonVelocity = m_ButtonMaxMoveVelocity;
	}

	if (m_ButtonVelocity < 0.f)
		m_ButtonVelocity = 0.f;

	return m_ButtonVelocity;
}

float CPlayer::CalculateTotalMoveSpeed()
{

	m_MoveVelocity = m_LeverVelocity + m_ButtonVelocity;

	if (m_MoveVelocity > m_MoveMaxVelocity)
		m_MoveVelocity = m_MoveMaxVelocity;


	// 최소 치 조절
	if (m_MoveVelocity <= 0.1f)
	{
		if (m_RightMove)
		{
			m_RightMove = false;
			// m_ToLeftWhenRightMove = false;
			m_ToRightWhenLeftMove = false;

		}
		if (m_LeftMove)
		{
			m_LeftMove = false;
			m_ToLeftWhenRightMove = false;
		}

		m_MoveVelocity = 0.f;
		m_LeverVelocity = 0.f;
		m_ButtonVelocity = 0.f;
	}

	return m_MoveVelocity;
}


void CPlayer::MoveLeft(float DeltaTime)
{
	if (m_TriangleJump)
	{
		if (m_RightMove)
			Move(Vector2(1.f, 0.f), m_MoveVelocity);
		else
			Move(Vector2(-1.f, 0.f), m_MoveVelocity);
		return;
	}

	// 오른쪽 버튼을 누르고 있다면 X
	if (m_RightMovePush)
	{
		// 그냥 계속 오른쪽으로 이동
		return ;;
	}

	// 이동 중임을 true로 --> 이동 안할 때 자동 감속을 시키기 위함이다.
	m_IsLeverMoving = true;

	// 왼쪽 버튼을 누르고 있다는 것을 표시한다.aa
	m_LeftMovePush = true;

	// 그런데 현재 오른쪽으로 이동 중이었다면
	// 오른쪽으로 가되, 감속을 해야 한다.d aa
	if (m_RightMove)
	{
		m_LeftMove = false;
		m_ToLeftWhenRightMove = true;
	}
	else
	{
		m_LeftMove = true;
		m_ToLeftWhenRightMove = false;
	}

	CalculateLeverMoveSpeed(DeltaTime);

	// 만약 Dash를 누르고 있지 않다면
	if (m_ButtonVelocity > 0.f)
	{
		m_ButtonVelocity -= m_ButtonMoveAccel * 2.f;
		if (m_ButtonVelocity <= 0.f)
			m_ButtonVelocity = 0.f;
	}

	CalculateTotalMoveSpeed();

	if (m_RightMove)
		Move(Vector2(1.f, 0.f), m_MoveVelocity);
	else
		Move(Vector2(-1.f, 0.f), m_MoveVelocity);
		

	ChangeAnimation("LucidNunNaRightWalk");
}

void CPlayer::MoveDashLeft(float DeltaTime)
{
	if (m_TriangleJump)
	{
		if (m_RightMove)
			Move(Vector2(1.f, 0.f), m_MoveVelocity);
		else
			Move(Vector2(-1.f, 0.f), m_MoveVelocity);
		return;
	}
	// 오른쪽 버튼을 누르고 있다면 X
	if (m_RightMovePush)
	{
		// 그냥 계속 오른쪽으로 이동
		return;;
	}

	// 이동 중임을 true로 --> 이동 안할 때 자동 감속을 시키기 위함이다.
	m_IsLeverMoving = true;
	m_IsButtonMoving = true;

	// 왼쪽 버튼을 누르고 있다는 것을 표시한다.aa
	m_LeftMovePush = true;

	// 그런데 현재 오른쪽으로 이동 중이었다면
	// 오른쪽으로 가되, 감속을 해야 한다.d aa
	if (m_RightMove)
	{
		m_LeftMove = false;
		m_ToLeftWhenRightMove = true;
	}
	else
	{
		m_LeftMove = true;
		m_ToLeftWhenRightMove = false;
	}

	CalculateLeverMoveSpeed(DeltaTime);

	CalculateButtonMoveSpeed(DeltaTime);

	CalculateTotalMoveSpeed();

	if (m_RightMove)
		Move(Vector2(1.f, 0.f), m_MoveVelocity);
	else
		Move(Vector2(-1.f, 0.f), m_MoveVelocity);

	ChangeAnimation("LucidNunNaRightIdle");
}

void CPlayer::MoveDashRight(float DeltaTime)
{
	if (m_TriangleJump)
	{
		if (m_RightMove)
			Move(Vector2(1.f, 0.f), m_MoveVelocity);
		else
			Move(Vector2(-1.f, 0.f), m_MoveVelocity);
		return;
	}

	// 왼쪽 버튼을 누르고 있다면 X
	if (m_LeftMovePush)
	{
		// 계속 왼쪽 이동
		return;
	}

	// 이동 중임을 true로 --> 이동 안할 때 자동 감속을 시키기 위함이다.
	m_IsLeverMoving = true;
	m_IsButtonMoving = true;

	// 오른쪽 이동 중임을 표시한다.
	m_RightMovePush = true;

	// 왼쪽으로 이미 이동중인 상황이라면
	if (m_LeftMove)
	{
		m_RightMove = false;
		m_ToRightWhenLeftMove = true;
	}
	else // 오른쪽 이동중 표시 
	{
		m_RightMove = true;
		m_ToRightWhenLeftMove = false;
	}

	CalculateLeverMoveSpeed(DeltaTime);

	CalculateButtonMoveSpeed(DeltaTime);

	CalculateTotalMoveSpeed();

	if (m_RightMove)
		Move(Vector2(1.f, 0.f), m_MoveVelocity);
	else
		Move(Vector2(-1.f, 0.f), m_MoveVelocity);

	ChangeAnimation("LucidNunNaRightIdle");
}

void CPlayer::RightLeverMoveEnd(float DeltaTime)
{
	m_RightMovePush = false;
	ResetMoveInfo();
}

void CPlayer::RightDashMoveEnd(float DeltaTime)
{
	m_RightMovePush = false;
	ResetMoveInfo();
}

void CPlayer::LeftDashMoveEnd(float DeltaTime)
{
	m_LeftMovePush = false;
	ResetMoveInfo();
}

void CPlayer::PlayerMoveUpdate(float DeltaTime)
{
	// 레버 움직임 --> 현재 움직이지 않고 있는 상황이라면 --> 자연 감속을 시켜줘야 한다.
	// 최대 움직임 제한
	if (m_MoveVelocity >= m_MoveMaxVelocity)
		m_MoveVelocity = m_MoveMaxVelocity;

	if (!m_IsLeverMoving)
	{
		// 감속 d
		m_LeverVelocity -= m_LeverMoveAccel * 0.5f;

		if (m_LeverVelocity < 0.f)
		{
			m_LeverVelocity = 0.f;
		}
	}

	if (!m_IsButtonMoving)
	{
		// 감속 d
		m_ButtonVelocity -= m_ButtonMoveAccel * 0.5f;

		if (m_ButtonVelocity < 0.f)
		{
			m_ButtonVelocity = 0.f;
		}
	}

	if (!m_IsLeverMoving && !m_IsButtonMoving && m_MoveVelocity > 0.f)
	{
		// 감속 d
		m_MoveVelocity = m_LeverVelocity + m_ButtonVelocity;

		// 이동을 멈추면, 오른쪽, 왼쪽 이동 표시를 취소해준다.
		if (m_MoveVelocity <= 0.1f)
		{
			if (m_RightMove)
			{
				m_RightMove = false;
				// m_ToLeftWhenRightMove = false;
				m_ToRightWhenLeftMove = false;

			}
			if (m_LeftMove)
			{
				m_LeftMove = false;
				m_ToLeftWhenRightMove = false;
			}

			m_MoveVelocity = 0.f;
		}

		// 감속 중임에도 이동은 시켜준다.
		if (m_RightMove)
			Move(Vector2(1.f, 0.f), m_MoveVelocity);
		if (m_LeftMove)
			Move(Vector2(-1.f, 0.f), m_MoveVelocity);

	}
}

void CPlayer::LeftLeverMoveEnd(float DeltaTime)
{
	m_LeftMovePush = false;
	ResetMoveInfo();
}


void CPlayer::MoveRight(float DeltaTime)
{
	if (m_TriangleJump)
	{
		if (m_RightMove)
			Move(Vector2(1.f, 0.f), m_MoveVelocity);
		else
			Move(Vector2(-1.f, 0.f), m_MoveVelocity);
		return;
	}

	// 왼쪽 버튼을 누르고 있다면 X
	if (m_LeftMovePush)
	{
		// 계속 왼쪽 이동
		return;
	}

	// 이동 중임을 true로 --> 이동 안할 때 자동 감속을 시키기 위함이다.
	m_IsLeverMoving = true;

	// 오른쪽 이동 중임을 표시한다.
	m_RightMovePush = true;

	// 왼쪽으로 이미 이동중인 상황이라면
	if (m_LeftMove)
	{
		m_RightMove = false;
		m_ToRightWhenLeftMove = true;
	}
	else // 오른쪽 이동중 표시 
	{
		m_RightMove = true;
		m_ToRightWhenLeftMove = false;
	}

	CalculateLeverMoveSpeed(DeltaTime);

	// 만약 Dash를 누르고 있지 않다면
	if (m_ButtonVelocity > 0.f)
	{
		m_ButtonVelocity -= m_ButtonMoveAccel * 2.f;
		if (m_ButtonVelocity <= 0.f)
			m_ButtonVelocity = 0.f;
	}

	CalculateTotalMoveSpeed();

	if (m_RightMove)
		Move(Vector2(1.f, 0.f), m_MoveVelocity);
	else
		Move(Vector2(-1.f, 0.f), m_MoveVelocity);

	ChangeAnimation("LucidNunNaRightWalk");

}

void CPlayer::ResetMoveInfo()
{
	m_IsButtonMoving = false;
	m_IsLeverMoving = false;
	m_ToRightWhenLeftMove = false;
	m_ToLeftWhenRightMove = false;
}

void CPlayer::AttackEnd()
{
	ChangeAnimation("LucidNunNaRightIdle");
}

void CPlayer::Fire()
{
	CSharedPtr<CBullet> Bullet = m_Scene->CreateObject<CBullet>("Bullet",
		"PlayerBullet",
		Vector2(m_Pos + Vector2(75.f, 0.f)),
		Vector2(50.f, 50.f));
}

void CPlayer::Skill1End()
{
	ChangeAnimation("LucidNunNaRightIdle");
}

void CPlayer::Skill1Enable()
{
	CGameManager::GetInst()->SetTimeScale(0.01f);
	SetTimeScale(100.f);
	m_Skill1Enable = true;
	//m_Skill1Time = 0.f;
}
