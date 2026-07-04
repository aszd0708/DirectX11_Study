#pragma once

enum FileMode : uint8
{
	Write,
	Read,
};

class FileUtils
{
public:
	FileUtil();
	~FileUtil();

	void Open(wstring filePath, FileMode mode);

	template<typename T>
	void Write(const T& data)
	{
		DWORD numOfBytes = 0;
		assert(::WriteFile(_handle, &data, sizeof(T), (LPDWORD)&numOfBytes, nullptr));
	}

	template<>
	void Write<string>(const string& data)
	{
		return Write(data);
	}

	void Wirte(void* data, uint32 dataSize);
	void Write(const string& data);

private:
	HANDLE _handle = INVALID_HANDLE_VALUE;
};

