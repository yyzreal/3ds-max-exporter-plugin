#ifndef __CSM_MERGING_MESH_H__
#define __CSM_MERGING_MESH_H__

#include "CSMSkinMesh.h"

#include <map>
using std::map;

/*! \brief 组合模型，一般由head(头部)，upper（上半身），lower（下半身），property（道具）这些子模型组合而成 */
class CSMMergingMesh : public AnimatableBase
{
public:
	/*! \brief 默认构造函数 */
	CSMMergingMesh();

	/*! \brief 析构函数 */
	~CSMMergingMesh();

	/*! \brief 模型属于哪一部分 */
	enum PartIdentity
	{
		LOWER,						//!< 下半身
		UPPER,						//!< 上半身
		HEAD,						//!< 头部
		PROPERTY,					//!< 道具
		ENTIRETY,					//!< 整体
		NUM_PART_IDENTITY
	};

	/*! \brief 包裹模型的一个节点 */
	struct MeshNode
	{
		CSMSkinMesh *pMesh;			//!< Skin Mesh
		MeshNode *pPreNode;			//!< 前一个Mesh节点
		int	tagID;					//!< 绑定两个Mesh的tag ID

		MeshNode()
		{
			pMesh = NULL;
			pPreNode = NULL;
			tagID = -1;
		}
	};

	typedef map< string, MeshNode > SkinMeshList;	//!< 模型列表

	/*! \brief 初始化 */
	BOOL Initialize( ID3D10Device *pd3dDevice, const string &pacName, ID3D10Effect *pEffect );

	/*! \brief 渲染 */
	virtual void Render( ID3D10Device *pd3dDevice, const float time, 
		ID3D10EffectMatrixVariable *worldMatVar, ID3D10EffectShaderResourceVariable *texVar );

	/*! \brief 绘制一帧 */
	virtual void RenderSingleFrame( ID3D10Device *pd3dDevice, 
		ID3D10EffectMatrixVariable *worldMatVar, ID3D10EffectShaderResourceVariable *texVar );

	/*! \brief 更新顶点 */
	virtual void UpdateFrame();

	/*! \brief 更新矩阵 */
	void UpdateMatrix( int frame );

	/*! \brief 获得顶点总数 */
	virtual int GetTotalNumVtx() const;

	/*! \brief 获得UV坐标总数 */
	virtual int GetTotalNumUV() const;

	/*! \brief 获得面总数 */
	virtual int GetTotalNumTri() const;

private:
	/*! \brief 计算最大包围盒 */
	virtual void CalcEntireBox();

private:
	SkinMeshList	m_vpMeshList;			//!< CSM Skin列表
};

#endif