#ifndef __MODEL_BASE_H__
#define __MODEL_BASE_H__

#include <vector>
using std::vector;

struct BoundingBox
{
	D3DXVECTOR3 boxMin;		//!< 包围盒的最小端
	D3DXVECTOR3 boxMax;		//!< 包围盒的最大端

	BoundingBox()
	{
		memset( &boxMin, 0, sizeof( D3DXVECTOR3 ) );
		memset( &boxMax, 0, sizeof( D3DXVECTOR3 ) );
	}
};

class ModelBase
{
public:
	ModelBase()
	{
		memset( &m_entireCenter, 0, sizeof( D3DXVECTOR3 ) );
		D3DXMatrixIdentity( &m_worldMatrix  );
	}
	virtual ~ModelBase(){};

	typedef vector< BoundingBox > BoxList;		//! 包围盒列表

	/*! \brief 计算最大包围盒 */
	virtual void CalcEntireBox() = 0;

	/*! \brief 计算所有的局部包围盒 */
	virtual void CalcPartialBoxes(){}

	/*! \brief 获得顶点总数 */
	virtual int GetTotalNumVtx() const = 0;

	/*! \brief 获得UV坐标总数 */
	virtual int GetTotalNumUV() const = 0;

	/*! \brief 获得面总数 */
	virtual int GetTotalNumTri() const = 0;

	/*! \brief 绘制模型 */
	virtual void Render( ID3D10Device *pd3dDevice, const float time, 
		ID3D10EffectMatrixVariable *worldMatVar, ID3D10EffectShaderResourceVariable *texVar ) = 0;

	/*! \brief 绘制一帧 */
	virtual void RenderSingleFrame( ID3D10Device *pd3dDevice, 
		ID3D10EffectMatrixVariable *worldMatVar, ID3D10EffectShaderResourceVariable *texVar ) = 0;

	virtual void RenderNormal( ID3D10Device *pd3dDevice ) {}
	
	D3DXVECTOR3 GetEntityCenter() { return m_entireCenter; }
	BoundingBox GetEntireBox() { return m_entireBox; }
	int	GetNumBoxes() { return static_cast< int >( m_partialBoxes.size() ); }
	BoundingBox GetPartialBox( int i ) { return m_partialBoxes[i]; }
	/*! \brief 获得当前世界矩阵 */
	D3DXMATRIX GetWorldMatrix() const { return m_worldMatrix; }
	/*! \brief 设置当前世界矩阵 */
	void SetWorldMatrix( D3DXMATRIX worMat ) { m_worldMatrix = worMat; }
	/*! \brief 获得跟踪点 */
	D3DXVECTOR3 GetTracePosition() const { return m_entireCenter; }
	/*! \brief 获得最大跟踪距离 */
	float GetTraceDistanceMax() const { return m_entireLengthMax * 3; }
	/*! \brief 获得最小跟踪距离 */
	float GetTraceDistanceMin() const { return m_entireLengthMax / 2; }

protected:
	/*! \brief 计算包围盒的最大端点间距离 */
	void CalcEntireBoxInfo();

protected:
	BoundingBox m_entireBox;		//!< 整体的最大包围盒
	D3DXVECTOR3 m_entireCenter;		//!< 整体的中心
	float		m_entireLengthMax;	//!< 包围盒的最大端点间距离
	
	BoxList m_partialBoxes;			//!< 整体分为多块，每块有一个包围盒

	D3DXMATRIX	m_worldMatrix;		//!< 世界矩阵
};

#endif