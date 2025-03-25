#pragma once

namespace dukat
{
	/// <summary>
	/// Downloads an index file from the given URL and stores it in the given directory.
	/// Returns a list of file names found in the index file.
	/// </summary>
	///	<param name="url">A URL to download the index file from.</param>
	/// <param name="temp_dir">A directory to store the downloaded file.</param>
	/// <returns>A list of file names.</returns>
	std::vector<std::string> download_index_file(const std::string& url, const std::string& temp_dir);

	/// <summary>
	/// Returns latest file version from the list of files greater than given base version.
	/// </summary>
	/// <param name="base">A base version.</param>
	/// <param name="files">A list of files.</param>
	/// <returns>The latest file entry or empty string.</returns>
	std::string find_latest_version(int base, const std::vector<std::string>& files);

	/// <summary>
	/// Downloads a file from the given URL and stores it in the given file.
	/// </summary>
	/// <param name="url">A URL to download the file from.</param>
	/// <param name="out_file">A file to store the downloaded file.</param>
	/// <returns>True on success, false on failure.</returns>
	bool download_url(const std::string& url, const std::string& out_file);

	/// <summary>
	/// Frees global resources associated with the curl library.
	/// </summary>
	void release_curl(void);
}