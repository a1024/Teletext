//tt_performance.cpp - Performance profiler
//Copyright (C) 2021 Ayman Wagih
//
//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include			"tt.h"
#include			<vector>
#include			<string>
#if defined TIMING_USE_QueryPerformanceCounter
#include			<Windows.h>
#endif
#ifdef PROFILER_CLIPBOARD
#include			<sstream>
#endif
#ifdef TIMING_USE_clock_gettime
#include			<time.h>
#endif
#ifdef PROFILER_CYCLES
#define				ELAPSED_FN		elapsed_cycles
#else
#define				ELAPSED_FN		elapsed_ms
#endif

static const char	file[]=__FILE__;
int					prof_on=PROF_INITIAL_STATE;
std::wstring		title;

//time-measuring functions
#ifdef __GNUC__
#define	__rdtsc	__builtin_ia32_rdtsc
#endif
double				time_sec()
{
#ifdef TIMING_USE_QueryPerformanceCounter
	static long long t=0;
	static LARGE_INTEGER li={};
	QueryPerformanceFrequency(&li);
	t=li.QuadPart;
	QueryPerformanceCounter(&li);
	return (double)li.QuadPart/t;
#elif defined TIMING_USE_clock_gettime
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return ts.tv_sec+ts.tv_nsec*1e-9;
#elif defined TIMING_USE_rdtsc
	static LARGE_INTEGER li={};
	QueryPerformanceFrequency(&li);
	return (double)__rdtsc()*0.001/li.QuadPart;//pre-multiplied by 1000
#elif defined TIMING_USE_GetProcessTimes
	FILETIME create, exit, kernel, user;
	int success=GetProcessTimes(GetCurrentProcess(), &create, &exit, &kernel, &user);
	if(success)
//#ifdef PROFILER_CYCLES
	{
		const auto hns2sec=100e-9;
		return hns2ms*(unsigned long long&)user;
	//	return hns2ms*(unsigned long long&)kernel;
	}
//#else
//	{
//		SYSTEMTIME t;
//		success=FileTimeToSystemTime(&user, &t);
//		if(success)
//			return t.wHour*3600000.+t.wMinute*60000.+t.wSecond*1000.+t.wMilliseconds;
//		//	return t.wHour*3600.+t.wMinute*60.+t.wSecond+t.wMilliseconds*0.001;
//	}
//#endif
	SYS_CHECK();
	return -1;
#elif defined TIMING_USE_GetTickCount
	return (double)GetTickCount()*0.001;//the number of milliseconds that have elapsed since the system was started
#elif defined TIMING_USE_timeGetTime
	return (double)timeGetTime()*0.001;//system time, in milliseconds
#endif
}
double				time_ms()
{
#ifdef TIMING_USE_QueryPerformanceCounter
	static long long t=0;
	static LARGE_INTEGER li={};
	QueryPerformanceFrequency(&li);
	t=li.QuadPart;
	QueryPerformanceCounter(&li);
	return (double)li.QuadPart*1000./t;
#elif defined TIMING_USE_clock_gettime
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return ts.tv_sec*1000.+ts.tv_nsec*1e-6;
#elif defined TIMING_USE_rdtsc
	static LARGE_INTEGER li={};
	QueryPerformanceFrequency(&li);
	return (double)__rdtsc()/li.QuadPart;//pre-multiplied by 1000
#elif defined TIMING_USE_GetProcessTimes
	FILETIME create, exit, kernel, user;
	int success=GetProcessTimes(GetCurrentProcess(), &create, &exit, &kernel, &user);
	if(success)
//#ifdef PROFILER_CYCLES
	{
		const auto hns2ms=100e-9*1000.;
		return hns2ms*(unsigned long long&)user;
	//	return hns2ms*(unsigned long long&)kernel;
	}
//#else
//	{
//		SYSTEMTIME t;
//		success=FileTimeToSystemTime(&user, &t);
//		if(success)
//			return t.wHour*3600000.+t.wMinute*60000.+t.wSecond*1000.+t.wMilliseconds;
//		//	return t.wHour*3600.+t.wMinute*60.+t.wSecond+t.wMilliseconds*0.001;
//	}
//#endif
	SYS_CHECK();
	return -1;
#elif defined TIMING_USE_GetTickCount
	return (double)GetTickCount();//the number of milliseconds that have elapsed since the system was started
#elif defined TIMING_USE_timeGetTime
	return (double)timeGetTime();//system time, in milliseconds
#endif
}
double				elapsed_ms()//since last call
{
	static double t1=0;
	double t2=time_ms(), diff=t2-t1;
	t1=t2;
	return diff;
}
double				elapsed_cycles()//since last call
{
	static long long t1=0;
	long long t2=__rdtsc();
	double diff=double(t2-t1);
	t1=t2;
	return diff;
}

