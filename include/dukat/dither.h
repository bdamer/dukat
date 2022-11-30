#pragma once

namespace dukat
{
	class Surface;
	struct Color;

	template<typename T, size_t N>
	class DitherAlgorithm
	{
	public:
		virtual ~DitherAlgorithm(void) { };
		virtual std::array<T,N> get_error(int x, int y) const = 0;
		virtual void distribute_error(int x, int y, const std::array<T, N>& error) = 0;
		virtual void next_row(void) = 0;
	};

	template<typename T, size_t N>
	class DistributingDitherAlgorithm : public DitherAlgorithm<T,N>
	{
	protected:
		const int width, height; // image dimensions
		std::vector<std::array<T, N>> cur_errors;
		std::vector<std::array<T, N>> next_errors;

	public:
		DistributingDitherAlgorithm(int width, int height) : width(width), height(height), cur_errors(width), next_errors(width)
		{
			std::fill(next_errors.begin(), next_errors.end(), std::array<T,N>());
		}

		virtual ~DistributingDitherAlgorithm(void) { }

		std::array<T,N> get_error(int x, int y) const
		{ 
			return cur_errors[x]; 
		}

		void next_row(void) 
		{
			cur_errors = next_errors;
			std::fill(next_errors.begin(), next_errors.end(), std::array<T, N>());
		}
	};

	template<typename T, size_t N>
	class FloydSteinbergDitherAlgorithm : public DistributingDitherAlgorithm<T,N>
	{
	public:
		FloydSteinbergDitherAlgorithm(int width, int height) : DistributingDitherAlgorithm(width, height) { }
		~FloydSteinbergDitherAlgorithm(void) { }

		void distribute_error(int x, int y, const std::array<T,N>& error)
		{
			for (auto i = 0; i < N; i++)
			{
				if (x < width - 1)
					cur_errors[x + 1][i] += error[i] * 7.f / 16.f;
				if (y < height - 1)
				{
					if (x > 0)
						next_errors[x - 1][i] += error[i] * 3.f / 16.f;
					next_errors[x][i] += error[i] * 5.f / 16.f;
					if (x < width - 1)
						next_errors[x + 1][i] += error[i] * 1.f / 16.f;
				}
			}
		}
	};
	
	template<typename T, size_t N>
	class SierraDitherAlgorithm : public DistributingDitherAlgorithm<T, N>
	{
	public:
		SierraDitherAlgorithm(int width, int height) : DistributingDitherAlgorithm(width, height) { }
		~SierraDitherAlgorithm(void) { }

		void distribute_error(int x, int y, const std::array<T, N>& error)
		{
			for (auto i = 0u; i < N; i++)
			{
				if (x < width - 1)
					cur_errors[x + 1][i] += error[i] * 4.f / 16.f;
				if (x < width - 2)
					cur_errors[x + 2][i] += error[i] * 3.f / 16.f;

				if (y < height - 1)
				{
					if (x > 1)
						next_errors[x - 2][i] += error[i] * 1.f / 16.f;
					if (x > 0)
						next_errors[x - 1][i] += error[i] * 2.f / 16.f;
					next_errors[x][i] += error[i] * 3.f / 16.f;
					if (x < width - 1)
						next_errors[x + 1][i] += error[i] * 2.f / 16.f;
					if (x < width - 2)
						next_errors[x + 2][i] += error[i] * 1.f / 16.f;
				}
			}
		}
	};

	/// <summary>
	/// Generates a NxN Bayer matrix used for ordered dithering.
	/// </summary>
	/// <param name="m">The matrix.</param>
	/// <param name="stride">Number of elements per row.</param>
	void generate_bayer_matrix(std::vector<int>& m, size_t stride);

	template<typename T, size_t N>
	class OrderedDitherAlgorithm : public DitherAlgorithm<T, N>
	{
	private:
		const int n;
		// bayer matrix
		std::vector<float> m;

	public:
		OrderedDitherAlgorithm(int n) : n(n), m(n* n)
		{
			std::vector<int> raw(n * n);
			generate_bayer_matrix(raw, static_cast<size_t>(n));
			// normalize
			const auto n2 = static_cast<float>(n * n);
			for (auto i = 0; i < n * n; i++)
				m[i] = static_cast<float>(raw[i]) / n2 - 0.5f;
		}

		std::array<T,N> get_error(int x, int y) const
		{
			std::array<T, N> res;
			const auto val = m[(y % n) * n + (x % n)];
			std::fill(res.begin(), res.end(), val);
			return res;
		}
		void distribute_error(int x, int y, const std::array<T, N>& error) { }
		void next_row(void) { }
	};

	void dither_image(const Surface& src, DitherAlgorithm<float, 3>& algorithm, Surface& dest);
	void dither_image(const Surface& src, DitherAlgorithm<float, 3>& algorithm, const std::vector<Color>& palette, Surface& dest);
}