// RenderCSMMFCDoc.cpp : implementation of the CRenderCSMMFCDoc class
//

#include "stdafx.h"
#include "RenderCSMMFC.h"

#include "RenderCSMMFCDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CRenderCSMMFCDoc

IMPLEMENT_DYNCREATE(CRenderCSMMFCDoc, CDocument)

BEGIN_MESSAGE_MAP(CRenderCSMMFCDoc, CDocument)
END_MESSAGE_MAP()


// CRenderCSMMFCDoc construction/destruction

CRenderCSMMFCDoc::CRenderCSMMFCDoc()
{
	// TODO: add one-time construction code here

}

CRenderCSMMFCDoc::~CRenderCSMMFCDoc()
{
}

BOOL CRenderCSMMFCDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CRenderCSMMFCDoc serialization

void CRenderCSMMFCDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}


// CRenderCSMMFCDoc diagnostics

#ifdef _DEBUG
void CRenderCSMMFCDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CRenderCSMMFCDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CRenderCSMMFCDoc commands
