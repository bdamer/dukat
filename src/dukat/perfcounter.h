#pragma once

namespace dukat
{
	// Simple performance counter to collect engine metrics.
	class PerformanceCounter
	{
	private:
		static const int max_counters = 256;
		// current counters
		long counters[max_counters];
		// accumulator
		long sums[max_counters];
		// last average
		long averages[max_counters];
		// number of samples collected
		long samples;

	public:
		// Well-known counters
		enum ID
		{
			FRAMES,
			MESHES,
			VERTICES,
			PARTICLES,
			TEXTURES,
			SHADERS,
			BUFFER_FREE,
			SPRITES,
			SAMPLES,
			BB_CHECKS
		};

		PerformanceCounter(void);
		~PerformanceCounter(void);

		void reset(void);
		// collects average values for all counters
		void collect_stats(void);

		void set(int counter, long value) { counters[counter] = value; }
		long get(int counter) const { return counters[counter]; }
		void inc(int counter, int val = 1) { counters[counter] += val; }
		void dec(int counter, int val = 1) { counters[counter] += val; }
		long sum(int counter) { return sums[counter]; }
		long avg(int counter) { return averages[counter]; }
	};

	extern PerformanceCounter perfc;
}