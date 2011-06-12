#pragma once

#include "DXManager.h"

// AnimDialog dialog

class AnimDialog : public CDialog
{
	DECLARE_DYNAMIC(AnimDialog)

public:
	AnimDialog(DXManager *pDM, CWnd* pParent = NULL);   // standard constructor
	virtual ~AnimDialog();

	void ClearAnimList();
	void LoadAnimList();

	// Dialog Data
	enum { IDD = IDD_DIALOG_ANIM };

private:
	DXManager *m_pDM;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	void EnableAnimControls( BOOL b );

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonPlay();
public:
	afx_msg void OnBnClickedButtonPause();
public:
	afx_msg void OnBnClickedButtonPreFrame();
public:
	afx_msg void OnBnClickedButtonFirstFrame();
public:
	afx_msg void OnBnClickedButtonNextFrame();
public:
	afx_msg void OnBnClickedButtonLastFrame();
public:
	afx_msg void OnLbnSelchangeListAnim();
public:
	afx_msg void OnClose();
public:
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
public:
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
};
