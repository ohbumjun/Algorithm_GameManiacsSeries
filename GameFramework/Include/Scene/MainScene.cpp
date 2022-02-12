
#include "MainScene.h"
#include "../Object/Player.h"
#include "../Object/Bullet.h"
#include "../Object/Monster.h"
#include "SceneResource.h"
#include "../Object/EffectHit.h"
#include "Camera.h"
#include "../UI/UIMain.h"
#include "../UI/UICharacterStateHUD.h"
#include "../Map/ScrollMap.h"
#include "../Map/TileMap.h"

CMainScene::CMainScene()
{
}

CMainScene::~CMainScene()
{
}

bool CMainScene::Init()
{
	// LoadSound();

	LoadAnimationSequence();

	GetCamera()->SetWorldResolution(10000.f, 10000.f);

	CEffectHit* EffectPrototype = CreatePrototype<CEffectHit>("HitEffect");

	CBullet* PlayerBullet = CreatePrototype<CBullet>("PlayerBullet");

	CCollider* Collider = PlayerBullet->FindCollider("Body");

	if (Collider)
		Collider->SetCollisionProfile("PlayerAttack");

	CBullet* MonsterBullet = CreatePrototype<CBullet>("MonsterBullet");

	Collider = MonsterBullet->FindCollider("Body");

	if (Collider)
		Collider->SetCollisionProfile("MonsterAttack");

	CPlayer* Player = CreateObject<CPlayer>("Player");

	Player->SetPos(300.f, 0.f);

	SetPlayer(Player);

	GetCamera()->SetTarget(Player);
	GetCamera()->SetTargetPivot(0.5f, 0.5f);

	// CMonster* Monster = CreateObject<CMonster>("Monster",
		// Vector2(1000.f, 100.f));

	/*CUIWindow* TestWindow = CreateUIWindow<CUIWindow>("TestWindow");

	CUIImage* Image = TestWindow->CreateWidget<CUIImage>("TestImage");

	Image->SetTexture("Test", TEXT("teemo.bmp"));
	Image->SetPos(100.f, 100.f);
	Image->SetZOrder(1);

	CUIImage* Image1 = TestWindow->CreateWidget<CUIImage>("TestImage1");

	Image1->SetTexture("Test1", TEXT("Start.bmp"));
	Image1->SetPos(150.f, 100.f);*/

	GetCamera()->SetWorldResolution(3000.f, 1200.f);
	//GetCamera()->SetWorldResolution(30000.f, 12000.f);

	/*CScrollMap* Map = CreateMap<CScrollMap>("ScrollMap");

	float	ScrollWidth = 1500.f - GetCamera()->GetResolution().x;
	float	ScrollHeight = 1200.f - GetCamera()->GetResolution().y;

	float	TileMapWidth = 3000.f - GetCamera()->GetResolution().x;
	float	TileMapHeight = 1200.f - GetCamera()->GetResolution().y;

	Map->SetSize(1280.f, 720.f);
	Map->SetTexture("ScrollBack", TEXT("Sky.bmp"));
	Map->SetLoop(true);
	Map->SetZOrder(0);*/

	CScrollMap* Map = CreateMap<CScrollMap>("ScrollMap");

	// 해상도를 빼줄 것이다.
	float	ScrollWidth = 1500.f - GetCamera()->GetResolution().x; // Scroll Map Texture 기준
	float	ScrollHeight = 1200.f - GetCamera()->GetResolution().y;

	float	TileMapWidth = 3000.f - GetCamera()->GetResolution().x; // World Map 기준
	float	TileMapHeight = 1200.f - GetCamera()->GetResolution().y;

	Map->SetSize(1280.f, 720.f);
	Map->SetTexture("ScrollBack", TEXT("Sky.bmp"));

	// 100 / 1000 --> 0.1 이라고 한다면, Scroll Ratio는  0.1이 될 것이다.
	// 즉, 카메라가 100m 이동할 때, 실제 Scroll Map 상에서는 10m만 이동한 형태를 띄게 되는 것이다.
	Map->SetScrollRatio(ScrollWidth / TileMapWidth, ScrollHeight / TileMapHeight);
	Map->SetZOrder(0);

	Map = CreateMap<CScrollMap>("ScrollMap");

	ScrollWidth = 2048.f - GetCamera()->GetResolution().x;
	ScrollHeight = 2048.f - GetCamera()->GetResolution().y;

	Map->SetSize(1280.f, 720.f);
	Map->SetTexture("Mountain", TEXT("Mountain.bmp"));
	Map->SetScrollRatio(ScrollWidth / TileMapWidth, ScrollHeight / TileMapHeight);
	Map->SetZOrder(1);
	Map->SetTextureColorKey(255, 0, 255);

	CTileMap*	TileMap = CreateMap<CTileMap>("TileMap");

	TileMap->LoadFile("MainMap.map");
	TileMap->SetZOrder(2);


	CUIMain* MainWindow = CreateUIWindow<CUIMain>("MainWindow");
	CUICharacterStateHUD* StateWindow = CreateUIWindow<CUICharacterStateHUD>("CharacterStateHUD");

	return true;
}

