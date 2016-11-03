// hookdll.c : Defines the entry point for the DLL application.
//

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>

#pragma data_seg("Shared")		//these variables will be shared among all processes to which this dll is linked
HHOOK hkKey = NULL;
HINSTANCE hInstHookDll=NULL;
#pragma data_seg()

#pragma comment(linker,"/section:Shared,rws")


BOOL APIENTRY DllMain( HANDLE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch(ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		hInstHookDll = (HINSTANCE)hModule;		//we initialize our variable with the value that is passed to us
		break;
	}
    return TRUE;
}

LRESULT CALLBACK procCharMsg(int nCode,WPARAM wParam, LPARAM lParam)		//this is the hook procedure
{
	MSG *msg;																//a pointer to hold the MSG structure that is passed as lParam
	char charCode;															//to hold the character passed in the MSG structure's wParam
	if(nCode >=0 && nCode == HC_ACTION)										//if nCode is less than 0 or nCode is not HC_ACTION we will call CallNextHookEx
	{
		msg=(MSG *)lParam;													//lParam contains pointer to MSG structure.
		if(msg->message==WM_CHAR)											//we handle only WM_CHAR messages
		{
			charCode = msg->wParam;											//For WM_CHAR message, wParam is the character code of the key pressed
			if(IsCharLower(charCode))										//we check if the character pressed is a small letter
			{
				charCode -=32;												//if so, make it to capital letter
				msg->wParam=(WPARAM)charCode;								//overwrite the msg structure's wparam with our new value.
			}
		}
	}
	return CallNextHookEx(hkKey,nCode,wParam,lParam);						//passing this message to target application
}

//set the hook
void __stdcall SetHook()
{
	if(hkKey == NULL)
		hkKey = SetWindowsHookEx(WH_GETMESSAGE,procCharMsg,hInstHookDll,0);
}

//remove the hook
void __stdcall RemoveHook()
{
	if(hkKey !=NULL)
		UnhookWindowsHookEx(hkKey);
	hkKey = NULL;
}
