#pragma once

#include <dukat/dukat.h>

namespace dukat
{
    class Game3;
    
    class DialogScene : public Scene, public Controller
    {
    private:
        Game3* game;
        MeshGroup overlay_meshes;
    
    public:
        DialogScene(Game3* game);
        ~DialogScene(void) { }
    
        void activate(void);
        void update(float delta);
        bool handle_keyboard(const SDL_Event& e);
        void render(void);
    };
}