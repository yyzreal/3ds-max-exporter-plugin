#include "CSMOpts.h"

HWND CSMOpts::m_hCheckAnim = 0;
HWND CSMOpts::m_hListAnim = 0;
HWND CSMOpts::m_hBtnAddAnim = 0;
HWND CSMOpts::m_hBtnEditAnim = 0;
HWND CSMOpts::m_hBtnDelAnim = 0;

HWND CSMOpts::m_hCheckTag = 0;
HWND CSMOpts::m_hBtnHead = 0;
HWND CSMOpts::m_hBtnUpper = 0;
HWND CSMOpts::m_hBtnLower = 0;
HWND CSMOpts::m_hBtnProp = 0;
HWND CSMOpts::m_hBtnHeadX = 0;
HWND CSMOpts::m_hBtnUpperX = 0;
HWND CSMOpts::m_hBtnLowerX = 0;
HWND CSMOpts::m_hBtnPropX = 0;
HWND CSMOpts::m_hCheckProp = 0;
HWND CSMOpts::m_hComBoxMount = 0;

HWND CSMOpts::m_hEditAnimName = 0;
HWND CSMOpts::m_hEditFirstFrame = 0;
HWND CSMOpts::m_hEditLastFrame = 0;

HWND CSMOpts::m_hListObj = 0;

CSMOpts::CSMOpts( int firstFrame, int lastFrame, vector< string > objList )
{
	m_bExportAnim = m_bExportTags = m_bExportProperty = FALSE;
	m_firstSceneFrame = firstFrame;
	m_lastSceneFrame = lastFrame;
	m_settedFrame = 0;
	m_bIsEditingAnim = FALSE;

	m_objList = objList;
}

CSMOpts::~CSMOpts()
{

}

BOOL CSMOpts::ShowOptionDialog()
{
	// 弹出一个模态对话框，如果返回0，则用户点击了Cancel
	return 0 != DialogBoxParam( hInstance, 
								MAKEINTRESOURCE( IDO_OPTIONS ),
								GetActiveWindow(), 
								OptionDialogProc, ( LPARAM )this );
}

void CSMOpts::EnableAnimControls( BOOL bEnable )
{
	EnableWindow( m_hListAnim, bEnable );
	EnableWindow( m_hBtnAddAnim, bEnable );
	EnableWindow( m_hBtnEditAnim, bEnable );
	EnableWindow( m_hBtnDelAnim, bEnable );
}

void CSMOpts::EnableTagControls( BOOL bEnable )
{
	EnableWindow( m_hBtnHead, bEnable );
	EnableWindow( m_hBtnUpper, bEnable );
	EnableWindow( m_hBtnLower, bEnable );
	EnableWindow( m_hBtnProp, bEnable );
	EnableWindow( m_hBtnHeadX, bEnable );
	EnableWindow( m_hBtnUpperX, bEnable );
	EnableWindow( m_hBtnLowerX, bEnable );
	EnableWindow( m_hCheckProp, bEnable );
	EnableWindow( m_hBtnPropX, m_bExportProperty );
	EnableWindow( m_hComBoxMount, m_bExportProperty );
}

void CSMOpts::EnablePropControls( BOOL bEnable )
{
	EnableWindow( m_hBtnProp, bEnable );
	EnableWindow( m_hBtnPropX, bEnable );
	EnableWindow( m_hComBoxMount, bEnable );
}

void CSMOpts::SetExportAnim( HWND hWnd, BOOL b )
{ 
	m_bExportAnim = b;

	int animCount = SendMessage( m_hListAnim, LVM_GETITEMCOUNT, 0, 0 );
	if ( m_bExportAnim == TRUE && animCount == 0 )
	{
		// 动画所需的信息不完整，不导出动画
		m_bExportAnim = FALSE;

		MessageBox( hWnd, "你选择了导出动画，但你没有添加动画帧信息，将不会导出动画", "警告！", MB_ICONWARNING );
	}
}


