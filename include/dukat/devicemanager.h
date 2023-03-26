#pragma once

#include <memory>
#include <list>
#include "feedback.h"
#include "messenger.h"

namespace dukat
{
	class InputDevice;
	class InputRecorder;
	class Settings;
	class Window;

	class DeviceManager : public Messenger
	{
	private:
		const Window& window;
		const Settings& settings;
		std::list<std::unique_ptr<InputDevice>> controllers;
		std::list<std::unique_ptr<FeedbackSequence>> feedback_stack;
		FeedbackValue feedback_val; // last known feedback value
		std::unique_ptr<InputRecorder> recorder;
		bool enabled;
		bool feedback_paused;
		bool recording;
		bool replaying;

		// Updates entire feedback stack.
		void update_feedback_stack(float delta);
		// Applies current feedback to active device.
		void apply_feedback(bool force);

	public:
		InputDevice* active;

		DeviceManager(const Window& window, const Settings& settings) : window(window), settings(settings), 
			recorder(nullptr), enabled(true), feedback_paused(false), recording(false), replaying(false), active(nullptr) { }
		~DeviceManager(void) { }

		void add_keyboard(void);
		void remove_keyboard(void);
		InputDevice* get_keyboard(void) const;
		void add_joystick(int index);
		void remove_joystick(SDL_JoystickID id);
		void update(float delta);
		void enable(void) { enabled = true; }
		void disable(void) { enabled = false; }

		// Haptic feedback
		FeedbackSequence* start_feedback(std::unique_ptr<FeedbackSequence> feedback);
		void cancel_feedback(FeedbackSequence* feedback);
		// Pauses all feedback.
		void pause_feedback(void);
		// Resumes all feedback
		void resume_feedback(void);
		// Resets feedback to initial state
		void reset_feedback(void);

		void start_record(const std::string& filename);
		void stop_record(void);
		bool is_recording(void) const { return recording; }

		void start_replay(const std::string& filename);
		void stop_replay(void);
		bool is_replaying(void) const { return replaying; }
	};
}