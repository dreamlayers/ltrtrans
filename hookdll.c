// hookdll.c : Defines the entry point for the DLL application.
//

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>

#pragma data_seg("Shared")		//these variables will be shared among all processes to which this dll is linked
HHOOK hkKey = NULL;
HINSTANCE hInstHookDll=NULL;
#pragma data_seg()

#pragma comment(linker,"/section:Shared,rws")

// https://msdn.microsoft.com/en-us/library/windows/desktop/ms686958(v=vs.85).aspx
#define SHMEMSIZE (sizeof(WPARAM) * 256)

static WPARAM *ltrmap = NULL;      // pointer to shared memory
static HANDLE hMapObject = NULL;  // handle to file mapping

static void init_shmem(void)
{
	BOOL fInit;

	// Create a named file mapping object
	hMapObject = CreateFileMapping(
		INVALID_HANDLE_VALUE,   // use paging file
		NULL,                   // default security attributes
		PAGE_READWRITE,         // read/write access
		0,                      // size: high 32-bits
		SHMEMSIZE,              // size: low 32-bits
		TEXT("ltrtransmap"));   // name of map object
	if (hMapObject == NULL) {
		ltrmap = NULL;
		return;
	}

	// The first process to attach initializes memory
	fInit = (GetLastError() != ERROR_ALREADY_EXISTS);

	// Get a pointer to the file-mapped shared memory
	ltrmap = (WPARAM *)MapViewOfFile(
		hMapObject,     // object to map view of
		FILE_MAP_WRITE, // read/write access
		0,              // high offset:  map from
		0,              // low offset:   beginning
		0);             // default: map entire file
	if (ltrmap == NULL)
		return;

	// Initialize memory if this is the first process
	if (fInit) {
		int i;
		for (i = 0; i < SHMEMSIZE/sizeof(*ltrmap); i++)
			ltrmap[i] = i;
	}
}

void free_shmem(void)
{
	// Unmap shared memory from the process's address space
	if (ltrmap != NULL) {
		UnmapViewOfFile(ltrmap);
		ltrmap = NULL;
	}
	// Close the process's handle to the file-mapping object
	if (hMapObject != NULL) {
		CloseHandle(hMapObject);
		hMapObject = NULL;
	}
}

BOOL APIENTRY DllMain( HANDLE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch(ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		hInstHookDll = (HINSTANCE)hModule;		//we initialize our variable with the value that is passed to us
		init_shmem();
		break;
	case DLL_PROCESS_DETACH:
		free_shmem();
		break;
	}
    return TRUE;
}

LRESULT CALLBACK procCharMsg(int nCode,WPARAM wParam, LPARAM lParam)		//this is the hook procedure
{
	MSG *msg;																//a pointer to hold the MSG structure that is passed as lParam
	unsigned int charCode;													//to hold the character passed in the MSG structure's wParam
	if(nCode >=0 && nCode == HC_ACTION)										//if nCode is less than 0 or nCode is not HC_ACTION we will call CallNextHookEx
	{
		msg=(MSG *)lParam;													//lParam contains pointer to MSG structure.
		if(msg->message==WM_CHAR)											//we handle only WM_CHAR messages
		{
			charCode = msg->wParam;											//For WM_CHAR message, wParam is the character code of the key pressed
			if(charCode < SHMEMSIZE/sizeof(*ltrmap) && ltrmap != NULL)
			{
				msg->wParam=ltrmap[charCode];								//Translate code using table
			}
		}
	}
	return CallNextHookEx(hkKey,nCode,wParam,lParam);						//passing this message to target application
}

//set the hook
void __stdcall SetHook()
{
	if(hkKey == NULL)
		// Unicode functionr equired to pass Unicode characters.
		// Otherwise they would be masked to 8 bits.
		hkKey = SetWindowsHookExW(WH_GETMESSAGE,procCharMsg,hInstHookDll,0);
}

//remove the hook
void __stdcall RemoveHook()
{
	if(hkKey !=NULL)
		UnhookWindowsHookEx(hkKey);
	hkKey = NULL;
}

//set character map
BOOL _stdcall SetCharMap(WPARAM *newmap)
{
	if (ltrmap == NULL) {
		return FALSE;
	} else {
		CopyMemory(ltrmap, newmap, SHMEMSIZE);
		return TRUE;
	}
}
