/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **		Source code and assets are property of Taiki, distribution is stricly forbidden		**
 **                                                                                         **
 *********************************************************************************************/

@implementation RakMDLSize

//Taille du buffer: 24 chars

- (id) init : (Prefs*) creator : (char*) inputData
{
	self = [super init];
	if(self != nil)
	{
		mammouth = creator;
		[self setExpectedBufferSize];
		
		widthMDLSerie = hex2intPrefs(inputData, 1000);
		if(widthMDLSerie == -1)
			widthMDLSerie = [self getDefaultSerieWidth];
		else
			widthMDLSerie /= 10;
		
		heightMDLReaderFocus = hex2intPrefs(&inputData[4], 1000);
		if(heightMDLReaderFocus == -1)
			heightMDLReaderFocus = [self getDefaultFocusReaderHeight];
		else
			heightMDLReaderFocus /= 10;
	}
	return self;
}

- (void) setExpectedBufferSize
{
	sizeInputBuffer = 4 + 4;
}

- (int) getExpectedBufferSize
{
	return sizeInputBuffer;
}

+ (int) getExpectedBufferSizeVirtual
{
	return 4 + 4 + 4 * 4;
}

//Save state when quit

- (void) dumpData : (char *) output : (uint) length
{
	if(length < sizeInputBuffer)	//Taille du buffer
	{
#ifdef DEV_VERSION
		NSLog(@"[%s]: Not enough room to same prefs: %d < %d", __PRETTY_FUNCTION__, length, sizeInputBuffer);
#endif
		return;
	}
	
	if(output == NULL)
		return;
		
	snprintf(output, 9, "%04x%04x", (uint) floor(widthMDLSerie * 10 + 0.5), (uint) floor(heightMDLReaderFocus * 10 + 0.5));
	
	//AAANNNND, We're done :D
}

//Getters

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
	}
#ifdef DEV_VERSION
	NSLog(@"[%s]: Couldn't identify request : %8x", __PRETTY_FUNCTION__, mainThread);
#endif
	
	return NSZeroRect;
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
	[Prefs directQuery:QUERY_CT :QUERY_GET_WIDTH :GUI_THREAD_CT :-1 :&output.size.width];
	output.origin.y = 0;
	
	return output;
}

- (NSRect) getFocusReader : (int) stateTabsReader
{
	NSRect output = { {0, 0}, {0, 0}};
	
	output.origin.x = 0;
	output.origin.y = 0;
	output.size.width = TAB_MDL_WIDTH_READER;
	
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
