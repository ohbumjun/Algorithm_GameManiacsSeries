
#include "ScrollMap.h"
#include "../Scene/Scene.h"
#include "../Scene/SceneResource.h"
#include "../Scene/Camera.h"

CScrollMap::CScrollMap()	:
	m_ScrollRatio(1.f, 1.f),
	m_Loop(false)
{
}

CScrollMap::~CScrollMap()
{
}

void CScrollMap::SetTexture(const std::string& Name, const TCHAR* FileName,
	const std::string& PathName)
{
	m_Scene->GetSceneResource()->LoadTexture(Name, FileName, PathName);

	SetTexture(Name);
}

void CScrollMap::SetTextureFullPath(const std::string& Name, 
	const TCHAR* FullPath)
{
	m_Scene->GetSceneResource()->LoadTextureFullPath(Name, FullPath);

	SetTexture(Name);
}

void CScrollMap::SetTexture(const std::string& Name,
	const std::vector<std::wstring>& vecFileName, const std::string& PathName)
{
	m_Scene->GetSceneResource()->LoadTexture(Name, vecFileName, PathName);

	SetTexture(Name);
}

void CScrollMap::SetTexture(const std::string& Name)
{
	m_ScrollTexture = m_Scene->GetSceneResource()->FindTexture(Name);
}

void CScrollMap::SetTextureColorKey(const unsigned char r, const unsigned char g, const unsigned char b)
{
	if (m_ScrollTexture)
		m_ScrollTexture->SetColorKey(r, g, b);
}

void CScrollMap::Start()
{
	CMapBase::Start();

	if (m_ScrollTexture)
		m_TextureSize = Vector2((float)m_ScrollTexture->GetWidth(), (float)m_ScrollTexture->GetHeight());
}

bool CScrollMap::Init()
{
	if (!CMapBase::Init())
		return false;

	return true;
}

void CScrollMap::Update(float DeltaTime)
{
	CMapBase::Update(DeltaTime);
}

void CScrollMap::PostUpdate(float DeltaTime)
{
	CMapBase::PostUpdate(DeltaTime);
}

void CScrollMap::PrevRender()
{
	CMapBase::PrevRender();
}

void CScrollMap::Render(HDC hDC)
{
	CMapBase::Render(hDC);

	if (m_ScrollTexture)
	{
		if (!m_Loop)
		{
			CCamera* Camera = m_Scene->GetCamera();

			// 해상도 가져오고 
			Vector2 Resolution = Camera->GetResolution();

			// Scroll Ratio 을 가져온다.
			// ex) 예를 들어, 카메라가 100m 오른쪽 이동 --> 실제 해당 Map 상에서는 30m만 오른쪽 이동
			// 이를 통해 원근 감 구현이 가능하다.
			Vector2	ImagePos = Camera->GetPos() * m_ScrollRatio;

			unsigned int	Width = m_ScrollTexture->GetWidth();
			unsigned int	Height = m_ScrollTexture->GetHeight();

			// 범위 조정 
			ImagePos.x = ImagePos.x < 0.f ? 0.f : ImagePos.x;

			// 오른쪽 끝 이상으로 나아가지는 않게 세팅한다.
			ImagePos.x = ImagePos.x + Resolution.x > Width ? Width - Resolution.x : ImagePos.x;

			ImagePos.y = ImagePos.y < 0.f ? 0.f : ImagePos.y;

			// 마찬가지로, 아래 끝까지 나아가지는 않게 세팅한다.
			ImagePos.y = ImagePos.y + Resolution.y > Height ? Height - Resolution.y : ImagePos.y;

			// 그러면, 해당 위치부터 Render를 할 것이다.
			m_ScrollTexture->Render(hDC, m_Pos, ImagePos, m_Size);
		}

		// 만약 Loop 라면, Image Pos 이전에 잘린 위치를, 오른쪽에 붙여서 Render 할 것이다.
		else
		{
			CCamera* Camera = m_Scene->GetCamera();

			Vector2	CameraPos = Camera->GetPos();

			Vector2 Resolution = Camera->GetResolution();

			Vector2	ImagePos = Camera->GetPos() * m_ScrollRatio;

			int	XRatio = (int)(CameraPos.x / Resolution.x);
			int	YRatio = (int)(CameraPos.y / Resolution.y);

			unsigned int	Width = m_ScrollTexture->GetWidth();
			unsigned int	Height = m_ScrollTexture->GetHeight();

			// 좌상단, 우상단, 좌하단, 우하단 출력
			unsigned int	WidthRatio = (unsigned int)(CameraPos.x / Width);
			unsigned int	HeightRatio = (unsigned int)(CameraPos.y / Height);

			Vector2	ConvertPos;
			ConvertPos.x = CameraPos.x - WidthRatio * Width;
			ConvertPos.y = CameraPos.y - HeightRatio * Height;

			Vector2	ConvertLeftTopSize;
			ConvertLeftTopSize.x = Width - ConvertPos.x;
			ConvertLeftTopSize.y = Height - ConvertPos.y;

			// 좌 상단 출력
			m_ScrollTexture->Render(hDC, Vector2(0.f, 0.f), ConvertPos, ConvertLeftTopSize);

			// 우 상단 출력
			m_ScrollTexture->Render(hDC, Vector2(ConvertLeftTopSize.x, 0.f), 
				Vector2(0.f, Height - ConvertLeftTopSize.y), 
				Vector2(Resolution.x - ConvertLeftTopSize.x, ConvertLeftTopSize.y));

			// 좌 하단 출력
			m_ScrollTexture->Render(hDC, Vector2(0.f, ConvertLeftTopSize.y),
				Vector2(ConvertPos.x, 0.f),
				Vector2(ConvertLeftTopSize.x, Resolution.y - ConvertLeftTopSize.y));

			// 우 하단 출력
			m_ScrollTexture->Render(hDC, Vector2(ConvertLeftTopSize.x, ConvertLeftTopSize.y),
				Vector2(0.f, 0.f),
				Vector2(Resolution.x - ConvertLeftTopSize.x, Resolution.y - ConvertLeftTopSize.y));
		}
	}
}

void CScrollMap::Save(FILE* pFile)
{
}

void CScrollMap::Load(FILE* pFile)
{
}
