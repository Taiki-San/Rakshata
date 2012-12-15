/*********************************************
**	        	 Rakshata v1.1 		        **
**     Licence propriétaire, code source    **
**        confidentiel, distribution        **
**          formellement interdite          **
**********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <curl/curl.h>

#ifdef _WIN32
	#ifdef __MINGW32__
		#include <dir.h>
	#else
		#ifdef MSVC
			#define getcwd(a, b) _getcwd(a, b)
			#define mkdir(a) _mkdir(a)
			#define chdir(a) _chdir(a)
		#endif
	#endif
    #include <windows.h>
#else
    #include <pthread.h>
    #include <utime.h>

	#include <sys/stat.h>
	#include <sys/file.h>
	#include <sys/wait.h>
	#include <sys/types.h>
#endif

#ifndef __APPLE__
	#include <SDL.h>
	#include <SDL_sysrender.h> //Header précis pour la structure SDL_Texture, permet de la dereferencer
	#include <SDL_sysvideo.h> //Header précis pour la structure SDL_Windows, permet de la dereferencer
    #include <SDL_image.h>
    #include <SDL_ttf.h>
#else
	#include <SDL/SDL.h>
	#include <SDL_Image/SDL_image.h>
	#include <SDL_TTF/SDL_ttf.h>
#endif

#define SDL_RenderPresent(renderer) SDL_RenderPresent(renderer)