void				prof_toggle()
{
	prof_on=!prof_on;
#ifdef PROFILER_TITLE
	if(prof_on)//start
	{
		const int size=1024;
		title.resize(size);
		get_window_title_w(&title[0], size);
//#ifdef _MSC_VER
//		GetWindowTextW(ghWnd, &title[0], size);
//#endif
	}
	else//end
		set_window_title_w(title.c_str());
//#ifdef _MSC_VER
//		SetWindowTextW(ghWnd, title.data());
//#endif
#elif defined PROFILER_CMD
	if(prof_on)//start
		log_start(LL_PROGRESS);
	else//end
		log_end();
#endif
}

typedef std::pair<std::string, double> ProfInfo;
std::vector<ProfInfo> prof;
int					prof_array_start_idx=0;
double				elapsed=0;
void				prof_start(){double elapsed=ELAPSED_FN();}
void				prof_add(const char *label, int divisor)
{
	if(prof_on)
	{
		double elapsed=ELAPSED_FN();
		prof.push_back(ProfInfo(label, elapsed));
	}
}
void				prof_sum(const char *label, int count)//add the sum of last 'count' steps
{
	if(prof_on)
	{
		double sum=0;
		for(int k=prof.size()-1, k2=0;k>=0&&k2<count;--k, ++k2)
			sum+=prof[k].second;
		prof.push_back(ProfInfo(label, sum));
	}
}
void				prof_loop_start(const char **labels, int n)//describe the loop body parts in 'labels'
{
	if(prof_on)
	{
		prof_array_start_idx=prof.size();
		for(int k=0;k<n;++k)
			prof.push_back(ProfInfo(labels[k], 0));
	}
}
void				prof_add_loop(int idx)//call on each part of loop body
{
	if(prof_on)
	{
		double elapsed=ELAPSED_FN();
		prof[prof_array_start_idx+idx].second+=elapsed;
	}
}
void				prof_print()
{
	if(prof_on)
	{
#ifdef PROFILER_TITLE
		int len=0;
		if(prof.size())
		{
			len+=sprintf_s(g_buf+len, g_buf_size-len, "%s: %lf", prof[0].first.c_str(), prof[0].second);
			for(int k=1, kEnd=prof.size();k<kEnd;++k)
				len+=sprintf_s(g_buf+len, g_buf_size-len, ", %s: %lf", prof[k].first.c_str(), prof[k].second);
			//	len+=sprintf_s(g_buf+len, g_buf_size-len, "%s\t\t%lf\r\n", prof[k].first.c_str(), prof[k].second);
		}
		set_window_title_a(g_buf);
		//int success=SetWindowTextA(ghWnd, g_buf);
		//SYS_ASSERT(success);
#elif defined PROFILER_CMD
		printf("\n");
		for(int k=0, kEnd=prof.size();k<kEnd;++k)
		{
			auto &p=prof[k];
			printf("%s\t%lf\n", p.first.c_str(), p.second);
		}
		printf("\n");
#else
		const int fontH=16;//18
		int xpos=w-400, xpos2=w-200;
		for(int k=0, kEnd=prof.size();k<kEnd;++k)
		{
			auto &p=prof[k];
			int ypos=k<<4;
			GUIPrint(xpos, ypos, p.first.c_str());
			GUIPrint(xpos2, ypos, "%lf", p.second);
		//	GUIPrint(xpos2, ypos, "%g", p.second);
		}
		static double t1=time_sec();
		double t2=time_sec();
		GUIPrint(xpos, prof.size()<<4, "fps=%lf, T=%lfms", 1/(t2-t1), 1000*(t2-t1));
		t1=t2;
#endif

		//copy to clipboard
#ifdef PROFILER_CLIPBOARD
		std::stringstream LOL_1;
		for(int k=0, kEnd=prof.size();k<kEnd;++k)
			LOL_1<<prof[k].first<<"\t\t"<<prof[k].second<<"\r\n";
		auto &str=LOL_1.str();
		copy_to_clipboard(str.c_str(), str.size());
	//	int len=0;
	//	for(int k=0, kEnd=prof.size();k<kEnd;++k)
	//		len+=sprintf_s(g_buf+len, g_buf_size-len, "%s\t\t%lf\r\n", prof[k].first.c_str(), prof[k].second);
	//	copy_to_clipboard(g_buf, len);
#endif
		prof.clear();
	}
}