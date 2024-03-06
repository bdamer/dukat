#pragma once

#include <array>

namespace dukat
{
	struct InputState
	{
		std::array<Uint32, 21> buttons; // tracks current button state
		float lx, ly; // relative left axis [-1..1]
		float rx, ry; // relative right axis [-1..1]
		float lt, rt; // left and right triggers
	
		InputState(void) : buttons(), lx(0.0f), ly(0.0f), rx(0.0f), ry(0.0f), lt(0.0f), rt(0.0f) { }
		InputState(const std::array<Uint32, 21>& buttons, float lx, float ly, float rx, float ry, float lt, float rt) 
			: buttons(buttons), lx(lx), ly(ly), rx(rx), ry(ry), lt(lt), rt(rt) { }

		// zeros out state
		void reset(void)
		{
			std::fill(buttons.begin(), buttons.end(), 0x0);
			lx = ly = 0.0f;
			rx = ry = 0.0f;
			lt = rt = 0.0f;
		}
	};
}