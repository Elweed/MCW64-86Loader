#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>

#define JAVA_URL "http://java.com/en/download/index.jsp"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "kernel32.lib")

char javaBinPath[257];
DWORD pathLen;

int StringContains(char *str, char *toCheck)
{
	int i = 0;
	int l1 = 0;
	int l2 = 0; 
	int x = 0;
	if (!str || !toCheck) { return 0;}
	l1 = strlen(str);
	l2 = strlen(toCheck);
	for (i = 0; i < l1; i++)
	{
		if (str[i] == toCheck[0])
		{
			x = 1;
			while (x < l2)
			{
				if (x+i > l1)
				{
					break;
				}
				if (str[i+x] != toCheck[x])
				{
					break;
				}
				x++;
			}
			if (x == l2) { return 1;}
		}
	}
	return 0;
}

int VerifyFileExists(char *f)
{
	FILE *pFile;
	if (!f) { return 0;}
	pFile = fopen(f, "r");
	if (!pFile)
	{
		return 0;
	}
	fclose(pFile);
	return 1;
}

int Launch(int arch, int java)
{
	PROCESS_INFORMATION pInfo;
	char stdMsg[] = "Attempting to run with java set for";
	BOOL result = FALSE;
	int opt = 0;
	DWORD err = 0;
	char *cmd[2]= {"-d64 -Xms1024M -Xmx2048M -jar p:\\tmp\\mc132\\minecraft.jar","-Xincgc -Xmx1024M -jar p:\tmp\mc132\minecraft.jar"};
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	if (arch == 64 && java == 64)
	{
		if (StringContains(javaBinPath, "(x86)") != 1)
		{
			printf("%s 64 bit architexture", stdMsg);
			opt = 0;
		}
		else
		{
			//Looks like its still stuck running in 32 bit mode
			opt = 1;
		}
	}
	else if (java == 86 )
	{
		printf("%s 86 bit architexture", stdMsg);
		opt = 1;
	}
	else if (java == 0 && (arch == 64 || arch == 86))
	{
		printf("Unable to detect java\nPlease visit %s and download the appropreite version\n", JAVA_URL);
		return 1;
	}
	else
	{
		printf("Something is fucked!\n");
		return 1;
	}
	result = CreateProcess(javaBinPath, cmd[opt], NULL, NULL, FALSE,CREATE_DEFAULT_ERROR_MODE, NULL, NULL, &si, &pInfo);
	if (!result)
	{
		err = GetLastError();
		printf("\nSomething is fucked! Error: %X\n",err);
	}
	return 0;
}

int GetJavaTypeAndPath(int arch)
{
	//Return 86 or 64 depending on which java is avialable. 0 means it isn't available. Also, set the reg key
	char buffer[20];
	HKEY key;
	char base[] = "SOFTWARE\\JavaSoft\\Java Runtime Environment";
	char regKeyPath[150];
	long tmp = 0;
	int type = 0;
	DWORD shit = 20;
	if (arch == 64)
	{
		//Not a cure-all, but it seems if its in SysWow64, then its 64 bit available
		if (VerifyFileExists("C:\\Windows\\Syswow64\\javaw.exe")) { type = 64;}	
	}
	pathLen = 257;
	ZeroMemory(buffer, 20);
	//Now lets get the folder path, since launching from Windows\SysWow64\javaw.exe doesn't allow us to run in 64-bit mode
	//We'll also try to verify if we can run in 64 bit mode by seeing if its in Program Files (x86) on a 64-bit machine
	strcpy(regKeyPath, base);
	tmp = RegOpenKeyEx(HKEY_LOCAL_MACHINE, base,0, KEY_READ | KEY_WOW64_64KEY ,&key);
	if (tmp != ERROR_SUCCESS)
		{ return 0;}
	tmp = RegGetValue(key, NULL,"CurrentVersion", RRF_RT_REG_SZ, NULL, (PVOID)&buffer, &shit);
	if (tmp != ERROR_SUCCESS)
		{ return 0;}
	strcat(regKeyPath, "\\");
	strcat(regKeyPath, buffer);
	tmp = RegOpenKeyEx(HKEY_LOCAL_MACHINE, regKeyPath,0, KEY_READ | KEY_WOW64_64KEY ,&key);
	if (tmp != ERROR_SUCCESS)
		{ return 0;}
	tmp = RegGetValue(key, NULL,"JavaHome", RRF_RT_REG_SZ, NULL, (PVOID)&javaBinPath, &pathLen);
	if (tmp != ERROR_SUCCESS)
		{ return 0;}
	//Append the exe
	strcat(javaBinPath, "\\bin\\javaw.exe");
	if (arch != 64) { arch = 86;} 
	return arch;
}

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

	if (sys.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
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
	int err = 0;
	int java = 0;
	printf("Verifying the loader is in the same directory as minecraft.jar..\n");
	if (VerifyFileExists("minecraft.jar") != 1)
	{
		printf("Unable to detect minecraft.jar. Please put the loader into the minecraft folder and re-run the loader\n");
		err = 1;
	}
	else
	{
		printf("Attempting to detect which architexture is available..\n");
		os = GetOSType();
		printf("Attempting to detect which java version is available..\n");
		java = GetJavaTypeAndPath(os);
		printf("Attempting to launch Minecraft..\n");
		err = Launch(os, java);
	}
#ifdef _DEBUG
	getchar();
#else
	if (err != 0)
	{
		//We'll break if we get an error
		printf("Please press enter to exit\n");
		getchar();
	}
#endif
	return 0;
}