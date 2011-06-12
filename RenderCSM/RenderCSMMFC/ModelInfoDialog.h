#pragma once

#include "DXManager.h"

#include <string>
#include <sstream>

using std::string;
using std::stringstream;

// ModelInfoDialog dialog

class ModelInfoDialog : public CDialog
{
	DECLARE_DYNAMIC(ModelInfoDialog)

public:
	ModelInfoDialog(DXManager *pDM, CWnd* pParent = NULL);   // standard constructor
	virtual ~ModelInfoDialog();

	// Dialog Data
	enum { IDD = IDD_DIALOG_MODEL };

	void LoadModelInfo();

private:
	DXManager *m_pDM;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
