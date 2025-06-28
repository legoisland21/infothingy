#define WINVER 0x0501
#include <windows.h>
#include <iostream>
#include <string>
#include <Wbemidl.h>
#include <shellapi.h>
#include <comdef.h>
#include <fstream>
#include <ctime>
#include <sstream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
using namespace std;
int opt;
string pid;
string exename;
bool admin;

string RunCommand(const string& cmd) {
    array<char, 256> buffer;
    string result;
    unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd.c_str(), "r"), _pclose);
    if (!pipe) throw runtime_error("Failed to run command.");
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

string BSTRToString(BSTR bstr) {
    int len = WideCharToMultiByte(CP_UTF8, 0, bstr, -1, nullptr, 0, nullptr, nullptr);
    string result(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, bstr, -1, &result[0], len, nullptr, nullptr);
    return result;
}

void LogToFile(const std::string& message) {
    ofstream logfile("infothing_log.txt", ios::app);
    if (logfile.is_open()) {
        time_t now = time(0);
        char* dt = ctime(&now);
        dt[strlen(dt) - 1] = '\0';
        logfile << "[" << dt << "] " << message << endl;
        logfile.close();
    }
}

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

string PrintArchitecture() {
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	int type = info.wProcessorArchitecture;
	if (type == 0) cout << "CPU Architecture: x86" << endl;
	if (type == 5) cout << "CPU Architecture: ARM" << endl;
	if (type == 6) cout << "CPU Architecture: IA64" << endl;
	if (type == 9) cout << "CPU Architecture: x64" << endl;
	if (type == 12) cout << "CPU Architecture: ARM64" << endl;
	if (type == 0) return "CPU Architecture: x86";
	if (type == 5) return "CPU Architecture: ARM";
	if (type == 6) return "CPU Architecture: IA64";
	if (type == 9) return "CPU Architecture: x64";
	if (type == 12) return "CPU Architecture: ARM64";
	return "NOT KNOWN";
}

string PrintMemory() {
    MEMORYSTATUSEX memInfo = { sizeof(memInfo) };
    GlobalMemoryStatusEx(&memInfo);
    cout << "Total RAM: " << memInfo.ullTotalPhys / (1024 * 1024) << " MB\n";
	return "Total RAM: " + to_string(memInfo.ullTotalPhys / (1024 * 1024));
}

string PrintCPUCores() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    cout << "CPU Cores: " << sysInfo.dwNumberOfProcessors << endl;
	return "CPU Cores: " + to_string(sysInfo.dwNumberOfProcessors);
}

string PrintCPUName() {
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
        0, KEY_READ, &hKey) == ERROR_SUCCESS) {

        char name[256];
        DWORD size = sizeof(name);
        RegQueryValueExA(hKey, "ProcessorNameString", nullptr, nullptr, (LPBYTE)&name, &size);
        cout << "CPU Name: " << name << endl;
		string info = string("CPU Name: ") + name;
		return info;
        RegCloseKey(hKey);
    }
	return "NOT KNOWN";
}

string PrintOSBuild() {
    HKEY hKey5;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
        0, KEY_READ, &hKey5) == ERROR_SUCCESS) {

        char version[64], build[64], sp[64];
        DWORD size = sizeof(version);
        RegQueryValueExA(hKey5, "CurrentVersion", nullptr, nullptr, (LPBYTE)version, &size);
        size = sizeof(build);
        RegQueryValueExA(hKey5, "CurrentBuildNumber", nullptr, nullptr, (LPBYTE)build, &size);
        size = sizeof(sp);
        RegQueryValueExA(hKey5, "CSDVersion", nullptr, nullptr, (LPBYTE)sp, &size);

        RegCloseKey(hKey5);
        
        std::string info = "OS Version: ";
        info += version;
        info += " (Build ";
        info += build;
        info += ")";
        if (strlen(sp) > 0) {
            info += " ";
            info += sp;
        }
        return info;
    }
    return "NOT KNOWN";
}


