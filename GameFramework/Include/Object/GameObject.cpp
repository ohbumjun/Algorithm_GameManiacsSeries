
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
	// 좌우 이동 확인하기
	float	DirX = m_Pos.x - m_PrevPos.x;
	bool	SideCollision = false;

	// m_SideWallCheck : 사이드 충돌을 적용하는 녀석 
	if (DirX != 0.f && m_SideWallCheck)
	{
		// 이전 위치, 현재 위치까지의 이동 범위 안의
		// 모든 Tile 들을 고려할 것이다.
		Vector2	LT = m_Pos - m_Pivot * m_Size;
		Vector2	RB = LT + m_Size;

		Vector2	PrevLT = m_PrevPos - m_Pivot * m_Size;
		Vector2	PrevRB = PrevLT + m_Size;

		CTileMap* TileMap = m_Scene->GetTileMap();

		int	LTIndexX, LTIndexY, RBIndexX, RBIndexY;


		// 오른쪽으로 이동할때 --> 왼쪽타일에서 오른쪽타일 순으로 조사하기
		if (DirX > 0.f)
		{
			Vector2	resultLT, resultRB;

			resultLT = LT;
			resultRB = RB;

			// 겹치는 충돌 영역
			// 즉, 파고든 영역에 대한 LT, RB를 구할 것이다.
			// 이 파고든 영역 안에서만 충돌 가능성이 존재하는 것이기 때문이다.
			resultLT.x = PrevRB.x;
			resultLT.y = LT.y < PrevLT.y ? LT.y : PrevLT.y;

			resultRB.y = RB.y > PrevRB.y ? RB.y : PrevRB.y;

			LTIndexX = TileMap->GetOriginTileIndexX(resultLT.x);
			LTIndexY = TileMap->GetOriginTileIndexY(resultLT.y);
			RBIndexX = TileMap->GetOriginTileIndexX(resultRB.x);
			RBIndexY = TileMap->GetOriginTileIndexY(resultRB.y);

			// 범위 고려하기 
			LTIndexX = LTIndexX < 0 ? 0 : LTIndexX;
			LTIndexY = LTIndexY < 0 ? 0 : LTIndexY;

			RBIndexX = RBIndexX > TileMap->GetTileCountX() - 1 ? TileMap->GetTileCountX() - 1 : RBIndexX;
			RBIndexY = RBIndexY > TileMap->GetTileCountY() - 1 ? TileMap->GetTileCountY() - 1 : RBIndexY;

			for (int i = LTIndexY; i <= RBIndexY; ++i)
			{
				for (int j = LTIndexX; j <= RBIndexX; ++j)
				{
					int	Index = i * TileMap->GetTileCountX() + j;

					// 예외 처리 
					if (TileMap->GetTile(Index)->GetTileOption() !=
						ETileOption::Wall ||
						!TileMap->GetTile(Index)->GetSideCollision())
						continue;

					Vector2	TilePos = TileMap->GetTile(Index)->GetPos();
					Vector2	TileSize = TileMap->GetTile(Index)->GetSize();

					// TilePos : Top
					// 즉,이 경우는 Object가 Tile 위에 거의 붙어있을 경우는
					// 충돌 처리를 하지 않는 것이다.
					// 이것을 하지 않으면, 아예 땅에서 이동 자체가 막혀 버릴 것이다.
					// 즉, 이 경우는 땅을 밟고 있다고 판단할 것이다.
					if (TilePos.y - 0.001f <= RB.y && RB.y <= TilePos.y + 0.001f)
						continue;

					// 바닥 착지 + 이동
					// 이 2개가 동시에 진행되면
					// C++ 절차적 언어 특성상,
					// 이동 먼저 + 그 다음 아래에서, 중력 (착지) 알고리즘이 적용된다.
					// 그런데, 너무 깊이 파고들게 되면, Side Collision은 무시하고
					// 중력 적용에 따른, 위로 밀어내기만을 적용할 예정이다.
					if (RB.x - TilePos.x > 8.f)
						continue;

					// 렉트충돌
					if (LT.x <= TilePos.x + TileSize.x &&
						LT.y <= TilePos.y + TileSize.y &&
						RB.x >= TilePos.x && RB.y >= TilePos.y)
					{
						SideCollision = true;

						// 오른쪽 이동 --> 해당 크기만큼 왼쪽으로 밀어내는 것
						// 약같 더 이동시킬 것이다 0.0001f 만큼 
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

		// 왼쪽으로 이동할때 --> 오른쪽타일에서 왼쪽타일 순으로 조사하기 
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

					// 바닥 착지 + 이동
					// 이 2개가 동시에 진행되면
					// C++ 절차적 언어 특성상,
					// 이동 먼저 + 그 다음 아래에서, 중력 (착지) 알고리즘이 적용된다.
					// 그런데, 너무 깊이 파고들게 되면, Side Collision은 무시하고
					// 중력 적용에 따른, 위로 밀어내기만을 적용할 예정이다.
					if (TilePos.x + TileSize.x - LT.x > 8.f)
						continue;

					// 렉트충돌
					if (LT.x <= TilePos.x + TileSize.x &&
						LT.y <= TilePos.y + TileSize.y &&
						RB.x >= TilePos.x && RB.y >= TilePos.y)
					{
						SideCollision = true;

						// 오른쪽으로 밀어내는 것
						// 0.001f 만큼 약간 더 밀어준다
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

		// 현재 위치, 이동 위치 사이의 이동 범위에 놓인 타일 전부에 대해
		// 조사할 것이다.
		// 여기는 위, 아래 범위만 고려할 것이다.
		// Side Collision 에서는 LT, RB을 고려할 것이다.
		float	PrevBottom = m_PrevPos.y + (1.f - m_Pivot.y) * m_Size.y;
		float	CurBottom = m_Pos.y + (1.f - m_Pivot.y) * m_Size.y;
		float	PrevLeft = m_PrevPos.x - m_Pivot.x * m_Size.x;
		float	CurLeft = m_Pos.x - m_Pivot.x * m_Size.x;
		float	PrevRight = PrevLeft + m_Size.x;
		float	CurRight = CurLeft + m_Size.x;

		// 오른쪽 혹은 왼쪽 이동에 따라서
		// 크고 작음이 바뀔 수도 있다.
		float	resultLeft = PrevLeft < CurLeft ? PrevLeft : CurLeft;
		float	resultRight = PrevRight > CurRight ? PrevRight : CurRight;
		// 더 작은 것 
		float	resultTop = PrevBottom < CurBottom ? PrevBottom : CurBottom;
		// 더 큰 것
		float	resultBottom = PrevBottom > CurBottom ? PrevBottom : CurBottom;

		// 이전 위치와 현재 위치의 타일 인덱스를 구해온다.
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

		// 위에서 아래로 반복한다.
		for (int i = TopIndex; i <= BottomIndex; ++i)
		{
			for (int j = LeftIndex; j <= RightIndex; ++j)
			{
				// 이전위치의 Bottom이 타일의 Top보다 클 경우 무시한다.
				// 즉, 내가 점프하는데 그 위에 Tile이 존재하는 것
				// 바닥체크는 내려갈 때만 체크하기 위함이다.
				if (TileMap->GetTile(j, i)->GetPos().y < PrevBottom)
					continue;

				// 못 가는 곳이라면 (즉, 바닥에 닿는다면 )
				if (TileMap->GetTile(j, i)->GetTileOption() == ETileOption::Wall)
				{
					// 땅에 닿음 
					Check = true;

					SetObjectLand();

					// FallTime 초기화 
					// m_FallTime = 0.f;

					// 해당 Tile의 Y값 + Pivot 등을 고려한 위치를 Pos를 세팅해준다.
					// m_Pos.y = TileMap->GetTile(j, i)->GetPos().y - (1.f - m_Pivot.y) * m_Size.y;
					m_Pos.y = TileMap->GetTile(j, i)->GetPos().y;

					// 바닥 도달 + Jump 세팅 
					// m_IsGround = true;
					// m_Jump = false;

					// 점프에 적용되는 누적가속도 시간도 세팅해둔다.
					// m_JumpAccelAccTime = 0.f;

					break;
				}
			}

			if (Check)
				break;
		}

		// 만약에 바닥에 닿지 않았다면
		// 이제 막 다시 떨어지기 시작하는 부분 
		if (!Check)
		{
			// FallTime, FallStartY 부분을 초기화 해두기 
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

	// 나중에 땅에 닿으면 갱신한다.
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

	// 중력을 적용한다. ( 점프 )
	if (!m_IsGround && m_PhysicsSimulate)
	{
		// 떨어지는 시간을 누적시켜준다.
		m_FallTime += DeltaTime * m_GravityAccel;

		// 9.8 m/s^2
		// t초 후 y값
		// V : 속도    A : 가속도    G : 중력
		// y = V * A - 0.5f * G * t * t
		// 0 = -0.5GA^2 VA - y
		// (-b +- 루트(b^2 - 4ac) / 2a
		float	Velocity = 0.f;
		 
		m_JumpAccelAccTime += m_JumpAccel;

		if (m_Jump)
			Velocity = m_JumpVelocity * m_FallTime;

		float	SaveY = m_Pos.y;

		m_FallVelocity = 0.5f * GRAVITY * m_FallTime * m_FallTime * m_JumpAccel;

		// 최대 낙하 속도를 조절한다.
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

	// 바닥 타일 체크하기 
	// if (m_PhysicsSimulate && m_Pos.y - m_PrevPos.y > 0.f)
	// ">=" 이어야 하는 이유는, 바닥 타일이 있는 곳에 있다가
	// 걸어서 떨어지려고 할때, 여전히 m_Pos.y 와 m_PrevPos.y는 같은 상태
	// 이 상태에서도 떨어짐을 적용 시키기 위함이다.
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
			// 이미지를 이용해서 출력한다.
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
