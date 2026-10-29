// SPDX-License-Identifier: GPL-3.0-or-later

#define WINVER 0x0601
#define _WIN32_WINNT 0x0601
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdlib.h>
#define TRAINER_PROCESS_NAME L"FFXTrainerFixture.exe"
#include "../src/trainer.c"
#include "../src/main.c"

enum { FIXTURE_SIZE = 0xF40000 };

static void initialize_image(uint8_t *image, uint32_t relocation) {
	IMAGE_DOS_HEADER *dos = (IMAGE_DOS_HEADER *)image;
	IMAGE_NT_HEADERS32 *nt = (IMAGE_NT_HEADERS32 *)(image + 0x80);
	IMAGE_SECTION_HEADER *sections = (IMAGE_SECTION_HEADER *)(nt + 1);
	IMAGE_BASE_RELOCATION *block;
	uint32_t saved = s_relocation, neighbor = 0x402000 + relocation;
	unsigned i;
	uint32_t battles = 55;
	s_relocation = relocation;
	dos->e_magic = IMAGE_DOS_SIGNATURE;
	dos->e_lfanew = 0x80;
	nt->Signature = IMAGE_NT_SIGNATURE;
	nt->FileHeader.Machine = IMAGE_FILE_MACHINE_I386;
	nt->FileHeader.NumberOfSections = 4;
	nt->FileHeader.SizeOfOptionalHeader = sizeof nt->OptionalHeader;
	nt->FileHeader.Characteristics = IMAGE_FILE_EXECUTABLE_IMAGE | IMAGE_FILE_32BIT_MACHINE;
	nt->OptionalHeader.Magic = IMAGE_NT_OPTIONAL_HDR32_MAGIC;
	nt->OptionalHeader.ImageBase = 0x400000;
	nt->OptionalHeader.SizeOfImage = FIXTURE_SIZE;
	nt->OptionalHeader.SizeOfHeaders = 0x1000;
	nt->OptionalHeader.SizeOfCode = 0x4FF000;
	nt->OptionalHeader.SizeOfInitializedData = FIXTURE_SIZE - 0xD30000 + 0x2000;
	nt->OptionalHeader.AddressOfEntryPoint = 0x1000;
	nt->OptionalHeader.BaseOfCode = 0x1000;
	nt->OptionalHeader.BaseOfData = 0xD30000;
	nt->OptionalHeader.SectionAlignment = 0x1000;
	nt->OptionalHeader.FileAlignment = 0x200;
	nt->OptionalHeader.MajorSubsystemVersion = 6;
	nt->OptionalHeader.MajorOperatingSystemVersion = 6;
	nt->OptionalHeader.Subsystem = IMAGE_SUBSYSTEM_WINDOWS_GUI;
	nt->OptionalHeader.DllCharacteristics = IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE | IMAGE_DLLCHARACTERISTICS_NX_COMPAT;
	nt->OptionalHeader.SizeOfStackReserve = 0x100000;
	nt->OptionalHeader.SizeOfStackCommit = 0x1000;
	nt->OptionalHeader.SizeOfHeapReserve = 0x100000;
	nt->OptionalHeader.SizeOfHeapCommit = 0x1000;
	nt->OptionalHeader.NumberOfRvaAndSizes = IMAGE_NUMBEROF_DIRECTORY_ENTRIES;
	nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress = 0x500000;
	nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size = 48;
	nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress = 0x501000;
	nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size = 40;
	nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress = 0x501050;
	nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].Size = 8;
	sections[0].VirtualAddress = sections[0].PointerToRawData = 0x1000;
	memcpy(sections[0].Name, ".text", 6);
	sections[0].Misc.VirtualSize = sections[0].SizeOfRawData = 0x4FF000;
	sections[0].Characteristics = IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_CNT_CODE;
	sections[1].VirtualAddress = sections[1].PointerToRawData = 0x500000;
	memcpy(sections[1].Name, ".reloc", 7);
	sections[1].Misc.VirtualSize = sections[1].SizeOfRawData = 0x1000;
	sections[1].Characteristics = IMAGE_SCN_MEM_READ | IMAGE_SCN_CNT_INITIALIZED_DATA;
	sections[2].VirtualAddress = sections[2].PointerToRawData = 0x501000;
	memcpy(sections[2].Name, ".idata", 7);
	sections[2].Misc.VirtualSize = sections[2].SizeOfRawData = 0x1000;
	sections[2].Characteristics = IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE | IMAGE_SCN_CNT_INITIALIZED_DATA;
	sections[3].VirtualAddress = sections[3].PointerToRawData = 0xD30000;
	memcpy(sections[3].Name, ".data", 6);
	sections[3].Misc.VirtualSize = sections[3].SizeOfRawData = FIXTURE_SIZE - 0xD30000;
	sections[3].Characteristics = IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE | IMAGE_SCN_CNT_INITIALIZED_DATA;
	block = (IMAGE_BASE_RELOCATION *)(image + 0x500000);
	block->VirtualAddress = 0x1000;
	block->SizeOfBlock = 12;
	((WORD *)(block + 1))[0] = 0x3004;
	block = (IMAGE_BASE_RELOCATION *)((uint8_t *)block + 12);
	block->VirtualAddress = 0x38B000;
	block->SizeOfBlock = 12;
	((WORD *)(block + 1))[0] = 0x382A;
	block = (IMAGE_BASE_RELOCATION *)((uint8_t *)block + 12);
	block->VirtualAddress = 0x490000;
	block->SizeOfBlock = 12;
	((WORD *)(block + 1))[0] = 0x3F14;
	block = (IMAGE_BASE_RELOCATION *)((uint8_t *)block + 12);
	block->VirtualAddress = 0x491000;
	block->SizeOfBlock = 12;
	((WORD *)(block + 1))[0] = 0x3C1A;
	((WORD *)(block + 1))[1] = 0x3F79;
	{
		IMAGE_IMPORT_DESCRIPTOR *import = (IMAGE_IMPORT_DESCRIPTOR *)(image + 0x501000);
		uint32_t thunk = 0x501060, iat = 0x901050 + relocation;
		import->OriginalFirstThunk = 0x501040;
		import->FirstThunk = 0x501050;
		import->Name = 0x501080;
		memcpy(image + 0x501040, &thunk, 4);
		memcpy(image + 0x501050, &thunk, 4);
		memcpy(image + 0x501062, "Sleep", 6);
		memcpy(image + 0x501080, "KERNEL32.dll", 13);
		memcpy(image + 0x1000, (uint8_t[]){0x6A, 0xFF, 0xFF, 0x15, 0, 0, 0, 0, 0xEB, 0xF6}, 10);
		memcpy(image + 0x1004, &iat, 4);
	}
	for (i = 0; i < SITE_COUNT; ++i) {
		site_bytes(&s_sites[i], 0, image + s_sites[i].rva);
	}
	memcpy(image + s_sites[0].rva - 6, &neighbor, sizeof neighbor);
	memcpy(image + BATTLES_RVA, &battles, sizeof battles);
	for (i = 0; i < 7; ++i) {
		uint32_t victories = 100 + i, kills = 200 + i;
		memcpy(image + CHARACTER_BLOCK + i * CHARACTER_STRIDE, &victories, 4);
		memcpy(image + CHARACTER_BLOCK + i * CHARACTER_STRIDE + 4, &kills, 4);
	}
	s_relocation = saved;
}

