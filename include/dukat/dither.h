#pragma once

namespace dukat
{
	class Surface;
	struct Color;

	/// <summary>
	/// Generates a square Bayer matrix used for ordered dithering.
	/// </summary>
	/// <param name="m">The matrix.</param>
	/// <param name="n">Number of elements per row / col is 2^n.</param>
	void generate_bayer_matrix(std::vector<int>& m, int n);

	/// <summary>
	/// Generates a normalized, square Bayer matrix used for ordered dithering.
	/// </summary>
	/// <param name="m">The matrix.</param>
	/// <param name="n">Number of elements per row / col is 2^n.</param>
	void generate_bayer_matrix(std::vector<float>& m, int n);

	/// <summary>
	/// Applies ordered dithering to a monochrome surface using a Bayer matrix.
	/// </summary>
	/// <param name="surface">The surface.</param>
	/// <param name="n">Determines the size of the Bayer matrix, size being 2^n</param>
	/// <param name="palette">Colors to apply for the monochrome min and max values</param>
	void dither_ordered(Surface& surface, int n, const std::array<Color, 2>& palette);

	/// <summary>
	/// Applies dithering algorithm to a monochrome surface.
	/// </summary>
	/// <typeparam name="Algorithm">The algorithm type.</typeparam>
	/// <param name="surface">The surface</param>
	/// <param name="algorithm">Instance of the dithering algorithm</param>
	/// <param name="palette">Colors to apply for the monochrome min and max values</param>
	template<class Algorithm>
	void dither(Surface& surface, Algorithm algorithm, const std::array<Color, 2>& palette)
	{
		for (auto y = 0; y < surface.height(); y++)
		{
			algorithm.next_row();
			for (auto x = 0; x < surface.width(); x++)
			{
				auto color = luminance(surface.get_color(x, y)) + algorithm.get_error(x, y);

				float error;
				if (color < 0.5f)
				{
					error = color;
					surface.set_color(x, y, palette[0]);
				}
				else
				{
					error = color - 1.0f;
					surface.set_color(x, y, palette[1]);
				}
				algorithm.distribute_error(x, y, error);
			}
		}
	}

	#pragma region Distributing dithering algorithms

	template<typename T>
	class FloydSteinbergDitherAlgorithm
	{
	private:
		const int width, height; // image dimensions
		std::vector<T> cur_errors;
		std::vector<T> next_errors;

	public:
		FloydSteinbergDitherAlgorithm(int width, int height) : width(width), height(height), cur_errors(width), next_errors(width)
		{ 
			std::fill(next_errors.begin(), next_errors.end(), static_cast<T>(0));
		}

		T get_error(int x, int y) const 
		{
			return cur_errors[x]; 
		}

		void next_row(void)
		{
			cur_errors = next_errors;
			std::fill(next_errors.begin(), next_errors.end(), static_cast<T>(0));
		}

		void distribute_error(int x, int y, const T& error)
		{
			if (x < width - 1)
				cur_errors[x + 1] += error * 7.f / 16.f;
			if (y < height - 1)
			{
				if (x > 0)
					next_errors[x - 1] += error * 3.f / 16.f;
				next_errors[x] += error * 5.f / 16.f;
				if (x < width - 1)
					next_errors[x + 1] += error * 1.f / 16.f;
			}
		}
	};
	
	template<typename T>
	class SierraDitherAlgorithm
	{
	private:
		const int width, height; // image dimensions
		std::vector<T> cur_errors;
		std::vector<T> next_errors;

	public:
		SierraDitherAlgorithm(int width, int height) : width(width), height(height), cur_errors(width), next_errors(width)
		{
			std::fill(next_errors.begin(), next_errors.end(), static_cast<T>(0));
		}

		T get_error(int x, int y) const
		{
			return cur_errors[x];
		}

		void next_row(void)
		{
			cur_errors = next_errors;
			std::fill(next_errors.begin(), next_errors.end(), static_cast<T>(0));
		}

		void distribute_error(int x, int y, const T& error)
		{
			if (x < width - 1)
				cur_errors[x + 1] += error * 4.f / 16.f;
			if (x < width - 2)
				cur_errors[x + 2] += error * 3.f / 16.f;

			if (y < height - 1)
			{
				if (x > 1)
					next_errors[x - 2] += error * 1.f / 16.f;
				if (x > 0)
					next_errors[x - 1] += error * 2.f / 16.f;
				next_errors[x] += error * 3.f / 16.f;
				if (x < width - 1)
					next_errors[x + 1] += error * 2.f / 16.f;
				if (x < width - 2)
					next_errors[x + 2] += error * 1.f / 16.f;
			}
		}
	};

	#pragma endregion

}