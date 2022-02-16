
#include "GameObject.h"
#include "../GameManager.h"
#include "../Scene/Scene.h"
#include "../Scene/SceneResource.h"
#include "../Scene/SceneCollision.h"
#include "../Resource/AnimationSequence.h"
#include "../Scene/Camera.h"
#include "../Map/TileMap.h"

CGameObject::CGameObject()	:
	m_Scene(nullptr),
	m_MoveSpeed(200.f),
	m_TimeScale(1.f),
	m_Animation(nullptr),
	m_CameraCull(false),
	m_Start(false),
	m_DamageEnable(true),
	m_ObjType(EObject_Type::GameObject),
	m_PhysicsSimulate(true),
	m_IsGround(true),
	m_FallTime(0.f),
	m_FallStartY(0.f),
	m_Jump(false),
	m_JumpVelocity(0.f),
	m_FallVelocity(0.f),
	m_FallVelocityMax(1000.f),
	m_GravityAccel(10.f),
	m_LifeTime(0.f),
	m_FloorCheck(false),
	m_SideWallCheck(false)
{
	m_PrevPos.y = -1.f;

	// Jump
	m_JumpAccel = 1.1f;
	m_JumpAccelAccTime = 0.f;
}

CGameObject::CGameObject(const CGameObject& obj)	:
	CRef(obj)
{
	m_PrevPos.y = -1.f;
	m_FloorCheck = obj.m_FloorCheck;
	m_GravityAccel = obj.m_GravityAccel;
	m_PhysicsSimulate = obj.m_PhysicsSimulate;
	m_IsGround = obj.m_IsGround;
	m_FallTime = obj.m_FallTime;
	m_FallStartY = obj.m_FallStartY;
	m_Jump = obj.m_Jump;
	m_JumpVelocity = obj.m_JumpVelocity;
	m_DamageEnable = obj.m_DamageEnable;
	m_ObjType = obj.m_ObjType;

	m_Start = false;

	m_Scene = obj.m_Scene;

	if (obj.m_Animation)
		m_Animation = obj.m_Animation->Clone();

	m_Animation->m_Owner = this;

	m_ColliderList.clear();

	{
		auto	iter = obj.m_ColliderList.begin();
		auto	iterEnd = obj.m_ColliderList.end();

		for (; iter != iterEnd; ++iter)
		{
			CCollider* Collider = (*iter)->Clone();

			Collider->SetOwner(this);
			Collider->SetScene(m_Scene);

			m_ColliderList.push_back(Collider);
		}
	}
	
	m_WidgetComponentList.clear();

	{
		auto	iter = obj.m_WidgetComponentList.begin();
		auto	iterEnd = obj.m_WidgetComponentList.end();

		for (; iter != iterEnd; ++iter)
		{
			CWidgetComponent* Widget = (*iter)->Clone();
			Widget->SetOwner(this);
			Widget->SetScene(m_Scene);

			m_WidgetComponentList.push_back(Widget);
		}
	}

	m_PrevPos = obj.m_PrevPos;
	m_Pos = obj.m_Pos;
	m_Size = obj.m_Size;
	m_Pivot = obj.m_Pivot;
	m_Velocity = obj.m_Velocity;
	m_Offset = obj.m_Offset;
	m_MoveSpeed = obj.m_MoveSpeed;
	m_TimeScale = obj.m_TimeScale;
	m_Texture = obj.m_Texture;
	m_ImageStart = obj.m_ImageStart;
}

CGameObject::~CGameObject()
{
	SAFE_DELETE(m_Animation);

	auto	iter = m_ColliderList.begin();
	auto	iterEnd = m_ColliderList.end();

	for (; iter != iterEnd; ++iter)
	{
		(*iter)->ClearCollisionList();
	}
}

CCollider* CGameObject::FindCollider(const std::string& Name)
{
	auto	iter = m_ColliderList.begin();
	auto	iterEnd = m_ColliderList.end();

	for (; iter != iterEnd; ++iter)
	{
		if ((*iter)->GetName() == Name)
			return *iter;
	}

	return nullptr;
}

void CGameObject::CreateAnimation()
{
	if (!m_Animation)
	{
		m_Animation = new CAnimation;

		m_Animation->m_Owner = this;
		m_Animation->m_Scene = m_Scene;
	}
}

