/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

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
	#define quit_thread(a) ExitThread(a)

	#define MUTEX_VAR HANDLE
	#define MUTEX_CREATE(a) (a = CreateSemaphore (NULL, 1, 1, NULL))
	#define MUTEX_LOCK(a) for(; WaitForSingleObject(a, 50) == WAIT_TIMEOUT; usleep(getRandom() & 0x80))
    #define MUTEX_UNLOCK(a) ReleaseSemaphore (a, 1, NULL)
    #define MUTEX_DESTROY(a) CloseHandle(a)
#else
    #include <pthread.h>
	#include <errno.h>
    #include <utime.h>
    #include <unistd.h>

	#include <sys/stat.h>
	#include <sys/types.h>
	#include <sys/file.h>
	#include <sys/wait.h>

    #define THREAD_TYPE pthread_t
	#define quit_thread(a) pthread_exit(a)

	#define MUTEX_VAR pthread_mutex_t
	#define MUTEX_CREATE(a) pthread_mutex_init(&a, NULL)
    #define MUTEX_LOCK(a) pthread_mutex_lock(&a)
    #define MUTEX_UNLOCK(a) pthread_mutex_unlock(&a)
    #define MUTEX_DESTROY(a) pthread_mutex_destroy(&a)
#endif
