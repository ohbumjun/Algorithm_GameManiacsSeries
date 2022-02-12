#pragma once

#include "../GameInfo.h"
#include "../Resource/Texture.h"

struct TileFrameData
{
	Vector2	Start;
	Vector2	End;
};

struct TileTextureFrameData
{
	std::vector<TileFrameData>	vecData;
};

class CEditorDlg
{
public:
	CEditorDlg();
	~CEditorDlg();

private:
	HWND	m_hDlg;
	int		m_ID;
	bool	m_Open;
	class CEditorScene* m_Scene;
	HWND	m_TextureListBox;
	int		m_SelectTextureListIndex;
	TCHAR	m_SelectTextureListText[256];
	CSharedPtr<CTexture>	m_SelectTileTexture;
	std::vector<TileTextureFrameData>	m_vecTextureFrameData;

	HWND	m_EditModeCombo;
	ETileEditMode	m_TileEditMode;

	HWND	m_TileOptionCombo;
	ETileOption	m_TileOption;

	HWND	m_FrameListBox;
	int		m_SelectFrameIndex;

	HWND	m_SideCollisionCheckHandle;
	bool	m_SideCollision;

public:
	bool GetSideCollision()	const
	{
		return m_SideCollision;
	}

	ETileOption GetTileOption()	const
	{
		return (ETileOption)SendMessage(m_TileOptionCombo, CB_GETCURSEL, 0, 0);
	}

	TileFrameData GetTileFrameData()	const
	{
		if (m_SelectTextureListIndex == -1 ||
			m_SelectFrameIndex == -1)
			return TileFrameData();

		return m_vecTextureFrameData[m_SelectTextureListIndex].vecData[m_SelectFrameIndex];
	}

	ETileEditMode GetTileEditMode()	const
	{
		return (ETileEditMode)SendMessage(m_EditModeCombo, CB_GETCURSEL, 0, 0);
	}

	bool IsOpen()	const
	{
		return m_Open;
	}

	RECT GetRect()	const
	{
		RECT	rc;
		GetWindowRect(m_hDlg, &rc);
		return rc;
	}

	void SetScene(class CEditorScene* Scene)
	{
		m_Scene = Scene;
	}

public:
	bool Init(int ID);
	void Show();
	void CreateMap();
	void LoadTileTexture();
	void SelectTexture();
	void SelectList();
	void AddFrame();
	void DeleteFrame();
	void ModifyFrame();
	void ChangeFrame();
	void Save();
	void Load();
	void TileImageAllClear();
	void SideCollisionCheck();

private:
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};
