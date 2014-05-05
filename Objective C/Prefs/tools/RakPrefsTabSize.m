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

//On ne overwrite pas toutes les méthodes liés au lecteur car les views ne changent pratiquement jamais de taille

@implementation RakSizeSeries

- (uint8_t) getFlagFocus
{
	return STATE_READER_TAB_SERIE_FOCUS;
}

- (NSRect) getDefaultFocusSerie
{
	NSRect output = { {0, 0}, {0, 0}};

	output.size.width = TAB_SERIE_ACTIVE;
	output.size.height = 100;

	return output;
}

- (NSRect) getDefaultFocusCT
{
	NSRect output = { {0, 0}, {0, 0}};
	
	output.size.width = TAB_SERIE_INACTIVE_CT;
	output.size.height = 100;
	
	return output;
}

- (NSRect) getDefaultFocusReaderOneCollapsed
{
	return [self getDefaultFocusReader];
}


- (NSRect) getDefaultFocusReader
{
	NSRect output = { {0, 0}, {0, 0}};
	
	output.origin.y = 0;
	output.size.width = TAB_SERIE_INACTIVE_LECTEUR;
	output.size.height = 100;
	
	return output;
}

- (NSRect) getDefaultFocusReaderAllCollapsed
{
	return [self getDefaultFocusReader];
}

- (NSRect) getDefaultFocusReaderMainTab
{
	return [self getDefaultFocusReader];
}

- (NSRect) getDefaultFocusReaderDFMode
{
	NSRect output = { {0, 0}, {0, 0}};
	
	output.origin.y = 0;
	output.size.width = TAB_SERIE_INACTIVE_DISTRACTION_FREE;
	output.size.height = 100;
	
	return output;
}

- (NSRect) getDefaultFocusMDLInSerie
{
	return [self getDefaultFocusSerie];
}

- (NSRect) getDefaultFocusMDLInCT
{
	return [self getDefaultFocusCT];
}

- (NSRect) getDefaultFocusMDLInReader
{
	return [self getDefaultFocusReader];
}

- (CGFloat) getDefaultFooterHeight
{
	return TAB_SERIE_FOOTER_HEIGHT;
}

@end

@implementation RakSizeCT

- (uint8_t) getFlagFocus
{
	return STATE_READER_TAB_CT_FOCUS;
}

- (NSRect) getDefaultFocusSerie
{
	NSRect output = { {0, 0}, {0, 0}};
	
	output.origin.x = TAB_SERIE_ACTIVE;
	output.size.width = TAB_CT_INACTIVE_SERIE;
	output.size.height = 100;
	
	return output;
}

- (NSRect) getDefaultFocusCT
{
	NSRect output = { {0, 0}, {0, 0}};
	
	output.origin.x = TAB_SERIE_INACTIVE_CT;
	output.size.width = TAB_CT_ACTIVE;
	output.size.height = 100;
	
	return output;
}

- (NSRect) getDefaultFocusReaderOneCollapsed
{
	NSRect output = { {0, 0}, {0, 0}};
	
	output.origin.y =  0;
	output.origin.x = TAB_SERIE_INACTIVE_LECTEUR;
	output.size.width = TAB_CT_INACTIVE_LECTEUR;	//_REDUCED displayed but to prevent too much redraw, we keep drawing a full size tab
	output.size.height = 100;
	
	return output;
}

- (NSRect) getDefaultFocusReader
{
	return [self getDefaultFocusReaderMainTab];
}

- (NSRect) getDefaultFocusReaderAllCollapsed
{
	NSRect output = { {0, 0}, {0, 0}};
	
	output.origin.y =  0;
	output.origin.x = TAB_SERIE_INACTIVE_LECTEUR_REDUCED;
	output.size.width = TAB_CT_INACTIVE_LECTEUR;
	output.size.height = 100;
	
	return output;
}

