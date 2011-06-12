/**********************************************************************
 *<
	FILE: CrowdSimulationModelExporter.cpp

	DESCRIPTION:	Appwizard generated plugin

	CREATED BY: 

	HISTORY: 

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/
#ifndef __CSMExport__CPP
#define __CSMExport__CPP

#include "CrowdSimulationModelExporter.h"
#include "CrowdSimulationModel.h"
#include "CSMOpts.h"

#define CSMExport_CLASS_ID	Class_ID(0x8ba2349a, 0x89d42ed3)

class CSMExport : public SceneExport {
	public:
		
		static HWND hParams;
		
		int				ExtCount();					// Number of extensions supported
		const TCHAR *	Ext(int n);					// Extension #n (i.e. "3DS")
		const TCHAR *	LongDesc();					// Long ASCII description (i.e. "Autodesk 3D Studio File")
		const TCHAR *	ShortDesc();				// Short ASCII description (i.e. "3D Studio")
		const TCHAR *	AuthorName();				// ASCII Author name
		const TCHAR *	CopyrightMessage();			// ASCII Copyright message
		const TCHAR *	OtherMessage1();			// Other message #1
		const TCHAR *	OtherMessage2();			// Other message #2
		unsigned int	Version();					// Version number * 100 (i.e. v3.01 = 301)
		void			ShowAbout(HWND hWnd);		// Show DLL's "About..." box

		BOOL SupportsOptions(int ext, DWORD options);
		int				DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0);

		void GetSceneObjList( INode *pNode );

		//Constructor/Destructor
		CSMExport();
		~CSMExport();

	private:
		// Our options
		CSMOpts *m_Opts;
		
		vector< string > m_objList;
};



class CSMExportClassDesc : public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { return new CSMExport(); }
	const TCHAR *	ClassName() { return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID() { return SCENE_EXPORT_CLASS_ID; }
	Class_ID		ClassID() { return CSMExport_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }

	const TCHAR*	InternalName() { return _T("CSMExport"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }					// returns owning module handle
	

};

static CSMExportClassDesc CSMExportDesc;
ClassDesc2* GetCSMExportDesc() { return &CSMExportDesc; }





INT_PTR CALLBACK CSMExportOptionsDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {
	static CSMExport *imp = NULL;

	switch(message) {
		case WM_INITDIALOG:
			imp = (CSMExport *)lParam;
			CenterWindow(hWnd,GetParent(hWnd));
			return TRUE;

		case WM_CLOSE:
			EndDialog(hWnd, 0);
			return 1;
	}
	return 0;
}


//--- CSMExport -------------------------------------------------------
CSMExport::CSMExport()
{
	m_Opts = NULL;
}

CSMExport::~CSMExport() 
{
	SAFE_DELETE( m_Opts );
}

int CSMExport::ExtCount()
{
	//TODO: Returns the number of file name extensions supported by the plug-in.
	return 1;
}

const TCHAR *CSMExport::Ext(int n)
{		
	//TODO: Return the 'i-th' file name extension (i.e. "3DS").
	return _T("csm");
}

const TCHAR *CSMExport::LongDesc()
{
	//TODO: Return long ASCII description (i.e. "Targa 2.0 Image File")
	return _T("Crowd Simulation Model 1.0 File");
}
	
const TCHAR *CSMExport::ShortDesc() 
{			
	//TODO: Return short ASCII description (i.e. "Targa")
	return _T("Crowd Simulation");
}

const TCHAR *CSMExport::AuthorName()
{			
	//TODO: Return ASCII Author name
	return _T("Benny Chen");
}

const TCHAR *CSMExport::CopyrightMessage() 
{	
	// Return ASCII Copyright message
	return _T("Copyright@BIT Software. All Right Reserved");
}

const TCHAR *CSMExport::OtherMessage1() 
{		
	//TODO: Return Other message #1 if any
	return _T("");
}

const TCHAR *CSMExport::OtherMessage2() 
{		
	//TODO: Return other message #2 in any
	return _T("");
}

unsigned int CSMExport::Version()
{				
	//TODO: Return Version number * 100 (i.e. v3.01 = 301)
	return 100;
}

void CSMExport::ShowAbout(HWND hWnd)
{			
	// Optional
}

BOOL CSMExport::SupportsOptions(int ext, DWORD options)
{
	// TODO Decide which options to support.  Simply return
	// true for each option supported by each Extension 
	// the exporter supports.

	return TRUE;
}


int	CSMExport::DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts, DWORD options)
{
	//TODO: Implement the actual file Export here and 
	//		return TRUE If the file is exported properly

	// 根节点
	INode *rootNode = i->GetRootNode();
	
	// 场景的节点列表
	GetSceneObjList( rootNode );

	// 场景的帧信息
	Interval ivAnimRange = GetCOREInterface()->GetAnimRange();
	int firstFrame = ivAnimRange.Start() / GetTicksPerFrame();
	int lastFrame = ivAnimRange.End() / GetTicksPerFrame();

	// 弹出选项对话框
	m_Opts = new CSMOpts( firstFrame, lastFrame, m_objList );
	if ( !suppressPrompts && !m_Opts->ShowOptionDialog() )
	{
		// User cancelled, return TRUE
		// Otherwise Max displays export failed dialog
		return TRUE;
	}

	ExportManager pEM;

	// 搜集3D MAX场景信息
	pEM.Gather3DSMAXSceneData( rootNode, m_Opts->IfExportAnim(), 
							   m_Opts->IfExportTags(), 
							   m_Opts->m_sHeadNode, 
							   m_Opts->m_sUpperNode, 
							   m_Opts->m_sLowerNode, 
							   m_Opts->IfExprotProperty(), 
							   m_Opts->m_sPropNode,
							   m_Opts->m_propMountTo );

	// 写出CSM模型文件
	if ( FALSE == pEM.WriteAllCSMFile( name ) )
	{
		return FALSE;
	}
	if ( TRUE == m_Opts->IfExportAnim() )
	{
		// 装入动画信息
		int animCount = m_Opts->GetAnimRecordCount();
		for ( int i = 0; i < animCount; i ++ )
		{
			AnimRecord anim = m_Opts->GetAnimRecord( i );
			CSMAnimation CSMAnim;
			CSMAnim.animName = anim.animName;
			CSMAnim.firstFrame = anim.firstFrame;
			CSMAnim.numFrames = anim.lastFrame - anim.firstFrame + 1;
			CSMAnim.numLoops = 1;
			CSMAnim.fps = GetFrameRate();
			pEM.AddAnim( CSMAnim );
		}

		// 写出动画文件
		if ( FALSE == pEM.WriteAMFile( name ) )
		{
			return FALSE;
		}
	}

	return TRUE;
}

void CSMExport::GetSceneObjList( INode *pNode )
{
	// 获得BaseObject
	ObjectState os = pNode->EvalWorldState( 0 );
	Object *pObj = os.obj;

	if ( pObj != NULL &&
		FALSE == pNode->IsNodeHidden() &&
		pObj->CanConvertToType( triObjectClassID ) )
	{
		string name = pNode->GetName();
		m_objList.push_back( name );
	}

	int numChildren = pNode->NumberOfChildren();
	for ( int i = 0; i < numChildren; i++ )
	{
		INode *childNode = pNode->GetChildNode( i );
		GetSceneObjList( childNode );
	}
}

#endif