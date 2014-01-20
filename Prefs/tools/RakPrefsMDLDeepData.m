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

@implementation RakPrefsMDLDeepData

- (void) setNumberElem
{
	numberElem = 14;
}

- (void) initJumpTable : (SEL *) jumpTable
{
	jumpTable[0] = @selector(getDefaultWidthSerie);
	jumpTable[1] = @selector(getDefaultWidthCT);
	jumpTable[2] = @selector(getDefaultWidthReader);
	jumpTable[3] = @selector(getDefaultWidthFull);
	jumpTable[4] = @selector(getDefaultHeightSerie);
	jumpTable[5] = @selector(getDefaultHeightCT);
	jumpTable[6] = @selector(getDefaultHeightReader);
	jumpTable[7] = @selector(getDefaultHeightFull);
	jumpTable[8] = @selector(getDefaultPosXSerie);
	jumpTable[9] = @selector(getDefaultPosXCT);
	jumpTable[10] = @selector(getDefaultPosXReader);
	jumpTable[11] = @selector(getDefaultPosXFull);
	jumpTable[12] = @selector(getDefaultPosY);
	jumpTable[13] = @selector(getDefaultPosYFull);
}

- (CGFloat) getData: (int) mainThread : (uint8_t) request
{
	return [self getAtIndex: [self getIndexFromInput:mainThread :request]];
}

- (CGFloat) getAtIndex: (uint8_t) index
{
	switch(index)
	{
		case 0:
			return widthMDLSerie;
			
		case 1:
			return widthMDLCT;
			
		case 2:
			return widthMDLReader;
			
		case 3:
			return widthMDLFull;
			
		case 4:
			return heightMDLSerie;
			
		case 5:
			return heightMDLCT;
			
		case 6:
			return heightMDLReader;
			
		case 7:
			return heightMDLFull;
			
		case 8:
			return posXMDLSerie;
			
		case 9:
			return posXMDLCT;
			
		case 10:
			return posXMDLReader;
			
		case 11:
			return posXMDLFull;
			
		case 12:
			return posYMDL;
			
		case 13:
			return posYMDLFull;
			
		default:
		{
#ifdef DEV_VERSION
			NSLog(@"%s : Couldn't identify the index", __PRETTY_FUNCTION__);
#endif
		}
	}
	return 0xff;
}

- (void) setAtIndex: (uint8_t) index : (CGFloat) data
{
	switch(index)
	{
		case 0:
		{
			widthMDLSerie = data;
			break;
		}
		case 1:
		{
			widthMDLCT = data;
			break;
		}
		case 2:
		{
			widthMDLReader = data;
			break;
		}
		case 3:
		{
			widthMDLFull = data;
			break;
		}
		case 4:
		{
			heightMDLSerie = data;
			break;
		}
		case 5:
		{
			heightMDLCT = data;
			break;
		}
		case 6:
		{
			heightMDLReader = data;
			break;
		}
		case 7:
		{
			heightMDLFull = data;
			break;
		}
		case 8:
		{
			posXMDLSerie = data;
			break;
		}
		case 9:
		{
			posXMDLCT = data;
			break;
		}
		case 10:
		{
			posXMDLReader = data;
			break;
		}
		case 11:
		{
			posXMDLFull = data;
			break;
		}
		case 12:
		{
			posYMDL = data;
			break;
		}
		case 13:
		{
			posYMDLFull = data;
			break;
		}
		default:
		{
#ifdef DEV_VERSION
			NSLog(@"[%s] : Couldn't identify the index: %d", __PRETTY_FUNCTION__, index);
#endif
		}
	}
}

- (uint8_t) getIndexFromInput: (int) mainThread : (int) request
{
	uint8_t ret_value = 0xff;
	
	if(request == QUERY_GET_POSY)		//Moins de choix pour la pos Y (généralement collé en bas
		ret_value = (mainThread & GUI_THREAD_MDL) ? 1 : 0;
	else if(mainThread & GUI_THREAD_SERIES)
		ret_value = 0;
	else if(mainThread & GUI_THREAD_CT)
		ret_value = 1;
	else if(mainThread & GUI_THREAD_READER)
		ret_value = 2;
	else if(mainThread & GUI_THREAD_MDL)
		ret_value = 3;
	else
#ifdef DEV_VERSION
		NSLog(@"[%s]: couldn't identify request", __PRETTY_FUNCTION__);
#endif
		
	return ret_value + request * 4;
}

//Defaults

- (CGFloat) getDefaultWidthSerie
{
	return TAB_SERIE_MDL_WIDTH;
}

- (CGFloat) getDefaultWidthCT
{
	CGFloat output;
	[Prefs directQuery:QUERY_CT :QUERY_GET_WIDTH :GUI_THREAD_CT :-1 :-1 :&output];
	return output;
}

- (CGFloat) getDefaultWidthReader
{
	CGFloat output;
	[Prefs directQuery:QUERY_READER :QUERY_GET_WIDTH :GUI_THREAD_READER :-1 :-1 :&output];
	return 100 - output;
}

- (CGFloat) getDefaultWidthFull
{
	return TAB_MDL_WIDTH;
}

- (CGFloat) getDefaultHeightSerie
{
	int8_t output;
	[Prefs getPref:PREFS_GET_SERIE_FOOTER_HEIGHT: &output];
	return output;
}

- (CGFloat) getDefaultHeightCT
{
	int8_t output;
	[Prefs getPref:PREFS_GET_CT_FOOTER_HEIGHT: &output];
	return output;
}

- (CGFloat) getDefaultHeightReader
{
	CGFloat output;
	[Prefs getPref:PREFS_GET_READER_FOOTER_HEIGHT: &output];
	return output;
}

- (CGFloat) getDefaultHeightFull
{
	return TAB_MDL_HEIGHT;
}

- (CGFloat) getDefaultPosXSerie
{
	int widthSerie;
	[Prefs getPref:PREFS_GET_TAB_SERIE_WIDTH: &widthSerie];
	return widthSerie - TAB_SERIE_MDL_WIDTH;
}

- (CGFloat) getDefaultPosXCT
{
	CGFloat output;
	[Prefs getPref:PREFS_GET_TAB_CT_POSX: &output];
	return output;
}

- (CGFloat) getDefaultPosXReader
{
	return 0;
}

- (CGFloat) getDefaultPosXFull
{
	return TAB_MDL_POSX;
}

- (CGFloat) getDefaultPosY
{
	return 0;
}

- (CGFloat) getDefaultPosYFull
{
	return TAB_MDL_POSY;
}

@end
