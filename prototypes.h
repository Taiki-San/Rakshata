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
#include <time.h>
#include <SDL.h>
#include <SDL_sysrender.h> //Header précis pour la structure SDL_Texture, permet de la dereferencer
#include <SDL_sysvideo.h> //Header précis pour la structure SDL_Windows, permet de la dereferencer
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <curl/curl.h>
#include <sys/stat.h>

#ifdef __INTEL_COMPILER
	#include <dirent.msvc.h>
	#include <io.h>
	#define F_OK 1
	#pragma comment(lib, "SDL.lib")
	#pragma comment(lib, "libSDL_image.lib")
	#pragma comment(lib, "libfreetype.lib")
	#pragma comment(lib, "SDL2_ttf.lib")
	#pragma comment(lib, "libcurl.lib")
	#pragma comment(lib, "libeay32.lib") //SSL
	#pragma comment(lib, "ssleay32.lib") //SSL
	#pragma comment(lib, "Wldap32.lib")
	#pragma comment(lib, "ws2_32.lib")
	#pragma comment(lib, "winmm.lib")
	#pragma comment(lib, "libzip.a")
	#pragma comment(lib, "Imm32.lib")
	#pragma comment(lib, "Version.lib")
#else
	#include <dirent.h>
#endif

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

    #define MUTEX_VAR HANDLE
    #define MUTEX_LOCK for(; WaitForSingleObject(mutex, 50) == WAIT_TIMEOUT; SDL_Delay(50))
    #define MUTEX_UNLOCK ReleaseSemaphore (mutex, 1, NULL)
    #define MUTEX_LOCK_DECRYPT for(; WaitForSingleObject(mutex_decrypt, 50) == WAIT_TIMEOUT; SDL_Delay(50))
    #define MUTEX_UNLOCK_DECRYPT ReleaseSemaphore (mutex_decrypt, 1, NULL)
    #define MUTEX_DESTROY(a) CloseHandle(a);
#else
    #include <pthread.h>
    #include <utime.h>
    #include <unistd.h>

	#include <sys/types.h>
	#include <sys/file.h>
	#include <sys/wait.h>

    #define MUTEX_VAR pthread_mutex_t
    #define MUTEX_LOCK pthread_mutex_lock(&mutex)
    #define MUTEX_UNLOCK pthread_mutex_unlock(&mutex)
    #define MUTEX_LOCK_DECRYPT pthread_mutex_lock(&mutex_decrypt)
    #define MUTEX_UNLOCK_DECRYPT pthread_mutex_unlock(&mutex_decrypt)
    #define MUTEX_DESTROY(a) pthread_mutex_destroy(&a)

    #ifdef __APPLE__
        #include <openssl/bio.h>
        #include <openssl/pem.h>
        #include <openssl/x509.h>
        #include <openssl/ssl.h>
        #define SDL_MapRGB(a, b, c, d) SDL_Swap32(SDL_MapRGB(a, b, c, d))
    #endif
#endif
