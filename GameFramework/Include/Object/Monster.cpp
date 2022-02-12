
#include "Monster.h"
#include "Bullet.h"
#include "../Scene/Scene.h"
#include "../Collision/ColliderBox.h"

CMonster::CMonster()	:
	m_FireTime(0.f),
	m_FireTimeMax(1.f),
	m_AI(EMonsterAI::Idle),
	m_DetectDistance(500.f),
	m_AttackDistance(200.f),
	m_AttackEnable(false),
	m_Skill1Enable(false)
{
	m_Dir.x = 0.f;
	m_Dir.y = 1.f;
}

CMonster::CMonster(const CMonster& obj) :
	CCharacter(obj)
{
	m_Dir = obj.m_Dir;
	m_FireTime = obj.m_FireTime;
	m_FireTimeMax = obj.m_FireTimeMax;
	m_Count = obj.m_Count;
}

CMonster::~CMonster()
{
}

void CMonster::Start()
{
	CCharacter::Start();
}

bool CMonster::Init()
{
	if (!CCharacter::Init())
		return false;

	SetPivot(0.5f, 1.f);

	CreateAnimation();
	AddAnimation("LucidNunNaLeftIdle");
	AddAnimation("LucidNunNaLeftWalk", true, 0.6f);
	AddAnimation("LucidNunNaLeftAttack", true, 0.5f);

	AddAnimationNotify<CMonster>("LucidNunNaLeftAttack", 2, this, &CMonster::Fire);
	SetAnimationEndNotify<CMonster>("LucidNunNaLeftAttack", this, &CMonster::AttackEnd);

	CColliderBox* Body = AddCollider<CColliderBox>("Body");
	Body->SetExtent(82.f, 73.f);
	Body->SetOffset(0.f, -36.5f);
	Body->SetCollisionProfile("Monster");

	m_CharacterInfo.HP = 1000;
	m_CharacterInfo.HPMax = 1000;

	m_MoveSpeed = 100.f;

	return true;
}

void CMonster::Update(float DeltaTime)
{
	CCharacter::Update(DeltaTime);

	CGameObject* Player = m_Scene->GetPlayer();

	// 주변에 플레이어가 있는지 판단한다.
	// 플레이어와의 거리를 구한다.
	float	Dist = Distance(Player->GetPos(), m_Pos);

	if (Dist <= m_DetectDistance)
	{
		if (Dist <= m_AttackDistance)
		{
			m_AI = EMonsterAI::Attack;
			m_AttackEnable = true;
		}

		else
		{
			if (!m_AttackEnable)
				m_AI = EMonsterAI::Trace;
		}
	}

	else
	{
		m_AI = EMonsterAI::Idle;
	}

	if (m_CharacterInfo.HP <= 0)
	{
		m_AI = EMonsterAI::Death;
	}

	else if (m_CharacterInfo.HP / (float)m_CharacterInfo.HPMax < 0.5f &&
		!m_Skill1Enable)
	{
		m_Skill1Enable = true;
		m_AI = EMonsterAI::Skill1;
	}

	//m_Pos += m_Dir * 300.f * DeltaTime;

	//if (m_Pos.y >= 720.f)
	//{
	//	m_Pos.y = 720.f;
	//	m_Dir.y = -1.f;
	//}

	//else if (m_Pos.y - m_Size.y <= 0.f)
	//{
	//	m_Pos.y = m_Size.y;
	//	m_Dir.y = 1.f;
	//}

	///*
	//몬스터가 일정 시간마다 한번씩 총알을 왼쪽으로 발사하게 만들어보세요.
	//*/
	//m_FireTime += DeltaTime;

	//if (m_FireTime >= m_FireTimeMax)
	//{
	//	m_FireTime -= m_FireTimeMax;

	//	++m_Count;

	//	CSharedPtr<CBullet> Bullet = m_Scene->CreateObject<CBullet>("Bullet",
	//		"MonsterBullet", Vector2(m_Pos - Vector2(m_Size.x / 2.f + 25.f, m_Size.y / 2.f)),
	//		Vector2(50.f, 50.f));

	//	if (m_Count % 3 != 0)
	//		Bullet->SetDir(-1.f, 0.f);

	//	else
	//	{
	//		//CGameObject* Player = m_Scene->FindObject("Player");
	//		CGameObject* Player = m_Scene->GetPlayer();

	//		float	Angle = GetAngle(Bullet->GetPos(), Player->GetPos());
	//		Bullet->SetDir(Angle);
	//	}
	//}



	switch (m_AI)
	{
	case EMonsterAI::Idle:
		AIIdle(DeltaTime);
		break;
	case EMonsterAI::Trace:
		AITrace(DeltaTime);
		break;
	case EMonsterAI::Attack:
		AIAttack(DeltaTime);
		break;
	case EMonsterAI::Death:
		AIDeath(DeltaTime);
		break;
	case EMonsterAI::Skill1:
		MessageBox(0, TEXT("스킬이 너무 강력하다."), TEXT("뻥이지롱"), MB_OK);
		break;
	}
}

void CMonster::PostUpdate(float DeltaTime)
{
	CCharacter::PostUpdate(DeltaTime);
}

void CMonster::Collision(float DeltaTime)
{
	CCharacter::Collision(DeltaTime);
}

void CMonster::Render(HDC hDC)
{
	CCharacter::Render(hDC);
}

CMonster* CMonster::Clone()
{
	return new CMonster(*this);
}

float CMonster::SetDamage(float Damage)
{
	Damage = CCharacter::SetDamage(Damage);

	return Damage;
}

void CMonster::AIIdle(float DeltaTime)
{
	ChangeAnimation("LucidNunNaLeftIdle");
}

void CMonster::AITrace(float DeltaTime)
{
	ChangeAnimation("LucidNunNaLeftWalk");

	CGameObject* Player = m_Scene->GetPlayer();

	Vector2	Dir = Player->GetPos() - m_Pos;
	Dir.Normalize();

	Move(Dir, m_MoveSpeed);
}

void CMonster::AIAttack(float DeltaTime)
{
	ChangeAnimation("LucidNunNaLeftAttack");
}

void CMonster::AIDeath(float DeltaTime)
{
	ChangeAnimation("LucidNunNaLeftIdle");
}

void CMonster::AttackEnd()
{
	m_AttackEnable = false;
}

void CMonster::Fire()
{
	CGameObject* Player = m_Scene->GetPlayer();

	CSharedPtr<CBullet> Bullet = m_Scene->CreateObject<CBullet>("Bullet",
		"MonsterBullet", Vector2(m_Pos - Vector2(m_Size.x / 2.f + 25.f, m_Size.y / 2.f)),
		Vector2(50.f, 50.f));

	float	Angle = GetAngle(Bullet->GetPos(), Player->GetPos());
	Bullet->SetDir(Angle);
}
