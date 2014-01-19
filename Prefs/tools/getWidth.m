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
			int widthSerie;
			getMDLWidth(output, mainThread, stateTabsReader);
			[Prefs getPref:PREFS_GET_TAB_SERIE_WIDTH: &widthSerie];
			*output = widthSerie - *output;
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