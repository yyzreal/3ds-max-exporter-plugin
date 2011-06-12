#include "stdafx.h"
#include "DXManager.h"

DXManager::DXManager() 
:	m_pD3DDevice( NULL ),
	FILL_MODE( D3D10_FILL_SOLID ),
	m_pSwapChain( NULL ),
	m_pRenderTargetView( NULL ),
	m_pDepthStencil( NULL ),
	m_pDepthStencilView( NULL ),
	m_pSkinMesh( NULL ),
	m_pMergingMesh( NULL ),
	m_pMesh( NULL ),
	m_pCamera( NULL ),
	m_pTimer( NULL ),
	m_bPaused( FALSE ),
	m_pCurState( SceneState_Nothing::Instance() ),
	m_pRS( NULL ),
	m_bDrawNormal( FALSE )
{

}

DXManager::~DXManager()
{	
	SAFE_RELEASE( m_pRenderTargetView );
	SAFE_RELEASE( m_pSwapChain );
	SAFE_RELEASE( m_pD3DDevice );
	SAFE_RELEASE( m_pDepthStencil );
	SAFE_RELEASE( m_pDepthStencilView );
	SAFE_RELEASE( m_pBasicEffect );
	SAFE_RELEASE( m_pRS );

	SAFE_DELETE( m_pSkinMesh );
	SAFE_DELETE( m_pMergingMesh );

	SAFE_DELETE( m_pCamera );
	SAFE_DELETE( m_pTimer );
}

BOOL DXManager::Initialize( HWND* hW )
{
	//window handle
	m_hWnd = hW;

	//get window dimensions
	RECT rc;
	GetClientRect( *m_hWnd, &rc );
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;
	if ( width == 0 || height == 0 )
	{
		width = 1024;
		height = 768;
	}

	// CREATE DEVICE
	//*****************************************************************************
	if ( FAILED( CreateSwapChainAndDevice( width, height ) ) )
	{
		return FALSE;
	}

	// INPUT ASSEMBLY STAGE
	//*****************************************************************************
	if ( FAILED( LoadShaders() ) ) 
	{
		return FALSE;
	}

	// RASTERIZER STAGE SETUP
	//*****************************************************************************
	CreateViewports( width, height );
	InitRasterizerState();	

	// OUTPUT-MERGER STAGE
	//*****************************************************************************
	if ( FAILED( CreateRenderTargets( width, height ) ) )
	{
		return FALSE;
	}

	// SET UP SCENE VARIABLES
	//*****************************************************************************
	if ( FAILED( InitScene() ) )
	{
		return FALSE;
	}

	// Set up the view and projection matrices
	//*****************************************************************************
	D3DXMatrixLookAtLH( &m_viewMatrix, 
		                new D3DXVECTOR3( 0.0f, 0.0f, -15.0f ), 
						new D3DXVECTOR3( 0.0f, 0.0f, 1.0f ),
						new D3DXVECTOR3( 0.0f, 1.0f, 0.0f ) );		

	D3DXMatrixPerspectiveFovLH( &m_projectionMatrix, 
								static_cast< float >( D3DX_PI * 0.3f ), 
								static_cast< float >( width ) / static_cast< float >( height ), 
								0.1f, 1000.0f );

	D3DXMATRIX w;
	D3DXMatrixIdentity( &w );		
	m_pWorldMatrixEffectVariable->SetMatrix( w );
	m_pViewMatrixEffectVariable->SetMatrix( m_viewMatrix );
	m_pProjectionMatrixEffectVariable->SetMatrix( m_projectionMatrix );

	//everything completed successfully
	return TRUE;
}

BOOL DXManager::CreateSwapChainAndDevice( UINT width, UINT height )
{
	//Set up DX swap chain
	//--------------------------------------------------------------
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory( &swapChainDesc, sizeof( swapChainDesc ) );

	//set buffer dimensions and format
	swapChainDesc.BufferCount = 2;
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;;

	//set refresh rate
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;

	//sampling settings
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.SampleDesc.Count = 1;

	//output window handle
	swapChainDesc.OutputWindow = *m_hWnd;
	swapChainDesc.Windowed = TRUE;    

	//Create the D3D device
	//--------------------------------------------------------------
	if ( FAILED( D3D10CreateDeviceAndSwapChain(	NULL, 
												D3D10_DRIVER_TYPE_HARDWARE, 
												NULL, 0, 
												D3D10_SDK_VERSION, 
												&swapChainDesc, 
												&m_pSwapChain, 
												&m_pD3DDevice ) ) ) 
	{
		return FatalError( "D3D device creation failed" );
	}

	return TRUE;
}

void DXManager::CreateViewports( UINT width, UINT height )
{	
	//create viewport structure	
	m_viewPort.Width = width;
	m_viewPort.Height = height;
	m_viewPort.MinDepth = 0.0f;
	m_viewPort.MaxDepth = 1.0f;
	m_viewPort.TopLeftX = 0;
	m_viewPort.TopLeftY = 0;

	m_pD3DDevice->RSSetViewports( 1, &m_viewPort );
}

