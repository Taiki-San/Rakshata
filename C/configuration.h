/*********************************************************************************************
**	__________         __           .__            __                 ________   _______   	**
**	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
**	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
**	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
**	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
**	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
**																							**
**		Source code and assets are property of Taiki, distribution is stricly forbidden		**
**																							**
*********************************************************************************************/

//#define MAINTAIN_SUPER_LEGACY_COMPATIBILITY
//#define INSTALLING_CONSIDERED_AS_INSTALLED
//#define FLUSH_PREFS_PROPERLY

//DB configuration
#define NUMBER_FAVORITE_INSTALL_IF_NONE_THERE 5
//#define EXPOSE_DECIMAL_VOLUMES

#ifndef DEV_VERSION
	#define PAID_CONTENT_ONLY_FOR_PAID_REPO
#else
//	#define VERBOSE_DB_MANAGEMENT
//	#define VERBOSE_REQUEST
#endif

//UI configuration
#define SELECT_DOWNLOAD_PROJECT
#define LARGE_FONT_FOR_PLACEHOLDERS
//#define SEVERAL_VIEWS	//In series
//#define CT_LIST_PREFER_LESS_COLUMN
#define LEAVE_DISTRACTION_FREE_AT_END

//#define FLUSH_UNUSED_REPO

#ifdef DEV_VERSION
//#define HIDE_EVERYTHING
#define RESUME_READING
#endif