static void write_fixture(const wchar_t *path, bool corrupt) {
	uint8_t *image = calloc(1, FIXTURE_SIZE);
	HANDLE file;
	DWORD transferred;
	assert(image);
	initialize_image(image, 0);
	if (corrupt) {
		image[s_sites[0].rva] ^= 0xFF;
	}
	file = CreateFileW(path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
	if (file == INVALID_HANDLE_VALUE) {
		fwprintf(stderr, L"Could not write %ls (Windows %lu).\n", path, (unsigned long)GetLastError());
		assert(false);
	}
	assert(WriteFile(file, image, FIXTURE_SIZE, &transferred, NULL) && transferred == FIXTURE_SIZE);
	CloseHandle(file);
	free(image);
}

static void stop_fixture(PROCESS_INFORMATION *process) {
	assert(TerminateProcess(process->hProcess, 0));
	assert(WaitForSingleObject(process->hProcess, 10000) == WAIT_OBJECT_0);
	CloseHandle(process->hThread);
	CloseHandle(process->hProcess);
}

static void start_fixture(const wchar_t *path, PROCESS_INFORMATION *process) {
	STARTUPINFOW startup;
	memset(&startup, 0, sizeof startup);
	memset(process, 0, sizeof *process);
	startup.cb = sizeof startup;
	if (!CreateProcessW(path, NULL, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &startup, process)) {
		fwprintf(stderr, L"Could not start %ls (Windows %lu).\n", path, (unsigned long)GetLastError());
		assert(false);
	}
}

static void await_status(TrainerSnapshot *snapshot, TrainerStatus status) {
	unsigned attempt;
	for (attempt = 0; attempt < 100; ++attempt) {
		trainer_refresh(snapshot);
		if (snapshot->status == status) {
			break;
		}
		Sleep(10);
	}
	if (snapshot->status != status) {
		fwprintf(stderr, L"Unexpected connection status: %ls\n", snapshot->detail);
	}
	assert(snapshot->status == status);
	if (status == TRAINER_CONNECTED) {
		assert(snapshot->available == UINT8_MAX);
	}
}

static void test_lifecycle(void) {
	wchar_t temp[MAX_PATH], directory[MAX_PATH], path[MAX_PATH];
	PROCESS_INFORMATION game, duplicate;
	WNDCLASSW cls;
	HINSTANCE instance = GetModuleHandleW(NULL);
	HWND hwnd;
	RECT bounds;
	TrainerSnapshot snapshot;
	uintptr_t base;
	uint8_t bytes[MAX_PATCH_BYTES];
	SIZE_T transferred;
	unsigned feature;
	uint32_t battles = UINT32_MAX;
	DWORD length = GetTempPathW(MAX_PATH, temp);
	assert(length > 0 && length < MAX_PATH);
	assert(GetTempFileNameW(temp, L"ffx", 0, directory));
	assert(DeleteFileW(directory) && CreateDirectoryW(directory, NULL));
	assert(wcslen(directory) + wcslen(TRAINER_PROCESS_NAME) + 2 < MAX_PATH);
	swprintf(path, MAX_PATH, L"%ls\\%ls", directory, TRAINER_PROCESS_NAME);
	write_fixture(path, false);
	trainer_refresh(&snapshot);
	assert(snapshot.status == TRAINER_WAITING && !snapshot.stats_available);
	start_fixture(path, &game);
	await_status(&snapshot, TRAINER_CONNECTED);
	assert(snapshot.stats[0] == 55 && snapshot.stats[13] == 206);
	InitDpi();
	CreateFonts();
	g_bgBrush = CreateSolidBrush(COL_BG);
	memset(&cls, 0, sizeof cls);
	cls.lpfnWndProc = WndProc;
	cls.hInstance = instance;
	cls.lpszClassName = APP_CLASS;
	assert(RegisterClassW(&cls));
	SetRect(&bounds, 0, 0, SCALE(DESIGN_W), SCALE(DESIGN_H));
	AdjustForDpi(&bounds, WINDOW_STYLE);
	hwnd = CreateWindowExW(0, APP_CLASS, APP_TITLE, WINDOW_STYLE, 0, 0,
		bounds.right - bounds.left, bounds.bottom - bounds.top, NULL, NULL, instance, NULL);
	assert(hwnd && g_snapshot.available == UINT8_MAX);
	assert(wcscmp(g_statValues[0], L"55") == 0 && wcscmp(g_statValues[13], L"206") == 0);
	assert(WriteProcessMemory(game.hProcess, (LPVOID)(s_base + BATTLES_RVA),
		&battles, sizeof battles, &transferred) && transferred == sizeof battles);
	SendMessageW(hwnd, WM_TIMER, 1, 0);
	assert(wcscmp(g_statValues[0], L"4294967295") == 0);
	for (feature = 0; feature < TRAINER_FEATURE_COUNT; ++feature) {
		SendMessageW(hwnd, WM_KEYDOWN, '1' + feature, 0);
		assert(g_snapshot.states[feature] == 1);
	}
	{
		const Run *percent = &g_rowLayout[1].runs[4];
		assert(percent->kind == KIND_NUM1);
		SendMessageW(hwnd, WM_LBUTTONDOWN, 0,
			MAKELPARAM(percent->x + percent->w / 2, g_rowLayout[1].y + 1));
		assert(g_snapshot.states[1] == 2);
	}
	base = s_base;
	SendMessageW(hwnd, WM_KEYDOWN, VK_ESCAPE, 0);
	assert(!IsWindow(hwnd) && !s_process);
	DeleteObject(g_font);
	DeleteObject(g_fontBold);
	DeleteObject(g_bgBrush);
	assert(UnregisterClassW(APP_CLASS, instance));
	assert(ReadProcessMemory(game.hProcess, (LPCVOID)(base + s_sites[0].rva),
		bytes, s_sites[0].size, &transferred));
	assert(transferred == s_sites[0].size);
	assert(memcmp(bytes, s_sites[0].bytes[1], s_sites[0].size) == 0);
	await_status(&snapshot, TRAINER_CONNECTED);
	assert(snapshot.states[0] == 1 && snapshot.states[1] == 2 && snapshot.stats[0] == UINT32_MAX);
	start_fixture(path, &duplicate);
	trainer_close();
	trainer_refresh(&snapshot);
	assert(snapshot.status == TRAINER_ERROR && snapshot.available == 0);
	stop_fixture(&duplicate);
	await_status(&snapshot, TRAINER_CONNECTED);
	stop_fixture(&game);
	trainer_refresh(&snapshot);
	assert(snapshot.status == TRAINER_WAITING && snapshot.available == 0 && !snapshot.stats_available);
	start_fixture(path, &game);
	await_status(&snapshot, TRAINER_CONNECTED);
	for (feature = 0; feature < TRAINER_FEATURE_COUNT; ++feature) {
		assert(snapshot.states[feature] == 0);
	}
	stop_fixture(&game);
	trainer_refresh(&snapshot);
	write_fixture(path, true);
	start_fixture(path, &game);
	await_status(&snapshot, TRAINER_UNSUPPORTED);
	assert(snapshot.status == TRAINER_UNSUPPORTED && snapshot.available == 0);
	assert(!trainer_action(0, TRAINER_KEY, &snapshot));
	stop_fixture(&game);
	trainer_close();
	assert(DeleteFileW(path) && RemoveDirectoryW(directory));
}

static int child(void) {
	uint8_t *image = VirtualAlloc((LPVOID)(uintptr_t)0x10000000,
		FIXTURE_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	uintptr_t base = (uintptr_t)image;
	DWORD transferred, protection;
	uint8_t stop;
	assert(image);
	initialize_image(image, (uint32_t)base - 0x400000);
	assert(VirtualProtect(image, FIXTURE_SIZE, PAGE_EXECUTE_READ, &protection));
	assert(WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), &base, sizeof base, &transferred, NULL));
	assert(transferred == sizeof base);
	assert(ReadFile(GetStdHandle(STD_INPUT_HANDLE), &stop, 1, &transferred, NULL));
	assert(VirtualFree(image, 0, MEM_RELEASE));
	return 0;
}

