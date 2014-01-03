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

int getWidthSerie(int mainThread, int stateTabsReader)
{
	if(mainThread & GUI_THREAD_SERIES)
		return TAB_SERIE_ACTIVE;
	else if(mainThread & (GUI_THREAD_CT | GUI_THREAD_MDL))
		return TAB_SERIE_INACTIVE_CT;
	else if(mainThread & GUI_THREAD_READER)
	{
		if(stateTabsReader & STATE_READER_TAB_SERIE_COLLAPSED)
			return TAB_SERIE_INACTIVE_LECTEUR_REDUCED;
		else if(stateTabsReader & STATE_READER_TAB_DISTRACTION_FREE)
			return TAB_SERIE_INACTIVE_DISTRACTION_FREE;
		else
			return TAB_SERIE_INACTIVE_LECTEUR;
		
	}
#ifdef DEV_VERSION
	else
		NSLog(@"Couldn't identify thread: %8x", mainThread);
#endif
	return 0;
}

int getWidthCT(int mainThread, int stateTabsReader)
{
	if(mainThread & GUI_THREAD_SERIES)
		return TAB_CT_INACTIVE_SERIE;
	else if(mainThread & (GUI_THREAD_CT | GUI_THREAD_MDL))
		return TAB_CT_ACTIVE;
	else if(mainThread & GUI_THREAD_READER)
	{
		if(stateTabsReader & STATE_READER_TAB_CT_COLLAPSED)
			return TAB_CT_INACTIVE_LECTEUR_REDUCED;
		else if(stateTabsReader & STATE_READER_TAB_DISTRACTION_FREE)
			return TAB_CT_INACTIVE_DISTRACTION_FREE;
		else
			return TAB_CT_INACTIVE_LECTEUR;
		
	}
#ifdef DEV_VERSION
	else
		NSLog(@"Couldn't identify thread: %8x", mainThread);
#endif
	return 0;
}
int getWidthReader(int mainThread, int stateTabsReader)
{
	if(mainThread & GUI_THREAD_SERIES)
		return TAB_READER_INACTIVE_SERIE;
	else if(mainThread & (GUI_THREAD_CT | GUI_THREAD_MDL))
		return TAB_READER_INACTIVE_CT;
	else if(mainThread & GUI_THREAD_READER)
	{
		if(stateTabsReader == STATE_READER_NONE_COLLAPSED)
			return TAB_READER_ACTIVE;
		else if(stateTabsReader == STATE_READER_TAB_ALL_COLLAPSED)
			return TAB_READER_ACTIVE_FULL;
		else if(stateTabsReader == STATE_READER_TAB_DISTRACTION_FREE)
			return TAB_READER_ACTIVE_DISTRACTION_FREE;
		else
			return TAB_READER_ACTIVE_PARTIAL;
	}
#ifdef DEV_VERSION
	else
		NSLog(@"Couldn't identify thread: %8x", mainThread);
#endif
	return 0;
}

/**		Code MDL	**/

void getMDLWidth(int * output, int mainThread, int stateTabsReader)
{
	switch(mainThread & GUI_THREAD_MASK)
	{
		case GUI_THREAD_SERIES:
		{
			*output = TAB_SERIE_MDL_WIDTH;
			break;
		}
			
		case GUI_THREAD_CT:
		{
			[Prefs getPref:PREFS_GET_TAB_CT_WIDTH: output];
			break;
		}
			
		case GUI_THREAD_READER:
		{
			[Prefs getPref:PREFS_GET_TAB_READER_WIDTH :output];
			*output = 100 - *output;
			break;
		}
	}
}

void getMDLHeight(int * output, int mainThread, int stateTabsReader)
{
	switch(mainThread & GUI_THREAD_MASK)
	{
		case GUI_THREAD_SERIES:
		{
			[Prefs getPref:PREFS_GET_SERIE_FOOTER_HEIGHT: output];
			break;
		}
			
		case GUI_THREAD_CT:
		{
			[Prefs getPref:PREFS_GET_CT_FOOTER_HEIGHT: output];
			break;
		}
			
		case GUI_THREAD_READER:
		{
			[Prefs getPref:PREFS_GET_READER_FOOTER_HEIGHT: output];
			break;
		}
	}
}

void getMDLPosX(int * output, int mainThread, int stateTabsReader)
{
	switch(mainThread & GUI_THREAD_MASK)
	{
		case GUI_THREAD_SERIES:
		{
			getMDLWidth(output, mainThread, stateTabsReader);
			*output = getWidthSerie(mainThread, stateTabsReader) - *output;
			break;
		}
			
		case GUI_THREAD_CT:
		{
			[Prefs getPref:PREFS_GET_TAB_CT_POSX: output];
			break;
		}
			
		case GUI_THREAD_READER:
		{
			*output = 0;
			break;
		}
	}
}

void getMDLPosY(int * output, int mainThread, int stateTabsReader)
{
	*output = 0;
}