void CMainScene::LoadAnimationSequence()
{
	GetSceneResource()->CreateAnimationSequence("LucidNunNaRightIdle",
		"LucidNunNaRightIdle", TEXT("Player/Right/astand.bmp"));

	GetSceneResource()->SetTextureColorKey("LucidNunNaRightIdle",
		255, 0, 255);

	for (int i = 0; i < 6; ++i)
	{
		GetSceneResource()->AddAnimationFrameData("LucidNunNaRightIdle",
			i * 82.f, 0.f, 82.f, 73.f);
	}

	GetSceneResource()->CreateAnimationSequence("LucidNunNaRightWalk",
		"LucidNunNaRightWalk", TEXT("Player/Right/awalk.bmp"));

	GetSceneResource()->SetTextureColorKey("LucidNunNaRightWalk",
		255, 0, 255);

	for (int i = 0; i < 4; ++i)
	{
		GetSceneResource()->AddAnimationFrameData("LucidNunNaRightWalk",
			i * 85.f, 0.f, 85.f, 75.f);
	}

	GetSceneResource()->CreateAnimationSequence("LucidNunNaRightAttack",
		"LucidNunNaRightAttack", TEXT("Player/Right/aswing.bmp"));

	GetSceneResource()->SetTextureColorKey("LucidNunNaRightAttack",
		255, 0, 255);

	for (int i = 0; i < 3; ++i)
	{
		GetSceneResource()->AddAnimationFrameData("LucidNunNaRightAttack",
			i * 176.f, 0.f, 176.f, 89.f);
	}

	GetSceneResource()->CreateAnimationSequence("LucidNunNaRightSkill1",
		"LucidNunNaRightSkill1", TEXT("Player/Right/ashoot1.bmp"));

	GetSceneResource()->SetTextureColorKey("LucidNunNaRightSkill1",
		255, 0, 255);

	for (int i = 0; i < 3; ++i)
	{
		GetSceneResource()->AddAnimationFrameData("LucidNunNaRightSkill1",
			i * 70.f, 0.f, 70.f, 81.f);
	}

	GetSceneResource()->CreateAnimationSequence("HitRight",
		"HitRight", TEXT("Hit2.bmp"));

	GetSceneResource()->SetTextureColorKey("HitRight",
		255, 0, 255);

	for (int i = 0; i < 6; ++i)
	{
		GetSceneResource()->AddAnimationFrameData("HitRight",
			i * 178.f, 0.f, 178.f, 164.f);
	}

	GetSceneResource()->CreateAnimationSequence("Bullet",
		"Bullet", TEXT("Smoke.bmp"));

	GetSceneResource()->SetTextureColorKey("Bullet",
		255, 0, 255);

	for (int i = 0; i < 8; ++i)
	{
		for (int j = 0; j < 8; ++j)
		{
			GetSceneResource()->AddAnimationFrameData("Bullet",
				j * 92.f, i * 92.f, 92.f, 92.f);
		}
	}


	GetSceneResource()->CreateAnimationSequence("LucidNunNaLeftIdle",
		"LucidNunNaLeftIdle", TEXT("Player/Left/astand_left.bmp"));

	GetSceneResource()->SetTextureColorKey("LucidNunNaLeftIdle",
		255, 0, 255);

	for (int i = 0; i < 6; ++i)
	{
		GetSceneResource()->AddAnimationFrameData("LucidNunNaLeftIdle",
			i * 82.f, 0.f, 82.f, 73.f);
	}

	GetSceneResource()->CreateAnimationSequence("LucidNunNaLeftWalk",
		"LucidNunNaLeftWalk", TEXT("Player/Left/awalk_left.bmp"));

	GetSceneResource()->SetTextureColorKey("LucidNunNaLeftWalk",
		255, 0, 255);

	for (int i = 0; i < 4; ++i)
	{
		GetSceneResource()->AddAnimationFrameData("LucidNunNaLeftWalk",
			i * 85.f, 0.f, 85.f, 75.f);
	}

	GetSceneResource()->CreateAnimationSequence("LucidNunNaLeftAttack",
		"LucidNunNaLeftAttack", TEXT("Player/Left/aswing_left.bmp"));

	GetSceneResource()->SetTextureColorKey("LucidNunNaLeftAttack",
		255, 0, 255);

	for (int i = 0; i < 3; ++i)
	{
		GetSceneResource()->AddAnimationFrameData("LucidNunNaLeftAttack",
			i * 176.f, 0.f, 176.f, 89.f);
	}
}

void CMainScene::LoadSound()
{
	GetSceneResource()->LoadSound("BGM", true, "MainBGM", "MainBgm.mp3");
	//GetSceneResource()->SoundPlay("MainBGM");
	//GetSceneResource()->SetVolume("MainBGM", 10);

	GetSceneResource()->LoadSound("Effect", false, "Fire", "Fire1.wav");
	GetSceneResource()->SetVolume("Effect", 10);

	GetSceneResource()->LoadSound("Effect", false, "TextSound", "water-step-01.ogg");
}
