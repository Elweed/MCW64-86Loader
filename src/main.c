#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "kernel32.lib")

/*Get OS Arch. 86 is returned for x86 and 64 for x64*/
int GetOSType()
{
	/*Not supported in all windows versions, so we'll use loadlibrary*/
	HMODULE lib;
	FARPROC add;
	SYSTEM_INFO sys;
	BOOL wow64;
	int arch = 0;
	lib = LoadLibrary("kernel32.dll");
	add = GetProcAddress(lib, "IsWow64Process");
	if (!add)
	{
		FreeLibrary(lib);
		return 86;
	}
	add(GetCurrentProcess(), &wow64);
	if (wow64)
	{
		//Returned true
		GetNativeSystemInfo(&sys);
	}
	else
	{
		GetSystemInfo(&sys);
	}

	if (sys.dwProcessorType == PROCESSOR_ARCHITECTURE_AMD64)
	{
		arch = 64;
	}
	else 
	{
		arch = 86;
	}

	FreeLibrary(lib);
	return arch;
}

int main(int argc, char *argv[])
{
	int os = 0;
	os = GetOSType();
	if (os == 86)
	{
		printf("32 bit detected! Starting Minecraft..\n");
		system("java -Xincgc -Xmx1024M -jar \"minecraft.jar\"");
	}
	else if (os == 64)
	{
		printf("64bit detected! Starting Minecraft..\n");
		system("java -d64 -Xms1024M -Xmx2048M -jar \"minecraft.jar\"");
	}
	else 
	{
		printf("Something is fucked!\n");
	}
#ifdef _DEBUG
	getchar();
#endif
	return 0;
}