#include <windows.h>
#include <iostream>
#include <string>
#include <wbemidl.h>
#include <comdef.h>
#pragma comment(lib, "wbemuuid.lib")
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
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr)) return;

    hr = CoInitializeSecurity(nullptr, -1, nullptr, nullptr,
                              RPC_C_AUTHN_LEVEL_DEFAULT,
                              RPC_C_IMP_LEVEL_IMPERSONATE,
                              nullptr, EOAC_NONE, nullptr);
    if (FAILED(hr) && hr != RPC_E_TOO_LATE) {
        CoUninitialize();
        return;
    }

    IWbemLocator* pLoc = nullptr;
    hr = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator, (LPVOID*)&pLoc);
    if (FAILED(hr)) {
        CoUninitialize();
        return;
    }

    IWbemServices* pSvc = nullptr;
    hr = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr, nullptr, 0, nullptr, nullptr, &pSvc);
    if (FAILED(hr)) {
        pLoc->Release();
        CoUninitialize();
        return;
    }

    hr = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                           nullptr, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
                           nullptr, EOAC_NONE);

    IEnumWbemClassObject* pEnumerator = nullptr;
    hr = pSvc->ExecQuery(
        SysAllocString(L"WQL"),
		SysAllocString(L"SELECT Name FROM Win32_VideoController"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr, &pEnumerator);

    if (SUCCEEDED(hr)) {
        IWbemClassObject* pObj = nullptr;
        ULONG returned = 0;
        while (pEnumerator->Next(WBEM_INFINITE, 1, &pObj, &returned) == S_OK) {
            VARIANT name, ram;
            VariantInit(&name);
            VariantInit(&ram);

            if (SUCCEEDED(pObj->Get(L"Name", 0, &name, nullptr, nullptr))) {
                std::wcout << L"GPU: " << name.bstrVal << L"\n";
            }
            if (SUCCEEDED(pObj->Get(L"AdapterRAM", 0, &ram, nullptr, nullptr)) && ram.vt == VT_I4) {
                std::wcout << L"VRAM: " << ram.intVal / (1024 * 1024) << L" MB\n";
            }

            VariantClear(&name);
            VariantClear(&ram);
            pObj->Release();
        }
        pEnumerator->Release();
    }

    pSvc->Release();
    pLoc->Release();
    CoUninitialize();
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
