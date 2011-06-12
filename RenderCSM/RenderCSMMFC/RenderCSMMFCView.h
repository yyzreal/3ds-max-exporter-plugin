// RenderCSMMFCView.h : interface of the CRenderCSMMFCView class
//


#pragma once

#include "DXManager.h"
#include "AnimDialog.h"
#include "ModelInfoDialog.h"

class CRenderCSMMFCView : public CView
{
protected: // create from serialization only
	CRenderCSMMFCView();
	DECLARE_DYNCREATE(CRenderCSMMFCView)

// Attributes
public:
	CRenderCSMMFCDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~CRenderCSMMFCView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

private:
	DXManager *m_pDXManager;
	AnimDialog *m_pAnimDlg;
	ModelInfoDialog *m_pModelInfoDlg;

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
public:
	void OnFrame(void);
	HWND m_hWnd;
public:
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
public:
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
public:
	afx_msg void OnSceneWireframe();
public:
	afx_msg void OnFileOpen();
public:
	afx_msg void OnFileOpenMerging();
public:
	afx_msg void OnViewAnimationdialog();
public:
	afx_msg void OnViewModelinfodialog();
public:
	afx_msg void OnSceneNormal();
};

#ifndef _DEBUG  // debug version in RenderCSMMFCView.cpp
inline CRenderCSMMFCDoc* CRenderCSMMFCView::GetDocument() const
   { return reinterpret_cast<CRenderCSMMFCDoc*>(m_pDocument); }
#endif

