#define WINVER 0x0501
#include <windows.h>
#include <iostream>
#include <string>
#include <Wbemidl.h>
#include <comdef.h>
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
    HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hr)) return;

    hr = CoInitializeSecurity(NULL, -1, NULL, NULL,
                              RPC_C_AUTHN_LEVEL_DEFAULT,
                              RPC_C_IMP_LEVEL_IMPERSONATE,
                              NULL, EOAC_NONE, NULL);
    if (FAILED(hr)) {
        CoUninitialize();
        return;
    }

    IWbemLocator* pLoc = NULL;
    hr = CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator, (LPVOID*)&pLoc);
    if (FAILED(hr)) {
        CoUninitialize();
        return;
    }

    IWbemServices* pSvc = NULL;
    BSTR namespacePath = SysAllocString(L"ROOT\\CIMV2");
    hr = pLoc->ConnectServer(namespacePath, NULL, NULL, NULL, 0, NULL, NULL, &pSvc);
    SysFreeString(namespacePath);
    pLoc->Release();

    if (FAILED(hr)) {
        CoUninitialize();
        return;
    }

    // ⬇️ Here's where your BSTR query goes
    BSTR queryLang = SysAllocString(L"WQL");
    BSTR queryText = SysAllocString(L"SELECT Name FROM Win32_VideoController");

    IEnumWbemClassObject* pEnumerator = NULL;
    hr = pSvc->ExecQuery(queryLang, queryText,
                         WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                         NULL, &pEnumerator);

    SysFreeString(queryLang);
    SysFreeString(queryText);

    if (FAILED(hr)) {
        pSvc->Release();
        CoUninitialize();
        return;
    }

    IWbemClassObject* pObj = NULL;
    ULONG uReturn = 0;

    while (pEnumerator->Next(WBEM_INFINITE, 1, &pObj, &uReturn) == S_OK) {
        VARIANT vtProp;
        if (SUCCEEDED(pObj->Get(L"Name", 0, &vtProp, NULL, NULL))) {
            std::wcout << L"GPU: " << vtProp.bstrVal << L"\n";
            VariantClear(&vtProp);
        }
        pObj->Release();
    }
    pSvc->Release();
    pEnumerator->Release();
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
	PrintGPU();
    PrintMemory();
    PrintCurrentUser();
    PrintDiskInfo();
    cin.get();
}
