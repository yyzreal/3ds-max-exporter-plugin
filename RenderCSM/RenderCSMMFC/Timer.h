#ifndef __TIMER_H__
#define __TIMER_H__

class Timer
{
public:
	/*! \brief 构造函数 */
	Timer();

	/*! \brief 开始计时器 */
	void Start();

	/*! \brief 停止计时器 */
	void Stop();

	/*! \brief 更新计时器\n
	    计算自上次更新后的逝去时间，更新FPS和总运行时间 */
	void Update();

	BOOL IsStopped() { return m_timerStopped; }
	float GetFPS() { return m_fps; }
	float GetRunningTime() { return m_runningTime; }
	float GetElapsedTime() { return m_timerStopped ? 0.0f : m_timeElapsed; }

private:
	INT64 m_ticksPerSecond;
	INT64 m_currentTime;
	INT64 m_lastTime;
	INT64 m_lastFPSUpdate;
	INT64 m_FPSUpdateInterval;
	UINT m_numFrames;
	float m_runningTime;
	float m_timeElapsed;
	float m_fps;
	BOOL m_timerStopped;		//timer是否停止
};
#endif