void CGameObject::AddAnimation(const std::string& SequenceName,
	bool Loop, float PlayTime, float PlayScale, bool Reverse)
{
	if (!m_Animation)
	{
		m_Animation = new CAnimation;

		m_Animation->m_Owner = this;
		m_Animation->m_Scene = m_Scene;
	}

	m_Animation->AddAnimation(SequenceName, Loop, PlayTime, PlayScale, Reverse);
}

void CGameObject::SetAnimationPlayTime(const std::string& Name, 
	float PlayTime)
{
	if (!m_Animation)
		return;

	m_Animation->SetPlayTime(Name, PlayTime);
}

void CGameObject::SetAnimationPlayScale(const std::string& Name, 
	float PlayScale)
{
	if (!m_Animation)
		return;

	m_Animation->SetPlayScale(Name, PlayScale);
}

void CGameObject::SetCurrentAnimation(const std::string& Name)
{
	if (!m_Animation)
		return;

	m_Animation->SetCurrentAnimation(Name);
}

void CGameObject::ChangeAnimation(const std::string& Name)
{
	if (!m_Animation)
		return;

	m_Animation->ChangeAnimation(Name);
}

bool CGameObject::CheckCurrentAnimation(const std::string& Name)
{
	return m_Animation->CheckCurrentAnimation(Name);
}

void CGameObject::SetAnimationReverse(const std::string& Name, bool Reverse)
{
	if (!m_Animation)
		return;

	m_Animation->SetReverse(Name, Reverse);
}

void CGameObject::SetAnimationLoop(const std::string& Name, bool Loop)
{
	if (!m_Animation)
		return;

	m_Animation->SetLoop(Name, Loop);
}

float CGameObject::GetBottom() const
{
	Vector2	Size = m_Size;

	if (m_Animation)
	{
		AnimationInfo* AnimInfo = m_Animation->m_CurrentAnimation;

		const AnimationFrameData& FrameData = AnimInfo->Sequence->GetFrameData(AnimInfo->Frame);

		Size = FrameData.Size;
	}

	return m_Pos.y + (1.f - m_Pivot.y) * Size.y + m_Offset.y;
}

void CGameObject::SetScene(CScene* Scene)
{
	m_Scene = Scene;

	if (m_Animation)
		m_Animation->m_Scene = Scene;
}

void CGameObject::Move(const Vector2& Dir)
{
	Vector2	CurrentMove = Dir * m_MoveSpeed * CGameManager::GetInst()->GetDeltaTime() * m_TimeScale;
	m_Velocity	+= CurrentMove;
	m_Pos += CurrentMove;
}

void CGameObject::Move(const Vector2& Dir, float Speed)
{
	Vector2	CurrentMove = Dir * Speed * CGameManager::GetInst()->GetDeltaTime() * m_TimeScale;
	m_Velocity += CurrentMove;
	m_Pos += CurrentMove;
}

void CGameObject::SetTexture(const std::string& Name)
{
	m_Texture = m_Scene->GetSceneResource()->FindTexture(Name);

	if (m_Texture)
	{
		m_Size.x = (float)m_Texture->GetWidth();
		m_Size.y = (float)m_Texture->GetHeight();
	}
}

void CGameObject::SetTexture(const std::string& Name, const TCHAR* FileName, const std::string& PathName)
{
	m_Scene->GetSceneResource()->LoadTexture(Name, FileName, PathName);

	m_Texture = m_Scene->GetSceneResource()->FindTexture(Name);

	if (m_Texture)
	{
		m_Size.x = (float)m_Texture->GetWidth();
		m_Size.y = (float)m_Texture->GetHeight();
	}
}

void CGameObject::SetTextureFullPath(const std::string& Name, const TCHAR* FullPath)
{
	m_Scene->GetSceneResource()->LoadTextureFullPath(Name, FullPath);

	m_Texture = m_Scene->GetSceneResource()->FindTexture(Name);

	if (m_Texture)
	{
		m_Size.x = (float)m_Texture->GetWidth();
		m_Size.y = (float)m_Texture->GetHeight();
	}
}

