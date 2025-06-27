#define WINVER 0x0501
#include <windows.h>
#include <iostream>
#include <string>
#include <Wbemidl.h>
#include <comdef.h>
using namespace std;
int opt;
string pid;

bool IsAdmin() {
    BOOL isAdmin = FALSE;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID adminGroup;

    if (AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS, 0,0,0,0,0,0, &adminGroup)) {
        CheckTokenMembership(NULL, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }
    return isAdmin;
}

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

void sysinfo() {
	system("cls");
	DWORD start = GetTickCount();
    cout << "========== SYSTEM INFO ==========" << endl;
	PrintOSVersion();
	PrintOSBuild();
	PrintReleaseId();
	PrintArchitecture();
	system("wmic bios get smbiosbiosversion");
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
	cout << "========== BIOS INFO ============" << endl;
	system("wmic bios get manufacturer");
    system("wmic bios get smbiosbiosversion");
    system("wmic bios get releasedate");
	cout << "=================================" << endl;
	DWORD elapsed = GetTickCount() - start;
	cout << "Scan Completed in " << elapsed / 1000.0 << " sec" << endl;
	Sleep(7500);
}

void TaskMenu() {
	system("cls");
	cout << "Task Manager" << endl;
	cout << "Tasks:" << endl;
	system("tasklist");
	cout << "Choose a option" << endl;
	cout << "1. TaskKill" << endl;
	cout << "2. Return to menu" << endl;
	cin >> opt;
	if (opt == 1) {
		cout << "Input PID: "<< endl;
		cin >> pid;
		string command = "taskkill /F /PID " + pid + " >nul 2>&1";
		system(command.c_str());
	}
}

void PrintMenu() {
	system("cls");
	system("title Infothing Older Windows Edition");
	cout << "                                             " << endl;
	cout << " _       ___       _    _    _                "<< endl;
	cout << "|_| ___ |  _| ___ | |_ | |_ |_| ___  ___  _ _ " << endl;
	cout << "| ||   ||  _|| . ||  _||   || ||   || . || | |" << endl;
	cout << "|_||_|_||_|  |___||_|  |_|_||_||_|_||_  ||_  |" << endl;
	cout << "                                    |___||___|" << endl;
	cout << endl;
	cout << "========== infothing ==========" << endl;
	cout << "1. Hardware Info" << endl;
	cout << "2. Task Manager" << endl;
	cout << "3. Repair System (Needs Admin)" << endl;
	cout << "4. Network Glitch Repair (Needs Admin)" << endl;
	cout << "5. IPConfig All" << endl;
	cout << "6. Restart Explorer" << endl;
	cout << "===============================" << endl;
	cout << "Choose an option: " << endl;
	cin >> opt;
	if(opt == 1) {
		sysinfo();
	}
	if(opt == 2) {
		TaskMenu();
	}
	if(opt == 3) {
		if (!IsAdmin()) {
			cout << "Launch with Admin rights!" << endl;
			Sleep(2500);
		}
		else {
			system("chkdsk C: /f /r");
		}
	}
	if(opt == 4) {
		if (!IsAdmin()) {
			cout << "Launch with Admin rights!" << endl;
			Sleep(2500);
		}
		else {
			system("netsh winsock reset");
			Sleep(1000);
		}
	}
	if(opt == 5) {
		system("ipconfig /all");
		Sleep(7500);
	}
	if(opt == 6) {
		system("taskkill /f /im explorer.exe");
		Sleep(1000);
		system("start explorer.exe");
	}
}

int main() {
    while (true) {
		PrintMenu();
	}
	return -1;
}