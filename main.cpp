#include <windows.h>
#include <iostream>
#include <string>
#include <dxgi.h>
#pragma comment(lib, "dxgi.lib")
using namespace std;

void PrintArchitecture() {
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	int type = info.wProcessorArchitecture;
	if (type == 0) cout << "CPU Architecture: x86" << endl;
	if (type == 5) cout << "CPU Architecture: ARM" << endl;
	if (type == 6) cout << "CPU Architecture: IA64" << endl;
	if (type == 9) cout << "CPU Architecture: x64" << endl;
	if (type == 12) cout << "CPU Architecture: ARM64" << endl;
}

void PrintMemory() {
    MEMORYSTATUSEX memInfo = { sizeof(memInfo) };
    GlobalMemoryStatusEx(&memInfo);
    cout << "Total RAM: " << memInfo.ullTotalPhys / (1024 * 1024) << " MB\n";
}

void PrintCPUCores() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    cout << "CPU Cores: " << sysInfo.dwNumberOfProcessors << endl;
}

void PrintCPUName() {
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
        0, KEY_READ, &hKey) == ERROR_SUCCESS) {

        char name[256];
        DWORD size = sizeof(name);
        RegQueryValueExA(hKey, "ProcessorNameString", nullptr, nullptr, (LPBYTE)&name, &size);
        cout << "CPU Name: " << name << endl;
        RegCloseKey(hKey);
    }
}

void PrintReleaseId() {
	HKEY hKey5;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
        0, KEY_READ, &hKey5) == ERROR_SUCCESS) {

        char name[128];
        DWORD size = sizeof(name);
        if (RegQueryValueExA(hKey5, "ReleaseId", nullptr, nullptr, (LPBYTE)name, &size) == ERROR_SUCCESS) {
            cout << "Release ID: " << name << endl;
        }
        RegCloseKey(hKey5);
    }	
}

void PrintOSBuild() {
	HKEY hKey1;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
        0, KEY_READ, &hKey1) == ERROR_SUCCESS) {

        char name[128];
        DWORD size = sizeof(name);
        if (RegQueryValueExA(hKey1, "DisplayVersion", nullptr, nullptr, (LPBYTE)name, &size) == ERROR_SUCCESS) {
            cout << "Version: " << name << endl;
        }
        RegCloseKey(hKey1);
    }
}

void PrintOwner() {
	HKEY hKey2;
	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
        0, KEY_READ, &hKey2) == ERROR_SUCCESS) {

        char name[128];
        DWORD size = sizeof(name);
        if (RegQueryValueExA(hKey2, "RegisteredOwner", nullptr, nullptr, (LPBYTE)name, &size) == ERROR_SUCCESS) {
            cout << "Owner: " << name << endl;
        }
        RegCloseKey(hKey2);
    }
}

void PrintRoot() {
	HKEY hKey3;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
        0, KEY_READ, &hKey3) == ERROR_SUCCESS) {

        char name[128];
        DWORD size = sizeof(name);
        if (RegQueryValueExA(hKey3, "SystemRoot", nullptr, nullptr, (LPBYTE)name, &size) == ERROR_SUCCESS) {
            cout << "Windows Root: " << name << endl;
        }
        RegCloseKey(hKey3);
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
            cout << "OS Name: " << name << endl;
        }
        RegCloseKey(hKey);
    }
}

void PrintGPU() {
    IDXGIFactory* pFactory = nullptr;
    if (SUCCEEDED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory))) {
        IDXGIAdapter* pAdapter = nullptr;
        if (SUCCEEDED(pFactory->EnumAdapters(0, &pAdapter))) {
            DXGI_ADAPTER_DESC desc;
            if (SUCCEEDED(pAdapter->GetDesc(&desc))) {
                wcout << L"GPU Name: " << desc.Description << endl;
                wcout << L"Total VRAM: "
                    << desc.DedicatedVideoMemory / (1024 * 1024) << L" MB" << endl;
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
        cout << "Current User: " << username << endl;
}

void PrintDiskInfo(const string& drive = "C:\\") {
	int total,free;
	string drivename;
    ULARGE_INTEGER freeBytesAvailable, totalBytes, totalFreeBytes;

    if (GetDiskFreeSpaceExA(drive.c_str(), &freeBytesAvailable, &totalBytes, &totalFreeBytes)) {
        cout << "Drive Letter: " << drive << endl;
        cout << "  Total Space: " << totalBytes.QuadPart / (1024 * 1024 * 1024) << " GB\n";
        cout << "  Free Space: " << totalFreeBytes.QuadPart / (1024 * 1024 * 1024) << " GB\n";
    }
    else {
        cerr << "ERROR:  " << drive << endl;
    }
}


int main() {
	system("title Infothing Modern Windows Edition");
    cout << "========== SYSTEM INFO ==========" << endl;
	PrintOSVersion();
	PrintOSBuild();
	PrintReleaseId();
	PrintArchitecture();
	cout << "---------------------------------" << endl;
	PrintCPUName();
	PrintCPUCores();
	PrintMemory();
	PrintGPU();
	PrintDiskInfo();
	PrintRoot();
	cout << "---------------------------------" << endl;
	PrintCurrentUser();
	PrintOwner();
	cout << "=================================" << endl;
    cin.get();
}