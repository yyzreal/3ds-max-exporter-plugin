#ifndef __CSM_SKIN_MESH_H__
#define __CSM_SKIN_MESH_H__

#include "CSMMeshData.h"
#include "AnimatableBase.h"

#include <fstream>
using std::ofstream;
using std::endl;

/*! \brief CSM模型顶点格式 */
struct DXCSMVertex
{
	D3DXVECTOR3 pos;
	D3DXVECTOR2 tex;

	DXCSMVertex(){}
	DXCSMVertex( D3DXVECTOR3 p, D3DXVECTOR2 t )
	{
		pos = p;
		tex = t;
	}
};

/*! \brief CSM模型 */
class CSMSkinMesh : public AnimatableBase
{
public:
	/*! \brief 默认构造函数 */
	CSMSkinMesh();

	/*! \brief 析构函数 */
	~CSMSkinMesh();

	/*! \brief 初始化CSM Skin模型 */
	BOOL Initialize( ID3D10Device *pd3dDevice, const string &CSMFile, ID3D10Effect *pEffect );

	/*! \brief 绘制模型 */
	void Render( ID3D10Device *pd3dDevice, const float time, 
		         ID3D10EffectMatrixVariable *worldMatVar, ID3D10EffectShaderResourceVariable *texVar );

	/*! \brief 绘制一帧 */
	void RenderSingleFrame( ID3D10Device *pd3dDevice, ID3D10EffectMatrixVariable *worldMatVar, ID3D10EffectShaderResourceVariable *texVar );

	/*! \brief 绘制法线 */
	void RenderNormal( ID3D10Device *pd3dDevice );

	/*! \brief 更新动画的顶点 */
	void UpdateFrame();

	void OutputInfoToTXT();

	/////////////////////////////////////Get&Set Functions/////////////////////////////////////
	/*! \brief 获得TAG的类型总数 */
	int GetNumTagType() const { return m_pMeshData->GetNumTags() / m_numSceneFrames; }

	/*! \brief 获得第i个TAG的类型名称 */
	string GetTagTypeName( int i ) const { return m_pMeshData->GetTag( i * m_numSceneFrames ).name; }

	/*! \brief 获得第i个TAG的第iFrame帧的绑定矩阵 */
	D3DXMATRIX GetTagFrame( int iTag, int iFrame ){ return m_pMeshData->GetTag( iTag ).vFrameData[iFrame]; }

	/*! \brief 获得顶点总数 */
	virtual int GetTotalNumVtx() const;

	/*! \brief 获得UV坐标总数 */
	virtual int GetTotalNumUV() const;

	/*! \brief 获得面总数 */
	virtual int GetTotalNumTri() const;
	///////////////////////////////////////////////////////////////////////////////////////////

private:
	/*! \brief 初始化纹理 */
	BOOL LoadTextures( ID3D10Device *pd3dDevice );

	/*! \brief 初始化顶点缓冲区 */
	BOOL InitVertexBuffer( ID3D10Device *pd3dDevice );

	/*! \brief 初始化索引缓冲区 */
	BOOL InitIndexBuffer( ID3D10Device *pd3dDevice );

	/*! \brief 初始化InputLayout */
	BOOL InitInputLayout( ID3D10Device *pd3dDevice, ID3D10EffectTechnique *pTechnique );

	BOOL CreateAnimationTexture( ID3D10Device *pd3dDevice, ID3D10Texture2D **ppAnimationTexture, ID3D10ShaderResourceView **ppAnimationResourceView );

	/*! \brief 计算最大包围盒 */
	virtual void CalcEntireBox();

private:
	static const string						TECHNIQUE_NAME;					//!< 基本technique的名称
	ID3D10EffectTechnique					*m_pBasicTechnique;				//!< 基本technique

	vector< ID3D10Buffer* >					m_vpVertexBuffers;				//!< 每个SubMesh对应一个VertexBuffer
	vector< ID3D10Buffer* >					m_vpIndexBuffers;				//!< 每个SubMesh对应一个IndexBuffer
	ID3D10InputLayout						*m_pVertexLayout;				//!< 顶点布局

	vector< ID3D10ShaderResourceView* >		m_vpTextureSRV;					//!< 每个SubMesh对应一个Texture
	ID3D10Texture2D							*m_pAnimationTexture;			//!< Animation Texture			
	ID3D10ShaderResourceView				*m_pAnimationResourceView;		//!< Animation Texture RV

	CSMMeshData								*m_pMeshData;					//!< CSM模型数据
	string									m_filePath;						//!< CSM文件名
};

#endif