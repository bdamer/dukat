#pragma once

#ifdef __ANDROID__
class AAssetManager;
#endif

namespace dukat
{
	class AssetLoader
	{
	private:
#ifdef __ANDROID__
		static AAssetManager* am;
#endif

	public:
#ifdef __ANDROID__
		static void set_asset_manager(AAssetManager* am) { AssetLoader::am = am; }
#endif

		AssetLoader(void) { }
		~AssetLoader(void) { }

		void load_text(const std::string& filename, std::stringstream& ss) const;

	};
}