void DXManager::InitRasterizerState()
{
	//set rasterizer	
	D3D10_RASTERIZER_DESC rasterizerState;
	rasterizerState.CullMode = D3D10_CULL_FRONT;
	rasterizerState.FillMode = FILL_MODE;
	rasterizerState.FrontCounterClockwise = true;
	rasterizerState.DepthBias = false;
	rasterizerState.DepthBiasClamp = 0;
	rasterizerState.SlopeScaledDepthBias = 0;
	rasterizerState.DepthClipEnable = true;
	rasterizerState.ScissorEnable = false;
	rasterizerState.MultisampleEnable = false;
	rasterizerState.AntialiasedLineEnable = true;

	if ( FAILED( m_pD3DDevice->CreateRasterizerState( &rasterizerState, &m_pRS ) ) )
	{
		throw runtime_error( "failed" );
	}

	m_pD3DDevice->RSSetState( m_pRS );
}

BOOL DXManager::CreateRenderTargets( UINT width, UINT height )
{
	//try to get the back buffer
	ID3D10Texture2D* pBackBuffer;	
	if ( FAILED( m_pSwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (LPVOID*) &pBackBuffer) ) )
	{
		return FatalError( "Could not get back buffer" );
	}

	//try to create render target view
	if ( FAILED( m_pD3DDevice->CreateRenderTargetView( pBackBuffer, NULL, &m_pRenderTargetView ) ) )
	{
		return FatalError( "Could not create render target view" );
	}

	pBackBuffer->Release();

	// Create depth stencil texture
	D3D10_TEXTURE2D_DESC descDepth;
	descDepth.Width = width;
	descDepth.Height = height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D10_USAGE_DEFAULT;
	descDepth.BindFlags = D3D10_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	if ( FAILED( m_pD3DDevice->CreateTexture2D( &descDepth, NULL, &m_pDepthStencil ) ) )
	{
		return FatalError( "Could not create render Depth Texture" );
	}

	// Create the depth stencil view
	D3D10_DEPTH_STENCIL_VIEW_DESC descDSV;
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	if ( FAILED( m_pD3DDevice->CreateDepthStencilView( m_pDepthStencil, &descDSV, &m_pDepthStencilView ) ) )
	{
		return FatalError( "Could not create Depth Stencil View" );
	}

	m_pD3DDevice->OMSetRenderTargets( 1, &m_pRenderTargetView, m_pDepthStencilView );

	return TRUE;
}

BOOL DXManager::LoadShaders()
{
	ID3D10Blob* pErrors = 0;
	if ( FAILED( D3DX10CreateEffectFromFile( "basicEffect.fx", 
		NULL, NULL, 
		"fx_4_0", 
		D3D10_SHADER_ENABLE_STRICTNESS, 
		0, 
		m_pD3DDevice, 
		NULL, 
		NULL, 
		&m_pBasicEffect, 
		&pErrors, 
		NULL	) ) ) 
	{
		char* pCompileErrors ;
		if ( pErrors )
		{
			pCompileErrors = static_cast< char* >( pErrors->GetBufferPointer() );
		}
		return FatalError( "Could not load effect file!" );	
	}

	//create matrix effect pointers
	m_pViewMatrixEffectVariable = m_pBasicEffect->GetVariableByName( "View" )->AsMatrix();
	m_pProjectionMatrixEffectVariable = m_pBasicEffect->GetVariableByName( "Projection" )->AsMatrix();
	m_pWorldMatrixEffectVariable = m_pBasicEffect->GetVariableByName( "World" )->AsMatrix();
	m_pTextureSR = m_pBasicEffect->GetVariableByName( "tex2D" )->AsShaderResource();

	return TRUE;
}

BOOL DXManager::InitScene()
{
	// 创建摄象机
	m_pCamera = new Camera( D3DXVECTOR3( 0.0f, 0.0f, 0.0f ), 10, 1.0f, 300.0f );

	// 开始计时
	m_pTimer = new Timer();
	m_pTimer->Start();

	return TRUE;
}

void DXManager::UpdateScene()
{
	assert( m_pTimer != NULL && m_pCamera != NULL );

	// 更新计时器
	m_pTimer->Update();

	// 更新视图矩阵
	m_viewMatrix = m_pCamera->UpdateViewMatrix();
	m_pViewMatrixEffectVariable->SetMatrix( m_viewMatrix );
}

