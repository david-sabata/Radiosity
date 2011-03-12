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

/**
 *	@file Timer.h
 *	@brief Multi-platform timer
 *	@author -tHE SWINe-
 *
 *	@date 2006-08-23
 *	passed code revision
 *
 *	removed some unnecessary #defines for linux
 *	added higher precision timer for linux (instead of clock(), gettimeofday() can be used)
 *
 *	@date 2007-05-10
 *
 *	added conditional #include <windows.h> to make it easier to include
 *
 *	@date 2008-04-09
 *
 *	added GetTickCount method, redesigned timer code. should cope nicely with counter overflow
 *	(previous version of timer with QueryPerformanceCounter had some trouble on core duo cpu's)
 *
 *	@date 2008-08-02
 *
 *	cleared up code arround n_MaxValue() a bit, removed MAX_VALUE_FUNCTION and MAX_VALUE_CONST
 *	and moved all the stuff to Integer.h
 *
 *	@date 2008-08-08
 *
 *	added #ifdef for windows 64
 *
 *	@date 2009-05-04
 *
 *	fixed mixed windows / linux line endings
 *
 *	@date 2010-01-08
 *
 *	added the PRItime and PRItimeparams macros for easy printfing of time values
 *
 *	@date 2010-10-29
 *
 *	Unified windows detection macro to "#if defined(_WIN32) || defined(_WIN64)".
 *
 */

#ifndef __TIMER_INCLUDED
#define __TIMER_INCLUDED

/**
 *	@def PRItime
 *
 *	@brief printf macro for printing time in the "hh:mm:ss.ss" format
 *
 *	This macro is used similar to standard PRI64 macro, it just defines
 *	proper formatting string. It should be used in conjunction with the
 *	PRItimeparams macro, as follows:
 *@code
 *	double f_time = 123456;
 *	printf("elapsed time: " PRItime "\n", PRItimeparams(f_time));
 *@endcode
 */
#define PRItime "%s%02d:%02d:%05.2f"

/**
 *	@def PRItimeprecise
 *
 *	@brief printf macro for printing time in the "hh:mm:ss.sssss" format
 *
 *	This macro is used similar to standard PRI64 macro, it just defines
 *	proper formatting string. It should be used in conjunction with the
 *	PRItimeparams macro, as follows:
 *@code
 *	double f_time = 123456;
 *	printf("elapsed time: " PRItimeprecise "\n", PRItimeparams(f_time));
 *@endcode
 */
#define PRItimeprecise "%s%02d:%02d:%08.5f"

/**
 *	@def PRItimeparams
 *
 *	@brief splits it's argument to integer hours, integer minutes
 *		and floating-point seconds
 *
 *	Calculates sign and three values in hours, minutes and seconds from given time.
 *	These values are separated using colons, and are intended to be used as
 *	function parameters. It could be used as:
 *
 *@code
 *	void PrintTime3(const char *sign, int hh, int mm, float ss)
 *	{
 *		printf("time is: %s%02d:%02d:%05.2f ...\n", sign, hh, mm, ss);
 *	}
 *
 *	void PrintTime(float seconds)
 *	{
 *		PrintTime3(PRItimeparams(seconds));
 *	}
 *@endcode
 *
 *	It's however intended to be used along with PRItime. Refer to PRItime
 *	macro documentation for more details.
 *
 *	@param[in] f is time in seconds to be split into hours, minutes, seconds
 */
#define PRItimeparams(f) (((f) >= 0)? "" : "-"), int(f) / 3600, \
	(((f) >= 0)? 1 : -1) * (int(f) / 60 - 60 * (int(f) / 3600)), \
	(((f) >= 0)? 1 : -1) * ((f) - 60 * (int(f) / 60))

/*
 *	- this macro enables use of QueryPerformanceCounter() function (win32-only)
 */
#define TIMER_ALLOW_QPC

/*
 *	- this macro enables use of GetTickCount() function (win32-only)
 */
#define TIMER_ALLOW_GETTICKCOUNT

/*
 *	- this macro enables use of gettimeofday() function (unix-only)
 */
#define TIMER_ALLOW_GETTIMEOFDAY

/*
 *	- this macro disables automatic timer method selection
 *	- one of following macros must be defined as well to choose timer method:
 *		TIMER_USE_QPC				(win32-only, resolution less than 1 usec, depends on cpu)
 *		TIMER_USE_GETTICKCOUNT		(win32-only, resolution 1 msec)
 *		TIMER_ALLOW_GETTIMEOFDAY	(unix-only, resolution 1 usec)
 *		TIMER_USE_CLOCK				(default fallback, resolution 1 msec or less, depends on os)
 */
//#define TIMER_METHOD_OVERRIDE

#ifndef TIMER_METHOD_OVERRIDE
#if defined(_WIN32) || defined (_WIN64)
#if defined(TIMER_ALLOW_QPC)
#define TIMER_USE_QPC
#elif defined(TIMER_ALLOW_GETTICKCOUNT)
#define TIMER_USE_GETTICKCOUNT
#else
#define TIMER_USE_CLOCK
#endif
#else // _WIN32, _WIN64
#ifdef TIMER_ALLOW_GETTIMEOFDAY
#define TIMER_USE_GETTIMEOFDAY
#else
#define TIMER_USE_CLOCK
#endif
#endif // _WIN32, _WIN64
#endif //TIMER_METHOD_OVERRIDE

//#include "Integer.h"

class CTimer {
	mutable int64_t m_n_freq, m_n_time;
	mutable double m_f_time;

public:
	/*
	 *	CTimer::CTimer()
	 *		- default constructor
	 */
	CTimer();

	/*
	 *	void CTimer::ResetTimer()
	 *		- resets timer (sets time to zero)
	 */
	void ResetTimer();

	/*
	 *	double CTimer::f_Time() const
	 *		- returns time in seconds
	 *		- should cope nicely with counter overflows
	 */
	double f_Time() const;

	/*
	 *	int64_t CTimer::n_Frequency() const
	 *		- returns timer frequency (inverse of smallest time step)
	 */
	int64_t n_Frequency() const;

protected:
	static inline int64_t n_SampleTimer();
};

#endif //__TIMER_INCLUDED
