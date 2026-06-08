#include "Licensing.hpp"
#include <string>
#include <sstream>
#include <algorithm>
#ifdef WINDOWS
#include <windows.h>
#include <wininet.h>
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "advapi32.lib")
#endif

namespace MyProjectNamespace {

GS::UniString ComputeLicenseHash(const GS::UniString& email, const GS::UniString& key) {
    std::string combined = email.ToCStr().Get();
    combined += "|";
    combined += key.ToCStr().Get();
    combined += "|Salt!Secret2026";

    uint64_t hash = 0xcbf29ce484222325ULL;
    for (unsigned char c : combined) {
        hash ^= (uint64_t)c;
        hash *= 0x100000001b3ULL;
    }
    return GS::UniString::Printf("%016llx", hash);
}

bool CheckLicenseOnline(const GS::UniString& email, const GS::UniString& key) {
#if defined(WINDOWS) || defined(_WIN32)
    HINTERNET hInternet = InternetOpenA("ArchiCAD-Addon", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) return false;

    const char* url = "https://raw.githubusercontent.com/parkjoohyung/archicad_addon_bam/main/licenses.txt";
    HINTERNET hUrl = InternetOpenUrlA(hInternet, url, NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (!hUrl) { InternetCloseHandle(hInternet); return false; }

    char buffer[4096]; DWORD read;
    std::string content;
    while (InternetReadFile(hUrl, buffer, sizeof(buffer)-1, &read) && read > 0) {
        buffer[read] = '\0';
        content += buffer;
    }

    InternetCloseHandle(hUrl);
    InternetCloseHandle(hInternet);

    GS::UniString myHash = ComputeLicenseHash(email, key);
    std::stringstream ss(content);
    std::string line;
    while (std::getline(ss, line)) {
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
        if (myHash == GS::UniString(line.c_str())) return true;
    }
    return false;
#else
    // Temporary bypass for macOS build
    return true;
#endif
}

bool SaveLicenseLocal(const GS::UniString& email, const GS::UniString& key) {
#if defined(WINDOWS) || defined(_WIN32)
    HKEY hKey;
    if (RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\ArchiCAD_Premium_Addon", 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        std::string eStr = email.ToCStr().Get();
        std::string kStr = key.ToCStr().Get();
        RegSetValueExA(hKey, "Email", 0, REG_SZ, (const BYTE*)eStr.c_str(), (DWORD)eStr.length() + 1);
        RegSetValueExA(hKey, "Key", 0, REG_SZ, (const BYTE*)kStr.c_str(), (DWORD)kStr.length() + 1);
        RegCloseKey(hKey);
        return true;
    }
    return false;
#else
    return true;
#endif
}

bool IsAlreadyLicensed() {
#if defined(WINDOWS) || defined(_WIN32)
    char emailBuf[255] = {0}, keyBuf[255] = {0};
    DWORD szEmail = 255, szKey = 255;
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\ArchiCAD_Premium_Addon", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        bool ok = (RegQueryValueExA(hKey, "Email", NULL, NULL, (BYTE*)emailBuf, &szEmail) == ERROR_SUCCESS &&
                   RegQueryValueExA(hKey, "Key", NULL, NULL, (BYTE*)keyBuf, &szKey) == ERROR_SUCCESS);
        RegCloseKey(hKey);
        if (ok) {
            return CheckLicenseOnline(GS::UniString(emailBuf), GS::UniString(keyBuf));
        }
    }
    return false;
#else
    return true;
#endif
}

} // namespace MyProjectNamespace
