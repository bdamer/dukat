// skydomeapp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "skydomeapp.h"

namespace dukat
{
	// Factor that determines how long an in-game second is. (1 in-game hour = 60 real-world seconds)
	const float time_factor = 60.0f;

	std::unique_ptr<MeshData> build_plane(float max_u, float max_t)
	{
		std::vector<VertexAttribute> attr;
		attr.push_back(VertexAttribute(Renderer::at_pos, 3, offsetof(Vertex3PNT, px)));
		attr.push_back(VertexAttribute(Renderer::at_normal, 3, offsetof(Vertex3PNT, nx)));
		attr.push_back(VertexAttribute(Renderer::at_texcoord, 2, offsetof(Vertex3PNT, tu)));

		Vertex3PNT verts[6] = {
			-1.0f,  -1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 0.0f, max_t,
			1.0f,  -1.0f,  1.0f, 0.0f, 1.0f, 0.0f, max_u, max_t,
			-1.0f,  -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
			1.0f,  -1.0f,  1.0f, 0.0f, 1.0f, 0.0f, max_u, max_t,
			1.0f,  -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, max_u, 0.0f,
			-1.0f,  -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		};

		auto res = std::make_unique<MeshData>(GL_TRIANGLES, 6, 0, attr);
		res->set_vertices(reinterpret_cast<GLfloat*>(verts));
		return res;
	}

	SkydomeScene::SkydomeScene(Game3* game) : game(game), multiplier(1), total_time(0.0f)
	{
		auto settings = game->get_settings();
		auto camera = std::make_unique<FirstPersonCamera3>(game);
		camera->transform.position.y = 1.0;
		camera->set_vertical_fov(settings.get_float("camera.fov"));
		camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
		camera->refresh();
		game->get_renderer()->set_camera(std::move(camera));

		object_meshes.stage = RenderStage::SCENE;
		object_meshes.visible = true;

		MeshBuilder3 mb3;

		// Box
		auto box = object_meshes.create_instance();
		box->set_mesh(game->get_meshes()->put("box", mb3.build_cube_single_face()));
		box->set_texture(game->get_textures()->get("box01_1024.png"));
		box->set_program(game->get_shaders()->get_program("sc_lighting.vsh", "sc_lighting.fsh"));
		box->transform.scale = Vector3{0.25f, 0.25f, 0.25f};
		box->transform.position.y = 0.25f;

		// Ground plane
		const float scale = 100.0f;
		ground_mesh = object_meshes.create_instance();
		ground_mesh->set_mesh(game->get_meshes()->put("ground", build_plane(scale, scale)));
		ground_mesh->set_texture(game->get_textures()->get("sand01_1024.png", ProfileAnisotropic));
		ground_mesh->set_program(game->get_shaders()->get_program("sc_lighting.vsh", "sc_lighting.fsh"));
		ground_mesh->transform.position.y = scale;
		ground_mesh->transform.scale = Vector3(scale, scale, scale);

		// Skydome
		skydome_mesh = object_meshes.create_instance();
		skydome_mesh->set_mesh(game->get_meshes()->put("skydome", mb3.build_dome(32, 24, true)));
		skydome_mesh->set_program(game->get_shaders()->get_program("sc_skydome.vsh", "sc_skydome.fsh"));

		// Create cubemap
		skydome_mesh->set_texture(game->get_textures()->get("skybox01.dds"), 0);

		init_environment();		

		overlay_meshes.stage = RenderStage::OVERLAY;
		overlay_meshes.visible = true;

		auto multiplier_text = game->create_text_mesh(1.0f / 20.0f);
		multiplier_text->transform.position = { -1.3f, 0.85f, 0.0f };
		std::stringstream ss;
		ss << "<#white>" << multiplier << "x" << std::endl
			<< "00:00</>";
		multiplier_text->set_text(ss.str());
		multiplier_text->transform.update();
		mult_mesh = static_cast<TextMeshInstance*>(overlay_meshes.add_instance(std::move(multiplier_text)));

		game->get<TimerManager>()->create_timer(1.0f, [&]() {
			// Compute wallclock time from absolute counter
			std::stringstream ss;
			float wallclock_time = (total_time * time_factor) + 6.0f * 60.0f * 60.0f;
			int hours = (int)(wallclock_time / 3600.0f) % 24;
			wallclock_time -= (float)(hours * 3600);
			int minutes = (int)(wallclock_time / 60.0f) % 60;
			ss << "<#white>" << multiplier << "x" << std::endl
				<< std::setfill('0') << std::setw(2) << hours << ":" << std::setw(2) << minutes << "</>";
			mult_mesh->set_text(ss.str());
		}, true);

		auto info_text = game->create_text_mesh(1.0f / 20.0f);
		info_text->transform.position = { -1.3f, -0.7f, 0.0f };
		ss.str("");
		ss << "<#white>"
			<< "<WASD> Move Camera" << std::endl
			<< "<,.> Change Sim Speed" << std::endl
			<< "<F1> Toggle Wirframe" << std::endl
			<< "<F2> Toggle Ground" << std::endl
			<< "<F11> Toggle Info" << std::endl
			<< std::endl;
		info_text->set_text(ss.str());
		info_text->transform.update();
		info_mesh = overlay_meshes.add_instance(std::move(info_text));

		// Sun
		sun_mesh = game->get_debug_meshes()->create_instance();
		sun_mesh->set_mesh(game->get_meshes()->put("sun", mb3.build_sphere(16, 16)));
		sun_mesh->set_program(game->get_shaders()->get_program("sc_texture.vsh", "sc_texture.fsh"));
		sun_mesh->set_texture(game->get_textures()->get("white.png"));
		Material m;
		m.ambient = color_rgb(0xffff00);
		sun_mesh->set_material(m);

		game->set_controller(this);
	}

