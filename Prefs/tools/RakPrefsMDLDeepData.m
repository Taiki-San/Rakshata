/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite  **
 **                                                                                         **
 *********************************************************************************************/

#include "superHeader.h"

// RELY ON PREFS FROM TABS! NEED TO BE LOADED AFTERWARD

@implementation RakMDLSize

- (id) init : (Prefs*) creator : (char*) inputData
{
	self = [super init];
	if(self != nil)
	{
		mammouth = creator;
		
		widthMDLSerie = hex2intPrefs(inputData, 1000);
		if(widthMDLSerie == -1)
			widthMDLSerie = [self getDefaultSerieWidth];
		else
			widthMDLSerie /= 10;
		
		heightMDLReaderFocus = hex2intPrefs(inputData, 1000);
		if(heightMDLReaderFocus == -1)
			heightMDLReaderFocus = [self getDefaultFocusReaderHeight];
		else
			heightMDLReaderFocus /= 10;
		
		focusMDLSize.origin.x	= hex2intPrefs(&inputData[4], 1000);
		focusMDLSize.origin.y	= hex2intPrefs(&inputData[8], 1000);
		focusMDLSize.size.height = hex2intPrefs(&inputData[12], 1000);
		focusMDLSize.size.width	= hex2intPrefs(&inputData[16], 1000);
		
		if(focusMDLSize.origin.x == -1 || focusMDLSize.origin.y == -1 || focusMDLSize.size.height == -1 || focusMDLSize.size.width == -1)
		{
			NSRect dataDefault = [self getDefaultFocusMDL];
			
			if(focusMDLSize.origin.x == -1)
				focusMDLSize.origin.x = dataDefault.origin.x;
			
			if(focusMDLSize.origin.y == -1)
				focusMDLSize.origin.y = dataDefault.origin.y;
			
			if(focusMDLSize.size.height == -1)
				focusMDLSize.size.height = dataDefault.size.height;
			
			if(focusMDLSize.size.width == -1)
				focusMDLSize.size.width = dataDefault.size.width;
		}
		else
		{
			focusMDLSize.origin.x	/= 10;
			focusMDLSize.origin.y	/= 10;
			focusMDLSize.size.width	/= 10;
			focusMDLSize.size.height /= 10;
		}
	}
	return self;
}

- (uint8_t) getFlagFocus
{
	return STATE_READER_TAB_MDL_FOCUS;
}

- (NSRect) getData:(int) mainThread : (int) stateTabsReader
{
	switch (mainThread)
	{
		case GUI_THREAD_SERIES:
			return [self getFocusSerie];
			
		case GUI_THREAD_CT:
			return [self getFocusCT];
			
		case GUI_THREAD_READER:
			return [self getFocusReader:stateTabsReader];
			
		case GUI_THREAD_MDL:
			return focusMDLSize;
	}
#ifdef DEV_VERSION
	NSLog(@"[%s]: Couldn't identify request : %8x", __PRETTY_FUNCTION__, mainThread);
#endif
	
	return focusMDLSize;	//Renvoyer quelque chose...
}

- (NSRect) getFocusSerie
{
	NSRect output = { {0, 0}, {0, 0}};
	
	output.size.width = widthMDLSerie;			//Pref modifiable

	CGFloat widthSerie;
	[Prefs getPref:PREFS_GET_TAB_SERIE_WIDTH: &widthSerie];
	
	output.origin.x = widthSerie - output.size.width;	//Fortement lié au précédent
	[Prefs getPref:PREFS_GET_SERIE_FOOTER_HEIGHT: &output.size.height];
	
	return output;
}

- (NSRect) getFocusCT
{
	NSRect output = { {0, 0}, {0, 0}};
	
	[Prefs getPref:PREFS_GET_TAB_CT_POSX: &output.origin.x];
	[Prefs getPref:PREFS_GET_CT_FOOTER_HEIGHT: &output.size.height];
	[Prefs directQuery:QUERY_CT :QUERY_GET_WIDTH :GUI_THREAD_CT :-1 :-1 :&output.size.width];
	output.origin.y = 0;
	
	return output;
}

- (NSRect) getFocusReader : (int) stateTabsReader
{
	NSRect output = { {0, 0}, {0, 0}};
	
	output.origin.x = 0;
	output.origin.y = 0;
	[Prefs getPref:PREFS_GET_TAB_READER_POSX: &output.size.width];
	
	if(stateTabsReader & [self getFlagFocus])
		output.size.height = heightMDLReaderFocus;
	else
		[Prefs getPref:PREFS_GET_READER_FOOTER_HEIGHT: &output.size.height];
	
	return output;
}

- (NSRect) getDefaultFocusMDL
{
	NSRect output = { {0, 0}, {0, 0}};
	
	output.origin.x = TAB_MDL_POSX;
	output.origin.y = TAB_MDL_POSY;
	output.size.width = TAB_MDL_WIDTH;
	output.size.height = TAB_MDL_HEIGHT;
	
	return output;
}

- (CGFloat) getDefaultSerieWidth
{
	return TAB_SERIE_MDL_WIDTH;
}

- (CGFloat) getDefaultFocusReaderHeight
{
	return 50;
}

@end
