#include "stdafx.h"
#include "perfcounter.h"

namespace dukat
{
	PerformanceCounter perfc;

	PerformanceCounter::PerformanceCounter(void)
	{
		for (int i = 0; i < max_counters; i++)
		{
			counters[i] = 0l;
			sums[i] = 0l;
			averages[i] = 0l;
		}
		samples = 0l;
	}

	PerformanceCounter::~PerformanceCounter(void)
	{
	}

	void PerformanceCounter::reset(void)
	{
		for (int i = 0; i < max_counters; i++)
		{
			sums[i] += counters[i];
			counters[i] = 0l;
		}
		samples++;
	}

	void PerformanceCounter::collect_stats(void)
	{
		reset();
		for (int i = 0; i < max_counters; i++)
		{
			averages[i] = (long)round((float)sums[i] / (float)samples);
			sums[i] = 0l;
		}
		samples = 0l;
	}
}