	void SkydomeScene::init_environment(void)
	{
		env = std::make_unique<Environment>(game, skydome_mesh);

        // Phase 0: Dawn (06:00-10:00)
        Environment::Phase dawn_phase;
        dawn_phase.duration = 4.0f * time_factor;
        dawn_phase.mat.ambient = color_rgb(0x91a3d1);
        dawn_phase.mat.diffuse = color_rgb(0x051340);
        dawn_phase.mat.specular = color_rgb(0xcf7012);
        dawn_phase.mat.custom = Color{5.0f,0.05f,0.0f,0.0f};
        dawn_phase.color_trans = sqrt_transition<Color>;
        dawn_phase.light.position = Vector3{1.0f, 0.0f, 0.0f};
        dawn_phase.light_pos_trans = linear_transition<Vector3>;
        dawn_phase.light.ambient = color_rgb(0x9f6227);
        dawn_phase.light.diffuse = color_rgb(0xcf7012);
        dawn_phase.light.specular = color_rgb(0x666666);
        env->add_phase(dawn_phase);

        // Phase 1: Day (10:00-16:00)
        Environment::Phase day_phase;
        day_phase.duration = 6.0f * time_factor;
        day_phase.mat.ambient = color_rgb(0xc7dced);
        day_phase.mat.diffuse = color_rgb(0x3f71b8);
        day_phase.mat.specular = color_rgb(0xffffff);
        day_phase.mat.custom = Color{5.0f,0.0f,0.0f,0.0f};
        day_phase.color_trans = linear_transition<Color>;
        day_phase.light.position = Vector3{0.5f, -1.0f, 0.5f};
        day_phase.light_pos_trans = linear_transition<Vector3>;
        day_phase.light.ambient = color_rgb(0x333333);
        day_phase.light.diffuse = color_rgb(0x666666);
        day_phase.light.specular = color_rgb(0x666666);
        env->add_phase(day_phase);

        // Repeat day phase (16:00-20:00)
        day_phase.duration = 4.0f * time_factor;
        day_phase.color_trans = quadratic_transition<Color>;
        day_phase.light.position = Vector3{-0.5f, -1.0f, 0.5f};
        env->add_phase(day_phase);

        // Phase 2: Dusk (20:00-22:00)
        Environment::Phase dusk_phase;
        dusk_phase.duration = 2.0f * time_factor;
        dusk_phase.mat.ambient = color_rgb(0x90a2d1);
        dusk_phase.mat.diffuse = color_rgb(0x051340);
        dusk_phase.mat.specular = color_rgb(0xd02200);
        dusk_phase.mat.custom = Color{5.0f,0.2f,0.0f,0.0f};
        dusk_phase.color_trans = sqrt_transition<Color>;
        dusk_phase.light.position = Vector3{-1.0f, -0.0f, 0.0f};
        dusk_phase.light_pos_trans = linear_transition<Vector3>;
        dusk_phase.light.ambient = color_rgb(0x4d2922);
        dusk_phase.light.diffuse = color_rgb(0x912c18);
        dusk_phase.light.specular = color_rgb(0x666666);
        env->add_phase(dusk_phase);

        // Phase 3: Night (22:00-4:00)
        Environment::Phase night_phase;
        night_phase.duration = 6.0f * time_factor;
        night_phase.mat.ambient = color_rgb(0x0);
        night_phase.mat.diffuse = color_rgb(0x0);
        night_phase.mat.specular = color_rgb(0x0);
        night_phase.mat.custom = Color{5.0f,1.0f,0.0f,0.0f};
        night_phase.color_trans = linear_transition<Color>;
        night_phase.light.position = Vector3{-0.5f, 0.5f, -0.5f};
        night_phase.light_pos_trans = linear_transition<Vector3>;
        night_phase.light.ambient = color_rgb(0x202020);
        night_phase.light.diffuse = color_rgb(0x0);
        night_phase.light.specular = color_rgb(0x0);
        env->add_phase(night_phase);

        // Repeat night phase (4:00-6:00)
        night_phase.duration = 2.0f * time_factor;
        night_phase.light.position = Vector3{0.5f, 0.5f, -0.5f};
        night_phase.color_trans = quadratic_transition<Color>;
        env->add_phase(night_phase);

		env->set_current_phase(0);
	}

