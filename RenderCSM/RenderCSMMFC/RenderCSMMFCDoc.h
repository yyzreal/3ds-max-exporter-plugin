// RenderCSMMFCDoc.h : interface of the CRenderCSMMFCDoc class
//


#pragma once


class CRenderCSMMFCDoc : public CDocument
{
protected: // create from serialization only
	CRenderCSMMFCDoc();
	DECLARE_DYNCREATE(CRenderCSMMFCDoc)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CRenderCSMMFCDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};


