
#include "EditorDlg.h"
#include "../GameManager.h"
#include "../PathManager.h"
#include "Scene.h"
#include "SceneResource.h"
#include "EditorScene.h"

CEditorDlg* g_Dlg;

CEditorDlg::CEditorDlg()	:
	m_ID(0),
	m_hDlg(0),
	m_Open(true),
	m_Scene(nullptr),
	m_SelectTextureListText{},
	m_SelectFrameIndex(-1)
{
	g_Dlg = this;
}

CEditorDlg::~CEditorDlg()
{
	if (m_hDlg)
		DestroyWindow(m_hDlg);
}

bool CEditorDlg::Init(int ID)
{
	// Modal : 부모윈도우는 멈추고 열리는 다이얼로그만 동작되는 방식
	// Modeless : 부모윈도우와 다이얼로그 모두 동작되는 방식
	m_hDlg	= CreateDialog(CGameManager::GetInst()->GetWindowInstance(),
		MAKEINTRESOURCE(ID), CGameManager::GetInst()->GetWindowHandle(),
		CEditorDlg::WndProc);

	ShowWindow(m_hDlg, SW_SHOW);

	SetDlgItemInt(m_hDlg, IDC_EDIT_TILECOUNTX, 40, TRUE);
	SetDlgItemInt(m_hDlg, IDC_EDIT_TILECOUNTY, 40, TRUE);

	SetDlgItemInt(m_hDlg, IDC_EDIT_TILESIZEX, 40, TRUE);
	SetDlgItemInt(m_hDlg, IDC_EDIT_TILESIZEY, 53, TRUE);

	SetDlgItemInt(m_hDlg, IDC_EDIT_STARTFRAMEX, 0, TRUE);
	SetDlgItemInt(m_hDlg, IDC_EDIT_STARTFRAMEY, 0, TRUE);

	SetDlgItemInt(m_hDlg, IDC_EDIT_ENDFRAMEX, 40, TRUE);
	SetDlgItemInt(m_hDlg, IDC_EDIT_ENDFRAMEY, 53, TRUE);

	m_TextureListBox = GetDlgItem(m_hDlg, IDC_LIST_TILETEXTURE);

	m_FrameListBox = GetDlgItem(m_hDlg, IDC_LIST_TEXTUREFRAME);

	m_SelectTextureListIndex = -1;

	m_EditModeCombo = GetDlgItem(m_hDlg, IDC_COMBO_EDITMODE);

	TCHAR	TileEditMode[(int)ETileEditMode::End][30] =
	{
		TEXT("타일옵션"),
		TEXT("타일이미지")
	};

	for (int i = 0; i < (int)ETileEditMode::End; ++i)
	{
		SendMessage(m_EditModeCombo, CB_ADDSTRING, 0, (LPARAM)TileEditMode[i]);
	}

	SendMessage(m_EditModeCombo, CB_SETCURSEL, 0, 0);

	m_TileEditMode = ETileEditMode::Option;


	m_TileOptionCombo = GetDlgItem(m_hDlg, IDC_COMBO_TILEOPTION);

	TCHAR	TileOptionText[(int)ETileOption::End][30] =
	{
		TEXT("Normal"),
		TEXT("Wall"),
		TEXT("Slow")
	};

	for (int i = 0; i < (int)ETileOption::End; ++i)
	{
		SendMessage(m_TileOptionCombo, CB_ADDSTRING, 0, (LPARAM)TileOptionText[i]);
	}

	SendMessage(m_TileOptionCombo, CB_SETCURSEL, 0, 0);

	m_TileOption = ETileOption::Normal;

	m_SideCollisionCheckHandle = GetDlgItem(m_hDlg, IDC_CHECK_SIDECOLLISION);
	m_SideCollision = false;

	return true;
}

