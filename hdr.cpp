#include "hdr.h"

CHDRSystem::CHDRSystem()
{
	rCount = 0;
	bChanged = false;
	CHAR WP[MAX_PATH];
	if (GetCurrentDirectoryA(MAX_PATH, WP)) {
		m_strBasePath = WP;
		m_strBasePath += "\\";
	}

	int dirCount = GetPrivateProfileIntA("Dir", "count", 0, (m_strBasePath + std::string("Path.ini")).c_str());
	for (int i = 0; i < dirCount; i++)
	{
		CHAR v[MAX_PATH];
		GetPrivateProfileStringA("Dir", std::to_string(i).c_str(), "", v, MAX_PATH, (m_strBasePath + std::string("Path.ini")).c_str());
		std::string b = v;
		if (!b.empty()) m_Dirs.push_back(b);
	}
}

CHDRSystem::~CHDRSystem()
{
	if (bChanged) {
		rCount--;
		if (rCount < 0 || rCount > 10) rCount = 0;
		WritePrivateProfileStringA("Version", "Count", std::to_string(rCount).c_str(), (m_strBasePath + std::string("Path.ini")).c_str());
	}
}

void CHDRSystem::Pack()
{
	rCount = GetPrivateProfileIntA("Version", "Count", 10, (m_strBasePath + std::string("Path.ini")).c_str());
	if (rCount < 0 || rCount > 10) rCount = 0;

	for (const auto& v : m_Dirs) Pack(v);
}

bool CHDRSystem::Prepare(const std::string& v)
{
	std::remove(std::format("{}{}\\{}2.hdr", m_strBasePath, v, v).c_str());
	std::remove(std::format("{}{}\\{}2.src", m_strBasePath, v, v).c_str());

	if (rCount == 0 || bRepair)
		return false;

	return true;
}

