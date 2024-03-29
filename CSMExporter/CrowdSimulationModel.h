#ifndef __CROWDSIMULATIONMODEL_H__
#define __CROWDSIMULATIONMODEL_H__

#include "Utility.h"

/*! \brief 导出管理器，负责搜集3dmax信息，输出CSM,AM,CFG等文件 */
class ExportManager
{
public:	
	/*! \brief 默认构造函数 */
	ExportManager();

	/*! \brief 析构函数 */
	~ExportManager();

	/*! \brief 存储绑定了骨骼的submesh以及其相关的蒙皮信息 */
	struct SubMeshBones
	{
		MaxTriObjData *pMaxObjData;			//!< 绑定了骨骼的MaxTriObjData
		ISkinContextData *pSkinContext;		//!< 存储了蒙皮信息
		ISkin *pSkin;						//!< 包含与mesh所相关联的骨骼列表

		SubMeshBones()
		{
			pMaxObjData = NULL;
			pSkinContext = NULL;
			pSkin = NULL;
		}
	};

	typedef vector< CSMTagData > TagList;					//!< 绑定点列表
	typedef vector< SubMeshBones > SubMeshBoneList;			//!< 包含蒙皮的子网格列表
	typedef vector< Matrix3 > BoneFrames;					//!< 骨骼的动画帧信息
	typedef vector< CSMAnimation > AnimList;				//!< 动画列表

	/*! \brief 骨骼信息 */
	struct Bone
	{
		INode *pBoneNode;				//!< 存储骨骼的3dmax节点
		int index;						//!< 索引
		Bone *pParentBone;				//!< 父骨骼
		Matrix3 relativeMat;			//!< 相对于父骨骼的矩阵
		BoneFrames localFrames;			//!< 在本地空间内的变换
		BoneFrames boneFrames;			//!< 关键帧

		Bone()
		{
			index = -1;
			pBoneNode = NULL;
			pParentBone = NULL;
			memset( &relativeMat, 0, sizeof( Matrix3 ) );
		}
	};

	typedef map< string, Bone > BoneList;			//!< （骨骼名称，骨骼）

	/*! \brief 模型属于哪一部分 */
	enum PartIdentity
	{
		LOWER,				//!< 下半身部
		UPPER,				//!< 上半身部
		HEAD,				//!< 头部	
		PROPERTY,			//!< 道具（武器）
		ENTIRETY,			//!< 整体模型
		NUM_PART_IDENTITY
	};

	/*! \brief 一个导出的CSM所包含的信息和数据 */
	struct CSMInfo
	{
		PartIdentity		identity;						//!< 标示模型
		string				strCSMFile;						//!< 输出的文件名称（包含完整的文件路径）
		string				strAppend;						//!< 文件名称需要加的内容，比如是_head或者_upper等等
		ofstream			ofCSMFile;						//!< 文件流
		CSMHeader			header;							//!< Header
		TagList				vpTags;							//!< 存储所有的Tags
		MaxTriObjList		maxObjList;						//!< 模型信息，如果identity不是ENTIRETY，则该列表最多只包含1个元素
		INode				*pNode;							//!< max节点，如果identity为ENTIRETY，则pNode为NULL

		CSMInfo()
		{
			identity = ENTIRETY;
			pNode = NULL;
		}

		CSMInfo( PartIdentity id )
		{
			identity = id;
			pNode = NULL;
		}

		~CSMInfo()
		{
			for ( MaxTriObjList::iterator i = maxObjList.begin(); i != maxObjList.end(); i ++ )
			{
				SAFE_DELETE( *i );
			}
		}
	};

	typedef vector< CSMInfo* > CSMList;		//!< CSM文件信息列表

	/*! \brief 从3DS MAX的场景中搜集我们所需的数据 */
	void Gather3DSMAXSceneData( INode *pRootNode, 
								BOOL bExportAnim, 
								BOOL bTag, 
								string headNode, 
								string upperNode, 
								string lowerNode, 
								BOOL bProp, 
								string propNode,
								string mountTo,
								const string &logFile = "log.txt" );

	/*! \brief 输出所有的CSM文件,CSM=CrowdSimulationModel */
	BOOL WriteAllCSMFile( const string &fileName );

	/*! \brief 输出CFG文件（动画配置文件） */
	BOOL WriteCFGFile( const string &fileName );

	/*! \brief 输出.AM文件,AM=AnimationTexture */
	BOOL WriteAMFile( const string &fileName );

	/*! \brief 输出.CSM_PAC文件 */
	BOOL WriteCSM_PACFile( const string &fileName );