void CEditorDlg::Show()
{
	if (m_Open)
	{
		m_Open = false;
		ShowWindow(m_hDlg, SW_HIDE);
	}

	else
	{
		m_Open = true;
		ShowWindow(m_hDlg, SW_SHOW);
	}
}

void CEditorDlg::CreateMap()
{
	BOOL	Transfer = FALSE;

	int	TileCountX = GetDlgItemInt(m_hDlg, IDC_EDIT_TILECOUNTX, &Transfer, TRUE);
	int	TileCountY = GetDlgItemInt(m_hDlg, IDC_EDIT_TILECOUNTY, &Transfer, TRUE);
	int	TileSizeX = GetDlgItemInt(m_hDlg, IDC_EDIT_TILESIZEX, &Transfer, TRUE);
	int	TileSizeY = GetDlgItemInt(m_hDlg, IDC_EDIT_TILESIZEY, &Transfer, TRUE);

	m_Scene->CreateTileMap();

	m_Scene->SetTileInfo(TileCountX, TileCountY, TileSizeX, TileSizeY);
}

void CEditorDlg::LoadTileTexture()
{
	TCHAR	FilePath[MAX_PATH] = {};

	OPENFILENAME	OpenFile = {};

	OpenFile.lStructSize = sizeof(OPENFILENAME);
	OpenFile.hwndOwner = m_hDlg;
	OpenFile.lpstrFilter = TEXT("모든파일\0*.*\0BmpFile\0*.bmp");
	OpenFile.lpstrFile = FilePath;
	OpenFile.nMaxFile = MAX_PATH;
	OpenFile.lpstrInitialDir = CPathManager::GetInst()->FindPath(TEXTURE_PATH)->Path;

	if (GetOpenFileName(&OpenFile) != 0)
	{
		// 전체 경로에서 파일 이름만 얻어온다.
		TCHAR	FileName[128] = {};

		_wsplitpath_s(FilePath, nullptr, 0, nullptr, 0, FileName, 128,
			nullptr, 0);

		char	TextureName[256] = {};

#ifdef UNICODE
		// 유니코드 문자열을 멀티바이트 문자열로 변환한다.
		int ConvertLength = WideCharToMultiByte(CP_ACP, 0, FileName, -1, nullptr, 0, 0, 0);

		WideCharToMultiByte(CP_ACP, 0, FileName, -1,
			TextureName, ConvertLength, 0, 0);
#else
		strcpy_s(TextureName, FileName);
#endif // UNICODE


		if (!m_Scene->GetSceneResource()->LoadTextureFullPath(TextureName,
			FilePath))
			return;

		CTexture* Texture = m_Scene->GetSceneResource()->FindTexture(TextureName);

		Texture->SetColorKey(255, 0, 255);

		SendMessage(m_TextureListBox, LB_ADDSTRING, 0, (LPARAM)FileName);

		TileTextureFrameData	data;
		m_vecTextureFrameData.push_back(data);
	}
}

void CEditorDlg::SelectTexture()
{
	if (m_SelectTextureListIndex != -1)
	{
		char	TextureName[256] = {};

#ifdef UNICODE
		// 유니코드 문자열을 멀티바이트 문자열로 변환한다.
		int ConvertLength = WideCharToMultiByte(CP_ACP, 0, m_SelectTextureListText, -1, nullptr, 0, 0, 0);

		WideCharToMultiByte(CP_ACP, 0, m_SelectTextureListText, -1,
			TextureName, ConvertLength, 0, 0);
#else
		strcpy_s(TextureName, FileName);
#endif // UNICODE

		m_SelectTileTexture = m_Scene->GetSceneResource()->FindTexture(TextureName);

		m_Scene->CreateTileMap();

		m_Scene->SetTileTexture(m_SelectTileTexture);
	}
}

