#include "stdafx.h"
#include "assetloader.h"

#ifdef __ANDROID__
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#endif

namespace dukat
{
#ifdef __ANDROID__
	AAssetManager* AssetLoader::am;
#endif

	void AssetLoader::load_text(const std::string& filename, std::stringstream& ss) const
	{
#ifdef __ANDROID__
		auto asset = AAssetManager_open(am, filename.c_str(), AASSET_MODE_BUFFER);
		if (asset == nullptr)
		{
			throw std::runtime_error("Could not load file: " + filename);
		}
		auto length = AAsset_getLength(asset);
		auto buffer = static_cast<const char*>(AAsset_getBuffer(asset));
		std::string s(buffer, buffer + length);
		ss << s;
		AAsset_close(asset);
#else
		std::fstream is(filename);
		if (!is)
		{
			throw std::runtime_error("Could not load file: " + filename);
		}
		else
		{
			ss << is.rdbuf();
		}
#endif
	}
}