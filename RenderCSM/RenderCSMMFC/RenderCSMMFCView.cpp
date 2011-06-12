// RenderCSMMFCView.cpp : implementation of the CRenderCSMMFCView class
//

#include "stdafx.h"
#include "RenderCSMMFC.h"

#include "RenderCSMMFCDoc.h"
#include "RenderCSMMFCView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CRenderCSMMFCView

IMPLEMENT_DYNCREATE(CRenderCSMMFCView, CView)

BEGIN_MESSAGE_MAP(CRenderCSMMFCView, CView)
	ON_WM_CREATE()
	ON_WM_KEYDOWN()
	ON_WM_MOUSEWHEEL()
	ON_COMMAND(ID_SCENE_WIREFRAME, &CRenderCSMMFCView::OnSceneWireframe)
	ON_COMMAND(ID_FILE_OPEN, &CRenderCSMMFCView::OnFileOpen)
	ON_COMMAND(ID_FILE_OPEN_MERGING, &CRenderCSMMFCView::OnFileOpenMerging)
	ON_COMMAND(ID_VIEW_ANIMATIONDIALOG, &CRenderCSMMFCView::OnViewAnimationdialog)
	ON_COMMAND(ID_VIEW_MODELINFODIALOG, &CRenderCSMMFCView::OnViewModelinfodialog)
	ON_COMMAND(ID_SCENE_NORMAL, &CRenderCSMMFCView::OnSceneNormal)
END_MESSAGE_MAP()

// CRenderCSMMFCView construction/destruction

CRenderCSMMFCView::CRenderCSMMFCView()
{
	// TODO: add construction code here
	m_pDXManager = NULL;
	m_pAnimDlg = NULL;
	m_pModelInfoDlg = NULL;
}

CRenderCSMMFCView::~CRenderCSMMFCView()
{
	SAFE_DELETE( m_pModelInfoDlg );
	SAFE_DELETE( m_pAnimDlg );
}

BOOL CRenderCSMMFCView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CRenderCSMMFCView drawing

void CRenderCSMMFCView::OnDraw(CDC* /*pDC*/)
{
	CRenderCSMMFCDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CRenderCSMMFCView diagnostics

#ifdef _DEBUG
void CRenderCSMMFCView::AssertValid() const
{
	CView::AssertValid();
}

void CRenderCSMMFCView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CRenderCSMMFCDoc* CRenderCSMMFCView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CRenderCSMMFCDoc)));
	return (CRenderCSMMFCDoc*)m_pDocument;
}
#endif //_DEBUG


// CRenderCSMMFCView message handlers


int CRenderCSMMFCView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	m_pDXManager = DXMANAGER;
	m_hWnd = this->GetSafeHwnd();
	m_pDXManager->Initialize( &m_hWnd );
	m_pDXManager->ChangeState( SceneState_Nothing::Instance() );

	m_pAnimDlg = new AnimDialog( m_pDXManager );
	if ( m_pAnimDlg->GetSafeHwnd() == 0 )
	{
		m_pAnimDlg->Create( AnimDialog::IDD );
	}

	m_pModelInfoDlg = new ModelInfoDialog( m_pDXManager );
	if ( m_pModelInfoDlg->GetSafeHwnd() == 0 )
	{
		m_pModelInfoDlg->Create( ModelInfoDialog::IDD );
	}

	return 0;
}

void CRenderCSMMFCView::OnFrame(void)
{
	if ( m_pDXManager )
	{
		m_pDXManager->RenderScene();
	}
}

void CRenderCSMMFCView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	m_pDXManager->ProcessKeyDown( nChar, nRepCnt, nFlags );

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

BOOL CRenderCSMMFCView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: Add your message handler code here and/or call default
	m_pDXManager->ProcessMouseWheel( nFlags, zDelta, pt );

	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

void CRenderCSMMFCView::OnSceneWireframe()
{
	// TODO: Add your command handler code here
	
	CMenu *pMenu = AfxGetMainWnd()->GetMenu();
	if(pMenu)
	{
		UINT state = pMenu->GetMenuState( ID_SCENE_WIREFRAME, MF_BYCOMMAND );
		if( state == 0xFFFFFFFF )
			return;

		if ( state & MF_CHECKED )
		{
			m_pDXManager->SetFillMode( D3D10_FILL_SOLID );
			pMenu->CheckMenuItem(ID_SCENE_WIREFRAME, MF_UNCHECKED | MF_BYCOMMAND);
		}
		else
		{
			m_pDXManager->SetFillMode( D3D10_FILL_WIREFRAME );
			pMenu->CheckMenuItem(ID_SCENE_WIREFRAME, MF_CHECKED | MF_BYCOMMAND);
		}

	}
}

