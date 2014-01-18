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

@implementation RakPosXSeries

- (uint8_t) getFlagFocus
{
	return STATE_READER_TAB_SERIE_FOCUS;
}

- (uint8_t) getDefaultFocusSerie
{
	return 0;
}

- (uint8_t) getDefaultFocusCT
{
	return 0;
}

- (uint8_t) getDefaultFocusReaderOneCollapsed
{
	return 0;
}

- (uint8_t) getDefaultFocusReader
{
	return [self getDefaultFocusReaderOneCollapsed];
}

- (uint8_t) getDefaultFocusReaderAllCollapsed
{
	return 0;
}

- (uint8_t) getDefaultFocusReaderMainTab
{
	return 0;
}

- (uint8_t) getDefaultFocusReaderDFMode
{
	return 0;
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

@implementation RakPosXCT

- (uint8_t) getFlagFocus
{
	return STATE_READER_TAB_CT_FOCUS;
}

- (uint8_t) getDefaultFocusSerie
{
	return TAB_SERIE_ACTIVE;	//Largeur du tab série
}

- (uint8_t) getDefaultFocusCT
{
	return TAB_SERIE_INACTIVE_CT;
}

- (uint8_t) getDefaultFocusReaderOneCollapsed
{
	return TAB_SERIE_INACTIVE_LECTEUR;
}

- (uint8_t) getDefaultFocusReader
{
	return [self getDefaultFocusReaderMainTab];
}

- (uint8_t) getDefaultFocusReaderAllCollapsed
{
	return TAB_SERIE_INACTIVE_LECTEUR_REDUCED;
}

- (uint8_t) getDefaultFocusReaderMainTab
{
	return TAB_SERIE_INACTIVE_LECTEUR_REDUCED;
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

@implementation RakPosXReader

- (uint8_t) getFlagFocus
{
	return STATE_READER_TAB_ALL_COLLAPSED;
}

- (uint8_t) getDefaultFocusSerie
{
	return TAB_SERIE_ACTIVE+TAB_CT_INACTIVE_SERIE;	//Largeur du tab série
}

- (uint8_t) getDefaultFocusCT
{
	return TAB_SERIE_INACTIVE_CT+TAB_CT_ACTIVE;
}

- (uint8_t) getDefaultFocusReader
{
	return [self getDefaultFocusReaderOneCollapsed];
}

- (uint8_t) getDefaultFocusReaderOneCollapsed
{
	return TAB_SERIE_INACTIVE_LECTEUR_REDUCED + TAB_CT_INACTIVE_LECTEUR;
}

- (uint8_t) getDefaultFocusReaderMainTab
{
	return [self getDefaultFocusReaderAllCollapsed];
}

- (uint8_t) getDefaultFocusReaderAllCollapsed
{
	return TAB_SERIE_INACTIVE_LECTEUR_REDUCED + TAB_CT_INACTIVE_LECTEUR_REDUCED;
}

- (uint8_t) getDefaultFocusReaderDFMode
{
	return TAB_SERIE_INACTIVE_DISTRACTION_FREE + TAB_CT_INACTIVE_DISTRACTION_FREE;
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