void CGameObject::SetTexture(const std::string& Name,
	const std::vector<std::wstring>& vecFileName, 
	const std::string& PathName)
{
	m_Scene->GetSceneResource()->LoadTexture(Name, vecFileName, PathName);

	m_Texture = m_Scene->GetSceneResource()->FindTexture(Name);

	if (m_Texture)
	{
		m_Size.x = (float)m_Texture->GetWidth();
		m_Size.y = (float)m_Texture->GetHeight();
	}
}

void CGameObject::SetTextureColorKey(unsigned char r, unsigned char g, unsigned char b, int Index)
{
	if (m_Texture)
		m_Texture->SetColorKey(r, g, b, Index);
}

bool CGameObject::CheckSideCollision()
{
	// �¿� �̵� Ȯ���ϱ�
	float	DirX = m_Pos.x - m_PrevPos.x;
	bool	SideCollision = false;

	// m_SideWallCheck : ���̵� �浹�� �����ϴ� �༮ 
	if (DirX != 0.f && m_SideWallCheck)
	{
		// ���� ��ġ, ���� ��ġ������ �̵� ���� ����
		// ��� Tile ���� ����� ���̴�.
		Vector2	LT = m_Pos - m_Pivot * m_Size;
		Vector2	RB = LT + m_Size;

		Vector2	PrevLT = m_PrevPos - m_Pivot * m_Size;
		Vector2	PrevRB = PrevLT + m_Size;

		CTileMap* TileMap = m_Scene->GetTileMap();

		int	LTIndexX, LTIndexY, RBIndexX, RBIndexY;


		// ���������� �̵��Ҷ� --> ����Ÿ�Ͽ��� ������Ÿ�� ������ �����ϱ�
		if (DirX > 0.f)
		{
			Vector2	resultLT, resultRB;

			resultLT = LT;
			resultRB = RB;

			// ��ġ�� �浹 ����
			// ��, �İ�� ������ ���� LT, RB�� ���� ���̴�.
			// �� �İ�� ���� �ȿ����� �浹 ���ɼ��� �����ϴ� ���̱� �����̴�.
			resultLT.x = PrevRB.x;
			resultLT.y = LT.y < PrevLT.y ? LT.y : PrevLT.y;

			resultRB.y = RB.y > PrevRB.y ? RB.y : PrevRB.y;

			LTIndexX = TileMap->GetOriginTileIndexX(resultLT.x);
			LTIndexY = TileMap->GetOriginTileIndexY(resultLT.y);
			RBIndexX = TileMap->GetOriginTileIndexX(resultRB.x);
			RBIndexY = TileMap->GetOriginTileIndexY(resultRB.y);

			// ���� ����ϱ� 
			LTIndexX = LTIndexX < 0 ? 0 : LTIndexX;
			LTIndexY = LTIndexY < 0 ? 0 : LTIndexY;

			RBIndexX = RBIndexX > TileMap->GetTileCountX() - 1 ? TileMap->GetTileCountX() - 1 : RBIndexX;
			RBIndexY = RBIndexY > TileMap->GetTileCountY() - 1 ? TileMap->GetTileCountY() - 1 : RBIndexY;

			for (int i = LTIndexY; i <= RBIndexY; ++i)
			{
				for (int j = LTIndexX; j <= RBIndexX; ++j)
				{
					int	Index = i * TileMap->GetTileCountX() + j;

					// ���� ó�� 
					if (TileMap->GetTile(Index)->GetTileOption() !=
						ETileOption::Wall ||
						!TileMap->GetTile(Index)->GetSideCollision())
						continue;

					Vector2	TilePos = TileMap->GetTile(Index)->GetPos();
					Vector2	TileSize = TileMap->GetTile(Index)->GetSize();

					// TilePos : Top
					// ��,�� ���� Object�� Tile ���� ���� �پ����� ����
					// �浹 ó���� ���� �ʴ� ���̴�.
					// �̰��� ���� ������, �ƿ� ������ �̵� ��ü�� ���� ���� ���̴�.
					// ��, �� ���� ���� ��� �ִٰ� �Ǵ��� ���̴�.
					if (TilePos.y - 0.001f <= RB.y && RB.y <= TilePos.y + 0.001f)
						continue;

					// �ٴ� ���� + �̵�
					// �� 2���� ���ÿ� ����Ǹ�
					// C++ ������ ��� Ư����,
					// �̵� ���� + �� ���� �Ʒ�����, �߷� (����) �˰����� ����ȴ�.
					// �׷���, �ʹ� ���� �İ��� �Ǹ�, Side Collision�� �����ϰ�
					// �߷� ���뿡 ����, ���� �о�⸸�� ������ �����̴�.
					if (RB.x - TilePos.x > 8.f)
						continue;

					// ��Ʈ�浹
					if (LT.x <= TilePos.x + TileSize.x &&
						LT.y <= TilePos.y + TileSize.y &&
						RB.x >= TilePos.x && RB.y >= TilePos.y)
					{
						SideCollision = true;

						// ������ �̵� --> �ش� ũ�⸸ŭ �������� �о�� ��
						// �ర �� �̵���ų ���̴� 0.0001f ��ŭ 
						float	MoveX = TilePos.x - RB.x - 0.001f;

						//Move(Vector2(MoveX, 0.f));
						m_Pos.x += MoveX;
						m_Velocity.x += MoveX;
					}
				}

				if (SideCollision)
					break;
			}
		}

		// �������� �̵��Ҷ� --> ������Ÿ�Ͽ��� ����Ÿ�� ������ �����ϱ� 
		else
		{
			Vector2	resultLT, resultRB;

			resultLT = LT;
			resultRB = RB;

			resultLT.y = LT.y < PrevLT.y ? LT.y : PrevLT.y;
			resultRB.y = RB.y > PrevRB.y ? RB.y : PrevRB.y;

			resultRB.x = PrevLT.x;

			LTIndexX = TileMap->GetOriginTileIndexX(resultLT.x);
			LTIndexY = TileMap->GetOriginTileIndexY(resultLT.y);
			RBIndexX = TileMap->GetOriginTileIndexX(resultRB.x);
			RBIndexY = TileMap->GetOriginTileIndexY(resultRB.y);

			LTIndexX = LTIndexX < 0 ? 0 : LTIndexX;
			LTIndexY = LTIndexY < 0 ? 0 : LTIndexY;

			RBIndexX = RBIndexX > TileMap->GetTileCountX() - 1 ? TileMap->GetTileCountX() - 1 : RBIndexX;
			RBIndexY = RBIndexY > TileMap->GetTileCountY() - 1 ? TileMap->GetTileCountY() - 1 : RBIndexY;

			for (int i = LTIndexY; i <= RBIndexY; ++i)
			{
				for (int j = RBIndexX; j >= LTIndexX; --j)
				{
					int	Index = i * TileMap->GetTileCountX() + j;

					if (TileMap->GetTile(Index)->GetTileOption() !=
						ETileOption::Wall ||
						!TileMap->GetTile(Index)->GetSideCollision())
						continue;

					Vector2	TilePos = TileMap->GetTile(Index)->GetPos();
					Vector2	TileSize = TileMap->GetTile(Index)->GetSize();

					if (TilePos.y - 0.001f <= RB.y && RB.y <= TilePos.y + 0.001f)
						continue;

					// �ٴ� ���� + �̵�
					// �� 2���� ���ÿ� ����Ǹ�
					// C++ ������ ��� Ư����,
					// �̵� ���� + �� ���� �Ʒ�����, �߷� (����) �˰����� ����ȴ�.
					// �׷���, �ʹ� ���� �İ��� �Ǹ�, Side Collision�� �����ϰ�
					// �߷� ���뿡 ����, ���� �о�⸸�� ������ �����̴�.
					if (TilePos.x + TileSize.x - LT.x > 8.f)
						continue;

					// ��Ʈ�浹
					if (LT.x <= TilePos.x + TileSize.x &&
						LT.y <= TilePos.y + TileSize.y &&
						RB.x >= TilePos.x && RB.y >= TilePos.y)
					{
						SideCollision = true;

						// ���������� �о�� ��
						// 0.001f ��ŭ �ణ �� �о��ش�
						float	MoveX = TilePos.x + TileSize.x - LT.x + 0.001f;

						//Move(Vector2(MoveX, 0.f));
						m_Pos.x += MoveX;
						m_Velocity.x += MoveX;
					}
				}

				if (SideCollision)
					break;
			}
		}
	}

	return SideCollision;
}