void CEditorDlg::SelectList()
{
	m_SelectTextureListIndex = (int)SendMessage(m_TextureListBox, LB_GETCURSEL, 0, 0);

	if (m_SelectTextureListIndex != -1)
	{
		memset(m_SelectTextureListText, 0, sizeof(TCHAR) * 256);
		SendMessage(m_TextureListBox, LB_GETTEXT, m_SelectTextureListIndex,
			(LPARAM)m_SelectTextureListText);

		int	Count = (int)SendMessage(m_FrameListBox, LB_GETCOUNT, 0, 0);

		for (int i = 0; i < Count; ++i)
		{
			SendMessage(m_FrameListBox, LB_DELETESTRING, 0, 0);
		}

		for (size_t i = 0; i < m_vecTextureFrameData[m_SelectTextureListIndex].vecData.size(); ++i)
		{
			TCHAR	Text[32] = {};

			wsprintf(Text, TEXT("%d"), (int)i);

			SendMessage(m_FrameListBox, LB_ADDSTRING, 0, (LPARAM)Text);
		}

		m_SelectFrameIndex = -1;
	}
}

void CEditorDlg::AddFrame()
{
	if (m_SelectTextureListIndex == -1)
		return;

	BOOL	Transfer = FALSE;

	int	StartFrameX = GetDlgItemInt(m_hDlg, IDC_EDIT_STARTFRAMEX, &Transfer, TRUE);
	int	StartFrameY = GetDlgItemInt(m_hDlg, IDC_EDIT_STARTFRAMEY, &Transfer, TRUE);
	int	EndFrameX = GetDlgItemInt(m_hDlg, IDC_EDIT_ENDFRAMEX, &Transfer, TRUE);
	int	EndFrameY = GetDlgItemInt(m_hDlg, IDC_EDIT_ENDFRAMEY, &Transfer, TRUE);

	TileFrameData	Data;
	Data.Start.x = (float)StartFrameX;
	Data.Start.y = (float)StartFrameY;
	Data.End.x = (float)EndFrameX;
	Data.End.y = (float)EndFrameY;

	TCHAR	Text[32] = {};

	wsprintf(Text, TEXT("%d"), 
		(int)m_vecTextureFrameData[m_SelectTextureListIndex].vecData.size());

	SendMessage(m_FrameListBox, LB_ADDSTRING, 0, (LPARAM)Text);

	m_vecTextureFrameData[m_SelectTextureListIndex].vecData.push_back(Data);
}

void CEditorDlg::DeleteFrame()
{
}

void CEditorDlg::ModifyFrame()
{
}

void CEditorDlg::ChangeFrame()
{
	m_SelectFrameIndex = (int)SendMessage(m_FrameListBox, LB_GETCURSEL, 0, 0);

	if (m_SelectFrameIndex != -1)
	{
		TileFrameData	Data = m_vecTextureFrameData[m_SelectTextureListIndex].vecData[m_SelectFrameIndex];

		SetDlgItemInt(m_hDlg, IDC_EDIT_STARTFRAMEX, (int)Data.Start.x, TRUE);
		SetDlgItemInt(m_hDlg, IDC_EDIT_STARTFRAMEY, (int)Data.Start.y, TRUE);

		SetDlgItemInt(m_hDlg, IDC_EDIT_ENDFRAMEX, (int)Data.End.x, TRUE);
		SetDlgItemInt(m_hDlg, IDC_EDIT_ENDFRAMEY, (int)Data.End.y, TRUE);
	}
}

void CEditorDlg::Save()
{
	TCHAR	FilePath[MAX_PATH] = {};

	OPENFILENAME	OpenFile = {};

	OpenFile.lStructSize = sizeof(OPENFILENAME);
	OpenFile.hwndOwner = m_hDlg;
	OpenFile.lpstrFilter = TEXT("모든파일\0*.*\0MapFile\0*.map");
	OpenFile.lpstrFile = FilePath;
	OpenFile.nMaxFile = MAX_PATH;
	OpenFile.lpstrInitialDir = CPathManager::GetInst()->FindPath(MAP_PATH)->Path;

	if (GetSaveFileName(&OpenFile) != 0)
	{

		char	FullPath[MAX_PATH] = {};

#ifdef UNICODE
		// 유니코드 문자열을 멀티바이트 문자열로 변환한다.
		int ConvertLength = WideCharToMultiByte(CP_ACP, 0, FilePath, -1,
			nullptr, 0, 0, 0);

		WideCharToMultiByte(CP_ACP, 0, FilePath, -1,
			FullPath, ConvertLength, 0, 0);
#else
		strcpy_s(FullPath, FilePath);
#endif // UNICODE

		m_Scene->Save(FullPath);
	}
}