	/*! \brief 添加动画信息 */
	void AddAnim( const CSMAnimation anim )
	{
		m_animList.push_back( anim );
	}

private:
	/*! \brief 屏蔽拷贝构造函数 */
	ExportManager( const ExportManager &rhEM );

	/*! \brief 屏蔽赋值操作符 */
	ExportManager& operator=( const ExportManager &rhEM );

	/*! \brief 开始写入文件 */
	BOOL BeginWriting( ofstream &outFile, const string &fileName, BOOL binary );

	/*! \brief 结束写入文件 */
	void EndWriting( ofstream &outFile );

	/*! \brief 根据节点的名称寻找节点 */
	INode *FindNodeByName( INode *pNode, string nodeName );

	/*! \brief 搜集节点信息，这是一个递归搜索场景数的函数 */
	void GatherRecursiveNodeData( INode *pNode, BOOL bExportAnim );

	/*! \brief 搜索特定的节点信息 */
	void GatherSpecifiedNodeData( CSMList::iterator beg, CSMList::iterator end, BOOL bExportAnim );

	/*! \brief 搜集节点信息 */
	BOOL GatherNodeData( INode *pNode, BOOL bExportAnim );

	/*! \brief 搜集Mesh的数据存储到CSMSubMesh中 */
	void GatherMeshData( Mesh &lMesh, MaxTriObjData *pMaxTriData );

	/*! \brief 搜集Material（只纹理）存储到CSMSubMesh中 */
	void GatherMaterialData( Mtl *pMtl, MaxTriObjData *pMaxTriData );

	/*! \brief 生成TAG数据 */
	void GenTag( INode *pTagedNode, INode *pNode, CSMInfo *pCSM, string tagName );

	/*! \brief 搜索INode的modifier stack，查找skin modifier(SKIN_CLASSID) */
	Modifier* GetSkinMode( INode *pNode );

	/*! \brief 搜集顶点的Skin信息（影响骨骼、权重）存储到CSMSubMesh中 */
	void GatherSkinData( ISkinContextData* pContext, ISkin *pSkin, MaxTriObjData *pMaxTriData );

	/*! \brief 搜集ISkin中所关联的骨骼 */
	void GatherBones( ISkin *pSkin );

	/*! \brief 生成骨骼树 */
	void GenBoneTree();

	/*! \brief 检测一个节点是否是骨骼 */
	BOOL IsBone( INode *pNode );

	/*! \brief 获取骨骼矩阵（去除scale） */
	Matrix3 GetBoneTM( INode *pNode, TimeValue t = 0 );

	/*! \brief 搜集骨骼动画 */
	void GatherBoneFrames( Bone *pBone );

	/*! \brief 生成CSM文件的Header信息 */
	void GenCSMHeaderInfo( int numAnimFrames );

	/*! \brief 获得节点相对于其父节点的相对矩阵 */
	Matrix3 GetRelativeMatrix( INode *pNode, TimeValue t = 0 );

	/*! \brief 获得骨骼相对于其父骨骼的相对矩阵 */
	Matrix3 GetRelativeBoneMatrix( Bone *pBone, TimeValue t = 0 );

	/*! \brief 获得骨骼在本地空间内的变换矩阵 */
	Matrix3 GetLocalBoneTranMatrix( Bone *pBone, TimeValue t );

	/*! \brief 输出CSM文件 */
	BOOL WriteCSMFile( CSMInfo *pCSM );

private:
	//! CSM的魔数 
	static const int CSM_MAGIC_NUMBER = 'C' + ( 'S' << 8 ) + ( 'M' << 16 ) + ( '1' << 24 );

	static const BOOL BINARY_MODE = TRUE;		//!< 是否采用二进制写模式

	int			m_numAnimFrames;				//!< 场景的帧信息

	MaxTriObjList	m_vpMaxObjs;				//!< 存储场景中所有的节点

	BOOL		m_bSplitMode;					//!< TRUE表示是将模型拆分（多个CSM文件），否则整个导出（1个CSM文件）
	CSMList		m_vpCSM;						//!< 所有需要导出的CSM信息

	SubMeshBoneList m_submeshBones;				//!< Sub Mesh的骨骼信息
	BoneList	m_boneList;						//!< 骨骼列表
	AnimList	m_animList;						//!< 动画列表

	ofstream	m_ofFileAM;						//!< 输出的.AM文件
	ofstream	m_ofFileCFG;					//!< 输出的.CFG文件
	ofstream	m_ofFileCSM_PAC;				//!< 输出的.CSM_PAC文件
	ofstream	m_ofLog;						//!< 输出LOG
};

#endif