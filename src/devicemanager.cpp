#include "stdafx.h"
#include <dukat/devicemanager.h>
#include <dukat/inputrecorder.h>
#include <dukat/log.h>
#include <dukat/keyboarddevice.h>
#include <dukat/gamepaddevice.h>
#include <dukat/playbackdevice.h>
#include <dukat/mathutil.h>
#include <dukat/settings.h>

#ifdef XINPUT_SUPPORT
#include <dukat/xboxdevice.h>
#endif

namespace dukat
{
	void DeviceManager::add_keyboard(void)
	{
		log->info("Keyboard device added.");
		controllers.push_back(std::make_unique<KeyboardDevice>(window, settings));
		active = controllers.back().get();
		trigger(Message{ Events::DeviceBound });
	}

	void DeviceManager::remove_keyboard(void)
	{
		trigger(Message{ Events::DeviceUnbound });
	}

	void DeviceManager::add_joystick(int joystick_index)
	{
		if (!settings.get_bool("input.joystick.support", true))
			return;

		const std::string name = SDL_JoystickNameForIndex(joystick_index);
		log->info("Joystick device @ index [{}] added: {}", joystick_index, name);
		
		if (active != nullptr && active->id() != KeyboardDevice::keyboard_id)
		{
			log->warn("Already using joystick device, ignoring new device.");
			return;
		}
#ifdef XINPUT_SUPPORT
		const std::string xinput = "XInput Controller";
		if (name.rfind(xinput, 0) == 0 || 
			name == "Controller (Xbox 360 Wireless Receiver for Windows)")
		{
			controllers.push_back(std::make_unique<XBoxDevice>(window, settings, joystick_index));
		}
		else
		{
#endif
			controllers.push_back(std::make_unique<GamepadDevice>(window, settings, joystick_index));
#ifdef XINPUT_SUPPORT
		}
#endif		
		trigger(Message{ Events::DeviceUnbound });
		active = controllers.back().get();
		trigger(Message{ Events::DeviceBound });
		apply_feedback(true);
	}

	void DeviceManager::remove_joystick(SDL_JoystickID id)
	{
		if (!settings.get_bool("input.joystick.support", true))
			return;
		log->info("Joystick device removed: {}", id);
		auto it = std::find_if(controllers.begin(), controllers.end(),
			[id](const std::unique_ptr<InputDevice>& dev) { return dev->id() == id; });
		if (it == controllers.end())
			return; // device not found
		controllers.erase(it);
		trigger(Message{ Events::DeviceUnbound });
		active = controllers.back().get();
		trigger(Message{ Events::DeviceBound });
		apply_feedback(true);
	}

	void DeviceManager::update(float delta)
	{
		if (active != nullptr && enabled)
		{
			active->update();
			if (recording && recorder != nullptr)
				recorder->record_frame(active->get_state());

			update_feedback_stack(delta);
		}
	}

	void DeviceManager::update_feedback_stack(float delta)
	{
		if (feedback_paused || feedback_stack.empty())
			return;

		auto it = feedback_stack.begin();
		while (it != feedback_stack.end())
		{
			(*it)->update(delta);
			if ((*it)->is_done()) 
			{
				log->debug("Feedback sequence is done.");
				it = feedback_stack.erase(it);
			}
			else
				++it;
		}

		apply_feedback(false);
	}

	void DeviceManager::apply_feedback(bool force)
	{
		auto duration = 0.0f;
		auto low = 0.0f, high = 0.0f;

		if (!feedback_stack.empty())
		{
			const auto& f = feedback_stack.front();
			duration = f->keys[f->next_key].index - f->index; // how long till next transition
			low = f->keys[f->cur_key].value.low;
			high = f->keys[f->cur_key].value.high;
		}

		if (force || low != feedback_val.low || high != feedback_val.high)
		{
			active->start_feedback(low, high, duration);
			feedback_val.low = low;
			feedback_val.high = high;
		}
	}

	FeedbackSequence* DeviceManager::start_feedback(std::unique_ptr<FeedbackSequence> feedback)
	{
		log->debug("Starting feedback sequence.");
		feedback_stack.push_front(std::move(feedback));
		return feedback_stack.front().get();
	}

	void DeviceManager::cancel_feedback(FeedbackSequence* feedback)
	{
		auto it = std::find_if(feedback_stack.begin(), feedback_stack.end(),
			[&](const std::unique_ptr<FeedbackSequence>& seq) { return seq.get() == feedback; });
		if (it != feedback_stack.end())
		{
			log->debug("Cancelling feedback sequence.");
			feedback_stack.erase(it);
			if (feedback_stack.empty())
				active->cancel_feedback();
			else
				apply_feedback(false);
		}
	}

	void DeviceManager::pause_feedback(void)
	{
		if (feedback_paused) return;
		feedback_paused = true;
		active->cancel_feedback();
	}

	void DeviceManager::resume_feedback(void)
	{
		if (!feedback_paused) return;
		feedback_paused = false;
		apply_feedback(true);
	}

	void DeviceManager::reset_feedback(void)
	{
		feedback_stack.clear();
		active->cancel_feedback();
	}

	void DeviceManager::start_record(const std::string& filename)
	{
		assert(recorder == nullptr);

		log->info("Starting recording: {}", filename);
		recorder = std::make_unique<InputRecorder>();
		recorder->start_recording(filename, active->id());
		recording = true;
	}

	void DeviceManager::stop_record(void)
	{
		if (recording)
		{
			log->info("Stopping recording");
			recorder = nullptr;
			recording = false;
		}
	}

	void DeviceManager::start_replay(const std::string& filename)
	{
		assert(recorder == nullptr);

		log->info("Starting replay: {}", filename);
		recorder = std::make_unique<InputRecorder>();
		recorder->start_playback(filename);

		controllers.push_back(std::make_unique<PlaybackDevice>(window, settings, recorder.get()));
		active = controllers.back().get();
		replaying = true;

		trigger(Message{ Events::DeviceBound });
	}
	
	void DeviceManager::stop_replay(void)
	{
		if (replaying)
		{
			log->info("Stopping replay");
			recorder = nullptr;
			controllers.pop_back();
			active = controllers.back().get();
			replaying = false;
		}
	}
}