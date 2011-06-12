#include "stdafx.h"
#include "AnimatableBase.h"

/*!	\param fileAM 需要读取的AM文件名
	\param fileCFG 需要读取的CFG文件名
	\return 初始化失败返回FALSE
 */
BOOL AnimatableBase::InitAnim( const string &fileAM, const string &fileCFG )
{
	if ( FALSE == LoadCFG( fileCFG ) )
	{
		return FALSE;
	}
	if ( FALSE == LoadAM( fileAM, m_numTotalFrames ) )
	{
		return FALSE;
	}
	m_bInitAnim = TRUE;
	return TRUE;
}

/*!	\param frame 设置的帧数 */
void AnimatableBase::SetFrame( int frame )
{
	assert( m_bInitAnim &&
		"请先初始化动画信息" );

	if ( frame < m_curAnimState.startframe )
	{
		frame = m_curAnimState.endframe;
	}
	if ( frame > m_curAnimState.endframe )
	{
		frame = m_curAnimState.startframe;
	}

	m_curAnimState.curr_frame = frame;
	m_curAnimState.next_frame = frame + 1;

	if( m_curAnimState.next_frame > m_curAnimState.endframe )
	{
		m_curAnimState.next_frame = m_curAnimState.startframe;
	}

	UpdateFrame();
}

/*! \param time 当前时间 */
void AnimatableBase::Animate( const float time )
{
	assert( m_bInitAnim &&
		"请先初始化动画信息" );

	m_curAnimState.curr_time = time;

	// 计算当前帧和下一帧
	if( m_curAnimState.curr_time - m_curAnimState.old_time > ( 1.0 / m_curAnimState.fps ) )
	{
		m_curAnimState.curr_frame = m_curAnimState.next_frame;
		m_curAnimState.next_frame++;

		if( m_curAnimState.next_frame > m_curAnimState.endframe )
		{
			m_curAnimState.next_frame = m_curAnimState.startframe;
		}

		m_curAnimState.old_time = m_curAnimState.curr_time;
	}

	if( m_curAnimState.curr_frame > ( m_numSceneFrames - 1 ) )
	{
		m_curAnimState.curr_frame = 0;
	}

	if( m_curAnimState.next_frame > ( m_numSceneFrames - 1 ) )
	{
		m_curAnimState.next_frame = 0;
	}

	// 插值
	m_curAnimState.interpol = m_curAnimState.fps * ( m_curAnimState.curr_time - m_curAnimState.old_time );

	// 更新顶点
	UpdateFrame();
}

/*! \param animName 动画名称
	\return 没有动画返回FALSE
 */
BOOL AnimatableBase::SetAnimation( const string &animName )
{
	assert( m_bInitAnim &&
		"请先初始化动画信息" );

	if ( 0 == m_numSceneFrames )
	{
		return FALSE;
	}

	CSMAnimation anim;
	anim.animName = animName;
	AnimList::iterator i = find( m_animList.begin(), m_animList.end(), anim );
	if ( i == m_animList.end() )
	{
		return FALSE;
	}

	m_curAnimState.startframe	= i->firstFrame;
	m_curAnimState.endframe		= i->firstFrame + i->numFrames - 1;
	m_curAnimState.curr_frame   = i->firstFrame;
	m_curAnimState.next_frame	= i->firstFrame + 1;
	if ( m_curAnimState.next_frame > m_curAnimState.endframe )
	{
		m_curAnimState.next_frame = m_curAnimState.startframe;
	}
	m_curAnimState.fps			= i->fps;
	m_curAnimState.name			= i->animName;

	// 更新顶点
	UpdateFrame();

	return TRUE;
}

/*! \param index 动画ID
	\return 没有动画返回FALSE
 */
BOOL AnimatableBase::SetAnimation( const int index )
{
	assert( m_bInitAnim &&
		"请先初始化动画信息" );

	if ( 0 == m_numSceneFrames )
	{
		return FALSE;
	}

	CSMAnimation anim = m_animList[index];

	m_curAnimState.startframe	= anim.firstFrame;
	m_curAnimState.endframe		= anim.firstFrame + anim.numFrames - 1;
	m_curAnimState.curr_frame   = anim.firstFrame;
	m_curAnimState.next_frame	= anim.firstFrame + 1;
	if ( m_curAnimState.next_frame > m_curAnimState.endframe )
	{
		m_curAnimState.next_frame = m_curAnimState.startframe;
	}
	m_curAnimState.fps			= anim.fps;
	m_curAnimState.name			= anim.animName;

	// 更新顶点
	UpdateFrame();

	return TRUE;
}


/*! \param fileName AM文件名称
	\param numFrames 读取的动画总帧数
	\return 读取失败返回FALSE
 */
BOOL AnimatableBase::LoadAM( const string &fileName, int numFrames )
{
	if ( numFrames <= 0 )
	{
		return FALSE;
	}

	if ( BeginReading( m_inFileAM, fileName ) )
	{
		// 逐帧读取
		for ( int n = 0; n < numFrames; n ++ )
		{
			D3DXMATRIX m;
			int frameSeek = n * sizeof( D3DXMATRIX );
			m_inFileAM.seekg( frameSeek, ifstream::beg );
			m_inFileAM.read( reinterpret_cast< char* >( &m ), sizeof( D3DXMATRIX ) );

			m_frameList.push_back( m );
		}
		EndReading( m_inFileAM );

		return TRUE;
	}

	return FALSE;
}

/*! \param fileName CFG文件名称
	\return 读取失败返回FALSE
*/
BOOL AnimatableBase::LoadCFG( const string &fileName )
{
	if ( BeginReading( m_inFileCFG, fileName, FALSE ) )
	{
		string placeholder;

		//! 读入场景帧数
		m_inFileCFG >> placeholder >> m_numSceneFrames;

		//! 读入骨骼数
		int boneNum = 0;
		m_inFileCFG >> placeholder >> boneNum;

		m_numTotalFrames = boneNum * m_numSceneFrames;

		//! 读入动画个数
		int numAnim = 0;
		m_inFileCFG >> placeholder >> numAnim;

		//! 读入每个动画的详细信息
		for ( int i = 0; i < numAnim; i ++ )
		{
			CSMAnimation animation;
			m_inFileCFG >> animation.animName
				>> animation.firstFrame
				>> animation.numFrames
				>> animation.numLoops
				>> animation.fps;

			m_animList.push_back( animation );
		}

		EndReading( m_inFileCFG );

		return TRUE;
	}

	return FALSE;
}