- (NSRect) getDefaultFocusReaderMainTab
{
	NSRect output = { {0, 0}, {0, 0}};
	
	output.origin.y = 0;
	output.origin.x = TAB_SERIE_INACTIVE_LECTEUR_REDUCED;
	output.size.width = TAB_CT_INACTIVE_LECTEUR;
	output.size.height = 100;
	
	return output;
}

- (NSRect) getDefaultFocusReaderDFMode
{
	NSRect output = { {0, 0}, {0, 0}};
	
	output.origin.x = TAB_SERIE_INACTIVE_DISTRACTION_FREE;
	output.size.width = TAB_CT_INACTIVE_DISTRACTION_FREE;
	output.size.height = 100;
	
	return output;
}

- (NSRect) getDefaultFocusMDLInSerie
{
	return [self getDefaultFocusSerie];
}

- (NSRect) getDefaultFocusMDLInCT
{
	return [self getDefaultFocusCT];
}

- (NSRect) getDefaultFocusMDLInReader
{
	return [self getDefaultFocusReader];
}

- (CGFloat) getDefaultFooterHeight
{
	return TAB_CT_FOOTER_HEIGHT;
}

@end

@implementation RakSizeReader

- (uint8_t) getFlagFocus
{
	return STATE_READER_TAB_ALL_COLLAPSED;
}

- (NSRect) getDefaultFocusSerie
{
	NSRect output = { {0, 0}, {0, 0}};
	
	output.origin.x = TAB_SERIE_ACTIVE+TAB_CT_INACTIVE_SERIE;
	output.size.width = TAB_READER_INACTIVE_SERIE;
	output.size.height = 100;
	
	return output;
}

- (NSRect) getDefaultFocusCT
{
	NSRect output = { {0, 0}, {0, 0}};
	
	output.origin.x = TAB_SERIE_INACTIVE_CT+TAB_CT_ACTIVE;
	output.size.width = TAB_READER_INACTIVE_CT;
	output.size.height = 100;
	
	return output;
}

- (NSRect) getDefaultFocusReaderOneCollapsed
{
	NSRect output;
	
	output.origin.x = TAB_SERIE_INACTIVE_LECTEUR_REDUCED + TAB_CT_INACTIVE_LECTEUR;
	output.origin.y = 0;
	output.size.width = TAB_READER_ACTIVE_PARTIAL;
	output.size.height = 100;
	
	return output;
}

- (NSRect) getDefaultFocusReader
{
	return [self getDefaultFocusReaderOneCollapsed];
}

- (NSRect) getDefaultFocusReaderAllCollapsed
{
	return [self getDefaultFocusReaderMainTab];
}

- (NSRect) getDefaultFocusReaderMainTab
{
	NSRect output = { {0, 0}, {0, 0}};
	
	output.origin.x = TAB_SERIE_INACTIVE_LECTEUR_REDUCED + TAB_CT_INACTIVE_LECTEUR_REDUCED;
	output.size.width = TAB_READER_ACTIVE;
	output.size.height = 100;
	
	return output;
}

- (NSRect) getDefaultFocusReaderDFMode
{
	NSRect output = { {0, 0}, {0, 0}};
	
	output.origin.x = TAB_SERIE_INACTIVE_DISTRACTION_FREE + TAB_CT_INACTIVE_DISTRACTION_FREE;
	output.size.width = TAB_READER_ACTIVE_DISTRACTION_FREE;
	output.size.height = 100;
	
	return output;
}

- (NSRect) getDefaultFocusMDLInSerie
{
	return [self getDefaultFocusSerie];
}

- (NSRect) getDefaultFocusMDLInCT
{
	return [self getDefaultFocusCT];
}

- (NSRect) getDefaultFocusMDLInReader
{
	return [self getDefaultFocusReader];
}

- (CGFloat) getDefaultFooterHeight
{
	return TAB_READER_FOOTER_HEIGHT;
}

@end