bool CGameObject::CheckBottomCollision()
{
	if (m_PhysicsSimulate && m_Pos.y - m_PrevPos.y >= 0.f)
	{
		CTileMap* TileMap = m_Scene->GetTileMap();

		// ���� ��ġ, �̵� ��ġ ������ �̵� ������ ���� Ÿ�� ���ο� ����
		// ������ ���̴�.
		// ����� ��, �Ʒ� ������ ����� ���̴�.
		// Side Collision ������ LT, RB�� ����� ���̴�.
		float	PrevBottom = m_PrevPos.y + (1.f - m_Pivot.y) * m_Size.y;
		float	CurBottom = m_Pos.y + (1.f - m_Pivot.y) * m_Size.y;
		float	PrevLeft = m_PrevPos.x - m_Pivot.x * m_Size.x;
		float	CurLeft = m_Pos.x - m_Pivot.x * m_Size.x;
		float	PrevRight = PrevLeft + m_Size.x;
		float	CurRight = CurLeft + m_Size.x;

		// ������ Ȥ�� ���� �̵��� ����
		// ũ�� ������ �ٲ� ���� �ִ�.
		float	resultLeft = PrevLeft < CurLeft ? PrevLeft : CurLeft;
		float	resultRight = PrevRight > CurRight ? PrevRight : CurRight;
		// �� ���� �� 
		float	resultTop = PrevBottom < CurBottom ? PrevBottom : CurBottom;
		// �� ū ��
		float	resultBottom = PrevBottom > CurBottom ? PrevBottom : CurBottom;

		// ���� ��ġ�� ���� ��ġ�� Ÿ�� �ε����� ���ؿ´�.
		int	LeftIndex, TopIndex, RightIndex, BottomIndex;

		LeftIndex = TileMap->GetOriginTileIndexX(resultLeft);
		TopIndex = TileMap->GetOriginTileIndexY(resultTop);
		RightIndex = TileMap->GetOriginTileIndexX(resultRight);
		BottomIndex = TileMap->GetOriginTileIndexY(resultBottom);

		if (LeftIndex < 0)
			LeftIndex = 0;

		if (TopIndex < 0)
			TopIndex = 0;

		if (RightIndex >= TileMap->GetTileCountX())
			RightIndex = TileMap->GetTileCountX() - 1;

		if (BottomIndex >= TileMap->GetTileCountY())
			BottomIndex = TileMap->GetTileCountY() - 1;

		bool	Check = false;

		// ������ �Ʒ��� �ݺ��Ѵ�.
		for (int i = TopIndex; i <= BottomIndex; ++i)
		{
			for (int j = LeftIndex; j <= RightIndex; ++j)
			{
				// ������ġ�� Bottom�� Ÿ���� Top���� Ŭ ��� �����Ѵ�.
				// ��, ���� �����ϴµ� �� ���� Tile�� �����ϴ� ��
				// �ٴ�üũ�� ������ ���� üũ�ϱ� �����̴�.
				if (TileMap->GetTile(j, i)->GetPos().y < PrevBottom)
					continue;

				// �� ���� ���̶�� (��, �ٴڿ� ��´ٸ� )
				if (TileMap->GetTile(j, i)->GetTileOption() == ETileOption::Wall)
				{
					// ���� ���� 
					Check = true;

					SetObjectLand();

					// FallTime �ʱ�ȭ 
					// m_FallTime = 0.f;

					// �ش� Tile�� Y�� + Pivot ���� ����� ��ġ�� Pos�� �������ش�.
					// m_Pos.y = TileMap->GetTile(j, i)->GetPos().y - (1.f - m_Pivot.y) * m_Size.y;
					m_Pos.y = TileMap->GetTile(j, i)->GetPos().y;

					// �ٴ� ���� + Jump ���� 
					// m_IsGround = true;
					// m_Jump = false;

					// ������ ����Ǵ� �������ӵ� �ð��� �����صд�.
					// m_JumpAccelAccTime = 0.f;

					break;
				}
			}

			if (Check)
				break;
		}

		// ���࿡ �ٴڿ� ���� �ʾҴٸ�
		// ���� �� �ٽ� �������� �����ϴ� �κ� 
		if (!Check)
		{
			// FallTime, FallStartY �κ��� �ʱ�ȭ �صα� 
			if (m_IsGround)
			{
				m_FallTime = 0.f;
				m_FallStartY = m_Pos.y;
			}

			m_IsGround = false;
		}
	}

	return true;
}

