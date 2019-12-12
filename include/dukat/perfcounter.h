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
			FRAMES,			// No# of frames
			MESHES,			// No# of meshes rendered
			VERTICES,		// No# of vertices rendered
			PARTICLES,		// No# of particles rendered
			TEXTURES,		// No# of texture switches
			SHADERS,		// No# of shader switches
			BUFFER_FREE,	// No# of freed buffers
			FRAME_BUFFERS,	// No# of frame buffer switches
			SPRITES,		// No# of sprites rendered
			SPRITES_TOTAL,	// No# of total sprites on layers
			SAMPLES,		// No# of sampling operations
			BB_CHECKS,		// No# of bounding-box checks
			ENTITIES,		// No# of active game entities
			ENTITIES_TOTAL, // No# of total game entities
			BODIES,			// No# of collision bodies
			TIMERS,			// No# of active timers
			CUSTOM1,		// Custom counters
			CUSTOM2,
			CUSTOM3,
			CUSTOM4,
			CUSTOM5
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