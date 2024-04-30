#include "WinAPIFunc.h"

static PROCESS_INFORMATION proccessInfo;
HHOOK hook;

LRESULT CALLBACK LowLevelKeyboardProc(int code, WPARAM wParam, LPARAM lParam)
{
    if (WaitForSingleObject(proccessInfo.hProcess, 0) == WAIT_OBJECT_0)
    {
        std::cout << "Application stopped" << std::endl;
        PostQuitMessage(0);
    }

    if (code == HC_ACTION)
    {
        PKBDLLHOOKSTRUCT pkb = (PKBDLLHOOKSTRUCT)lParam;
        if (wParam == WM_KEYDOWN && pkb->vkCode == TERMINATE_KEY)
        {
            if (GetAsyncKeyState(VK_CONTROL) < 0)
            {
                DWORD proccessId = 0;
                HWND windowHandle = GetForegroundWindow();
                GetWindowThreadProcessId(windowHandle, &proccessId);
                if (proccessId == proccessInfo.dwProcessId)
                {
                    TerminateProcess(proccessInfo.hProcess, 0);
                    PostQuitMessage(0);
                }
            }
        }
    }
    return CallNextHookEx(hook, code, wParam, lParam);
}

void WinAPIFunc::openInApp(const PhotoViewApp app, const Picture picture)
{
	std::string appPath;
	STARTUPINFO appWindow = { 0 };
	switch (app) // Get chosen app path
	{
	case IRFANVIEW:
		appPath = IRFANVIEW_PATH;
		break;
	case PAINT:
		appPath = PAINT_PATH;
		break;
	}
	// Set app window settings
	appWindow.cb = sizeof(STARTUPINFO);
	appWindow.dwFlags = STARTF_USESHOWWINDOW;
	appWindow.wShowWindow = true;

	appPath.append(picture.getPath());
	CreateProcess(NULL, const_cast<char*>(appPath.c_str()), NULL, NULL, false, 0, NULL, NULL, &appWindow, &proccessInfo);

	if (WaitForSingleObject(proccessInfo.hProcess, 0) == WAIT_TIMEOUT)
	{
		hook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0);
        MSG message;
        while (GetMessage(&message, NULL, 0, 0))
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
	}
    else
    {
        std::cerr << "!!  Something went wrong  !!" << std::endl;

        WaitForSingleObject(proccessInfo.hProcess, -1);
        UnhookWindowsHookEx(hook);

        CloseHandle(proccessInfo.hThread);
        CloseHandle(proccessInfo.hProcess);
    }
}

std::string WinAPIFunc::copyPicture(const Picture picture)
{
    std::regex fileName(R"((.+?)(\w+)(\..+)$)");
    std::string duplicateFilePath = std::regex_replace(picture.getPath(), fileName, "$1CopyOf_$2$3"); // add 'Copy_Of' to the beggining of the file name

    try
    {
        BOOL exists = FileExists(duplicateFilePath.c_str());
        if (exists)
            throw std::exception("File already exists.");

        BOOL copied = CopyFile(picture.getPath().c_str(), duplicateFilePath.c_str(), FALSE);
        if (!copied)
            throw std::exception("Something went wrong.");
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return "NULL";
    }

    return duplicateFilePath;
}

bool WinAPIFunc::FileExists(const LPCTSTR path)
{
    DWORD dwAttrib = GetFileAttributes(path);

    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
        !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}
