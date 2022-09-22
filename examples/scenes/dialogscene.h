#pragma once

#include <dukat/dukat.h>

namespace dukat
{
    class Game3;
    
    class DialogScene : public Scene, public Controller
    {
    private:
        static constexpr auto option_mode = "Mode: ";
        static constexpr auto option_windowed = "Windowed";
        static constexpr auto option_borderless = "Borderless";
        static constexpr auto option_fullscreen = "Fullscreen";
        static constexpr auto option_resolution = "Resolution: ";
        static constexpr auto option_apply = "Apply";
        static constexpr auto option_back = "Back";

        enum WindowMode
        {
            Windowed,
            Borderless,
            Fullscreen
        };

        Game3* game;
        MeshGroup overlay_meshes;
        TextMeshInstance* title_text;
        TextMeshInstance* mode_text;
        TextMeshInstance* resolution_text;
        TextMeshInstance* apply_text;
        TextMeshInstance* return_text;
		std::unique_ptr<TextButton> mode_button;
        std::unique_ptr<TextButton> resolution_button;
        std::unique_ptr<TextButton> apply_button;
        std::unique_ptr<TextButton> return_button;

        WindowMode window_mode;
        std::vector<SDL_DisplayMode> display_modes;
        int display_mode_idx;

        void update_layout(void);
        void update_display_modes(void);
        void update_mode_button(void);
        void update_resolution_button(void);

    public:
        DialogScene(Game3* game);
        ~DialogScene(void) { }
    
        void activate(void);
		void deactivate(void);
		void update(float delta);
        void handle_keyboard(const SDL_Event& e);
        void render(void);
    };
}