void CRenderCSMMFCView::OnSceneNormal()
{
	// TODO: Add your command handler code here

	CMenu *pMenu = AfxGetMainWnd()->GetMenu();
	if(pMenu)
	{
		UINT state = pMenu->GetMenuState( ID_SCENE_NORMAL, MF_BYCOMMAND );
		if( state == 0xFFFFFFFF )
			return;

		if ( state & MF_CHECKED )
		{
			m_pDXManager->SetDrawNormal( FALSE );
			pMenu->CheckMenuItem(ID_SCENE_NORMAL, MF_UNCHECKED | MF_BYCOMMAND );
		}
		else
		{
			m_pDXManager->SetDrawNormal( TRUE );
			pMenu->CheckMenuItem(ID_SCENE_NORMAL, MF_CHECKED | MF_BYCOMMAND );
		}

	}
}

void CRenderCSMMFCView::OnFileOpen()
{
	// TODO: Add your command handler code here
	CFileDialog fOpenDlg( TRUE, "CSM", "", OFN_HIDEREADONLY|OFN_FILEMUSTEXIST, 
		"CSM Model File (*.CSM)|*.csm||", this );

	fOpenDlg.m_pOFN->lpstrTitle="Open CSM File";

	fOpenDlg.m_pOFN->lpstrInitialDir = "..";

	if( fOpenDlg.DoModal() == IDOK )
	{
		// Do something useful here
		string fileName = fOpenDlg.GetPathName();

		if ( TRUE == m_pDXManager->LoadEntireMesh( fileName ) )
		{
			m_pDXManager->ChangeState( SceneState_EntireModel::Instance() );
			m_pAnimDlg->LoadAnimList();
			m_pModelInfoDlg->LoadModelInfo();
		}
	}
}

void CRenderCSMMFCView::OnFileOpenMerging()
{
	// TODO: Add your command handler code here
	CFileDialog fOpenDlg( TRUE, "CSM_PAC", "", OFN_HIDEREADONLY|OFN_FILEMUSTEXIST, 
		"CSM Package File (*.CSM_PAC)|*.csm_pac||", this );

	fOpenDlg.m_pOFN->lpstrTitle="Open CSM Package File";

	fOpenDlg.m_pOFN->lpstrInitialDir = "..";

	if( fOpenDlg.DoModal() == IDOK )
	{
		// Do something useful here
		string fileName = fOpenDlg.GetPathName();

		if ( TRUE == m_pDXManager->LoadMergingMesh( fileName ) )
		{
			m_pDXManager->ChangeState( SceneState_MergingModel::Instance() );
			m_pAnimDlg->LoadAnimList();
			m_pModelInfoDlg->LoadModelInfo();
		}
	}
}

void CRenderCSMMFCView::OnViewAnimationdialog()
{
	// TODO: Add your command handler code here
	CMenu *pMenu = AfxGetMainWnd()->GetMenu();
	if(pMenu)
	{
		UINT state = pMenu->GetMenuState( ID_VIEW_ANIMATIONDIALOG, MF_BYCOMMAND );
		if( state == 0xFFFFFFFF )
			return;

		if ( state & MF_CHECKED )
		{
			m_pAnimDlg->ShowWindow( FALSE );
			pMenu->CheckMenuItem(ID_VIEW_ANIMATIONDIALOG, MF_UNCHECKED | MF_BYCOMMAND);
		}
		else
		{
			m_pAnimDlg->ShowWindow( TRUE );
			pMenu->CheckMenuItem(ID_VIEW_ANIMATIONDIALOG, MF_CHECKED | MF_BYCOMMAND);
		}
	}
}

void CRenderCSMMFCView::OnViewModelinfodialog()
{
	// TODO: Add your command handler code here
	CMenu *pMenu = AfxGetMainWnd()->GetMenu();
	if(pMenu)
	{
		UINT state = pMenu->GetMenuState( ID_VIEW_MODELINFODIALOG, MF_BYCOMMAND );
		if( state == 0xFFFFFFFF )
			return;

		if ( state & MF_CHECKED )
		{
			m_pModelInfoDlg->ShowWindow( FALSE );
			pMenu->CheckMenuItem(ID_VIEW_MODELINFODIALOG, MF_UNCHECKED | MF_BYCOMMAND);
		}
		else
		{
			m_pModelInfoDlg->ShowWindow( TRUE );
			pMenu->CheckMenuItem(ID_VIEW_MODELINFODIALOG, MF_CHECKED | MF_BYCOMMAND);
		}
	}
}