void DXManager::RenderScene()
{
	// Update scene first
	UpdateScene();

	// Clear scene
	m_pD3DDevice->ClearRenderTargetView( m_pRenderTargetView, D3DXCOLOR( 0.0f, 0.0f, 0.0f,1.0f ) );
	// Clear the depth buffer to 1.0 (max depth)
	m_pD3DDevice->ClearDepthStencilView( m_pDepthStencilView, D3D10_CLEAR_DEPTH, 1.0f, 0 );

	if ( ! IsInState( *SceneState_Nothing::Instance() ) )
	{
		if ( m_bPaused == FALSE )
		{
			m_pMesh->Render( m_pD3DDevice, m_pTimer->GetRunningTime(), m_pWorldMatrixEffectVariable, m_pTextureSR );
		}
		else
		{
			m_pMesh->RenderSingleFrame( m_pD3DDevice, m_pWorldMatrixEffectVariable, m_pTextureSR );
		}
		
		if ( m_bDrawNormal == TRUE )
		{
			m_pMesh->RenderNormal( m_pD3DDevice );
		}
	}

	// Flip buffers
	m_pSwapChain->Present(0,0);
}

BOOL DXManager::FatalError( const LPCSTR msg )
{
	MessageBox( *m_hWnd, msg, "Fatal Error!", MB_ICONERROR );
	return FALSE;
}

void DXManager::ProcessInput( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch ( message ) 
	{
		// Allow the user to press the escape key to end the application
	case WM_KEYDOWN	:	
		switch( wParam )
		{
			// Check if the user hit the escape key
		case VK_ESCAPE : PostQuitMessage(0);
			break;
		case VK_UP:
			m_pCamera->IncreaseDistance( 1.0f );
			m_pCamera->CalculatePossibleCamPosition();
			break;
		case VK_DOWN:
			m_pCamera->IncreaseDistance( -1.0f );
			m_pCamera->CalculatePossibleCamPosition();
			break;
		}
		break;

	case WM_CHAR:	
		switch( wParam )
		{							
		case 'w': case 'W':	
			m_pCamera->IncreaseVArc( 0.1f );
			m_pCamera->CalculatePossibleCamPosition();
			break;
		case 's': case 'S':
			m_pCamera->IncreaseVArc( -0.1f );
			m_pCamera->CalculatePossibleCamPosition();
			break;	
		case 'a': case 'A':
			m_pCamera->IncreaseHArc( 0.1f );
			m_pCamera->CalculatePossibleCamPosition();
			break;
		case 'd': case 'D':
			m_pCamera->IncreaseHArc( -0.1f );
			m_pCamera->CalculatePossibleCamPosition();
			break;
		}		
		break;

		// The user hit the close button, close the application
	case WM_DESTROY	:	PostQuitMessage(0);
		break;
	}
}

void DXManager::SetFillMode( D3D10_FILL_MODE fillMode )
{ 
	FILL_MODE = fillMode; 
	InitRasterizerState();
}

void DXManager::ProcessKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
{
	switch( nChar )
	{
		case 'w': case 'W':	
			m_pCamera->IncreaseVArc( 0.1f );
			m_pCamera->CalculatePossibleCamPosition();
			break;
		case 's': case 'S':
			m_pCamera->IncreaseVArc( -0.1f );
			m_pCamera->CalculatePossibleCamPosition();
			break;	
		case 'a': case 'A':
			m_pCamera->IncreaseHArc( 0.1f );
			m_pCamera->CalculatePossibleCamPosition();
			break;
		case 'd': case 'D':
			m_pCamera->IncreaseHArc( -0.1f );
			m_pCamera->CalculatePossibleCamPosition();
			break;
		default:
			break;
	}
}

void DXManager::ProcessMouseWheel( UINT nFlags, short zDelta, CPoint pt )
{
	m_pCamera->IncreaseDistance( -0.03f * zDelta );
	m_pCamera->CalculatePossibleCamPosition();
}

void DXManager::StateUpdate()
{
	if ( m_pCurState )
	{
		m_pCurState->Execute( this );
	}
}

void DXManager::ChangeState( State< DXManager > *pNewState )
{
	assert(pNewState && 
		"<DXManager::ChangeState>: trying to change to NULL state");

	//call the exit method of the existing state
	m_pCurState->Exit( this );

	//change state to the new state
	m_pCurState = pNewState;

	//call the entry method of the new state
	m_pCurState->Enter( this );
}

BOOL DXManager::LoadEntireMesh( const string &meshName )
{
	SAFE_DELETE( m_pSkinMesh );
	m_pSkinMesh = new CSMSkinMesh();

	if ( FALSE == m_pSkinMesh->Initialize( m_pD3DDevice, meshName, m_pBasicEffect ) )
	{
		return FatalError( "Initialize CSM Skin Mesh Failed" );
	}
	m_pMesh = m_pSkinMesh;

	return TRUE;
}

BOOL DXManager::LoadMergingMesh( const string &pacName )
{
	SAFE_DELETE( m_pMergingMesh );
	m_pMergingMesh = new CSMMergingMesh();

	if ( FALSE == m_pMergingMesh->Initialize( m_pD3DDevice, pacName, m_pBasicEffect ) )
	{
		return FatalError( "Initialize CSM Merging Mesh Failed" );
	}
	m_pMesh = m_pMergingMesh;

	return TRUE;
}