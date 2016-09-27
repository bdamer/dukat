#pragma once

namespace dukat
{
	class Matrix2
	{
	public:
		float m[4];

		Matrix2(void) { }
		~Matrix2(void) { }

		void setup_rotation(float angle);
	};
}