void CSMOpts::SetExportTags( HWND hWnd, BOOL b )
{
	m_bExportTags = b;

	char buf[64];
	string strHead, strUpper, strLower;

	ZeroMemory( buf, 64 );
	GetDlgItemText( hWnd, IDC_EDIT_HEAD, buf, 64 );
	strHead = buf;
	ZeroMemory( buf, 64 );
	GetDlgItemText( hWnd, IDC_EDIT_UPPER, buf, 64 );
	strUpper = buf;
	ZeroMemory( buf, 64 );
	GetDlgItemText( hWnd, IDC_EDIT_LOWER, buf, 64 );
	strLower = buf;

	if ( m_bExportTags == TRUE &&
		 ( strHead == "" ||
		   strUpper == "" ||
		   strLower == "" ) )
	{
		// TAG所需的信息不完整，不导出TAG
		m_bExportTags = FALSE;

		MessageBox( hWnd, "你选择了导出TAG信息，但你没有为Head, Upper, Lower全部绑定到场景节点，将不会导出TAG", "警告！", MB_ICONWARNING );
	}
}

void CSMOpts::SetExportProp( HWND hWnd, BOOL b )
{
	m_bExportProperty = b;

	char buf[64];
	string strProp, strMountTo;

	ZeroMemory( buf, 64 );
	GetDlgItemText( hWnd, IDC_EDIT_PROPERTY, buf, 64 );
	strProp = buf;

	ZeroMemory( buf, 64 );
	int curSel = SendMessage( m_hComBoxMount, CB_GETCURSEL, 0, 0 );
	SendMessage( m_hComBoxMount, CB_GETLBTEXT, ( WPARAM )curSel, ( LPARAM )buf );
	strMountTo = buf;

	if ( m_bExportProperty == TRUE && ( m_bExportTags == FALSE || strProp == "" || strMountTo == "" ) )
	{
		// 道具所需的信息不完整，不导出道具
		m_bExportTags = FALSE;

		MessageBox( hWnd, "你选择了导出道具，但道具信息填写不完整，将不会导出道具", "警告！", MB_ICONWARNING );
	}
}

