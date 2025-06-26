#define WINVER 0x0501
#include <windows.h>
#ifndef __MINGW32__
extern "C" __declspec(dllexport) BOOL IsDebuggerPresent() {
    return FALSE;
}
extern "C" __declspec(dllexport) BOOL __stdcall Dummy_IsDebuggerPresent() {
    return FALSE;
}

#endif
#include <iostream>
#include <string>
using namespace std;

void PrintMemory() {
    MEMORYSTATUS memStat;
    memStat.dwLength = sizeof(memStat);
    GlobalMemoryStatus(&memStat);

    std::cout << "TOTAL: " << (memStat.dwTotalPhys / (1024 * 1024)) << " MB\n";
    std::cout << "F: " << (memStat.dwAvailPhys / (1024 * 1024)) << " MB\n";
}

void PrintCPUCores() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    std::cout << "CPU Cores: " << sysInfo.dwNumberOfProcessors << "\n";
}

void PrintCPUName() {
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
        0, KEY_READ, &hKey) == ERROR_SUCCESS) {

        char name[256] = {0};
        DWORD size = sizeof(name);
        LONG status = RegQueryValueExA(hKey, "ProcessorNameString", nullptr, nullptr, (LPBYTE)&name, &size);

        if (status != ERROR_SUCCESS || name[0] == '\0') {
            size = sizeof(name);
            status = RegQueryValueExA(hKey, "Identifier", nullptr, nullptr, (LPBYTE)&name, &size);
        }

        if (status == ERROR_SUCCESS) {
            std::string clean;
            for (DWORD i = 0; i < size && name[i] != '\0'; ++i) {
                unsigned char c = name[i];
                if (c >= 32 && c < 127)
                    clean += c;
                else if (c == 174)
                    clean += "(R)";
                else if (c == 153)
                    clean += "(TM)";
                else
                    clean += '?';
            }

            std::cout << "CPU: " << clean << "\n";
        }

        RegCloseKey(hKey);
    }
}

void PrintOSVersion() {
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
        0, KEY_READ, &hKey) == ERROR_SUCCESS) {

        char name[128];
        DWORD size = sizeof(name);
        if (RegQueryValueExA(hKey, "ProductName", nullptr, nullptr, (LPBYTE)name, &size) == ERROR_SUCCESS) {
            std::cout << "OS: " << name << "\n";
        }
        RegCloseKey(hKey);
    }
    HKEY hKey1;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
        0, KEY_READ, &hKey1) == ERROR_SUCCESS) {

        char name[128];
        DWORD size = sizeof(name);
        if (RegQueryValueExA(hKey1, "DisplayVersion", nullptr, nullptr, (LPBYTE)name, &size) == ERROR_SUCCESS) {
            std::cout << "VERSION: " << name << "\n";
        }
        RegCloseKey(hKey1);
    }
    HKEY hKey2;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
        0, KEY_READ, &hKey2) == ERROR_SUCCESS) {

        char name[128];
        DWORD size = sizeof(name);
        if (RegQueryValueExA(hKey2, "RegisteredOwner", nullptr, nullptr, (LPBYTE)name, &size) == ERROR_SUCCESS) {
            std::cout << "OWNER: " << name << "\n";
        }
        RegCloseKey(hKey2);
    }
}

void PrintCurrentUser() {
    char username[128];
    DWORD size = sizeof(username);
    if (GetUserNameA(username, &size))
        std::cout << "USER: " << username << "\n";
}

void PrintDiskInfo(const std::string& drive = "C:\\") {
    ULARGE_INTEGER freeBytesAvailable, totalBytes, totalFreeBytes;

    if (GetDiskFreeSpaceExA(drive.c_str(), &freeBytesAvailable, &totalBytes, &totalFreeBytes)) {
        std::cout << "DRIVE: " << drive << "\n";
        std::cout << "  TOTAL: " << totalBytes.QuadPart / (1024 * 1024 * 1024) << " GB\n";
        std::cout << "  FREE: " << totalFreeBytes.QuadPart / (1024 * 1024 * 1024) << " GB\n";
    }
    else {
        std::cerr << "ERROR:  " << drive << "\n";
    }
}

int main() {
    PrintOSVersion();
    PrintCPUName();
    PrintCPUCores();
    PrintMemory();
    PrintCurrentUser();
    PrintDiskInfo();
    cin.get();
}
