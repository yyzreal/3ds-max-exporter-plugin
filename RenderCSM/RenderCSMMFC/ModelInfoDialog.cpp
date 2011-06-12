// ModelInfoDialog.cpp : implementation file
//

#include "stdafx.h"
#include "RenderCSMMFC.h"
#include "ModelInfoDialog.h"


// ModelInfoDialog dialog

IMPLEMENT_DYNAMIC(ModelInfoDialog, CDialog)

ModelInfoDialog::ModelInfoDialog(DXManager *pDM, CWnd* pParent /*=NULL*/)
: CDialog(ModelInfoDialog::IDD, pParent),
m_pDM( pDM )
{

}

ModelInfoDialog::~ModelInfoDialog()
{
}

void ModelInfoDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

void ModelInfoDialog::LoadModelInfo()
{
	ModelBase *pMesh = m_pDM->GetMesh();
	if ( pMesh != NULL )
	{
		stringstream ss;
		string strNumVtx,strNumUV, strNumTri;
		ss << pMesh->GetTotalNumVtx();
		ss >> strNumVtx;
		ss.clear();
		ss << pMesh->GetTotalNumUV();
		ss >> strNumUV;
		ss.clear();
		ss << pMesh->GetTotalNumTri();
		ss >> strNumTri;

		SetDlgItemText( IDC_STATIC_VTX, strNumVtx.c_str() );
		SetDlgItemText( IDC_STATIC_UV, strNumUV.c_str() );
		SetDlgItemText( IDC_STATIC_FACE, strNumTri.c_str() );

		CTreeCtrl *pModelTree = static_cast< CTreeCtrl* >( GetDlgItem( IDC_TREE_MODEL ) );
	}
}

BEGIN_MESSAGE_MAP(ModelInfoDialog, CDialog)
END_MESSAGE_MAP()


// ModelInfoDialog message handlers
