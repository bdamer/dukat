// roomapp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "roomapp.h"
#include "roomcamera.h"

namespace dukat
{
	std::unique_ptr<MeshData> build_inverted_cube(float max_u, float max_t)
	{
		std::vector<VertexAttribute> attr;
		attr.push_back(VertexAttribute(Renderer::at_pos, 3, offsetof(Vertex3PNT, px)));
		attr.push_back(VertexAttribute(Renderer::at_normal, 3, offsetof(Vertex3PNT, nx)));
		attr.push_back(VertexAttribute(Renderer::at_texcoord, 2, offsetof(Vertex3PNT, tu)));

		Vertex3PNT verts[36] = {
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

		auto res = std::make_unique<MeshData>(GL_TRIANGLES, 36, 0, attr);
		res->set_vertices(reinterpret_cast<GLfloat*>(verts));
		return res;
	}

	RoomScene::RoomScene(Game3* game) : game(game)
	{
		auto settings = game->get_settings();
		auto camera = std::make_unique<RoomCamera>(game);
		camera->transform.position.y = 1.0;
		camera->set_vertical_fov(settings.get_float("camera.fov"));
		camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
		camera->refresh();
		game->get_devices()->active->on_press(InputDevice::VirtualButton::Button2,
			std::bind(&RoomCamera::jump, camera.get()));
		game->get_renderer()->set_camera(std::move(camera));

		object_meshes.stage = RenderStage::Scene;
		object_meshes.visible = true;

		// Origin
		dukat::MeshBuilder3 builder3;
		game->get_meshes()->put("default-axis", builder3.build_axis());
		auto mi = object_meshes.create_instance();
		mi->set_mesh(game->get_meshes()->get("default-axis"));
		mi->set_program(game->get_shaders()->get_program("sc_color.vsh", "sc_color.fsh"));

		// Room
		auto cube_mesh = game->get_meshes()->put("cube", build_inverted_cube(25.0f, 25.0f));
		mi = object_meshes.create_instance();
		mi->set_mesh(cube_mesh);
		mi->set_texture(game->get_textures()->get("holodeck.png", ProfileAnisotropic));
		mi->set_program(game->get_shaders()->get_program("sc_texture.vsh", "sc_texture.fsh"));
		mi->transform.position.y = 25.0f;
		mi->transform.scale = Vector3(25.0f, 25.0f, 25.0f);

		game->set_controller(this);	
	}

	void RoomScene::update(float delta)
	{
		object_meshes.update(delta);
	}

	void RoomScene::render(void)
	{
		std::vector<Mesh*> meshes;
		meshes.push_back(game->get_debug_meshes());
		meshes.push_back(&object_meshes);
		game->get_renderer()->render(meshes);
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
		dukat::Game3 app(settings);
		app.add_scene("main", std::make_unique<dukat::RoomScene>(&app));
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