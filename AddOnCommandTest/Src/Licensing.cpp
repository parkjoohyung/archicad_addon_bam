#include "Licensing.hpp"
#include <string>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <cstdlib>

#if defined(WINDOWS) || defined(_WIN32)
#include <windows.h>
#include <wininet.h>
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "advapi32.lib")
#else
#include <stdio.h>
#endif

namespace MyProjectNamespace {

#if !defined(WINDOWS) && !defined(_WIN32)
static std::string GetMacLicenseFilePath() {
    const char* home = getenv("HOME");
    if (!home) return "";
    std::string dir = std::string(home) + "/Library/Application Support/ac_bam";
    std::string cmd = "mkdir -p \"" + dir + "\"";
    std::system(cmd.c_str());
    return dir + "/license.txt";
}
#endif

static std::string FetchUrl(const std::string& url) {
#if defined(WINDOWS) || defined(_WIN32)
    std::string content;
    HINTERNET hInternet = InternetOpenA("ArchiCAD-Addon", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) return "";
    HINTERNET hUrl = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (!hUrl) { InternetCloseHandle(hInternet); return ""; }
    char buffer[4096]; DWORD read;
    while (InternetReadFile(hUrl, buffer, sizeof(buffer)-1, &read) && read > 0) {
        buffer[read] = '\0';
        content += buffer;
    }
    InternetCloseHandle(hUrl);
    InternetCloseHandle(hInternet);
    return content;
#else
    std::string result;
    std::string cmd = "curl -s -f -L \"" + url + "\"";
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "";
    char buffer[512];
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        result += buffer;
    }
    pclose(pipe);
    return result;
#endif
}

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
    const char* url = "https://raw.githubusercontent.com/parkjoohyung/archicad_addon_bam/main/licenses.txt";
    std::string content = FetchUrl(url);
    if (content.empty()) return false;

    GS::UniString myHash = ComputeLicenseHash(email, key);
    std::stringstream ss(content);
    std::string line;
    while (std::getline(ss, line)) {
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
        if (myHash == GS::UniString(line.c_str())) return true;
    }
    return false;
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
    std::string filePath = GetMacLicenseFilePath();
    if (filePath.empty()) return false;
    std::ofstream ofs(filePath);
    if (!ofs.is_open()) return false;
    ofs << email.ToCStr().Get() << "\n" << key.ToCStr().Get() << "\n";
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
    std::string filePath = GetMacLicenseFilePath();
    if (filePath.empty()) return false;
    std::ifstream ifs(filePath);
    if (!ifs.is_open()) return false;
    std::string email, key;
    if (std::getline(ifs, email) && std::getline(ifs, key)) {
        return CheckLicenseOnline(GS::UniString(email.c_str()), GS::UniString(key.c_str()));
    }
    return false;
#endif
}

} // namespace MyProjectNamespace
