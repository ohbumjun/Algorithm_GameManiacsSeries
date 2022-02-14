
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
		m_Loop = true;
		if (!m_Loop)
		{
			CCamera* Camera = m_Scene->GetCamera();

			// �ػ� �������� 
			Vector2 Resolution = Camera->GetResolution();

			// Scroll Ratio �� �����´�.
			// ex) ���� ���, ī�޶� 100m ������ �̵� --> ���� �ش� Map �󿡼��� 30m�� ������ �̵�
			// �̸� ���� ���� �� ������ �����ϴ�.
			Vector2	ImagePos = Camera->GetPos() * m_ScrollRatio;

			unsigned int	Width = m_ScrollTexture->GetWidth();
			unsigned int	Height = m_ScrollTexture->GetHeight();

			// ���� ���� 
			ImagePos.x = ImagePos.x < 0.f ? 0.f : ImagePos.x;

			// ������ �� �̻����� ���ư����� �ʰ� �����Ѵ�.
			ImagePos.x = ImagePos.x + Resolution.x > Width ? Width - Resolution.x : ImagePos.x;

			ImagePos.y = ImagePos.y < 0.f ? 0.f : ImagePos.y;

			// ����������, �Ʒ� ������ ���ư����� �ʰ� �����Ѵ�.
			ImagePos.y = ImagePos.y + Resolution.y > Height ? Height - Resolution.y : ImagePos.y;

			// �׷���, �ش� ��ġ���� Render�� �� ���̴�.
			m_ScrollTexture->Render(hDC, m_Pos, ImagePos, m_Size);
		}

		// ���� Loop ���, Image Pos ������ �߸� ��ġ��, �����ʿ� �ٿ��� Render �� ���̴�.
		else
		{
			CCamera* Camera = m_Scene->GetCamera();

			Vector2	CameraPos = Camera->GetPos();

			Vector2 Resolution = Camera->GetResolution();

			Vector2	ImagePos = Camera->GetPos() * m_ScrollRatio;

			// int	XRatio = (int)(CameraPos.x / Resolution.x); 
			// int	YRatio = (int)(CameraPos.y / Resolution.y); 

			unsigned int	Width = m_ScrollTexture->GetWidth();
			unsigned int	Height = m_ScrollTexture->GetHeight();

			// �»��, ����, ���ϴ�, ���ϴ� ���
			// ex) (int) --> CameraPos.x �� m_ScrollTexture.Width �� �ȳѾ���� 0�� ������, �Ѿ�� 1�� ���´�.
			unsigned int	WidthRatio = (unsigned int)(ImagePos.x / Width);
			unsigned int	HeightRatio = (unsigned int)(ImagePos.y / Height);

			Vector2	ConvertPos; // Texture �󿡼��� Camera Pos�� ���� ���̴�.
			ConvertPos.x = ImagePos.x - WidthRatio * Width;
			ConvertPos.y = ImagePos.y - HeightRatio * Height;

			Vector2	ConvertLeftTopSize; // �»�� �簢�� Size 
			ConvertLeftTopSize.x = Width - ConvertPos.x;
			ConvertLeftTopSize.y = Height - ConvertPos.y;

			// �� ��� ���
			m_ScrollTexture->Render(hDC, Vector2(0.f, 0.f), ConvertPos, ConvertLeftTopSize);

			// �� ��� ���
			m_ScrollTexture->Render(hDC, Vector2(ConvertLeftTopSize.x, 0.f),  // �ǿ��� 0.f ���� �»�� ũ�⸸ŭ ������ �̵��� ��ġ
				Vector2(0.f, Height - ConvertLeftTopSize.y),  // �̹��� �󿡼���, �Ʒ��� �̵��ؼ� �� �������� �׸���
				Vector2(Resolution.x - ConvertLeftTopSize.x, ConvertLeftTopSize.y)); // �»�� ũ�� ����, Texture ũ�⸸ŭ �׸���

			// �� �ϴ� ���
			m_ScrollTexture->Render(hDC, Vector2(0.f, ConvertLeftTopSize.y), // �»�� ũ�⸸ŭ �Ʒ����� �׸���
				Vector2(ConvertPos.x, 0.f),// �̹��� �󿡼��� ������ �̵��� ��ŭ 
				Vector2(ConvertLeftTopSize.x, Resolution.y - ConvertLeftTopSize.y)); // �»�� ����ũ�� + (�ػ� - �»�� Box ����)

			// �� �ϴ� ���
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
