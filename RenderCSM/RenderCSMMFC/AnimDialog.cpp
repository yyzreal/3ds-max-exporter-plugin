// AnimDialog.cpp : implementation file
//

#include "stdafx.h"
#include "RenderCSMMFC.h"
#include "AnimDialog.h"


// AnimDialog dialog

IMPLEMENT_DYNAMIC(AnimDialog, CDialog)

AnimDialog::AnimDialog(DXManager *pDM, CWnd* pParent /*=NULL*/)
: CDialog(AnimDialog::IDD, pParent),
m_pDM( pDM )
{
}

AnimDialog::~AnimDialog()
{
}

void AnimDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(AnimDialog, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_PLAY, &AnimDialog::OnBnClickedButtonPlay)
	ON_BN_CLICKED(IDC_BUTTON_PAUSE, &AnimDialog::OnBnClickedButtonPause)
	ON_BN_CLICKED(IDC_BUTTON_PRE_FRAME, &AnimDialog::OnBnClickedButtonPreFrame)
	ON_BN_CLICKED(IDC_BUTTON_FIRST_FRAME, &AnimDialog::OnBnClickedButtonFirstFrame)
	ON_BN_CLICKED(IDC_BUTTON_NEXT_FRAME, &AnimDialog::OnBnClickedButtonNextFrame)
	ON_BN_CLICKED(IDC_BUTTON_LAST_FRAME, &AnimDialog::OnBnClickedButtonLastFrame)
	ON_LBN_SELCHANGE(IDC_LIST_ANIM, &AnimDialog::OnLbnSelchangeListAnim)
	ON_WM_CLOSE()
	ON_WM_MOUSEWHEEL()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()


// AnimDialog message handlers

void AnimDialog::EnableAnimControls( BOOL b )
{
	GetDlgItem( IDC_LIST_ANIM )->EnableWindow( b );
	GetDlgItem( IDC_BUTTON_PLAY )->EnableWindow( b );
	GetDlgItem( IDC_BUTTON_PAUSE )->EnableWindow( b );
	GetDlgItem( IDC_BUTTON_FIRST_FRAME )->EnableWindow( b );
	GetDlgItem( IDC_BUTTON_NEXT_FRAME )->EnableWindow( b );
	GetDlgItem( IDC_BUTTON_PRE_FRAME )->EnableWindow( b );
	GetDlgItem( IDC_BUTTON_LAST_FRAME )->EnableWindow( b );
}

void AnimDialog::ClearAnimList()
{
	CListBox *pAnimList = static_cast< CListBox* >( GetDlgItem( IDC_LIST_ANIM ) );
	pAnimList->ResetContent();
}

void AnimDialog::LoadAnimList()
{
	CListBox *pAnimList = static_cast< CListBox* >( GetDlgItem( IDC_LIST_ANIM ) );

	ClearAnimList();

	ModelBase *pMB = m_pDM->GetMesh();
	AnimatableBase *pMesh = dynamic_cast< AnimatableBase* >( pMB );
	if ( pMesh != NULL )
	{
		int animNum = pMesh->GetAnimNum();
		for ( int i = 0; i < animNum; i ++ )
		{
			CSMAnimation anim = pMesh->GetAnimInfo( i );

			pAnimList->AddString( anim.animName.c_str() );
		}
		if ( animNum == 0 )
		{
			EnableAnimControls( FALSE );
		}
		else
		{
			EnableAnimControls( TRUE );
			pAnimList->SetCurSel( 0 );
		}
	}
}

void AnimDialog::OnBnClickedButtonPlay()
{
	// TODO: Add your control notification handler code here
	m_pDM->Pause( FALSE );
}

void AnimDialog::OnBnClickedButtonPause()
{
	// TODO: Add your control notification handler code here
	m_pDM->Pause( TRUE );
}

void AnimDialog::OnBnClickedButtonPreFrame()
{
	// TODO: Add your control notification handler code here
	ModelBase *pMB = m_pDM->GetMesh();
	AnimatableBase *pMesh = dynamic_cast< AnimatableBase* >( pMB );
	if ( pMesh != NULL )
	{
		pMesh->SetFrame( pMesh->GetCurFrame() - 1 );
	}
}

void AnimDialog::OnBnClickedButtonFirstFrame()
{
	// TODO: Add your control notification handler code here
	ModelBase *pMB = m_pDM->GetMesh();
	AnimatableBase *pMesh = dynamic_cast< AnimatableBase* >( pMB );
	if ( pMesh != NULL )
	{
		pMesh->SetFrame( pMesh->GetCurAnimState().startframe );
	}
}

void AnimDialog::OnBnClickedButtonNextFrame()
{
	// TODO: Add your control notification handler code here
	ModelBase *pMB = m_pDM->GetMesh();
	AnimatableBase *pMesh = dynamic_cast< AnimatableBase* >( pMB );
	if ( pMesh != NULL )
	{
		pMesh->SetFrame( pMesh->GetCurFrame() + 1 );
	}
}

void AnimDialog::OnBnClickedButtonLastFrame()
{
	// TODO: Add your control notification handler code here
	ModelBase *pMB = m_pDM->GetMesh();
	AnimatableBase *pMesh = dynamic_cast< AnimatableBase* >( pMB );
	if ( pMesh != NULL )
	{
		pMesh->SetFrame( pMesh->GetCurAnimState().endframe );
	}
}

void AnimDialog::OnLbnSelchangeListAnim()
{
	// TODO: Add your control notification handler code here
	CListBox *pAnimList = static_cast< CListBox* >( GetDlgItem( IDC_LIST_ANIM ) );
	int nSel = pAnimList->GetCurSel();
	if ( nSel >= 0 )
	{
		CString str;
		pAnimList->GetText( nSel, str );

		string animName = str;

		ModelBase *pMB = m_pDM->GetMesh();
		AnimatableBase *pMesh = dynamic_cast< AnimatableBase* >( pMB );
		if( pMesh != NULL && pMesh->GetCurAnimState().name != animName )
		{
			pMesh->SetAnimation( animName );
		}
	}
}

void AnimDialog::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	CMenu *pMenu = AfxGetMainWnd()->GetMenu();
	if(pMenu)
	{
		UINT state = pMenu->GetMenuState( ID_VIEW_ANIMATIONDIALOG, MF_BYCOMMAND );
		if( state == 0xFFFFFFFF )
			return;

		pMenu->CheckMenuItem(ID_VIEW_ANIMATIONDIALOG, MF_UNCHECKED | MF_BYCOMMAND);
	}

	CDialog::OnClose();
}

BOOL AnimDialog::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: Add your message handler code here and/or call default
	m_pDM->ProcessMouseWheel( nFlags, zDelta, pt );

	return CDialog::OnMouseWheel(nFlags, zDelta, pt);
}

void AnimDialog::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	m_pDM->ProcessKeyDown( nChar, nRepCnt, nFlags );

	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}