INT_PTR CALLBACK CSMOpts::OptionDialogProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	static CSMOpts *imp = NULL;

	switch( message ) 
	{
		case WM_INITDIALOG:	// 初始化选项对话框
			{
				imp = reinterpret_cast< CSMOpts* >( lParam );
				CenterWindow( hWnd,GetParent( hWnd ) );

				m_hCheckAnim		= GetDlgItem( hWnd, IDC_CHECK_ANIM );
				m_hListAnim		= GetDlgItem( hWnd, IDC_LIST_ANIM );
				m_hBtnAddAnim		= GetDlgItem( hWnd, IDC_BUTTON_ADD );
				m_hBtnEditAnim	= GetDlgItem( hWnd, IDC_BUTTON_EDIT );
				m_hBtnDelAnim		= GetDlgItem( hWnd, IDC_BUTTON_DELETE );

				m_hCheckTag		=	GetDlgItem( hWnd, IDC_CHECK_TAGS );
				m_hBtnHead		=	GetDlgItem( hWnd, IDC_BUTTON_HEAD );
				m_hBtnUpper		=	GetDlgItem( hWnd, IDC_BUTTON_UPPER );
				m_hBtnLower		=	GetDlgItem( hWnd, IDC_BUTTON_LOWER );
				m_hBtnProp		=	GetDlgItem( hWnd, IDC_BUTTON_PROPERTY );
				m_hBtnHeadX		=	GetDlgItem( hWnd, IDC_BUTTON_HEAD_X );
				m_hBtnUpperX	=	GetDlgItem( hWnd, IDC_BUTTON_UPPER_X );
				m_hBtnLowerX	=	GetDlgItem( hWnd, IDC_BUTTON_LOWER_X );
				m_hBtnPropX		=	GetDlgItem( hWnd, IDC_BUTTON_PROPERTY_X );
				m_hCheckProp	=	GetDlgItem( hWnd, IDC_CHECK_PROPERTY );
				m_hComBoxMount	=	GetDlgItem( hWnd, IDC_COMBO_MOUNTTO );

				SendMessage( m_hListAnim, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, ( LPARAM )LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
				// 创建动画列表的列标题
				LVCOLUMN listCol;
				memset( &listCol, 0, sizeof( LVCOLUMN ) );
				listCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT | LVCF_SUBITEM;
				listCol.fmt = LVCFMT_LEFT;
				listCol.pszText = "Name";
				listCol.cchTextMax = sizeof( listCol.pszText );
				listCol.cx = 80;
				listCol.iSubItem = 0;
				SendMessage( m_hListAnim, LVM_INSERTCOLUMN, 0, ( LPARAM )&listCol );
				listCol.pszText = "First Frame";
				listCol.cchTextMax = sizeof( listCol.pszText );
				listCol.cx = 80;
				listCol.iSubItem = 0;
				SendMessage( m_hListAnim, LVM_INSERTCOLUMN, 1, ( LPARAM )&listCol );
				listCol.pszText = "Last Frame";
				listCol.cchTextMax = sizeof( listCol.pszText );
				listCol.cx = 80;
				listCol.iSubItem = 0;
				SendMessage( m_hListAnim, LVM_INSERTCOLUMN, 2, ( LPARAM )&listCol );

				SetCheckBox( hWnd, IDC_CHECK_ANIM, FALSE );
				imp->EnableAnimControls( FALSE );
				
				if ( FALSE )//imp->GetNumObj() < 3 )
				{
					// 不足以导出头部、上身、下身，禁止TAG
					SetCheckBox( hWnd, IDC_CHECK_TAGS, FALSE );
					imp->EnableTagControls( imp->IfExportTags() );

					EnableWindow( m_hCheckTag, FALSE );
				}
				else
				{
					SetCheckBox( hWnd, IDC_CHECK_TAGS, FALSE );
					imp->EnableTagControls( FALSE );

					string str;
					str = "Upper";
					SendMessage( m_hComBoxMount, CB_ADDSTRING, 0, ( LPARAM )str.c_str() );
					str = "Head";
					SendMessage( m_hComBoxMount, CB_ADDSTRING, 0, ( LPARAM )str.c_str() );
					str = "Lower";
					SendMessage( m_hComBoxMount, CB_ADDSTRING, 0, ( LPARAM )str.c_str() );

				}

				return TRUE;
			}

		case WM_COMMAND:
			switch( wParam )
			{
			case IDC_CHECK_ANIM:
				imp->EnableAnimControls( IsDlgButtonChecked( hWnd, IDC_CHECK_ANIM ) );
				break;
			case IDC_CHECK_TAGS:
				imp->EnableTagControls( IsDlgButtonChecked( hWnd, IDC_CHECK_TAGS ) );
				break;
			case IDC_CHECK_PROPERTY:
				imp->EnablePropControls( IsDlgButtonChecked( hWnd, IDC_CHECK_PROPERTY ) );
				break;
			case IDC_BUTTON_ADD:	// 添加动画
				{
					imp->SetEditingAnim( FALSE );
					if ( TRUE == imp->ShowAnimDialog() )
					{
						AnimRecord &newAnim = imp->GetNewAnimRecord();

						int itemCount = SendMessage( m_hListAnim, LVM_GETITEMCOUNT, 0, 0 );
						
						LVITEM listItem;
						memset( &listItem, 0, sizeof( LVITEM ) );

						char buffer[64];
						ZeroMemory( buffer, 64 );
						strcpy( buffer, newAnim.animName.c_str() );
						listItem.mask = LVIF_TEXT;
						listItem.cchTextMax = static_cast< int >( newAnim.animName.size() );
						listItem.pszText = buffer;
						listItem.iItem = itemCount;
						listItem.iSubItem = 0;
						SendMessage( m_hListAnim, LVM_INSERTITEM, 0, ( LPARAM )&listItem );

						itoa( newAnim.firstFrame, buffer, 10 );
						listItem.cchTextMax = sizeof( buffer );
						listItem.pszText = buffer;
						listItem.iItem = itemCount;
						listItem.iSubItem = 1;
						SendMessage( m_hListAnim, LVM_SETITEM, 0, ( LPARAM )&listItem );

						itoa( newAnim.lastFrame, buffer, 10 );
						listItem.cchTextMax = sizeof( buffer );
						listItem.pszText = buffer;
						listItem.iItem = itemCount;
						listItem.iSubItem = 2;
						SendMessage( m_hListAnim, LVM_SETITEM, 0, ( LPARAM )&listItem );

						imp->SetSettedFrame( max( newAnim.lastFrame, imp->GetSettedFrame() ) );
					}
					break;
				}
			case IDC_BUTTON_EDIT:	// 编辑动画
				{
					int nSel = -1;
					nSel = SendMessage( m_hListAnim, LVM_GETSELECTIONMARK, 0, 0 );
					int nSelCount = SendMessage( m_hListAnim, LVM_GETSELECTEDCOUNT, 0, 0 );
					if ( nSelCount != 0 )
					{
						AnimRecord curAnim;

						LVITEM listItem;
						memset( &listItem, 0, sizeof( LVITEM ) );

						char buffer[64];
						ZeroMemory( buffer, 64 );
						listItem.mask = LVIF_TEXT;
						listItem.iItem = nSel;
						listItem.cchTextMax = 64;
						listItem.pszText = buffer;
						listItem.iSubItem = 0;
						SendMessage( m_hListAnim, LVM_GETITEM, 0, ( LPARAM )&listItem );
						curAnim.animName = listItem.pszText;

						listItem.iSubItem = 1;
						SendMessage( m_hListAnim, LVM_GETITEM, 0, ( LPARAM )&listItem );
						curAnim.firstFrame = atoi( listItem.pszText );
						
						listItem.iSubItem = 2;
						SendMessage( m_hListAnim, LVM_GETITEM, 0, ( LPARAM )&listItem );
						curAnim.lastFrame = atoi( listItem.pszText );

						imp->SetCurAnimRecord( curAnim.animName, curAnim.firstFrame, curAnim.lastFrame );

						imp->SetEditingAnim( TRUE );
						if ( TRUE == imp->ShowAnimDialog() )
						{
							curAnim = imp->GetCurAnimRecord();
							
							ZeroMemory( buffer, 64 );
							strcpy( buffer, curAnim.animName.c_str() );
							listItem.iSubItem = 0;
							listItem.pszText = buffer;
							SendMessage( m_hListAnim, LVM_SETITEM, 0, ( LPARAM )&listItem );

							itoa( curAnim.firstFrame, buffer, 10 );
							listItem.iSubItem = 1;
							listItem.pszText = buffer;
							SendMessage( m_hListAnim, LVM_SETITEM, 0, ( LPARAM )&listItem );

							itoa( curAnim.lastFrame, buffer, 10 );
							listItem.iSubItem = 2;
							listItem.pszText = buffer;
							SendMessage( m_hListAnim, LVM_SETITEM, 0, ( LPARAM )&listItem );
						}
					}
					break;
				}
			case IDC_BUTTON_DELETE:	// 删除动画
				{
					int nSel = -1;
					nSel = SendMessage( m_hListAnim, LVM_GETSELECTIONMARK, 0, 0 );
					int nSelCount = SendMessage( m_hListAnim, LVM_GETSELECTEDCOUNT, 0, 0 );
					if ( nSelCount != 0 )
					{
						SendMessage( m_hListAnim, LVM_DELETEITEM, nSel, 0 );
					}
					break;
				}
			case IDC_BUTTON_HEAD: // 选择头部
				{
					if ( TRUE == imp->ShowObjListDialog() )
					{
						imp->m_sHeadNode = imp->m_sGettingNode;
						SetDlgItemText( hWnd, IDC_EDIT_HEAD, imp->m_sHeadNode.c_str() );
					}
					break;
				}
			case IDC_BUTTON_UPPER: // 选择上半身
				{
					if ( TRUE == imp->ShowObjListDialog() )
					{
						imp->m_sUpperNode = imp->m_sGettingNode;
						SetDlgItemText( hWnd, IDC_EDIT_UPPER, imp->m_sUpperNode.c_str() );
					}
					break;
				}
			case IDC_BUTTON_LOWER: // 选择下半身
				{
					if ( TRUE == imp->ShowObjListDialog() )
					{
						imp->m_sLowerNode = imp->m_sGettingNode;
						SetDlgItemText( hWnd, IDC_EDIT_LOWER, imp->m_sLowerNode.c_str() );
					}
					break;
				}
			case IDC_BUTTON_PROPERTY: // 选择道具
				{
					if ( TRUE == imp->ShowObjListDialog() )
					{
						imp->m_sPropNode = imp->m_sGettingNode;
						SetDlgItemText( hWnd, IDC_EDIT_PROPERTY, imp->m_sPropNode.c_str() );
					}
					break;
				}
			case IDC_BUTTON_HEAD_X: // 取消头部的选择
				{
					imp->m_sHeadNode = "";
					SetDlgItemText( hWnd, IDC_EDIT_HEAD, imp->m_sHeadNode.c_str() );
					break;
				}
			case IDC_BUTTON_UPPER_X: // 取消上半身的选择
				{
					imp->m_sUpperNode = "";
					SetDlgItemText( hWnd, IDC_EDIT_UPPER, imp->m_sUpperNode.c_str() );
					break;
				}
			case IDC_BUTTON_LOWER_X: // 取消下半身的选择
				{
					imp->m_sLowerNode = "";
					SetDlgItemText( hWnd, IDC_EDIT_LOWER, imp->m_sLowerNode.c_str() );
					break;
				}
			case IDC_BUTTON_PROPERTY_X: // 取消道具的选择
				{
					imp->m_sPropNode = "";
					SetDlgItemText( hWnd, IDC_EDIT_PROPERTY, imp->m_sPropNode.c_str() );
					break;
				}
			case IDC_OK:	// 确定
				{
					// 是否导出动画、TAG、道具
					imp->SetExportAnim( hWnd, IsDlgButtonChecked( hWnd, IDC_CHECK_ANIM ) );
					imp->SetExportTags( hWnd, IsDlgButtonChecked( hWnd, IDC_CHECK_TAGS ) );
					imp->SetExportProp( hWnd, IsDlgButtonChecked( hWnd, IDC_CHECK_PROPERTY ) );

					if ( imp->IfExportAnim() == TRUE )
					{
						// 从UI获取动画数据
						int animCount = SendMessage( m_hListAnim, LVM_GETITEMCOUNT, 0, 0 );
						for ( int i = 0; i < animCount; i ++ )
						{
							AnimRecord anim;

							LVITEM listItem;
							memset( &listItem, 0, sizeof( LVITEM ) );

							char buffer[64];
							ZeroMemory( buffer, 64 );
							listItem.mask = LVIF_TEXT;
							listItem.iItem = i;
							listItem.cchTextMax = 64;
							listItem.pszText = buffer;
							listItem.iSubItem = 0;
							SendMessage( m_hListAnim, LVM_GETITEM, 0, ( LPARAM )&listItem );
							anim.animName = listItem.pszText;

							listItem.iSubItem = 1;
							SendMessage( m_hListAnim, LVM_GETITEM, 0, ( LPARAM )&listItem );
							anim.firstFrame = atoi( listItem.pszText );

							listItem.iSubItem = 2;
							SendMessage( m_hListAnim, LVM_GETITEM, 0, ( LPARAM )&listItem );
							anim.lastFrame = atoi( listItem.pszText );

							imp->AddAnimRecord( anim );
						}
					}

					if ( imp->IfExportTags() == TRUE )
					{
						char buf[64];
						ZeroMemory( buf, 64 );
						GetDlgItemText( hWnd, IDC_EDIT_HEAD, buf, 64 );
						imp->m_sHeadNode = buf;

						ZeroMemory( buf, 64 );
						GetDlgItemText( hWnd, IDC_EDIT_UPPER, buf, 64 );
						imp->m_sUpperNode = buf;

						ZeroMemory( buf, 64 );
						GetDlgItemText( hWnd, IDC_EDIT_LOWER, buf, 64 );
						imp->m_sLowerNode = buf;

						if ( imp->IfExprotProperty() )
						{
							ZeroMemory( buf, 64 );
							GetDlgItemText( hWnd, IDC_EDIT_PROPERTY, buf, 64 );
							imp->m_sPropNode = buf;

							int curSel = SendMessage( m_hComBoxMount, CB_GETCURSEL, 0, 0 );
							ZeroMemory( buf, 64 );
							SendMessage( m_hComBoxMount, CB_GETLBTEXT, ( WPARAM )curSel, ( LPARAM )buf );
							imp->m_propMountTo = buf;
						}
					}

					EndDialog( hWnd, IDOK );
					return TRUE;
				}

			case IDC_CANCEL:	// 取消
				EndDialog( hWnd, 0 );
				return TRUE;	
			}
			return TRUE;

		case WM_CLOSE:
			EndDialog( hWnd, 0 );
			return TRUE;
	}
	return 0;
}

