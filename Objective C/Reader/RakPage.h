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

#include "lecteur.h"

@interface RakPage : NSScrollView
{
	NSData *prevPage;
	NSData *pageData;
	NSImage* page;
	NSData *nextPage;
	
	NSImageView * pageView;
	
	NSRect frameReader;
	NSRect selfFrame;
	
	BOOL pageTooLarge;
	BOOL pageTooHigh;
	BOOL areSlidersHidden;
	
	/*context data*/
@private
	bool cacheBeingBuilt;
	bool actionBeingProcessessed;
	
	MANGAS_DATA project;
	DATA_LECTURE data;
	
	int currentElem;
	int posElemInStructure;
	bool isTome;
	
	char texteTrad[SIZE_TRAD_ID_21][TRAD_LENGTH];
}

- (id) init : (Reader*)superView : (MANGAS_DATA) dataRequest : (int) elemRequest : (BOOL) isTomeRequest;

- (void) initialPositionning : (BOOL) canIHazSuperview : (NSRect) frameWindow;

- (void) failure;

- (void) nextPage;
- (void) prevPage;
- (void) moveSliderX : (int) move;
- (void) moveSliderY : (int) move;

- (BOOL) initialLoading : (MANGAS_DATA) dataRequest : (int) elemRequest : (BOOL) isTomeRequest;
- (void) buildCache;
- (void) changePage : (byte) switchType;
- (BOOL) craftPageAndSetupEnv : (Reader *) superView : (byte) switchType;
- (void) addPageToView;

- (void) flushCache;
- (void) getTheFuckOut;

@end
