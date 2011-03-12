/*
								+----------------------------------+
								|                                  |
								| ***   Multi-platform timer   *** |
								|                                  |
								|   Copyright © -tHE SWINe- 2006   |
								|                                  |
								|             Timer.h              |
								|                                  |
								+----------------------------------+
*/

/*
 *	passed code revision at 2006-08-23
 *
 *	removed some unnecessary #defines for linux
 *	added higher precision timer for linux (instead of clock(), gettimeofday() can be used)
 *
 *	2007-05-10
 *
 *	added conditional #include <windows.h> to make it easier to include
 *
 *	2008-04-09
 *
 *	added GetTickCount method, redesigned timer code. should cope nicely with counter overflow
 *	(previous version of timer with QueryPerformanceCounter had some trouble on core duo cpu's)
 *
 *	2008-08-02
 *
 *	cleared up code arround n_MaxValue() a bit, removed MAX_VALUE_FUNCTION and MAX_VALUE_CONST
 *	and moved all the stuff to Integer.h
 *
 *	2008-08-08
 *
 *	added #ifdef for windows 64
 *
 *	2009-05-04
 *
 *	fixed mixed windows / linux line endings
 *
 */

//#include "NewFix.h"

#include <stdint.h>
//#include "CallStack.h"
#if defined(_WIN32) || defined (_WIN64)
#include <windows.h>
#else //_WIN32 || _WIN64
#include <unistd.h>
#include <sys/time.h>
#endif //_WIN32 || _WIN64
#include <time.h>
#include "Timer.h"

/*
 *								=== CTimer ===
 */

/*
 *	CTimer::CTimer()
 *		- default constructor
 */
CTimer::CTimer()
{
	//__FuncGuard("CTimer::CTimer");

#if defined(TIMER_USE_QPC)
	LARGE_INTEGER t_freq;
	QueryPerformanceFrequency(&t_freq);
	//_ASSERTE(t_freq.QuadPart < INT64_MAX);
	m_n_freq = t_freq.QuadPart;
	// determine QPC frequency

#ifdef _DEBUG
	LARGE_INTEGER t_tmp = {0};
	//_ASSERTE(n_MaxIntValue(t_tmp.QuadPart) == INT64_MAX);
	// make sure max value of QPC's counter is INT64_MAX
#endif //_DEBUG
#elif defined(TIMER_USE_GETTICKCOUNT)
	m_n_freq = 1000; // one milisecond

#ifdef _DEBUG
	_ASSERTE(n_MaxIntValue(GetTickCount()) == UINT32_MAX);
	// make sure max value of GetTickCount's counter is UINT32_MAX
#endif //_DEBUG
#elif defined(TIMER_USE_GETTIMEOFDAY)
	m_n_freq = 1000000; // one microsecond

#ifdef _DEBUG
	timeval t_tmp_time = {0, 0};
	_ASSERTE(n_MaxIntValue(t_tmp_time.tv_sec) < UINT64_MAX / 1000000);
	_ASSERTE(n_MaxIntValue(t_tmp_time.tv_sec) * 1000000 <= UINT64_MAX - 999999);
	// make sure we fit into int64_t
#endif //_DEBUG
#else // clock
	m_n_freq = CLOCKS_PER_SEC;
#endif
	ResetTimer();
}

/*
 *	int64_t CTimer::n_Frequency() const
 *		- returns timer frequency (inverse of smallest time step)
 */
int64_t CTimer::n_Frequency() const
{
	//__FuncGuard("CTimer::n_Frequency");

	return m_n_freq;
}

/*
 *	static inline int64_t CTimer::n_SampleTimer()
 *		- returns time counter sample
 */
inline int64_t CTimer::n_SampleTimer()
{
	//__FuncGuard("CTimer::n_SampleTimer");

#if defined(TIMER_USE_QPC)
	LARGE_INTEGER t_time;
	QueryPerformanceCounter(&t_time);
	//_ASSERTE(t_time.QuadPart < INT64_MAX);
	return t_time.QuadPart;
#elif defined(TIMER_USE_GETTICKCOUNT)
	return GetTickCount();
#elif defined(TIMER_USE_GETTIMEOFDAY)
	timeval t_tmp_time;
    gettimeofday(&t_tmp_time, NULL);
	return t_tmp_time.tv_sec * 1000000 + t_tmp_time.tv_usec;
#else // clock
	return clock();
#endif
}

/*
 *	void CTimer::ResetTimer()
 *		- resets timer (sets time to zero)
 */
void CTimer::ResetTimer()
{
	//__FuncGuard("CTimer::ResetTimer");

	m_f_time = 0;
	m_n_time = n_SampleTimer();
}

/*
 *	double CTimer::f_Time() const
 *		- returns time in seconds
 *		- should cope nicely with counter overflows
 */
double CTimer::f_Time() const
{
	//__FuncGuard("CTimer::f_Time");

	int64_t n_cur_time = n_SampleTimer();
	// determine current time

	int64_t n_delta_time;
	if(n_cur_time >= m_n_time)
		n_delta_time = n_cur_time - m_n_time;
	else {
#if defined(TIMER_USE_QPC)
		int64_t n_max_time_value = INT64_MAX;
#elif defined(TIMER_USE_GETTICKCOUNT)
		int64_t n_max_time_value = UINT32_MAX;
#elif defined(TIMER_USE_GETTIMEOFDAY)
		timeval t_tmp_time = {0, 0};
		int64_t n_max_time_value = n_MaxValue(t_tmp_time.tv_sec) * 1000000 + 999999;
		_ASSERTE(n_MaxValue(t_tmp_time.tv_sec) < UINT64_MAX / 1000000);
		_ASSERTE(n_MaxValue(t_tmp_time.tv_sec) * 1000000 <= UINT64_MAX - 999999);
#else // clock
		int64_t n_max_time_value = CMaxIntValue<clock_t>::result();
#endif
		// determine maximal time value, based on used counter

		n_delta_time = n_max_time_value - m_n_time;
		if(n_delta_time <= INT64_MAX - n_cur_time)
			n_delta_time += n_cur_time;
		else {
			m_f_time += double(n_cur_time) / m_n_freq;
			// adding n_cur_time would cause overflow ... so add it this way
		}
		// calculate proper difference time
	}
	// calculate delta time

	m_f_time += double(n_delta_time) / m_n_freq;
	m_n_time = n_cur_time;
	// integrate time

	return m_f_time;
}

/*
 *								=== ~CTimer ===
 */