void CGameObject::SetObjectLand()
{
	m_FallTime = 0.f;
	m_IsGround = true;
	m_Jump = false;
	m_JumpAccelAccTime = 0.f;
}

void CGameObject::Start()
{
	m_Start = true;

	// ���߿� ���� ������ �����Ѵ�.
	m_FallStartY = m_Pos.y;
}

bool CGameObject::Init()
{
	return true;
}

void CGameObject::Update(float DeltaTime)
{
	if (!m_Start)
	{
		Start();
	}

	if (m_LifeTime > 0.f)
	{
		m_LifeTime -= DeltaTime;

		if (m_LifeTime <= 0.f)
		{
			Destroy();
			return;
		}
	}

	if (m_Animation)
		m_Animation->Update(DeltaTime);

	// �߷��� �����Ѵ�. ( ���� )
	if (!m_IsGround && m_PhysicsSimulate)
	{
		// �������� �ð��� ���������ش�.
		m_FallTime += DeltaTime * m_GravityAccel;

		// 9.8 m/s^2
		// t�� �� y��
		// V : �ӵ�    A : ���ӵ�    G : �߷�
		// y = V * A - 0.5f * G * t * t
		// 0 = -0.5GA^2 VA - y
		// (-b +- ��Ʈ(b^2 - 4ac) / 2a
		float	Velocity = 0.f;
		 
		m_JumpAccelAccTime += m_JumpAccel;

		if (m_Jump)
			Velocity = m_JumpVelocity * m_FallTime;

		float	SaveY = m_Pos.y;

		m_FallVelocity = 0.5f * GRAVITY * m_FallTime * m_FallTime * m_JumpAccel;

		// �ִ� ���� �ӵ��� �����Ѵ�.
		if (Velocity - m_FallVelocity < m_FallVelocityMax * -1.f)
			m_FallVelocity = Velocity + m_FallVelocityMax;

		// m_Pos.y = m_FallStartY - (Velocity - 0.5f * GRAVITY * m_FallTime * m_FallTime);
		m_Pos.y = m_FallStartY - (Velocity - m_FallVelocity);
	}


	{
		auto	iter = m_ColliderList.begin();
		auto	iterEnd = m_ColliderList.end();

		for (; iter != iterEnd;)
		{
			if (!(*iter)->IsActive())
			{
				iter = m_ColliderList.erase(iter);
				iterEnd = m_ColliderList.end();
				continue;
			}

			else if ((*iter)->GetEnable())
			{
				(*iter)->Update(DeltaTime);
			}

			++iter;
		}
	}

	{
		auto	iter = m_WidgetComponentList.begin();
		auto	iterEnd = m_WidgetComponentList.end();

		for (; iter != iterEnd; ++iter)
		{
			(*iter)->Update(DeltaTime);
		}
	}
}