BOOL CSMOpts::ShowAnimDialog()
{
	// 弹出一个模态对话框，如果返回0，则用户点击了Cancel
	return 0 != DialogBoxParam( hInstance, 
		MAKEINTRESOURCE( IDD_DLG_ANIM ),
		GetActiveWindow(), 
		AnimDialogProc, ( LPARAM )this );
}

INT_PTR CALLBACK CSMOpts::AnimDialogProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	static CSMOpts *imp = NULL;

	switch( message ) 
	{
	case WM_INITDIALOG:	// 初始化动画窗口
		{
			imp = reinterpret_cast< CSMOpts* >( lParam );
			CenterWindow( hWnd,GetParent( hWnd ) );

			m_hEditAnimName		= GetDlgItem( hWnd, IDC_EDIT_ANIM_NAME );
			m_hEditFirstFrame	= GetDlgItem( hWnd, IDC_EDIT_FFRAME );
			m_hEditLastFrame	= GetDlgItem( hWnd, IDC_EDIT_LFRAME );

			if( imp->IsEditingAnim() == TRUE )
			{
				char buffer[64];
				ZeroMemory( buffer, 64 );
				AnimRecord &curAnim = imp->GetCurAnimRecord();
				SetDlgItemText( hWnd, IDC_EDIT_ANIM_NAME, curAnim.animName.c_str() );
				itoa( curAnim.firstFrame, buffer, 10 );
				SetDlgItemText( hWnd, IDC_EDIT_FFRAME, buffer );
				itoa( curAnim.lastFrame, buffer, 10 );
				SetDlgItemText( hWnd, IDC_EDIT_LFRAME, buffer );
			}
			else
			{
				char buffer[64];
				ZeroMemory( buffer, 64 );
				itoa( imp->GetSettedFrame(), buffer, 10 );
				SetDlgItemText( hWnd, IDC_EDIT_FFRAME, buffer );
				itoa( imp->GetLastFrame(), buffer, 10 );
				SetDlgItemText( hWnd, IDC_EDIT_LFRAME, buffer );
			}

			return TRUE;
		}

	case WM_COMMAND:
		switch( wParam )
		{
		case IDC_BUTTON_FF_MINUS:	// 动画首帧减一
			{
				char buffer[64];
				ZeroMemory( buffer, 64 );
				GetDlgItemText( hWnd, IDC_EDIT_FFRAME, buffer, 64 );
				int frame = atoi( buffer );
				if ( frame <= imp->GetFristFrame() )
				{
					frame = imp->GetFristFrame();
				}
				else
				{
					frame--;
				}
				itoa( frame, buffer, 10 );
				SetDlgItemText( hWnd, IDC_EDIT_FFRAME, buffer );

				break;
			}
		case IDC_BUTTON_FF_PLUS:	// 动画首帧加一
			{
				char buffer[64];
				ZeroMemory( buffer, 64 );
				GetDlgItemText( hWnd, IDC_EDIT_FFRAME, buffer, 64 );
				int frame = atoi( buffer );
				ZeroMemory( buffer, 64 );
				GetDlgItemText( hWnd, IDC_EDIT_LFRAME, buffer, 64 );
				int curLastFrame = atoi( buffer );
				if ( frame >= curLastFrame )
				{
					frame = curLastFrame;
				}
				else
				{
					frame++;
				}
				itoa( frame, buffer, 10 );
				SetDlgItemText( hWnd, IDC_EDIT_FFRAME, buffer );

				break;
			}
		case IDC_BUTTON_LF_MINUS:	// 动画末帧减一
			{
				char buffer[64];
				ZeroMemory( buffer, 64 );
				GetDlgItemText( hWnd, IDC_EDIT_LFRAME, buffer, 64 );
				int frame = atoi( buffer );
				ZeroMemory( buffer, 64 );
				GetDlgItemText( hWnd, IDC_EDIT_FFRAME, buffer, 64 );
				int curFirstFrame = atoi( buffer );
				if ( frame <= curFirstFrame )
				{
					frame = curFirstFrame;
				}
				else
				{
					frame--;
				}
				itoa( frame, buffer, 10 );
				SetDlgItemText( hWnd, IDC_EDIT_LFRAME, buffer );

				break;
			}
		case IDC_BUTTON_LF_PLUS:	// 动画末帧加一
			{
				char buffer[64];
				ZeroMemory( buffer, 64 );
				GetDlgItemText( hWnd, IDC_EDIT_LFRAME, buffer, 64 );
				int frame = atoi( buffer );
				if ( frame >= imp->GetLastFrame() )
				{
					frame = imp->GetLastFrame();
				}
				else
				{
					frame++;
				}
				itoa( frame, buffer, 10 );
				SetDlgItemText( hWnd, IDC_EDIT_LFRAME, buffer );

				break;
			}
		case ID_ANIM_BTN_OK:	// 确定
			{
				char buffer[64];
				ZeroMemory( buffer, 64 );
				GetDlgItemText( hWnd, IDC_EDIT_ANIM_NAME, buffer, 64 );
				string animName = buffer;
				ZeroMemory( buffer, 64 );
				GetDlgItemText( hWnd, IDC_EDIT_FFRAME, buffer, 64 );
				int firstFrame = atoi( buffer );
				ZeroMemory( buffer, 64 );
				GetDlgItemText( hWnd, IDC_EDIT_LFRAME, buffer, 64 );
				int lastFrame = atoi( buffer );

				if ( animName.size() != 0 )
				{
					if ( imp->IsEditingAnim() == FALSE )
					{
						imp->SetNewAnimRecord( animName, firstFrame, lastFrame );
					}
					else
					{
						imp->SetCurAnimRecord( animName, firstFrame, lastFrame );
					}

					EndDialog( hWnd, IDOK );
				}
				else
				{
					MessageBox( hWnd, "请输入动画名称", "动画名称空缺", MB_ICONERROR );
					break;
				}
				return TRUE;
			}
			
		case ID_ANIM_BTN_CANCEL:	// 取消
			EndDialog( hWnd, 0 );
			return TRUE;
		}
		return TRUE;

	case WM_CLOSE:
		EndDialog( hWnd, 0 );
		return TRUE;
	}
	return 0;
}

