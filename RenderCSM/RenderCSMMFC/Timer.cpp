#include "stdafx.h"
#include "Timer.h"

Timer::Timer()
{

	QueryPerformanceFrequency( (LARGE_INTEGER *)&m_ticksPerSecond );

	m_currentTime     =   m_lastTime  =  m_lastFPSUpdate = 0;
	m_numFrames     =   0;
	m_runningTime    =    m_timeElapsed  =  m_fps  =  0.0f;
	m_FPSUpdateInterval   =  m_ticksPerSecond >> 1;

	m_timerStopped = TRUE;

}

void Timer::Start()
{
	if ( !m_timerStopped )
	{
		// Already started
		return;
	}
	QueryPerformanceCounter( ( LARGE_INTEGER * )&m_lastTime );
	m_timerStopped = FALSE;
}

void Timer::Stop()
{
	if ( m_timerStopped )
	{
		// Already stopped
		return;
	}
	INT64 stopTime = 0;
	QueryPerformanceCounter( (LARGE_INTEGER *)&stopTime );
	m_runningTime     += (float)(stopTime - m_lastTime) / (float)m_ticksPerSecond;
	m_timerStopped     = TRUE;
}

void Timer::Update()
{
	if ( m_timerStopped )
	{
		return;
	}

	// Get the current time
	QueryPerformanceCounter( (LARGE_INTEGER *)&m_currentTime );

	m_timeElapsed = (float)(m_currentTime - m_lastTime) / (float)m_ticksPerSecond;
	m_runningTime += m_timeElapsed;

	// Update FPS
	m_numFrames++;
	if ( m_currentTime - m_lastFPSUpdate >= m_FPSUpdateInterval )//在达到一定的interval计数后才计算FPS，否则只是累加帧数m_numFrames
	{
		float currentTime = (float)m_currentTime / (float)m_ticksPerSecond;
		float lastTime = (float)m_lastFPSUpdate / (float)m_ticksPerSecond;
		m_fps = (float)m_numFrames / (currentTime - lastTime);

		m_lastFPSUpdate = m_currentTime;
		m_numFrames = 0;
	}

	m_lastTime = m_currentTime;
}