	void SkydomeScene::handle_keyboard(const SDL_Event &e)
	{
		Material mat;
		switch (e.key.keysym.sym)
		{
		case SDLK_ESCAPE:
			game->set_done(true);
			break;
		case SDLK_COMMA:
			multiplier = std::max(1, multiplier / 2);
			break;
		case SDLK_PERIOD:
			multiplier = std::min(64, multiplier * 2);
			break;
		case SDLK_F1:
			game->get_renderer()->toggle_wireframe();
			break;
		case SDLK_F2:
			ground_mesh->visible = !ground_mesh->visible;
			break;
		case SDLK_F11:
			info_mesh->visible = !info_mesh->visible;
			break;
		}
	}

	void SkydomeScene::update(float delta)
	{
		env->update(delta * (float)multiplier);
		total_time += delta * (float)multiplier;

		// Update sun mesh position
		auto l0 = game->get_renderer()->get_light(Renderer3::dir_light_idx);
		sun_mesh->transform.position = -64.0f * l0->position;

		object_meshes.update(delta);
		overlay_meshes.update(delta);
	}

	void SkydomeScene::render(void)
	{
		std::vector<Mesh*> meshes;
		meshes.push_back(game->get_debug_meshes());
		meshes.push_back(&object_meshes);
		meshes.push_back(&overlay_meshes);
		game->get_renderer()->render(meshes);
	}
}

int main(int argc, char** argv)
{
	try
	{
		std::string config = "../assets/skydome.ini";
		if (argc > 1)
		{
			config = argv[1];
		}
		dukat::Settings settings(config);
		dukat::Game3 app(settings);
		app.add_scene("main", std::make_unique<dukat::SkydomeScene>(&app));
		app.push_scene("main");
		return app.run();
	}
	catch (const std::exception& e)
	{
		dukat::logger << "Application failed with error." << std::endl << e.what() << std::endl;
		return -1;
	}
	return 0;
}