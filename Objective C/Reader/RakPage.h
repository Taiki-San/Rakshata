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
 ********************************************************************************************/

#define READER_PAGE_TOP_BORDER	78
#define READER_PAGE_BORDERS_HIGH (RD_CONTROLBAR_HEIGHT + RD_CONTROLBAR_POSY + READER_PAGE_TOP_BORDER + 10)

#include "lecteur.h"

@interface RakPage : NSScrollView
{
	NSImage *page;
	NSImageView * pageView;
	
	NSRect frameReader;
	NSRect selfFrame;
	
	BOOL pageTooLarge;
	BOOL pageTooHigh;
	BOOL areSlidersHidden;
	
	/*context data*/
@private
	MANGAS_DATA project;
	DATA_LECTURE data;
	
	int currentElem;
	int posElemInStructure;
	bool isTome;
	
	char texteTrad[SIZE_TRAD_ID_21][TRAD_LENGTH];
}

- (id) init : (Reader*)superView : (MANGAS_DATA) dataRequest : (int) elemRequest : (BOOL) isTomeRequest;
- (BOOL) craftPageAndSetupEnv : (Reader *) superView;
- (void) addPageToView;

- (void) initialPositionning : (BOOL) canIHazSuperview : (NSRect) frameWindow;

- (void) failure;

- (BOOL) initialLoading : (MANGAS_DATA) dataRequest : (int) elemRequest : (BOOL) isTomeRequest;
- (void) nextPage;

@end