void CGameObject::PostUpdate(float DeltaTime)
{
	if (m_Animation)
	{
		AnimationInfo* AnimInfo = m_Animation->m_CurrentAnimation;

		const AnimationFrameData& FrameData = AnimInfo->Sequence->GetFrameData(AnimInfo->Frame);

		m_Size = FrameData.Size;
	}

	{
		auto	iter = m_ColliderList.begin();
		auto	iterEnd = m_ColliderList.end();

		for (; iter != iterEnd;)
		{
			if (!(*iter)->IsActive())
			{
				iter = m_ColliderList.erase(iter);
				iterEnd = m_ColliderList.end();
				continue;
			}

			else if ((*iter)->GetEnable())
			{
				(*iter)->PostUpdate(DeltaTime);
			}

			++iter;
		}
	}

	{
		auto	iter = m_WidgetComponentList.begin();
		auto	iterEnd = m_WidgetComponentList.end();

		for (; iter != iterEnd; ++iter)
		{
			(*iter)->PostUpdate(DeltaTime);
		}
	}

	// Side Collision
	CheckSideCollision();

	// �ٴ� Ÿ�� üũ�ϱ� 
	// if (m_PhysicsSimulate && m_Pos.y - m_PrevPos.y > 0.f)
	// ">=" �̾�� �ϴ� ������, �ٴ� Ÿ���� �ִ� ���� �ִٰ�
	// �ɾ ���������� �Ҷ�, ������ m_Pos.y �� m_PrevPos.y�� ���� ����
	// �� ���¿����� �������� ���� ��Ű�� �����̴�.
	CheckBottomCollision();
	
}

