/*********************************************************************************************
**  __________         __           .__            __                  ____     ________    **
**  \______   \_____  |  | __  _____|  |__ _____ _/  |______    ___  _/_   |    \_____  \   **
**   |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \   \  \/ /|   |     /  ____/   **
**   |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   / |   |    /       \   **
**   |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /   \_/  |___| /\ \_______ \  **
**          \/      \/     \/     \/     \/     \/          \/               \/         \/  **
**                                                                                          **
**    Licence propriétaire, code source confidentiel, distribution formellement interdite   **
**                                                                                          **
*********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __INTEL_COMPILER
	#include <dirent.msvc.h>
	#pragma comment(lib, "SDL.lib")
	#pragma comment(lib, "libjpeg.lib")
	#pragma comment(lib, "libpng.lib")
	#pragma comment(lib, "libSDL_image.a")
	#pragma comment(lib, "libfreetype.lib")
	#pragma comment(lib, "SDL2_ttf.lib")
	#pragma comment(lib, "libcurl.lib")
	#pragma comment(lib, "libeay32.lib")
	#pragma comment(lib, "ssleay32.lib")
	#pragma comment(lib, "Wldap32.lib")
	#pragma comment(lib, "ws2_32.lib")
	#pragma comment(lib, "winmm.lib")
	#pragma comment(lib, "Gdi32.lib")
	#pragma comment(lib, "user32.lib")
	#pragma comment(lib, "dxguid.lib")
	#pragma comment(lib, "libzip.a")
	#pragma comment(lib, "Ole32.lib")
	#pragma comment(lib, "Imm32.lib")
	#pragma comment(lib, "Uuid.lib")
	#pragma comment(lib, "Version.lib")
	#pragma comment(lib, "OleAut32.lib")
#else
	#include <dirent.h>
#endif
#include <time.h>
#include <curl/curl.h>
#include <sys/stat.h>


#ifdef _WIN32
	#ifdef __MINGW32__
		#include <dir.h>
	#else
		#ifdef __INTEL_COMPILER
			#define getcwd(a, b) _getcwd(a, b)
			#define mkdir(a) _mkdir(a)
			#define chdir(a) _chdir(a)
			#define snprintf(a, b, ...) _snprintf_s(a, b, _TRUNCATE, ##__VA_ARGS__)
		#endif
	#endif
    #include <windows.h>
    #define bool BOOL
    #define true TRUE
    #define false FALSE
#else
    #include <pthread.h>
    #include <utime.h>

	#include <sys/types.h>
	#include <sys/file.h>
	#include <sys/wait.h>
#endif

#ifndef __APPLE__
	#include <SDL.h>
	#include <SDL_sysrender.h> //Header précis pour la structure SDL_Texture, permet de la dereferencer
	#include <SDL_sysvideo.h> //Header précis pour la structure SDL_Windows, permet de la dereferencer
    #include <SDL_image.h>
    #include <SDL_ttf.h>
#else
	#include <SDL2/SDL.h>
	#include <SDL2/SDL_sysrender.h>
	#include <SDL2/SDL_sysvideo.h>
	#include <SDL2_Image/SDL_image.h>
	#include <SDL_TTF/SDL_ttf.h>
	#include <openssl/bio.h>
	#include <openssl/ossl_typ.h>
	#define SDL_MapRGB(a, b, c, d) SDL_Swap32(SDL_MapRGB(a, b, c, d))
#endif