void CHDRSystem::Unpack(const std::string& v)
{
	rCount = 10;

	std::string path = m_strBasePath + v + "\\";
	DWORD rowCount = 0;
	HANDLE hFile = CreateFileA(std::format("{}{}.hdr", path, v).c_str(), FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	HANDLE hSource = CreateFileA(std::format("{}{}.src", path, v).c_str(), FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE || hSource == INVALID_HANDLE_VALUE)
		return;

	DWORD dwCount = 0;

	if (ReadFile(hFile, &rowCount, 4, &dwCount, FALSE) == TRUE)
	{
		for (DWORD i = 0; i < rowCount; i++) {
			int nameLen = 0;
			if (ReadFile(hFile, &nameLen, 4, &dwCount, FALSE) == FALSE)
				break;
			std::string fileName(nameLen, 0);
			if (ReadFile(hFile, fileName.data(), nameLen, &dwCount, FALSE) == FALSE)
				break;
			DWORD offset = 0, length = 0;
			if (ReadFile(hFile, &offset, 4, &dwCount, FALSE) == FALSE)
				break;
			if (ReadFile(hFile, &length, 4, &dwCount, FALSE) == FALSE)
				break;

			if (fileName.find("\\") != std::string::npos && !WriteDirectory(GetDirectory(std::format("{}{}", path, fileName).c_str())))
				continue;

			HANDLE nFile = CreateFileA(std::format("{}{}", path, fileName).c_str(), FILE_WRITE_DATA, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if (nFile == INVALID_HANDLE_VALUE)
				continue;

			SetFilePointer(hSource, offset, NULL, FILE_BEGIN);
			BYTE* buff = new BYTE[length];

			if (ReadFile(hSource, buff, length, &dwCount, FALSE) == FALSE)
				continue;
			if (WriteFile(nFile, buff, length, &dwCount, FALSE) == FALSE)
				continue;

			delete[] buff;
			CloseHandle(nFile);
		}
	}

	CloseHandle(hFile);
	CloseHandle(hSource);

	std::remove(std::format("{}{}.hdr", path, v).c_str());
	std::remove(std::format("{}{}.src", path, v).c_str());
}

void CHDRSystem::Pack(const std::string& v, bool ignoreUnpack)
{
	if (!Prepare(v) && !ignoreUnpack) Unpack(v);

	std::string path = m_strBasePath + v + "\\";

	DWORD rowCount = 0, rowBefore = 0;
	HANDLE hFile = CreateFileA(std::format("{}{}.hdr", path, v).c_str(), FILE_READ_DATA | FILE_WRITE_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	HANDLE hSource = CreateFileA(std::format("{}{}.src", path, v).c_str(), FILE_READ_DATA | FILE_WRITE_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE || hSource == INVALID_HANDLE_VALUE)
		return;

	std::map<std::string, DWORD> m_hdrFileList;

	DWORD dwCount = 0;
	if (ReadFile(hFile, &rowCount, 4, &dwCount, FALSE) == TRUE)
	{
		if (dwCount == 0)
		{
			if (WriteFile(hFile, &rowCount, 4, &dwCount, FALSE) == FALSE)
				return;
		}
		rowBefore = rowCount;
		for (DWORD i = 0; i < rowCount; i++) {
			DWORD nameLen = 0;
			if (ReadFile(hFile, &nameLen, 4, &dwCount, FALSE) == FALSE)
				break;
			std::string fileName(nameLen, 0);
			if (ReadFile(hFile, fileName.data(), nameLen, &dwCount, FALSE) == FALSE)
				break;
			DWORD offsetPointer = SetFilePointer(hFile, 0, 0, FILE_CURRENT);
			m_hdrFileList.insert(std::make_pair(fileName, offsetPointer));
			SetFilePointer(hFile, 8, 0, FILE_CURRENT);
		}
	}

	for (const auto& entry : std::filesystem::recursive_directory_iterator(v + "\\"))
	{
		if (!entry.is_directory())
		{
			try {
				auto fileName = entry.path().filename().string();
				if (fileName.ends_with(".hdr") || fileName.ends_with(".src"))
					continue;
			}
			catch (std::system_error e) {
				continue;
			}

			std::string name = entry.path().string().substr(v.length() + 1, entry.path().string().length());

			HANDLE hTemp = CreateFileA(entry.path().string().c_str(), FILE_READ_DATA | FILE_WRITE_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hTemp == INVALID_HANDLE_VALUE)
				continue;

			auto itr = m_hdrFileList.find(name);
			if (itr != m_hdrFileList.end())
			{
				DWORD offset = SetFilePointer(hSource, 0, 0, FILE_END);
				DWORD length = entry.file_size();

				SetFilePointer(hFile, itr->second, NULL, FILE_BEGIN);
				if (WriteFile(hFile, &offset, 4, &dwCount, FALSE) == FALSE)
					continue;
				if (WriteFile(hFile, &length, 4, &dwCount, FALSE) == FALSE)
					continue;
			}
			else {
				DWORD nameLen = name.length();
				DWORD offset = SetFilePointer(hSource, 0, 0, FILE_END);
				DWORD length = entry.file_size();

				SetFilePointer(hFile, NULL, NULL, FILE_END);
				if (WriteFile(hFile, &nameLen, 4, &dwCount, FALSE) == FALSE)
					continue;
				if (WriteFile(hFile, name.c_str(), nameLen, &dwCount, FALSE) == FALSE)
					continue;
				if (WriteFile(hFile, &offset, 4, &dwCount, FALSE) == FALSE)
					continue;
				if (WriteFile(hFile, &length, 4, &dwCount, FALSE) == FALSE)
					continue;
				rowCount++;
			}

			DWORD len = SetFilePointer(hTemp, NULL, NULL, FILE_END);
			SetFilePointer(hTemp, NULL, NULL, FILE_BEGIN);
			BYTE* buff = new BYTE[len];
			if (buff == NULL)
				continue;
			if (ReadFile(hTemp, buff, len, &dwCount, FALSE) == FALSE)
				continue;
			if (WriteFile(hSource, buff, len, &dwCount, FALSE) == FALSE)
				continue;
			delete[] buff;
			CloseHandle(hTemp);
			std::remove(entry.path().string().c_str());
		}
	}

	if (rowCount != rowBefore) {
		bChanged = true;
		SetFilePointer(hFile, NULL, NULL, FILE_BEGIN);
		if (WriteFile(hFile, &rowCount, 4, &dwCount, FALSE) == FALSE)
			return;
	}

	if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
	if (hSource != INVALID_HANDLE_VALUE) CloseHandle(hSource);
}

BOOL CHDRSystem::WriteDirectory(const std::string& dd)
{
	HANDLE fFile;
	WIN32_FIND_DATAA fileinfo;
	std::vector<std::string> m_arr;
	BOOL tt;
	size_t x1 = 0;
	std::string tem = "";

	fFile = FindFirstFileA(dd.c_str(), &fileinfo);
	if (fileinfo.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
	{
		FindClose(fFile);
		return TRUE;
	}

	m_arr.clear();
	for (x1 = 0; x1 < dd.length(); x1++)
	{
		if (dd.at(x1) != '\\')
			tem += dd.at(x1);
		else
		{
			m_arr.push_back(tem);
			tem += "\\";
		}
		if (x1 == dd.length() - 1)
			m_arr.push_back(tem);
	}
	FindClose(fFile);
	for (x1 = 1; x1 < m_arr.size(); x1++)
	{
		tem = m_arr.at(x1);
		tt = CreateDirectoryA(tem.c_str(), NULL);
		if (tt)
			SetFileAttributesA(tem.c_str(), FILE_ATTRIBUTE_NORMAL);
	}
	m_arr.clear();
	fFile = FindFirstFileA(dd.c_str(), &fileinfo);
	if (fileinfo.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
	{
		FindClose(fFile);
		return TRUE;
	}
	else
	{
		FindClose(fFile);
		return FALSE;
	}
}

std::string CHDRSystem::GetDirectory(const std::string& path)
{
	size_t found = path.find_last_of("/\\");
	return(path.substr(0, found));
}