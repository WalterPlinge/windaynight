// cl daynight.c /link kernel32.lib advapi32.lib

/*
New-ItemProperty -Path HKCU:\SOFTWARE\Microsoft\Windows\CurrentVersion\Themes\Personalize -Name SystemUsesLightTheme -Value 0 -Type Dword -Force
New-ItemProperty -Path HKCU:\SOFTWARE\Microsoft\Windows\CurrentVersion\Themes\Personalize -Name AppsUseLightTheme -Value 0 -Type Dword -Force
*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdio.h>

#define STRING_EQUAL(A, B) (strcmp(A, B) == 0)

#define HELP "daynight.exe\n\tToggles Windows theme between dark and light\n\tYou may also specify '-dark' or '-light'\n"

#define KEY     HKEY_CURRENT_USER
#define SUB_KEY "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize"
#define SYS_VAL "SystemUsesLightTheme"
#define APP_VAL "AppsUseLightTheme"

void print_error(LSTATUS status) {
	DWORD_PTR buffer = {0};
	DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM;
	FormatMessage(
		flags,           // [in]           DWORD   dwFlags,
		NULL,            // [in, optional] LPCVOID lpSource,
		status,          // [in]           DWORD   dwMessageId,
		0,               // [in]           DWORD   dwLanguageId,
		(LPTSTR)&buffer, // [out]          LPWSTR  lpBuffer,
		0,               // [in]           DWORD   nSize,
		NULL             // [in, optional] va_list *Arguments
	);
	printf("Error message: %s\n", (TCHAR *)buffer);
	LocalFree(&buffer);
}

int main(int argc, char** argv) {
	DWORD new_theme = 0;

	if (argc < 1 || argc > 2) {
		printf(HELP);
		return 1;
	} else if (argc == 2) {
		if (STRING_EQUAL(argv[1], "-dark")) {
			new_theme = 0;
		} else
		if (STRING_EQUAL(argv[1], "-light")) {
			new_theme = 1;
		} else {
			printf(HELP);
			return 0;
		}
	} else {
		DWORD type, data, size = sizeof(DWORD);
		LSTATUS status = RegGetValue(
			KEY,           // [in]                HKEY    hkey,
			TEXT(SUB_KEY), // [in, optional]      LPCSTR  lpSubKey,
			TEXT(SYS_VAL), // [in, optional]      LPCSTR  lpValue,
			RRF_RT_DWORD,  // [in, optional]      DWORD   dwFlags,
			&type,         // [out, optional]     LPDWORD pdwType,
			&data,         // [out, optional]     PVOID   pvData,
			&size          // [in, out, optional] LPDWORD pcbData
		);

		if (status != ERROR_SUCCESS) {
			printf("Error reading current theme\n");
			print_error(status);
			return 1;
		}

		new_theme = (data + 1) % 2;
	}

	printf("Switching to %s theme!\n", new_theme == 0 ? "dark" : "light");

	{
		LSTATUS status = RegSetKeyValue(
			KEY,           // [in]           HKEY    hKey,
			TEXT(SUB_KEY), // [in, optional] LPCSTR  lpSubKey,
			TEXT(SYS_VAL), // [in, optional] LPCSTR  lpValueName,
			REG_DWORD,     // [in]           DWORD   dwType,
			&new_theme,    // [in, optional] LPCVOID lpData,
			sizeof(DWORD)  // [in]           DWORD   cbData
		);
		if (status != ERROR_SUCCESS) {
			printf("Error setting system theme\n");
			print_error(status);
		}

		status = RegSetKeyValue(
			KEY,           // [in]           HKEY    hKey,
			TEXT(SUB_KEY), // [in, optional] LPCSTR  lpSubKey,
			TEXT(APP_VAL), // [in, optional] LPCSTR  lpValueName,
			REG_DWORD,     // [in]           DWORD   dwType,
			&new_theme,    // [in, optional] LPCVOID lpData,
			sizeof(DWORD)  // [in]           DWORD   cbData
		);
		if (status != ERROR_SUCCESS) {
			printf("Error setting app theme\n");
			print_error(status);
		}
	}

	return 0;
}