void CGameObject::Collision(float DeltaTime)
{
	auto	iter = m_ColliderList.begin();
	auto	iterEnd = m_ColliderList.end();

	for (; iter != iterEnd; ++iter)
	{
		m_Scene->GetSceneCollision()->AddCollider(*iter);
	}


	{
		auto	iter = m_WidgetComponentList.begin();
		auto	iterEnd = m_WidgetComponentList.end();

		for (; iter != iterEnd; ++iter)
		{
			(*iter)->Collision(DeltaTime);
		}
	}
}

void CGameObject::PrevRender()
{
	CCamera* Camera = m_Scene->GetCamera();

	m_RenderPos = m_Pos - Camera->GetPos();

	Vector2	Size = m_Size;

	if (m_Animation)
	{
		AnimationInfo* AnimInfo = m_Animation->m_CurrentAnimation;

		const AnimationFrameData& FrameData = AnimInfo->Sequence->GetFrameData(AnimInfo->Frame);

		Size = FrameData.Size;
	}

	Vector2	LT = m_RenderPos - m_Pivot * Size + m_Offset;

	m_CameraCull = false;

	if (LT.x + Size.x <= 0.f)
		m_CameraCull = true;

	else if (LT.x >= Camera->GetResolution().x)
		m_CameraCull = true;

	else if (LT.y + Size.y <= 0.f)
		m_CameraCull = true;

	else if (LT.y >= Camera->GetResolution().y)
		m_CameraCull = true;
}

void CGameObject::Render(HDC hDC)
{
	if (m_Animation)
	{
		AnimationInfo* AnimInfo = m_Animation->m_CurrentAnimation;

		const AnimationFrameData& FrameData = AnimInfo->Sequence->GetFrameData(AnimInfo->Frame);

		Vector2	LT = m_RenderPos - m_Pivot * FrameData.Size + m_Offset;

		if (AnimInfo->Sequence->GetTextureType() == ETexture_Type::Atlas)
		{
			AnimInfo->Sequence->GetTexture()->Render(hDC, LT,
				FrameData.StartPos, FrameData.Size);
		}

		else
		{
			AnimInfo->Sequence->GetTexture()->Render(hDC, LT,
				FrameData.StartPos, FrameData.Size, AnimInfo->Frame);
		}
	}

	else
	{
		Vector2	LT = m_RenderPos - m_Pivot * m_Size + m_Offset;
		if (m_Texture)
		{
			// �̹����� �̿��ؼ� ����Ѵ�.
			m_Texture->Render(hDC, LT, m_ImageStart, m_Size);
		}
	}


	auto	iter = m_ColliderList.begin();
	auto	iterEnd = m_ColliderList.end();

	for (; iter != iterEnd;)
	{
		if (!(*iter)->IsActive())
		{
			iter = m_ColliderList.erase(iter);
			iterEnd = m_ColliderList.end();
			continue;
		}

		else if ((*iter)->GetEnable())
		{
			(*iter)->Render(hDC);
		}

		++iter;
	}

	{
		auto	iter = m_WidgetComponentList.begin();
		auto	iterEnd = m_WidgetComponentList.end();

		for (; iter != iterEnd; ++iter)
		{
			(*iter)->Render(hDC);
		}
	}

	m_PrevPos = m_Pos;
	m_Velocity = Vector2(0.f, 0.f);
}

CGameObject* CGameObject::Clone()
{
	return nullptr;
}

float CGameObject::SetDamage(float Damage)
{
	if (!m_DamageEnable)
		return 0.f;

	return Damage;
}