void CEditorDlg::Load()
{
	TCHAR	FilePath[MAX_PATH] = {};

	OPENFILENAME	OpenFile = {};

	OpenFile.lStructSize = sizeof(OPENFILENAME);
	OpenFile.hwndOwner = m_hDlg;
	OpenFile.lpstrFilter = TEXT("모든파일\0*.*\0MapFile\0*.map");
	OpenFile.lpstrFile = FilePath;
	OpenFile.nMaxFile = MAX_PATH;
	OpenFile.lpstrInitialDir = CPathManager::GetInst()->FindPath(MAP_PATH)->Path;

	if (GetOpenFileName(&OpenFile) != 0)
	{

		char	FullPath[MAX_PATH] = {};

#ifdef UNICODE
		// 유니코드 문자열을 멀티바이트 문자열로 변환한다.
		int ConvertLength = WideCharToMultiByte(CP_ACP, 0, FilePath, -1,
			nullptr, 0, 0, 0);

		WideCharToMultiByte(CP_ACP, 0, FilePath, -1,
			FullPath, ConvertLength, 0, 0);
#else
		strcpy_s(FullPath, FilePath);
#endif // UNICODE

		m_Scene->Load(FullPath);
	}
}

void CEditorDlg::TileImageAllClear()
{
	m_Scene->TileImageAllClear();
}

void CEditorDlg::SideCollisionCheck()
{
	if (SendMessage(m_SideCollisionCheckHandle, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
	{
		m_SideCollision = false;
	}

	else
	{
		m_SideCollision = true;
	}
}

LRESULT CEditorDlg::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_LIST_TILETEXTURE:
			switch (HIWORD(wParam))
			{
			case LBN_SELCHANGE:
			{
				g_Dlg->SelectList();
			}
				break;
			}
			break;
		case IDC_LIST_TEXTUREFRAME:
			switch (HIWORD(wParam))
			{
			case LBN_SELCHANGE:
			{
				g_Dlg->ChangeFrame();
			}
			break;
			}
			break;
			break;
		case IDOK:
			break;
		case IDCANCEL:
			break;
		case IDC_BUTTON_CREATEMAP:
			//MessageBox(0, TEXT("맵 생성"), TEXT("맵 생성"), MB_OK);
			g_Dlg->CreateMap();
			break;
		case IDC_BUTTON_TILETEXTURELOAD:
			g_Dlg->LoadTileTexture();
			break;
		case IDC_BUTTON_SETTEXTURE:
			g_Dlg->SelectTexture();
			break;
		case IDC_BUTTON_ADDFRAME:
			g_Dlg->AddFrame();
			break;
		case IDC_BUTTON_DELETEFRAME:
			g_Dlg->DeleteFrame();
			break;
		case IDC_BUTTON_MODIFYFRAME:
			g_Dlg->ModifyFrame();
			break;
		case IDC_BUTTON_SAVE:
			g_Dlg->Save();
			break;
		case IDC_BUTTON_LOAD:
			g_Dlg->Load();
			break;
		case IDC_BUTTON_IMAGECLEAR:
			g_Dlg->TileImageAllClear();
			break;
		case IDC_CHECK_SIDECOLLISION:
			g_Dlg->SideCollisionCheck();
			break;
		}
		break;
	default:
		break;
	}

	return 0;
}
