#include "Platform.h"
#include "SDL_platform.h"

char PathBuffer[2048];

#ifdef __WIN32__

#include <stdio.h>
#include <string.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

char *GetSavePath()
{
	GetModuleFileName(NULL, PathBuffer, 2048);

	char *slashpointer = PathBuffer + strlen(PathBuffer);
	while(slashpointer > PathBuffer && *slashpointer != '\\')
		slashpointer--;
	slashpointer[1] = '\0';
	return PathBuffer;
}

#else

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char *GetSavePath()
{	
	strcpy(PathBuffer, getenv("HOME"));
	
	if(PathBuffer[strlen(PathBuffer)-1] == '/')
		sprintf(&PathBuffer[strlen(PathBuffer)], ".");
	else
		sprintf(&PathBuffer[strlen(PathBuffer)], "/.");

	return PathBuffer;
}

#endif
