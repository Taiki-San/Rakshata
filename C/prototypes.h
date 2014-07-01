/*********************************************************************************************
**	__________         __           .__            __                 ________   _______   	**
**	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
**	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
**	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
**	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
**	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
**                                                                                          **
**		Source code and assets are property of Taiki, distribution is stricly forbidden		**
**                                                                                          **
*********************************************************************************************/

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include <dirent.h>

#ifdef _WIN32
	#ifdef __MINGW32__
		#include <io.h>
	#else
        #define getcwd(a, b) _getcwd(a, b)
        #define mkdir(a) _mkdir(a)
        #define chdir(a) _chdir(a)
        #define snprintf(a, b, ...) _snprintf_s(a, b, _TRUNCATE, ##__VA_ARGS__)
	#endif

    #include <windows.h>

    #define THREAD_TYPE HANDLE
    #define MUTEX_VAR HANDLE
    #define MUTEX_LOCK(a) for(; WaitForSingleObject(a, 50) == WAIT_TIMEOUT; usleep(getRandom() & 0x80))
    #define MUTEX_UNLOCK(a) ReleaseSemaphore (a, 1, NULL)
    #define MUTEX_DESTROY(a) CloseHandle(a)
#else
    #include <pthread.h>
    #include <utime.h>
    #include <unistd.h>

	#include <sys/types.h>
	#include <sys/file.h>
	#include <sys/wait.h>

    #define THREAD_TYPE pthread_t
    #define MUTEX_VAR pthread_mutex_t
    #define MUTEX_LOCK(a) pthread_mutex_lock(&a)
    #define MUTEX_UNLOCK(a) pthread_mutex_unlock(&a)
    #define MUTEX_DESTROY(a) pthread_mutex_destroy(&a)

    #ifdef __APPLE__
        #define SDL_MapRGB(a, b, c, d) SDL_Swap32(SDL_MapRGB(a, b, c, d))
	#endif
#endif

