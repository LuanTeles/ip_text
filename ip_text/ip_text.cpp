
#include "ip_text.hpp"
#include <cell/cell_fs.h>
#include "Utils/Memory/Detours.hpp"


// helpers
sys_prx_id_t GetModuleHandle(const char* moduleName)
{
	return (moduleName) ? sys_prx_get_module_id_by_name(moduleName, 0, nullptr) : sys_prx_get_my_module_id();
}

sys_prx_module_info_t GetModuleInfo(sys_prx_id_t handle)
{
	sys_prx_module_info_t info{};
	static sys_prx_segment_info_t segments[10]{};
	static char filename[SYS_PRX_MODULE_FILENAME_SIZE]{};

	stdc::memset(segments, 0, sizeof(segments));
	stdc::memset(filename, 0, sizeof(filename));

	info.size = sizeof(info);
	info.segments = segments;
	info.segments_num = sizeof(segments) / sizeof(sys_prx_segment_info_t);
	info.filename = filename;
	info.filename_size = sizeof(filename);

	sys_prx_get_module_info(handle, 0, &info);
	return info;
}

std::string GetModuleFilePath(const char* moduleName)
{
	sys_prx_module_info_t info = GetModuleInfo(GetModuleHandle(moduleName));
	return std::string(info.filename);
}

std::string RemoveBaseNameFromPath(const std::string& filePath)
{
	size_t lastpath = filePath.find_last_of("/");
	if (lastpath == std::string::npos)
		return filePath;
	return filePath.substr(0, lastpath);
}

std::string GetCurrentDir()
{
	static std::string cachedModulePath;
	if (cachedModulePath.empty())
	{
		std::string path = RemoveBaseNameFromPath(GetModuleFilePath(nullptr));

		path += "/";  // include trailing slash

		cachedModulePath = path;
	}
	return cachedModulePath;
}

bool FileExist(const std::string& filePath)
{
	CellFsStat stat;
	if (cellFsStat(filePath.c_str(), &stat) == CELL_FS_SUCCEEDED)
		return (stat.st_mode & CELL_FS_S_IFREG);
	return false;
}

bool ReadFile(const std::string& filePath, void* data, size_t size)
{
	int fd;
	if (cellFsOpen(filePath.c_str(), CELL_FS_O_RDONLY, &fd, nullptr, 0) == CELL_FS_SUCCEEDED)
	{
		cellFsLseek(fd, 0, CELL_FS_SEEK_SET, nullptr);
		cellFsRead(fd, data, size, nullptr);
		cellFsClose(fd);

		return true;
	}
	return false;
}

bool ReplaceStr(std::wstring& str, const std::wstring& from, const std::string& to)
{
	size_t start_pos = str.find(from);
	if (start_pos == std::wstring::npos)
		return false;
	str.replace(start_pos, from.length(), std::wstring(to.begin(), to.end()));
	return true;
}


// main stuff
wchar_t gIpBuffer[512]{0};
paf::View* xmb_plugin{};
paf::View* system_plugin{};
paf::PhWidget* page_notification{};

bool LoadIpText()
{
	std::string ipTextPath = GetCurrentDir() + "ip_text.txt";
	char fileBuffer[512]{0};

	if (!FileExist(ipTextPath) || !ReadFile(ipTextPath, fileBuffer, sizeof(fileBuffer)))
		return false;

	stdc::swprintf(gIpBuffer, 512, L"%s", fileBuffer);
	return true;
}

std::wstring GetText()
{
	char ip[16];
	netctl::netctl_main_9A528B81(16, ip);

	std::wstring text(gIpBuffer);
	ReplaceStr(text, L"%d.%d.%d.%d", ip[0] ? ip : "0.0.0.0");
	return text;
}

void CreateText()
{
	if (!page_notification)
		return;

	paf::PhText* ip_text = (paf::PhText*)page_notification->FindChild("ip_text", 0);

	if (ip_text)
		return;

	ip_text = new paf::PhText(page_notification, nullptr);

	ip_text->SetName("ip_text")
		.SetColor({ 1.f, 1.f, 1.f, 1.f })
		.SetStyle(19, int(112));

	ip_text->SetLayoutPos(0x60000, 0x50000, 0, { 820.f, -465.f, 0.f, 0.f });
	ip_text->SetLayoutStyle(0, 20, 0.f);
	ip_text->SetLayoutStyle(1, 217, 0.f);
	ip_text->SetStyle(56, true);
	ip_text->SetStyle(18, int(34));
	ip_text->SetStyle(49, int(2));
}


// hooking
Detour* pafWidgetDrawThis_Detour;

int pafWidgetDrawThis_Hook(paf::PhWidget* _this, unsigned int r4, bool r5)
{
	if (_this && _this->m_Data.name == "ip_text")
	{
		paf::PhText* ip_text = (paf::PhText*)_this;

		ip_text->m_Data.metaAlpha = xmb_plugin ? 1.f : 0.f;

		if (ip_text->m_Data.metaAlpha > 0.f)
			ip_text->SetText(GetText(), 0);
	}

	return pafWidgetDrawThis_Detour->GetOriginal<int>(_this, r4, r5);
}

void Install()
{
	pafWidgetDrawThis_Detour = new Detour(((opd_s*)paf::paf_63D446B8)->sub, pafWidgetDrawThis_Hook);
}

void Remove()
{
	if (pafWidgetDrawThis_Detour)
		delete pafWidgetDrawThis_Detour;
}