string PrintOwner() {
	HKEY hKey2;
	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
        0, KEY_READ, &hKey2) == ERROR_SUCCESS) {

        char name[128];
        DWORD size = sizeof(name);
        if (RegQueryValueExA(hKey2, "RegisteredOwner", nullptr, nullptr, (LPBYTE)name, &size) == ERROR_SUCCESS) {
            cout << "Owner: " << name << endl;
			string info = string("Owner: ") + name;
			return info;
        }
        RegCloseKey(hKey2);
    }
	return "NOT KNOWN";
}

string PrintRoot() {
	HKEY hKey3;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
        0, KEY_READ, &hKey3) == ERROR_SUCCESS) {

        char name[128];
        DWORD size = sizeof(name);
        if (RegQueryValueExA(hKey3, "SystemRoot", nullptr, nullptr, (LPBYTE)name, &size) == ERROR_SUCCESS) {
            cout << "Windows Root: " << name << endl;
			string info = string("Windows Root: ") + name;
			return info;
        }
        RegCloseKey(hKey3);
    }	
	return "NOT KNOWN";
}

string PrintOSVersion() {
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
        0, KEY_READ, &hKey) == ERROR_SUCCESS) {

        char name[128];
        DWORD size = sizeof(name);
        if (RegQueryValueExA(hKey, "ProductName", nullptr, nullptr, (LPBYTE)name, &size) == ERROR_SUCCESS) {
            cout << "OS Name: " << name << endl;
			string info = string("OS Name: ") + name;
			return info;
        }
        RegCloseKey(hKey);
    }
	return "NOT KNOWN";
}

string PrintGPU() {
    HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hr)) return "ERROR";

    hr = CoInitializeSecurity(NULL, -1, NULL, NULL,
                              RPC_C_AUTHN_LEVEL_DEFAULT,
                              RPC_C_IMP_LEVEL_IMPERSONATE,
                              NULL, EOAC_NONE, NULL);
    if (FAILED(hr)) {
        CoUninitialize();
        return "ERROR";
    }

    IWbemLocator* pLoc = NULL;
    hr = CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator, (LPVOID*)&pLoc);
    if (FAILED(hr)) {
        CoUninitialize();
        return "ERROR";
    }

    IWbemServices* pSvc = NULL;
    BSTR namespacePath = SysAllocString(L"ROOT\\CIMV2");
    hr = pLoc->ConnectServer(namespacePath, NULL, NULL, NULL, 0, NULL, NULL, &pSvc);
    SysFreeString(namespacePath);
    pLoc->Release();

    if (FAILED(hr)) {
        CoUninitialize();
        return "ERROR";
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
        return "ERROR";
    }

    IWbemClassObject* pObj = NULL;
    ULONG uReturn = 0;

    while (pEnumerator->Next(WBEM_INFINITE, 1, &pObj, &uReturn) == S_OK) {
        VARIANT vtProp;
        if (SUCCEEDED(pObj->Get(L"Name", 0, &vtProp, NULL, NULL))) {
            wcout << L"GPU Name: " << vtProp.bstrVal << endl;
			string gpuName = "GPU Name: " + BSTRToString(vtProp.bstrVal);
			return gpuName;
            VariantClear(&vtProp);
        }
        pObj->Release();
    }
    pSvc->Release();
    pEnumerator->Release();
    CoUninitialize();
	return "NOT KNOWN";
}

string PrintCurrentUser() {
    char username[128];
    DWORD size = sizeof(username);
    if (GetUserNameA(username, &size))
        cout << "Current User: " << username << endl;
	string info = string("Current User: ") + username;
	return info;
}

string PrintDiskInfo(const string& drive = "C:\\") {
	int total,free;
	string drivename;
    ULARGE_INTEGER freeBytesAvailable, totalBytes, totalFreeBytes;

    if (GetDiskFreeSpaceExA(drive.c_str(), &freeBytesAvailable, &totalBytes, &totalFreeBytes)) {
        cout << "Drive Letter: " << drive << endl;
        cout << "  Total Space: " << totalBytes.QuadPart / (1024 * 1024 * 1024) << " GB\n";
        cout << "  Free Space: " << totalFreeBytes.QuadPart / (1024 * 1024 * 1024) << " GB\n";
		string letter = drive;
		string totalsp = to_string(totalBytes.QuadPart / (1024 * 1024 * 1024));
		string freesp = to_string(totalFreeBytes.QuadPart / (1024 * 1024 * 1024));
		return "Drive Letter: " + letter + " | Total Space: " + totalsp + "GB | Free Space: " + freesp + "GB";
    }
    else {
        cerr << "ERROR:  " << drive << endl;
		return "NOT KNOWN";
    }
	return "NOT KNOWN";
}

