// roomapp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "roomapp.h"
#include "firstpersoncamera.h"

#include <dukat/log.h>
#include <dukat/meshbuilder3.h>
#include <dukat/renderer3.h>
#include <dukat/settings.h>
#include <dukat/vertextypes3.h>

namespace dukat
{
	std::unique_ptr<Mesh> build_inverted_cube(float max_u, float max_t)
	{
		std::vector<VertexAttribute> attr;
		attr.push_back(VertexAttribute(Renderer::at_pos, 3, offsetof(VertexPosNorTex, pos)));
		attr.push_back(VertexAttribute(Renderer::at_normal, 3, offsetof(VertexPosNorTex, nor)));
		attr.push_back(VertexAttribute(Renderer::at_texcoord, 2, offsetof(VertexPosNorTex, u)));

		VertexPosNorTex verts[36] = {
			// Top
			-1.0f, 1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f, 1.0f, -1.0f, 0.0f, -1.0f, 0.0f, max_u, 0.0f,
			-1.0f, 1.0f,  1.0f, 0.0f, -1.0f, 0.0f, 0.0f , max_t,
			1.0f, 1.0f, -1.0f, 0.0f, -1.0f, 0.0f, max_u, 0.0f,
			1.0f, 1.0f,  1.0f, 0.0f, -1.0f, 0.0f, max_u, max_t,
			-1.0f, 1.0f,  1.0f, 0.0f, -1.0f, 0.0f, 0.0f, max_t,
			// Bottom
			-1.0f,  -1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 0.0f, max_t,
			1.0f,  -1.0f,  1.0f, 0.0f, 1.0f, 0.0f, max_u, max_t,
			-1.0f,  -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
			1.0f,  -1.0f,  1.0f, 0.0f, 1.0f, 0.0f, max_u, max_t,
			1.0f,  -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, max_u, 0.0f,
			-1.0f,  -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
			// Front
			1.0f, -1.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
			-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, -1.0f, max_u, 0.0f,
			1.0f,  1.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, max_t,
			-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, -1.0f, max_u, 0.0f,
			-1.0f,  1.0f, 1.0f, 0.0f, 0.0f, -1.0f, max_u, max_t,
			1.0f,  1.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, max_t,
			// Back
			-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, max_u, 0.0f,
			-1.0f,  1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, max_t,
			1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, max_u, 0.0f,
			1.0f,  1.0f, -1.0f, 0.0f, 0.0f, 1.0f, max_u, max_t,
			-1.0f,  1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, max_t,
			// Right
			1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
			1.0f, -1.0f,  1.0f, -1.0f, 0.0f, 0.0f, max_u, 0.0f,
			1.0f,  1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, max_t,
			1.0f, -1.0f,  1.0f, -1.0f, 0.0f, 0.0f, max_u, 0.0f,
			1.0f,  1.0f,  1.0f, -1.0f, 0.0f, 0.0f, max_u, max_t,
			1.0f,  1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, max_t,
			// Left
			-1.0f, -1.0f,  1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
			-1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, max_u, 0.0f,
			-1.0f,  1.0f,  1.0f, 1.0f, 0.0f, 0.0f, 0.0f, max_t,
			-1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, max_u, 0.0f,
			-1.0f,  1.0f, -1.0f, 1.0f, 0.0f, 0.0f, max_u, max_t,
			-1.0f,  1.0f,  1.0f, 1.0f, 0.0f, 0.0f, 0.0f, max_t
		};

		auto res = std::make_unique<Mesh>(GL_TRIANGLES, 36, 0, attr);
		res->set_vertices(reinterpret_cast<GLfloat*>(verts));
		return res;
	}

	void Game::init(void)
	{
		Game3::init();

		auto camera = std::make_unique<FirstPersonCamera>(window.get(), this);
		camera->transform.position.y = 1.0;
		camera->set_fov(settings.get_float("camera.fov"));
		camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
		camera->refresh();
		renderer->set_camera(std::move(camera));

		object_meshes.stage = RenderStage::SCENE;
		object_meshes.mat_model.identity();
		object_meshes.visible = true;

		// Origin
		dukat::MeshBuilder3 builder3;
		mesh_cache->put("default-axis", builder3.build_axis());
		auto mi = object_meshes.create_instance();
		mi->set_mesh(mesh_cache->get("default-axis"));
		mi->set_program(shader_cache->get_program("sc_color.vsh", "sc_color.fsh"));

		// Room
		auto cube_mesh = mesh_cache->put("cube", build_inverted_cube(25.0f, 25.0f));
		mi = object_meshes.create_instance();
		mi->set_mesh(cube_mesh);
		mi->set_texture(texture_cache->get("holodeck.png", ProfileAnisotropic));
		mi->set_program(shader_cache->get_program("sc_texture.vsh", "sc_texture.fsh"));
		mi->transform.position.y = 25.0f;
		mi->transform.scale = Vector3(25.0f, 25.0f, 25.0f);

		debug_meshes.stage = RenderStage::OVERLAY;
		debug_meshes.mat_model.identity();

		std::unique_ptr<TextMeshInstance> debug_text;
		debug_text = create_text_mesh(1.0f / 20.0f);
		debug_text->transform.position.x = -1.0f;
		debug_text->transform.position.y = 1.0f;
		debug_text->transform.update();
		debug_meshes.add_instance(std::move(debug_text));
	}

	void Game::handle_keyboard(const SDL_Event& e)
	{
		switch (e.key.keysym.sym)
		{
		default:
			Game3::handle_keyboard(e);
		}
	}

	void Game::update(float delta)
	{
		Game3::update(delta);
		object_meshes.update(delta);
		debug_meshes.update(delta);
	}

	void Game::render(void)
	{
		std::vector<MeshGroup*> meshes;
		meshes.push_back(&debug_meshes);
		meshes.push_back(&object_meshes);
		renderer->render(meshes);
	}

	void Game::toggle_debug(void)
	{
		debug_meshes.visible = !debug_meshes.visible;
	}

	void Game::update_debug_text(void)
	{
		std::stringstream ss;
		auto cam = renderer->get_camera();
		ss << "WIN: " << window->get_width() << "x" << window->get_height()
			<< " FPS: " << get_fps()
			<< " MESH: " << dukat::perfc.avg(dukat::PerformanceCounter::MESHES)
			<< " VERT: " << dukat::perfc.avg(dukat::PerformanceCounter::VERTICES) << std::endl;
		auto debug_text = dynamic_cast<TextMeshInstance*>(debug_meshes.get_instance(0));
		debug_text->set_text(ss.str());
	}
}

int main(int argc, char** argv)
{
	try
	{
		std::string config = "../assets/room.ini";
		if (argc > 1)
		{
			config = argv[1];
		}
		dukat::Settings settings(config);
		dukat::Game app(settings);
		return app.run();
	}
	catch (const std::exception& e)
	{
		dukat::logger << "Application failed with error." << std::endl << e.what() << std::endl;
		return -1;
	}
	return 0;
}