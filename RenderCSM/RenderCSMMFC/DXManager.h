#ifndef __DXMANAGER_H__
#define __DXMANAGER_H__

#include <vector>
#include <string>

using std::vector;
using std::string;

#include "SceneState.h"
#include "CSMSkinMesh.h"
#include "CSMMergingMesh.h"
#include "Camera.h"
#include "Timer.h"

class DXManager
{	
public:
	/*! \brief D3D初始化 */
	BOOL Initialize( HWND* );

	/*! \brief 渲染场景 */
	void RenderScene();

	/*! \brief 处理输入 */
	void ProcessInput( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

	/*! \brief 处理键盘输入 */
	void ProcessKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );

	/*! \brief 处理鼠标滑轮输入 */
	void ProcessMouseWheel( UINT nFlags, short zDelta, CPoint pt );

	/*! \brief 场景状态更新 */
	void StateUpdate();

	/*! \brief 更换场景状态 */
	void ChangeState( State< DXManager > *pNewState );

	/*! \brief 是否处于某一状态 */
	bool  IsInState( const State<DXManager>& st ) const
	{
		return typeid( *m_pCurState ) == typeid( st );
	}

	/*! \brief 读取整体模型（只有一个CSM） */
	BOOL LoadEntireMesh( const string &meshName );

	/*! \brief 读取合并模型（多个CSM） */
	BOOL LoadMergingMesh( const string &pacname );

	void SetFillMode( D3D10_FILL_MODE fillMode );
	D3D10_FILL_MODE GetFillMode() { return FILL_MODE; }

	ModelBase *GetMesh() { return m_pMesh; }
	void Pause( BOOL bPaused ) { m_bPaused = bPaused; }

	Camera *GetCamera() { return m_pCamera; }

	BOOL IsDrawNormal() const { return m_bDrawNormal; }
	void SetDrawNormal( BOOL b ) { m_bDrawNormal = b; }

	static DXManager * GetManager()
	{
		static DXManager *pSingleton = new DXManager();
		return pSingleton;
	}

private:
	DXManager();		//!< 默认构造函数
	~DXManager();		//!< 析构函数

	/*! \brief 创建交换链和设备 */
	BOOL CreateSwapChainAndDevice( UINT width, UINT height );

	/*! \brief 初始化Shader*/
	BOOL LoadShaders();

	/*! \brief 创建视口 */
	void CreateViewports( UINT width, UINT height );

	/*! \brief 初始化光栅化状态 */
	void InitRasterizerState();

	/*! \brief 创建渲染目标 */
	BOOL CreateRenderTargets( UINT width, UINT height );

	/*! \brief 初始化场景 */
	BOOL InitScene();

	/*! \brief 更新场景 */
	void UpdateScene();

	/*! \brief Fatal Error Handler */
	BOOL FatalError( const LPCSTR msg );

private:

	D3D10_FILL_MODE FILL_MODE;

	HWND*						m_hWnd;					//!< 窗口句柄

	ID3D10Device				*m_pD3DDevice;			//!< D3D设备
	IDXGISwapChain				*m_pSwapChain;			//!< D3D Swap Chain
	ID3D10RenderTargetView		*m_pRenderTargetView;	//!< D3D渲染目标
	D3D10_VIEWPORT				m_viewPort;				//!< 视口
	ID3D10Texture2D				*m_pDepthStencil;		//!< 深度模板纹理
	ID3D10DepthStencilView		*m_pDepthStencilView;	//!< 深度模板视图

	ID3D10Effect*				m_pBasicEffect;			//!< FX文件
	ID3D10RasterizerState*		m_pRS;

	ID3D10EffectMatrixVariable* m_pViewMatrixEffectVariable;			//!< FX的视图矩阵变量指针
	ID3D10EffectMatrixVariable* m_pProjectionMatrixEffectVariable;		//!< FX的投影矩阵变量指针
	ID3D10EffectMatrixVariable* m_pWorldMatrixEffectVariable;			//!< FX的世界矩阵变量指针
	ID3D10EffectShaderResourceVariable		*m_pTextureSR;				//!< FX的纹理变量指针

	D3DXMATRIX                  m_viewMatrix;					//!< 视图矩阵
	D3DXMATRIX                  m_projectionMatrix;				//!< 投影矩阵

	CSMSkinMesh					*m_pSkinMesh;			//!< CSM Skin模型
	CSMMergingMesh				*m_pMergingMesh;		//!< 组合的CSM模型
	ModelBase					*m_pMesh;				//!< 当前渲染的模型

	Camera						*m_pCamera;				//!< 摄像机
	Timer						*m_pTimer;				//!< 计时器

	BOOL						m_bPaused;				//!< 是否处于暂停

	// 状态控制
	State< DXManager >			*m_pCurState;			//!< 当前所处的状态

	BOOL						m_bDrawNormal;			//!< 是否绘制法线
};

#define DXMANAGER DXManager::GetManager()


#endif