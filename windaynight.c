// cl.exe windaynight.c /nologo /W3 /WX /O1 /GS- /link /fixed /incremental:no /opt:icf /opt:ref /subsystem:console kernel32.lib advapi32.lib shell32.lib

// Thanks to @mmozeiko for the gist on how to minimise linking to the CRT:
// - https://gist.github.com/mmozeiko/81e9c0253cc724638947a53b826888e9

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <strsafe.h>

#define HELP L"WinDayNight\n\tToggles Windows theme between dark and light\n\tYou may also specify '-dark' or '-light'\n"

#define KEY     HKEY_CURRENT_USER
#define SUB_KEY L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize"
#define SYS_VAL L"SystemUsesLightTheme"
#define APP_VAL L"AppsUseLightTheme"

#define THEME_DARK  0
#define THEME_LIGHT 1

#define STRING_EQUAL(A, B) (lstrcmpiW(A, B) == 0)

void print(LPWSTR string) {
	size_t length = 0;
	StringCchLengthW(string, STRSAFE_MAX_CCH, &length);
	WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), string, (DWORD)length, NULL, NULL);
}

void print_error(LSTATUS status) {
	DWORD flags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER;
	LPWSTR buffer = {0};
	DWORD wchar_count = FormatMessageW(flags, NULL, status, 0, (LPWSTR)&buffer, 0, NULL);
	print(buffer);
	print(L"\n");
	LocalFree(&buffer);
}

int mainCRTStartup() {
	LPWSTR cmdline = GetCommandLineW();

	int argc = 0;
	LPWSTR *argv = CommandLineToArgvW(cmdline, &argc);

	DWORD new_theme = THEME_DARK;

	if (argc < 1 || argc > 2) {
		print(HELP);
		return 1;
	} else if (argc == 2) {
		if (STRING_EQUAL(argv[1], L"-dark")) {
			new_theme = THEME_DARK;
		} else
		if (STRING_EQUAL(argv[1], L"-light")) {
			new_theme = THEME_LIGHT;
		} else {
			print(HELP);
			return 0;
		}
	} else {
		DWORD data, size = sizeof(DWORD);
		LSTATUS status = RegGetValueW(KEY, SUB_KEY, SYS_VAL, RRF_RT_REG_DWORD, NULL, &data, &size);

		if (status != ERROR_SUCCESS) {
			print(L"Error reading current theme: ");
			print_error(status);
			return 1;
		}

		if (data == THEME_DARK) {
			new_theme = THEME_LIGHT;
		}
	}

	print(L"Switching to ");
	print(new_theme == THEME_DARK ? L"dark" : L"light");
	print(L" theme...\n");

	LSTATUS status = RegSetKeyValueW(KEY, SUB_KEY, SYS_VAL, REG_DWORD, &new_theme, sizeof(DWORD));
	if (status != ERROR_SUCCESS) {
		print(L"Error setting system theme: ");
		print_error(status);
	}

	status = RegSetKeyValueW(KEY, SUB_KEY, APP_VAL, REG_DWORD, &new_theme, sizeof(DWORD));
	if (status != ERROR_SUCCESS) {
		print(L"Error setting app theme: ");
		print_error(status);
	}

	return 0;
}