BOOL CSMOpts::ShowObjListDialog()
{
	// 弹出一个模态对话框，如果返回0，则用户点击了Cancel
	return 0 != DialogBoxParam( hInstance, 
								MAKEINTRESOURCE( IDD_DLG_OBJLIST ),
								GetActiveWindow(), 
								ObjListDialogProc, ( LPARAM )this );
}

INT_PTR CALLBACK CSMOpts::ObjListDialogProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	static CSMOpts *imp = NULL;

	switch( message ) 
	{
	case WM_INITDIALOG:	// 初始化列表窗口
		{
			imp = reinterpret_cast< CSMOpts* >( lParam );
			CenterWindow( hWnd,GetParent( hWnd ) );

			m_hListObj = GetDlgItem( hWnd, IDC_LIST_OBJ );

			for ( int i = 0; i < imp->GetNumObj(); i ++ )
			{
				string name = imp->GetObj( i );
				if ( name != imp->m_sHeadNode &&
					 name != imp->m_sLowerNode && 
					 name != imp->m_sUpperNode &&
					 name != imp->m_sPropNode )
				{
					SendMessage( m_hListObj, LB_ADDSTRING, 0, ( LPARAM )name.c_str() );
				}
			}
			return TRUE;
		}

	case WM_COMMAND:
		switch( wParam )
		{
		case ID_OBJLIST_OK:	// 确定
			{
				int curSel = SendMessage( m_hListObj, LB_GETCURSEL, 0, 0 );
				char buf[64];
				ZeroMemory( buf, 64 );
				SendMessage( m_hListObj, LB_GETTEXT, curSel, ( LPARAM )buf );
				imp->m_sGettingNode = buf;

				EndDialog( hWnd, IDOK );

				return TRUE;
			}

		case ID_OBJLIST_CANCEL:	// 取消
			EndDialog( hWnd, 0 );
			return TRUE;
		}
		return TRUE;

	case WM_CLOSE:
		EndDialog( hWnd, 0 );
		return TRUE;
	}
	return 0;
}
