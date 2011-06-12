#ifndef __CAMERA_H__
#define __CAMERA_H__

class Camera
{
public:
	/*! \brief 默认构造函数 */
	Camera( const D3DXVECTOR3 tracePos, const float distance, const float minDis, const float maxDis );

	/*! \brief 析构函数 */
	~Camera();

	/*! \brief 重置摄像机 */
	void Reset();

	/*! \brief 重置摄像机视图矩阵 */
	D3DXMATRIX UpdateViewMatrix();

	/*! \brief 计算摄像机的预测位置 */
	D3DXVECTOR3 CalculatePossibleCamPosition();

	//set
	void SetTracePosition( const D3DXVECTOR3 pos ) { m_tracePos = pos; }
	void SetPossibleCamPosition( const D3DXVECTOR3 possibleCamPosition ) { m_possibleCameraPos = possibleCamPosition; }
	void IncreaseTracePosition( const D3DXVECTOR3 posInc ) { m_tracePos += posInc; } 
	void SetDistance( float distance ) { m_distance = distance; }
	void SetMinMaxDistance( float minDis, float maxDis ) { m_fMinDis = minDis; m_fMaxDis = maxDis; }
	void SetHArc( const float arc ) { m_HArc = arc; }
	void SetVArc( const float arc ) { m_VArc = arc; }

	/* \brief 增加水平角度 */
	void IncreaseHArc( const float arcInc );

	/* \brief 增加垂直角度 */
	void IncreaseVArc( const float arcInc );

	/*! \brief 增加跟踪距离 */
	void IncreaseDistance( float distanceInc );

	//get
	D3DXVECTOR3 GetTracePosition() const { return m_tracePos; }
	D3DXVECTOR3 GetCamPosition() const { return m_cameraPos; }

	/*! \brief 获得摄像机方向 */
	D3DXVECTOR3 GetDirection();

	D3DXVECTOR3 GetPossibleCamPosition() { return m_possibleCameraPos; }
	float GetHArc() { return m_HArc; }
	float GetVArc() { return m_VArc; }
	D3DXMATRIX GetViewMatrix() { return m_matView; }

private:
	D3DXVECTOR3 m_cameraPos;			//!< 摄像机位置
	D3DXVECTOR3 m_possibleCameraPos;	//!< 预测的摄像机位置
	D3DXVECTOR3 m_tracePos;				//!< 跟踪点
	float m_distance;					//!< 距离跟踪点的距离
	float m_HArc;						//!< 水平旋转的角度
	float m_VArc;						//!< 垂直旋转的角度
	D3DXMATRIX m_matView;				//!< 摄像机视图矩阵

	float m_fMaxDis;					//!< 最大距离
	float m_fMinDis;					//!< 最短距离
};

#endif