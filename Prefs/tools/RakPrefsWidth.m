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

@implementation RakWidthSeries

- (uint8_t) getDefaultFocusSerie
{
	return TAB_SERIE_ACTIVE;
}

- (uint8_t) getDefaultFocusCT
{
	return TAB_SERIE_INACTIVE_CT;
}

- (uint8_t) getDefaultFocusReader
{
	return TAB_SERIE_INACTIVE_LECTEUR;
}

- (uint8_t) getDefaultFocusReaderDFMode
{
	return TAB_SERIE_INACTIVE_DISTRACTION_FREE;
}

- (uint8_t) getDefaultFocusMDLInSerie
{
	return [self getDefaultFocusSerie];
}

- (uint8_t) getDefaultFocusMDLInCT
{
	return [self getDefaultFocusCT];
}

- (uint8_t) getDefaultFocusMDLInReader
{
	return [self getDefaultFocusReader];
}

@end

@implementation RakWidthCT

- (uint8_t) getDefaultFocusSerie
{
	return TAB_CT_INACTIVE_SERIE;
}

- (uint8_t) getDefaultFocusCT
{
	return TAB_CT_ACTIVE;
}

- (uint8_t) getDefaultFocusReader
{
	return TAB_CT_INACTIVE_LECTEUR;
}

- (uint8_t) getDefaultFocusReaderDFMode
{
	return TAB_CT_INACTIVE_DISTRACTION_FREE;
}

- (uint8_t) getDefaultFocusMDLInSerie
{
	return [self getDefaultFocusSerie];
}

- (uint8_t) getDefaultFocusMDLInCT
{
	return [self getDefaultFocusCT];
}

- (uint8_t) getDefaultFocusMDLInReader
{
	return [self getDefaultFocusReader];
}

@end

@implementation RakWidthReader

- (uint8_t) getDefaultFocusSerie
{
	return TAB_READER_INACTIVE_SERIE;
}

- (uint8_t) getDefaultFocusCT
{
	return TAB_READER_INACTIVE_CT;
}

- (uint8_t) getDefaultFocusReader
{
	return TAB_READER_ACTIVE;		//All Collapsed
}

- (uint8_t) getDefaultFocusReaderDFMode
{
	return TAB_READER_ACTIVE_DISTRACTION_FREE;
}

- (uint8_t) getDefaultFocusMDLInSerie
{
	return [self getDefaultFocusSerie];
}

- (uint8_t) getDefaultFocusMDLInCT
{
	return [self getDefaultFocusCT];
}

- (uint8_t) getDefaultFocusMDLInReader
{
	return [self getDefaultFocusReaderOneCollapsed];
}

- (uint8_t) getDefaultFocusReaderOneCollapsed
{
	return TAB_READER_ACTIVE_PARTIAL;
}

@end