#include "stdafx.h"
#include <dukat/inputrecorder.h>
#include <dukat/inputdevice.h>
#include <dukat/log.h>
#include <dukat/mathutil.h>

namespace dukat
{
	void InputRecorder::start_recording(const std::string& filename, int32_t device_id)
	{
		os = std::make_unique<std::ofstream>(filename, std::ios::out | std::ios::binary);
		os->write(reinterpret_cast<const char*>(&device_id), sizeof(int32_t));
	}

	void InputRecorder::stop_recording(void)
	{
		if (os != nullptr)
		{
			os->close();
			os = nullptr;
		}
	}

	void InputRecorder::start_playback(const std::string& filename)
	{
		is = std::make_unique<std::ifstream>(filename, std::ios::in | std::ios::binary);
		is->read(reinterpret_cast<char*>(&device_id), sizeof(int32_t));
	}

	void InputRecorder::stop_playback(void)
	{
		if (is != nullptr)
		{
			is->close();
			is = nullptr;
		}
	}

	void InputRecorder::record_frame(const InputState& state)
	{
		if (os == nullptr)
			return;

		uint8_t buffer[14] = {
			0, 0, 0, 0, // <ctrl> <buttons>
			0, 0, 0, 0, // lx ly
			0, 0, 0, 0, // rx ry
			0, 0,		// lt rt
		};
		auto idx = 1;

		// Encode button states
		uint32_t buttons = 0x0;
		for (auto i = 0; i < InputDevice::VirtualButton::_Count; i++)
		{
			if (state.buttons[i])
				buttons |= 1 << i;
		}
		if (buttons != 0)
		{
			buttons = buttons << 8; // shift up to free up 1st byte
			memcpy(buffer, &buttons, 4); // copy 4 bytes, but then override 1st right away
			buffer[0] = ControlFlag::Buttons;
			idx += 3;
		}

		// Encode axis state
		if (state.lx != 0.0f)
		{
			int16_t val = denormalize(state.lx, static_cast<int16_t>(SHRT_MAX));
			memcpy(buffer + idx, &val, 2);
			idx += 2;
			buffer[0] |= ControlFlag::LX;
		}
		if (state.ly != 0.0f)
		{
			int16_t val = denormalize(state.ly, static_cast<int16_t>(SHRT_MAX));
			memcpy(buffer + idx, &val, 2);
			idx += 2;
			buffer[0] |= ControlFlag::LY;
		}
		if (state.rx != 0.0f)
		{
			int16_t val = denormalize(state.rx, static_cast<int16_t>(SHRT_MAX));
			memcpy(buffer + idx, &val, 2);
			idx += 2;
			buffer[0] |= ControlFlag::RX;
		}
		if (state.ry != 0.0f)
		{
			int16_t val = denormalize(state.ry, static_cast<int16_t>(SHRT_MAX));
			memcpy(buffer + idx, &val, 2);
			idx += 2;
			buffer[0] |= ControlFlag::RY;
		}
		if (state.lt != 0.0f)
		{
			uint8_t val = denormalize(state.lt, static_cast<uint8_t>(255));
			memcpy(buffer + idx, &val, 1);
			idx += 1;
			buffer[0] |= ControlFlag::LT;
		}
		if (state.rt != 0.0f)
		{
			uint8_t val = denormalize(state.rt, static_cast<uint8_t>(255));
			memcpy(buffer + idx, &val, 1);
			idx += 1;
			buffer[0] |= ControlFlag::RT;
		}

		os->write(reinterpret_cast<const char*>(buffer), idx);
	}

	void InputRecorder::replay_frame(InputState& state)
	{
 		if (is == nullptr || is->eof())
		{
			state.reset();
			return;
		}

		uint8_t ctrl = 0x0;
		is->read(reinterpret_cast<char*>(&ctrl), sizeof(uint8_t));

		if (ctrl == 0x0) // empty frame
		{
			state.reset();
		}
		else
		{
			uint32_t val32 = 0x0;
			int16_t val16 = 0x0;
			uint8_t val8 = 0x0;

			if (ctrl & ControlFlag::Buttons)
			{
				is->read(reinterpret_cast<char*>(&val32), 3 * sizeof(uint8_t));

				for (auto i = 0; i < InputDevice::VirtualButton::_Count; i++)
					state.buttons[i] = val32 & (1 << i);
			}
			else
			{
				std::fill(state.buttons.begin(), state.buttons.end(), 0x0);
			}

			if (ctrl & ControlFlag::LX)
			{
				is->read(reinterpret_cast<char*>(&val16), sizeof(int16_t));
				state.lx = normalize(val16);
			}
			else
			{
				state.lx = 0.0f;
			}

			if (ctrl & ControlFlag::LY)
			{
				is->read(reinterpret_cast<char*>(&val16), sizeof(int16_t));
				state.ly = normalize(val16);
			}
			else
			{
				state.ly = 0.0f;
			}

			if (ctrl & ControlFlag::RX)
			{
				is->read(reinterpret_cast<char*>(&val16), sizeof(int16_t));
				state.rx = normalize(val16);
			}
			else
			{
				state.rx = 0.0f;
			}

			if (ctrl & ControlFlag::RY)
			{
				is->read(reinterpret_cast<char*>(&val16), sizeof(int16_t));
				state.ry = normalize(val16);
			}
			else
			{
				state.ry = 0.0f;
			}

			if (ctrl & ControlFlag::LT)
			{
				is->read(reinterpret_cast<char*>(&val8), sizeof(uint8_t));
				state.lt = normalize(val8);
			}
			else
			{
				state.lt = 0.0f;
			}

			if (ctrl & ControlFlag::RT)
			{
				is->read(reinterpret_cast<char*>(&val8), sizeof(uint8_t));
				state.rt = normalize(val8);
			}
			else
			{
				state.rt = 0.0f;
			}
		}
	}
}