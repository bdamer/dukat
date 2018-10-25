#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <fstream>
#include <memory>
#include <string>
#include <sstream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <dukat/dukat.h>
#include "hellodukat.h"

namespace dukat
{
    extern "C" JNIEXPORT void JNICALL Java_com_afqa123_dukat_DukatLib_init(JNIEnv* env, jobject obj, jobject asset_mgr);

    JNIEXPORT void JNICALL Java_com_afqa123_dukat_DukatLib_init(JNIEnv* env, jobject obj, jobject asset_mgr)
    {
        auto am = AAssetManager_fromJava(env, asset_mgr);
        AssetLoader::set_asset_manager(am);
    }

    HelloDukat::HelloDukat(Game2* game2) : Scene2(game2)
    {
        auto settings = game->get_settings();
        auto layer = game->get_renderer()->create_layer("main", 1.0f);
        // Set up default camera centered around origin
        auto camera = std::make_unique<Camera2>(game, Vector2(camera_width, camera_height));
        camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
        camera->refresh();
        game->get_renderer()->set_camera(std::move(camera));

        // Set up info text
        auto info_layer = game->get_renderer()->create_layer("overlay", 25.0f);
        info_layer->stage = RenderStage::OVERLAY;
        info_text = game->create_text_mesh();
        info_text->set_size(8.0f);
        info_text->transform.position = Vector3(-0.25f * (float)camera_width, 0.0f, 0.0f);
        info_text->transform.update();
        std::stringstream ss;
        ss << "Hello Dukat!" << std::endl;
        info_text->set_text(ss.str());
        info_layer->add(info_text.get());

        // Set up debug layer
        auto debug_layer = game->get_renderer()->create_layer("debug", 1000.0f);
        debug_layer->stage = RenderStage::OVERLAY;
        debug_text = game->create_text_mesh();
        debug_text->set_size(4.0f);
        debug_text->transform.position = Vector3(-0.5f * (float)camera_width, -0.5f * (float)camera_height, 0.0f);
        debug_text->transform.update();
        debug_layer->add(debug_text.get());
       // debug_layer->hide();

        auto main_layer = game->get_renderer()->create_layer("main", 20.0f);

        auto tc = game->get_textures();
        sprite = std::make_unique<Sprite>(tc->get("dukat.png"));
        sprite->p.x = 50.0f;
        main_layer->add(sprite.get());

        game->get<TimerManager>()->create_timer(1.0f, [&]() {
            std::stringstream ss;
            auto window = game->get_window();
            auto cam = game->get_renderer()->get_camera();
            ss << "WIN: " << window->get_width() << "x" << window->get_height()
               << " VIR: " << cam->transform.dimension.x << "x" << cam->transform.dimension.y
               << " FPS: " << game->get_fps()
               << " MESH: " << dukat::perfc.avg(dukat::PerformanceCounter::MESHES)
               << " VERT: " << dukat::perfc.avg(dukat::PerformanceCounter::VERTICES) << std::endl;
            debug_text->set_text(ss.str());
        }, true);

        game->set_controller(this);
    }

    void HelloDukat::handle_keyboard(const SDL_Event& e)
    {
        switch (e.key.keysym.sym)
        {
            case SDLK_ESCAPE:
                game->set_done(true);
                break;
        }
    }
}

int main(int argc, char** argv)
{
    try
    {
        std::string config = "hello.ini";
        if (argc > 1)
        {
            config = argv[1];
        }
        dukat::Settings settings(config);
        dukat::Game2 app(settings);
        app.add_scene("main", std::make_unique<dukat::HelloDukat>(&app));
        app.push_scene("main");
        return app.run();
    }
    catch (const std::exception& e)
    {
        dukat::log->error("Application failed with error: {}", e.what());
        return -1;
    }
    return 0;
}

