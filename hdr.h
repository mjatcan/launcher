#pragma once
#include <Windows.h>
#include <vector>
#include <string>
#include <string_view>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <map>
#include <format>

class CHDRSystem
{
public:
	bool bRepair = false;
private:
	int rCount, bChanged;
	std::string m_strBasePath;
	std::vector<std::string> m_Dirs;
private:
	bool Prepare(const std::string& v);
	void Pack(const std::string& v, bool ignoreUnpack = false);
	void Unpack(const std::string& v);
public:
	CHDRSystem();
	~CHDRSystem();
	void Pack();
private:
	std::string GetDirectory(const std::string& path);
	BOOL WriteDirectory(const std::string& dd);
};