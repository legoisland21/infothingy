#include <windows.h>
#include <iostream>
#include <string>
#include <dxgi.h>
#pragma comment(lib, "dxgi.lib")
using namespace std;

void PrintMemory() {
    MEMORYSTATUSEX memInfo = { sizeof(memInfo) };
    GlobalMemoryStatusEx(&memInfo);
    std::cout << "RAM: " << memInfo.ullTotalPhys / (1024 * 1024) << " MB\n";
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

        char name[256];
        DWORD size = sizeof(name);
        RegQueryValueExA(hKey, "ProcessorNameString", nullptr, nullptr, (LPBYTE)&name, &size);
        std::cout << "CPU: " << name << "\n";
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

void PrintGPU() {
    IDXGIFactory* pFactory = nullptr;
    if (SUCCEEDED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory))) {
        IDXGIAdapter* pAdapter = nullptr;
        if (SUCCEEDED(pFactory->EnumAdapters(0, &pAdapter))) {
            DXGI_ADAPTER_DESC desc;
            if (SUCCEEDED(pAdapter->GetDesc(&desc))) {
                std::wcout << L"GPU: " << desc.Description << std::endl;
                std::wcout << L"VRAM: "
                    << desc.DedicatedVideoMemory / (1024 * 1024) << L" MB" << std::endl;
            }
            pAdapter->Release();
        }
        pFactory->Release();
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
    PrintGPU();
    PrintCurrentUser();
    PrintDiskInfo();
    cin.get();
}