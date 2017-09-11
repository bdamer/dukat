// scenes.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <dukat/dukat.h>
#include "titlescene.h"
#include "gamescene.h"

int main(int argc, char** argv)
{
	try
	{
		std::string config = "../assets/scenes.ini";
		dukat::Settings settings(config);
		dukat::Game3 app(settings);
		app.add_scene("title", std::make_unique<dukat::TitleScene>(&app));
		app.add_scene("game", std::make_unique<dukat::GameScene>(&app));
		app.push_scene("title");
		return app.run();
	}
	catch (const std::exception& e)
	{
		dukat::logger << "Application failed with error." << std::endl << e.what() << std::endl;
		return -1;
	}
	return 0;
}