#include "stdafx.h"
#include <dukat/updater.h>
#include <dukat/log.h>

#include <curl/curl.h>

namespace dukat
{
	static bool curl_initialized = false;

	static void init_curl(void)
	{
		if (curl_initialized) return;

		log->info("Initializing libcurl.");
		const auto res = curl_global_init(CURL_GLOBAL_ALL);
		if (res)
			log->warn("Failed to initialize libcurl: {}", res);
		else
			curl_initialized = true;
	}

	void release_curl(void)
	{
		if (!curl_initialized) return;
		log->info("Releasing libcurl.");
		curl_global_cleanup();
		curl_initialized = false;
	}

	static size_t write_data(void* ptr, size_t size, size_t nmemb, void* stream)
	{
		const auto written = fwrite(ptr, size, nmemb, static_cast<FILE*>(stream));
		return written;
	}

	static void check_result(CURLcode res, const std::string& operation)
	{
		if (res != CURLE_OK)
		{
			std::stringstream ss;
			ss << "Failed to " << operation << ": " << curl_easy_strerror(res);
			throw std::runtime_error(ss.str());
		}
	}

	std::vector<std::string> download_index_file(const std::string& url, const std::string& temp_dir)
	{
		std::vector<std::string> lines;
		const auto temp_file = temp_dir + "/index";
		if (download_url(url, temp_file))
		{
			std::ifstream infile(temp_file);
			std::string line;
			while (std::getline(infile, line))
			{
				if (!line.empty())
					lines.push_back(line);
			}
		}
		return lines;
	}

	std::string find_latest_version(int base, const std::vector<std::string>& files)
	{
		std::string res = "";
		for (const auto& file : files)
		{
			const auto from = file.find_first_of('-');
			const auto to = file.find_last_of('-');
			if (from == std::string::npos || to == std::string::npos)
				continue;
			const auto version = std::stoi(file.substr(from + 1, to));
			if (version > base)
			{
				base = version;
				res = file;
			}
		}
		return res;
	}

	bool download_url(const std::string& url, const std::string& target_file)
	{
		init_curl();

		auto handle = curl_easy_init();
		if (!handle)
		{
			log->error("Failed to initialize libcurl.");
			return false;
		}

#if WIN32
		FILE* file = nullptr;
		auto error = fopen_s(&file, target_file.c_str(), "wb");
		if (error || file == nullptr)
			throw std::runtime_error("Failed to download file.");
#else
		auto file = fopen(target_file.c_str(), "wb");
		if (file == nullptr)
			throw std::runtime_error("Failed to download file.");
#endif

		auto code = curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
		check_result(code, "set URL");
		// testing
		//auto code = curl_easy_setopt(handle, CURLOPT_VERBOSE, 1L);
		//check_result(code, "enable verbose output");
		code = curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 1L);
		check_result(code, "disable progress meter");
		code = curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_data);
		check_result(code, "set write function");
		code = curl_easy_setopt(handle, CURLOPT_WRITEDATA, file);
		check_result(code, "set write target");
		code = curl_easy_perform(handle);
		check_result(code, "download file");

		fclose(file);
		curl_easy_cleanup(handle);

		return true;
	}
}
