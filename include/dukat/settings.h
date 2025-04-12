#pragma once

#include <map>
#include <string>

namespace dukat
{
	class Settings
	{
	private:
		std::map<std::string, std::string> map;
		void load_ini(const std::string& filename);

	public:
		Settings(void) { }
		Settings(const std::string& filename);
		Settings(const Settings& settings);
		~Settings(void) { }

		// Getters
		std::string get_string(const std::string& name, const std::string& default_value = "") const;
		int get_int(const std::string& name, int default_value = 0) const;
		float get_float(const std::string& name, float default_value = 0.0f) const;
		bool get_bool(const std::string& name, bool default_value = false) const;

		// Setters
		void set(const std::string& name, const std::string& value);
		void set(const std::string& name, int value);
		void set(const std::string& name, float value);
		void set(const std::string& name, bool value);

		friend void save_settings(const Settings& settings, const std::string& filename);
		friend void load_settings(const std::string& filename, Settings& settings);
	};

	namespace settings
	{
		// audio
		static constexpr auto audio_enabled = "audio.enabled";
		static constexpr auto audio_channels = "audio.channels";
		static constexpr auto audio_sample_volume = "audio.sample.volume";
		static constexpr auto audio_music_volume = "audio.music.volume";

		// game
		static constexpr auto game_height = "game.height";
		static constexpr auto game_width = "game.width";

		// input
		static constexpr auto input_longpress = "input.longpress";
		static constexpr auto input_gamepad_deadzone = "input.gamepad.deadzone";
		static constexpr auto input_gamepad_inverty = "input.gamepad.inverty";
		static constexpr auto input_gamepad_support = "input.gamepad.support";
		static constexpr auto input_gamepad_feedback = "input.gamepad.feedback";
		static constexpr auto input_mouse_cursor = "input.mouse.cursor";
		static constexpr auto input_mouse_lock = "input.mouse.lock";
		static constexpr auto input_mouse_sensitivity = "input.mouse.sensitivity";
		static constexpr auto input_profile_prefix = "input.profiles.";

		// logging
		static constexpr auto logging_file = "logging.file";
		static constexpr auto logging_flush = "logging.flush";
		static constexpr auto logging_level = "logging.level";
		static constexpr auto logging_output = "logging.output";
		static constexpr auto logging_truncate = "logging.truncate";

		// renderer
		static constexpr auto renderer_effects_enabled = "renderer.effects.enabled";

		// resources
		static constexpr auto resources_fonts = "resources.fonts";
		static constexpr auto resources_models = "resources.models";
		static constexpr auto resources_music = "resources.music";
		static constexpr auto resources_samples = "resources.samples";
		static constexpr auto resources_shaders = "resources.shaders";
		static constexpr auto resources_textures = "resources.textures";

		// update
		static constexpr auto update_url = "update.url";
		static constexpr auto update_dir = "update.dir";

		// video
		static constexpr auto video_forcesync = "video.forcesync";
		static constexpr auto video_camera_nearclip = "video.camera.nearclip";
		static constexpr auto video_camera_farclip = "video.camera.farclip";
		static constexpr auto video_camera_fov = "video.camera.fov";

		// window
		static constexpr auto window_title = "window.title";
		static constexpr auto window_width = "window.width";
		static constexpr auto window_height = "window.height";
		static constexpr auto window_fullscreen = "window.fullscreen";
		static constexpr auto window_fullscreen_desktop = "window.fullscreen_desktop";
		static constexpr auto window_resizable = "window.resizable";
		static constexpr auto window_borderless = "window.borderless";
		static constexpr auto window_msaa = "window.msaa";
		static constexpr auto window_vsync = "window.vsync";
	}
}