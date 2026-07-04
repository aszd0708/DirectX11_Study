#include "pch.h"
#include "Utils.h"
#include <locale>
#include <codecvt>

bool Utils::StartsWith(string str, string comp)
{
	wstring::size_type index = str.find(comp);
	if (index != wstring::npos && index == 0)
		return true;

	return false;
}

bool Utils::StartsWith(wstring str, wstring comp)
{
	wstring::size_type index = str.find(comp);
	if (index != wstring::npos && index == 0)
		return true;

	return false;
}

std::wstring Utils::ToWString(string value)
{
	if (value.empty())
	{
		return wstring();
	}

	int sizeNeeded = ::MultiByteToWideChar(CP_UTF8, 0, value.c_str(), static_cast<int>(value.length()), NULL, 0);
	wstring wstr(sizeNeeded , 0);
	::MultiByteToWideChar(CP_UTF8, 0, value.c_str(), static_cast<int>(value.length()), &wstr[0], sizeNeeded);

	return wstr;
}

std::string Utils::ToString(wstring value)
{
	if (value.empty())
	{
		return string();
	}

	int sizeNeeded = ::WideCharToMultiByte(CP_UTF8, 0, value.c_str(), static_cast<int>(value.length()), NULL, 0, NULL, NULL);
	string str(sizeNeeded, 0);
	::WideCharToMultiByte(CP_UTF8, 0, value.c_str(), static_cast<int>(value.length()), &str[0], sizeNeeded, NULL, NULL);

	return str;
}

void Utils::Replace(OUT string& str, string comp, string rep)
{
	string temp = str;

	size_t start_pos = 0;
	while ((start_pos = temp.find(comp, start_pos)) != wstring::npos)
	{
		temp.replace(start_pos, comp.length(), rep);
		start_pos += rep.length();
	}

	str = temp;
}

void Utils::Replace(OUT wstring& str, wstring comp, wstring rep)
{
	wstring temp = str;

	size_t start_pos = 0;
	while ((start_pos = temp.find(comp, start_pos)) != wstring::npos)
	{
		temp.replace(start_pos, comp.length(), rep);
		start_pos += rep.length();
	}

	str = temp;
}