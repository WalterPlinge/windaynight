package daynight

import "core:fmt"
import "core:os"

import win "core:sys/windows"

HELP :: "daynight.exe\n\tToggles Windows theme between dark and light\n\tYou may also specify '-dark' or '-light'\n"

SUB_KEY :: "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize"
SYS_VAL :: "SystemUsesLightTheme"
APP_VAL :: "AppsUseLightTheme"

winstr :: proc(str: string) -> [^]u16 {
	return raw_data(win.utf8_to_utf16(str))
}

winerr :: proc(status: win.LSTATUS) {
	buffer: win.DWORD_PTR
	defer win.LocalFree(&buffer)
	flags : win.DWORD = win.FORMAT_MESSAGE_ALLOCATE_BUFFER | win.FORMAT_MESSAGE_FROM_SYSTEM
	win.FormatMessageW(
		flags,
		nil,
		win.DWORD(status),
		0,
		win.LPWSTR(&buffer),
		0,
		nil,
	)
	// FIXME
	fmt.printf("Error message: %s\n", buffer)
}

main :: proc() {
	new_theme : win.DWORD = 0

	if len(os.args) < 1 || len(os.args) > 2 {
		fmt.print(HELP)
		return
	} else if len(os.args) == 2 {
		switch os.args[1] {
			case "-dark": new_theme = 0
			case "-light": new_theme = 1
			case:
				fmt.print(HELP)
				return
		}
	} else {
		type, data, size : win.DWORD = 0, 0, size_of(win.DWORD)
		status := win.RegGetValueW(
			win.HKEY_CURRENT_USER,
			winstr(SUB_KEY),
			winstr(SYS_VAL),
			win.RRF_RT_DWORD,
			&type,
			&data,
			&size,
		)

		if status != win.LSTATUS(win.ERROR_SUCCESS) {
			fmt.println("Error reading current theme")
			winerr(status)
			return
		}

		new_theme = (data + 1) % 2
	}

	fmt.printf("Switching to %s theme", new_theme == 0 ? "dark" : "light")

	{
		status := win.RegSetKeyValueW(
			win.HKEY_CURRENT_USER,
			winstr(SUB_KEY),
			winstr(SYS_VAL),
			win.REG_DWORD,
			&new_theme,
			size_of(win.DWORD),
		)
		if status != win.LSTATUS(win.ERROR_SUCCESS) {
			fmt.println("Error setting system theme")
			winerr(status)
		}

		status = win.RegSetKeyValueW(
			win.HKEY_CURRENT_USER,
			winstr(SUB_KEY),
			winstr(APP_VAL),
			win.REG_DWORD,
			&new_theme,
			size_of(win.DWORD),
		)
		if status != win.LSTATUS(win.ERROR_SUCCESS) {
			fmt.println("Error setting app theme")
			winerr(status)
		}
	}
}