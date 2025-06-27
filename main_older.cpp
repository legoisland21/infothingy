#define WINVER 0x0501
#include <windows.h>
#include <iostream>
#include <string>
#include <Wbemidl.h>
#include <comdef.h>
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
            wcout << L"GPU Name: " << vtProp.bstrVal << endl;
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
        cout << "Current User: " << username << endl;
}

void PrintDiskInfo(const string& drive = "C:\\") {
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
	system("title Infothing Older Windows Edition");
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