#include "Updater.hpp"
#include "ACAPinc.h"
#include <string>

#ifdef WINDOWS
#include <windows.h>
#include <wininet.h>
#include <shellapi.h>
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "shell32.lib")
#endif

namespace MyProjectNamespace {

static const char* CURRENT_VERSION = "v1.0.1";
static const char* VERSION_URL = "https://raw.githubusercontent.com/parkjoohyung/archicad_addon_bam/main/version.txt";
static const char* RELEASES_URL = "https://github.com/parkjoohyung/archicad_addon_bam/releases/latest";

void CheckForUpdates(bool manual) {
#if defined(WINDOWS) || defined(_WIN32)
    static bool updateChecked = false;
    if (updateChecked && !manual) return;
    updateChecked = true;

    HINTERNET hInternet = InternetOpenA("ArchiCAD_Addon_Updater", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) return;
    HINTERNET hConnect = InternetOpenUrlA(hInternet, VERSION_URL, NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (hConnect) {
        char buffer[128]; DWORD bytesRead; std::string content;
        while (InternetReadFile(hConnect, buffer, sizeof(buffer)-1, &bytesRead) && bytesRead > 0) {
            buffer[bytesRead] = '\0';
            content += buffer;
        }
        InternetCloseHandle(hConnect);
        
        content.erase(content.find_last_not_of(" \n\r\t") + 1);
        content.erase(0, content.find_first_not_of(" \n\r\t"));
        
        if (!content.empty()) {
            GS::UniString latestVersion(content.c_str(), CC_UTF8);
            latestVersion.TrimLeft(); latestVersion.TrimRight();
            GS::UniString currentVersion(CURRENT_VERSION, CC_UTF8);

            auto parseVer = [](const GS::UniString& ver, int& maj, int& min, int& pat) {
                std::string s = ver.ToCStr().Get();
                if (!s.empty() && (s[0] == 'v' || s[0] == 'V')) {
                    s = s.substr(1);
                }
                maj = min = pat = 0;
                sscanf(s.c_str(), "%d.%d.%d", &maj, &min, &pat);
            };
            int lMaj, lMin, lPat, cMaj, cMin, cPat;
            parseVer(latestVersion, lMaj, lMin, lPat);
            parseVer(currentVersion, cMaj, cMin, cPat);
            bool isNewer = (lMaj > cMaj) || (lMaj == cMaj && lMin > cMin) || (lMaj == cMaj && lMin == cMin && lPat > cPat);

            if (isNewer) {
                GS::UniString title ("Update Notification");
                GS::UniString msg = GS::UniString::Printf("A new update is available!\n\nLatest version: %s\nCurrent version: %s\n\nWould you like to go to the download page now?", 
                                                          latestVersion.ToCStr().Get(), currentVersion.ToCStr().Get());
                
                const WCHAR* pMsg = (const WCHAR*)msg.ToUStr().Get();
                const WCHAR* pTitle = (const WCHAR*)title.ToUStr().Get();
                if (::MessageBoxW(nullptr, pMsg, pTitle, MB_YESNO | MB_ICONINFORMATION | MB_TOPMOST) == IDYES) {
                    ShellExecuteA(NULL, "open", RELEASES_URL, NULL, NULL, SW_SHOWNORMAL);
                }
            } else if (manual) {
                GS::UniString title ("Update Notification");
                GS::UniString msg ("You are using the latest version.");
                const WCHAR* pMsg = (const WCHAR*)msg.ToUStr().Get();
                const WCHAR* pTitle = (const WCHAR*)title.ToUStr().Get();
                ::MessageBoxW(nullptr, pMsg, pTitle, MB_OK | MB_ICONINFORMATION | MB_TOPMOST);
            }
        }
    }
    InternetCloseHandle(hInternet);
#else
    if (manual) {
        ACAPI_WriteReport(GS::UniString ("You are using the latest version."), true);
    }
#endif
}

const char* GetCurrentVersion() {
    return CURRENT_VERSION;
}

} // namespace MyProjectNamespace
