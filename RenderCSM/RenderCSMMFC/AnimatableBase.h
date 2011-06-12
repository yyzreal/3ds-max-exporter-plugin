#ifndef __ANIMATABLE_BASE_H__
#define __ANIMATABLE_BASE_H__

#include "CSM.h"
#include "ModelBase.h"

#include <algorithm>

/*! \brief 动画状态 */
struct AnimState
{
	int		startframe;				//!< first frame
	int		endframe;				//!< last frame
	int		fps;					//!< frame per second for this animation

	float	curr_time;				//!< current time
	float	old_time;				//!< old time
	float	interpol;				//!< percent of interpolation

	string  name;					//!< animation name

	int		curr_frame;				//!< current frame
	int		next_frame;				//!< next frame

	AnimState()
	{
		startframe = endframe = fps = 0;
		curr_time = old_time = interpol = 0.0f;
		curr_frame = next_frame = 0;
	}
} ;

/*! \brief 一个抽象动画类 */
class AnimatableBase : public ModelBase
{
public:
	/*! \brief 构造函数 */
	AnimatableBase()
	{
		m_numSceneFrames = 1;
		m_numTotalFrames = 0;
		m_bInitAnim = FALSE;
	}

	/*! \brief 析构函数 */
	virtual ~AnimatableBase() {};

	typedef vector< D3DXMATRIX > BoneFrames;		//!< 动画帧列表
	typedef vector< CSMAnimation > AnimList;		//!< 动画列表

	/*! \brief 初始化动画信息 */
	BOOL InitAnim( const string &fileAM, const string &fileCFG );

	/*! \brief 设置当前帧 */
	void SetFrame( int frame );

	/*! \brief 根据动画名称设置动画 */
	BOOL SetAnimation( const string &animName );

	/*! \brief 根据索引设置动画 */
	BOOL SetAnimation( const int index );

	/*! \brief 读入.AM文件,AM=AnimationTexture */
	BOOL LoadAM( const string &fileName, int numFrames );

	/*! \brief 读入.CFG文件（动画配置文件） */
	BOOL LoadCFG( const string &fileName );

	///////////////////////////Get&Set Functions//////////////////////////////////////
	/*! \brief 获得动画数量 */
	int GetAnimNum() const { return static_cast< int >( m_animList.size() ); }
	/*! \brief 获得动画信息 */
	CSMAnimation GetAnimInfo( int i ) const { return m_animList[i]; } 
	/*! \brief 获得动画进行到的帧数 */
	int GetCurFrame() { return m_curAnimState.curr_frame; }
	/*! \brief 获得动画的当前状态 */
	AnimState GetCurAnimState() { return m_curAnimState; }
	//////////////////////////////////////////////////////////////////////////////////

protected:
	/*! \brief 计算当前帧，下一帧，以及插值信息 */
	void Animate( const float time );

	/*! \brief 更新帧信息 */
	virtual void UpdateFrame() = 0;

protected:
	AnimState					m_curAnimState;					//!< 当前动画状态

	AnimList					m_animList;						//!< 动画列表
	BoneFrames					m_frameList;					//!< 帧列表
	int							m_numSceneFrames;				//!< 场景的总帧数
	int							m_numTotalFrames;				//!< 动画文件所包含的总帧数

	BOOL						m_bInitAnim;					//!< 是否初始化了动画

	ifstream	m_inFileAM;						//!< .AM文件
	ifstream	m_inFileCFG;					//!< .CFG文件
};

#endif