int main(int argc, char **argv) {
	HANDLE outputRead, outputWrite, inputRead, inputWrite;
	SECURITY_ATTRIBUTES security = {sizeof security, NULL, TRUE};
	STARTUPINFOW startup;
	PROCESS_INFORMATION process;
	wchar_t executable[MAX_PATH], command[MAX_PATH + 32], temp[MAX_PATH], fixture[MAX_PATH];
	uintptr_t base;
	DWORD transferred, exitCode, protection, pathLength;
	MEMORY_BASIC_INFORMATION info;
	uint8_t bytes[MAX_PATCH_BYTES], stop = 1;
	TrainerSnapshot snapshot;
	unsigned feature, state;

	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
	_set_error_mode(_OUT_TO_STDERR);
	if (argc == 2 && strcmp(argv[1], "--child") == 0) {
		return child();
	}
	assert(CreatePipe(&outputRead, &outputWrite, &security, 0));
	assert(CreatePipe(&inputRead, &inputWrite, &security, 0));
	assert(SetHandleInformation(outputRead, HANDLE_FLAG_INHERIT, 0));
	assert(SetHandleInformation(inputWrite, HANDLE_FLAG_INHERIT, 0));
	pathLength = GetModuleFileNameW(NULL, executable, MAX_PATH);
	assert(pathLength > 0 && pathLength < MAX_PATH);
	swprintf(command, sizeof command / sizeof *command, L"\"%ls\" --child", executable);
	memset(&startup, 0, sizeof startup);
	memset(&process, 0, sizeof process);
	startup.cb = sizeof startup;
	startup.dwFlags = STARTF_USESTDHANDLES;
	startup.hStdInput = inputRead;
	startup.hStdOutput = outputWrite;
	startup.hStdError = outputWrite;
	assert(CreateProcessW(executable, command, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &startup, &process));
	CloseHandle(inputRead);
	CloseHandle(outputWrite);
	assert(ReadFile(outputRead, &base, sizeof base, &transferred, NULL));
	assert(transferred == sizeof base);
	s_process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ |
		PROCESS_VM_WRITE | PROCESS_VM_OPERATION | SYNCHRONIZE, FALSE, process.dwProcessId);
	assert(s_process);
	s_base = base;
	s_imageSize = FIXTURE_SIZE;
	pathLength = GetTempPathW(MAX_PATH, temp);
	assert(pathLength > 0 && pathLength < MAX_PATH);
	assert(GetTempFileNameW(temp, L"ffx", 0, fixture));
	write_fixture(fixture, false);
	assert(validate_image());
	if (!validate_file(fixture)) {
		fwprintf(stderr, L"Fixture validation failed: %ls\n", s_snapshot.detail);
		assert(false);
	}
	assert(read_snapshot());
	assert(s_snapshot.stats[0] == 55 && s_snapshot.stats[11] == 203);
	assert(apply_action(7, TRAINER_KEY));
	assert(read_memory(s_sites[8].rva, bytes, s_sites[8].size));
	assert(memcmp(bytes, s_sites[8].bytes[1], s_sites[8].size) == 0);
	assert(VirtualQueryEx(s_process, (LPCVOID)(base + s_sites[8].rva), &info, sizeof info));
	assert(info.Protect == PAGE_EXECUTE_READ);
	assert(apply_action(7, TRAINER_KEY));
	assert(!read_memory((uint32_t)s_imageSize - 1, bytes, 2));
	assert(!write_memory((uint32_t)s_imageSize - 1, bytes, 2));
	{
		WORD machine = IMAGE_FILE_MACHINE_AMD64;
		uint32_t rva = 0x80 + (uint32_t)offsetof(IMAGE_NT_HEADERS32, FileHeader.Machine);
		assert(write_memory(rva, &machine, sizeof machine));
		assert(!validate_image() && s_snapshot.status == TRAINER_UNSUPPORTED);
		machine = IMAGE_FILE_MACHINE_I386;
		assert(write_memory(rva, &machine, sizeof machine));
		assert(validate_image());
		assert(validate_file(fixture));
	}
	write_fixture(fixture, true);
	assert(!validate_file(fixture));
	assert(!s_profileReady);
	write_fixture(fixture, false);
	assert(validate_file(fixture));
	/* Cross two different page protections and restore each independently. */
	assert(VirtualProtectEx(s_process, (LPVOID)(base + 0x2000), 0x1000, PAGE_READONLY, &protection));
	assert(write_memory(0x1FFE, (uint8_t[]){1, 2, 3, 4}, 4));
	assert(read_memory(0x1FFE, bytes, 4));
	assert(memcmp(bytes, (uint8_t[]){1, 2, 3, 4}, 4) == 0);
	assert(VirtualQueryEx(s_process, (LPCVOID)(base + 0x1000), &info, sizeof info));
	assert(info.Protect == PAGE_EXECUTE_READ);
	assert(VirtualQueryEx(s_process, (LPCVOID)(base + 0x2000), &info, sizeof info));
	assert(info.Protect == PAGE_READONLY);
	assert(VirtualProtectEx(s_process, (LPVOID)(base + 0x2000), 0x1000, PAGE_NOACCESS, &protection));
	assert(!read_memory(0x1FFE, bytes, 4));
	assert(!write_memory(0x1FFE, bytes, 4));
	assert(VirtualQueryEx(s_process, (LPCVOID)(base + 0x1000), &info, sizeof info));
	assert(info.Protect == PAGE_EXECUTE_READ);
	assert(VirtualProtectEx(s_process, (LPVOID)(base + 0x2000), 0x1000, protection, &protection));
	assert(VirtualProtectEx(s_process, (LPVOID)(base + 0x2000), 0x1000, PAGE_READONLY | PAGE_GUARD, &protection));
	assert(!read_memory(0x1FFE, bytes, 4));
	assert(!write_memory(0x1FFE, bytes, 4));
	assert(VirtualQueryEx(s_process, (LPCVOID)(base + 0x2000), &info, sizeof info));
	assert(info.Protect == (PAGE_READONLY | PAGE_GUARD));
	assert(VirtualProtectEx(s_process, (LPVOID)(base + 0x2000), 0x1000, protection, &protection));
	assert(VirtualProtectEx(s_process, (LPVOID)(base + 0x2000), 0x1000, PAGE_EXECUTE, &protection));
	assert(!read_memory(0x2000, bytes, 1));
	assert(VirtualProtectEx(s_process, (LPVOID)(base + 0x2000), 0x1000, protection, &protection));
	/* Exercise the public API without a test-only verification bypass. */
	assert(read_snapshot());
	for (feature = 0; feature < TRAINER_FEATURE_COUNT; ++feature) {
		for (state = 1; state <= s_features[feature].states; ++state) {
			assert(trainer_action(feature, TRAINER_KEY, &snapshot));
			assert(snapshot.states[feature] == state % s_features[feature].states);
		}
	}
	assert(write_memory(s_sites[0].rva - 1, (uint8_t[]){0xCC}, 1));
	assert(!trainer_action(0, TRAINER_KEY, &snapshot));
	assert(snapshot.available == 0);
	assert(write_memory(s_sites[0].rva - 1, (uint8_t[]){0}, 1));
	assert(read_snapshot());
	assert(WriteFile(inputWrite, &stop, 1, &transferred, NULL));
	assert(WaitForSingleObject(process.hProcess, 10000) == WAIT_OBJECT_0);
	assert(GetExitCodeProcess(process.hProcess, &exitCode) && exitCode == 0);
	assert(!read_memory(0, bytes, 1));
	assert(DeleteFileW(fixture));
	trainer_close();
	assert(!s_process && !s_snapshot.stats_available);
	assert(!read_memory(0, bytes, 1));
	CloseHandle(process.hThread);
	CloseHandle(process.hProcess);
	CloseHandle(outputRead);
	CloseHandle(inputWrite);
	test_lifecycle();
	puts("Windows child-process memory checks passed.");
	return 0;
}
