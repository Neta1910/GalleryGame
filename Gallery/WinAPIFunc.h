#pragma once
#include <iostream>
#include <Windows.h>
#include "Constants.h"
#include "Picture.h"
#include <regex>


class WinAPIFunc
{
public:
	WinAPIFunc() = default;
	static void openInApp(const PhotoViewApp app, const Picture picture);
	static std::string copyPicture(const Picture picture);
private:
	static bool FileExists(const LPCTSTR path);
};
