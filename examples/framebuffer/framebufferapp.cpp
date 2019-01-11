// framebufferapp.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "fractalscene.h"
#include "ripplepondscene.h"
#include "titlescene.h"

int main(int argc, char** argv)
{
	try
	{
		std::string config = "../assets/framebuffer.ini";
		if (argc > 1)
		{
			config = argv[1];
		}
		dukat::Settings settings(config);
		dukat::Game3 app(settings);
		app.add_scene("fractals", std::make_unique<dukat::FractalScene>(&app));
		app.add_scene("ripplepond", std::make_unique<dukat::RipplePondScene>(&app));
		app.add_scene("title", std::make_unique<dukat::TitleScene>(&app));
		app.push_scene("title");
		return app.run();
	}
	catch (const std::exception& e)
	{
		dukat::log->error("Application failed with error: {}", e.what());
		return -1;
	}
	return 0;
}