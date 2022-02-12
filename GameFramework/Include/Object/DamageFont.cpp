
#include "DamageFont.h"
#include "Bullet.h"
#include "../Scene/Scene.h"
#include "../Collision/ColliderBox.h"

CDamageFont::CDamageFont()
{
	m_ObjType = EObject_Type::Effect;
}

CDamageFont::CDamageFont(const CDamageFont& obj) :
	CGameObject(obj)
{
}

CDamageFont::~CDamageFont()
{
}

void CDamageFont::Start()
{
	CGameObject::Start();

	m_DirX = rand() % 2 == 0 ? 1.f : -1.f;
	m_SpeedX = (float)(rand() % 301);
}

bool CDamageFont::Init()
{
	if (!CGameObject::Init())
		return false;

	SetPivot(0.5f, 0.5f);


	SetGravityAccel(30.f);
	SetPhysicsSimulate(true);
	SetJumpVelocity(50.f);
	Jump();

	CWidgetComponent* Widget = CreateWidgetComponent("DamageFont");

	m_NumberWidget = Widget->CreateWidget<CNumberWidget>("DamageFont");

	std::vector<std::wstring>	vecNumberFileName;

	for (int i = 0; i < 10; ++i)
	{
		TCHAR	FileName[256] = {};

		wsprintf(FileName, TEXT("Number/%d.bmp"), i);

		vecNumberFileName.push_back(FileName);
	}

	m_NumberWidget->SetTexture("Number", vecNumberFileName);

	for (int i = 0; i < 10; ++i)
	{
		m_NumberWidget->SetTextureColorKey(255, 255, 255, i);
	}

	m_NumberWidget->SetSize(29.f, 48.f);

	SetLifeTime(0.3f);

	return true;
}

void CDamageFont::Update(float DeltaTime)
{
	CGameObject::Update(DeltaTime);

	Move(Vector2(m_DirX * m_SpeedX * DeltaTime, 0.f));
}

void CDamageFont::PostUpdate(float DeltaTime)
{
	CGameObject::PostUpdate(DeltaTime);
}

void CDamageFont::Collision(float DeltaTime)
{
	CGameObject::Collision(DeltaTime);
}

void CDamageFont::Render(HDC hDC)
{
	CGameObject::Render(hDC);
}

CDamageFont* CDamageFont::Clone()
{
	return new CDamageFont(*this);
}