void sysinfo() {
	LogToFile("Opened Sysinfo");
	system("cls");
	DWORD start = GetTickCount();
    cout << "========== SYSTEM INFO ==========" << endl;
	LogToFile("========== SYSTEM INFO ==========");
	LogToFile(PrintOSVersion());
	LogToFile(PrintOSBuild());
	LogToFile(PrintArchitecture());
	cout << "---------------------------------" << endl;
	LogToFile("---------------------------------");
	LogToFile(PrintCPUName());
	LogToFile(PrintCPUCores());
	LogToFile(PrintMemory());
	LogToFile(PrintGPU());
	LogToFile(PrintDiskInfo());
	LogToFile(PrintRoot());
	cout << "---------------------------------" << endl;
	LogToFile("---------------------------------");
	LogToFile(PrintCurrentUser());
	LogToFile(PrintOwner());
	cout << "=================================" << endl;
	cout << "========== BIOS INFO ============" << endl;
	LogToFile("=================================");
	LogToFile("========== BIOS INFO ============");
	string wmicmanuf = RunCommand("wmic bios get manufacturer");
	LogToFile(wmicmanuf);
	string wmicsmbio = RunCommand("wmic bios get smbiosbiosversion");
	LogToFile(wmicsmbio);
	string wmicrelea = RunCommand("wmic bios get releasedate");
	LogToFile(wmicrelea);
	system("wmic bios get manufacturer");
	system("wmic bios get smbiosbiosversion");
	system("wmic bios get releasedate");
	cout << "=================================" << endl;
	LogToFile("=================================");
	DWORD elapsed = GetTickCount() - start;
	cout << "Scan Completed in " << elapsed / 1000.0 << " sec" << endl;
	Sleep(7500);
}

int TaskMenu() {
	LogToFile("Opened TaskMenu");
    system("cls");
    cout << "Task Manager" << endl;
    cout << "Tasks:" << endl;
    system("tasklist");
    cout << "Choose a option" << endl;
    cout << "1. TaskKill" << endl;
    cout << "2. Start Task" << endl;
    cout << "3. Return to menu" << endl;
    cin >> opt;
    if (opt == 1) {
        cout << "Input PID: " << endl;
        cin >> pid;
        string command = "taskkill /F /PID " + pid + " >nul 2>&1";
        system(command.c_str());
		LogToFile("Killed Task with PID: " + pid);
    }
    if (opt == 2) {
        cout << "Input EXE: " << endl;
        cin >> exename;
        cout << "Elevation: " << endl;
        cin >> admin;
        if (admin) {
			LogToFile("Trying to open " + exename + " with admin rights");
            if (IsAdmin()) {
				LogToFile("Opened " + exename + " with admin rights");
                ShellExecuteA(
                    NULL,
                    "runas",
                    exename.c_str(),
                    NULL,
                    NULL,
                    SW_SHOWNORMAL
                );
            }
            if (!IsAdmin()) {
				LogToFile("ERROR: Admin rights");
                cout << "This command needs admin elevation" << endl;
                system("pause");
            }
        }
        if (!admin) {
			LogToFile("Opened " + exename);
            system(("start " + exename).c_str());
        }
    }
    if (opt == 3) {
        return 0;
    }
    return 0;
}

int PrintMenu() {
	LogToFile("Opened PrintMenu");
	opt = 0;
	system("cls");
	system("title Infothing Modern Windows Edition");
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
	cout << "7. Exit" << endl;
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
			system("dism /online /cleanup-image /restorehealth");
			system("sfc /scannow");
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
	if (opt == 7) {
        return 0;
    }
	system("pause");
	return 1;
}

int main() {
    while (PrintMenu());
	LogToFile("Program Quit");
    return 0;
}