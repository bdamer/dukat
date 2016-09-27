#pragma once

namespace dukat
{
	class bytebuffer : public std::basic_streambuf<char>
	{
	public:
		bytebuffer(const uint8_t* data, size_t length)
		{
			setg((char*)data, (char*)data, (char*)(data + length));
		}
	};

	class bytestream : public std::istream
	{
	private:
		bytebuffer buffer;

	public:
		bytestream(const uint8_t* data, size_t length) : std::istream(&buffer), buffer(data, length)
		{
			rdbuf(&buffer);
		}
	};
}

