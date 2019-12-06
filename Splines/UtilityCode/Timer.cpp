//	----==== TIMER.CPP ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		1
//	Date:			1/04
//	Description:	Sets up and gives functionality to the high speed Windows timer
//	--------------------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include "timer.h"

/*-----------------
---- FUNCTIONS ----
-----------------*/

////////// class Timer //////////

//-----------------------------------------------------------------------
//	Updates member variables for current time, and time difference since
//	the last time the function was called. Private since it is called by
//	calcTimeFix()
//-----------------------------------------------------------------------
float Timer::getTimePassed(void) {
	QueryPerformanceCounter((LARGE_INTEGER *)&currentTime);
	const float offset = (float)(currentTime - lastTime) * timerFrequencyFix;
	lastTime = currentTime;
	
	return offset;
}


//-----------------------------------------------------------------------
//	Calculates the frames per second, updates every 75 frames
//-----------------------------------------------------------------------
void Timer::calcFPS(void)
{
	numFrames++;

	if (numFrames == 75) {
		QueryPerformanceCounter((LARGE_INTEGER *)&fpsCurrentTime);
		fps = 75.0f / ((float)(fpsCurrentTime - fpsLastTime) * timerFrequencyFix);
		fpsLastTime = fpsCurrentTime;
		numFrames = 0;
	}
}


//-----------------------------------------------------------------------
//	Sets up the high speed Windows timer. If this feature is not
//	available on a system this function may cause the program to crash.
//-----------------------------------------------------------------------
void Timer::initTimer(void)
{
	QueryPerformanceFrequency((LARGE_INTEGER *)&timerFrequency);
	timerFrequencyFix = 1.0f / (float)timerFrequency;

	QueryPerformanceCounter((LARGE_INTEGER *)&fpsLastTime);
	timeFix = getTimePassed();
}