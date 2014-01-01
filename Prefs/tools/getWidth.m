/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                          **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite   **
 **                                                                                          **
 *********************************************************************************************/

#import "superHeader.h"

uint64 getWidthSerie(int mainThread)
{
	if(mainThread & GUI_THREAD_SERIES)
		return TAB_SERIE_ACTIVE;
	else if(mainThread & (GUI_THREAD_CT | GUI_THREAD_MDL))
		return TAB_SERIE_INACTIVE_CT;
	else if(mainThread & GUI_THREAD_READER)
		return TAB_SERIE_INACTIVE_LECTEUR;
#ifdef DEV_VERSION
	else
		NSLog(@"Couldn't identify thread: %8x", mainThread);
#endif
	return 0;
}

uint64 getWidthCT(int mainThread)
{
	if(mainThread & GUI_THREAD_SERIES)
		return TAB_CT_INACTIVE_SERIE;
	else if(mainThread & (GUI_THREAD_CT | GUI_THREAD_MDL))
		return TAB_CT_ACTIVE;
	else if(mainThread & GUI_THREAD_READER)
		return TAB_CT_INACTIVE_LECTEUR;
#ifdef DEV_VERSION
	else
		NSLog(@"Couldn't identify thread: %8x", mainThread);
#endif
	return 0;
}
uint64 getWidthReader(int mainThread)
{
	if(mainThread & GUI_THREAD_SERIES)
		return TAB_READER_INACTIVE_SERIE;
	else if(mainThread & (GUI_THREAD_CT | GUI_THREAD_MDL))
		return TAB_READER_INACTIVE_CT;
	else if(mainThread & GUI_THREAD_READER)
		return TAB_READER_ACTIVE;
#ifdef DEV_VERSION
	else
		NSLog(@"Couldn't identify thread: %8x", mainThread);
#endif
	return 0;
}