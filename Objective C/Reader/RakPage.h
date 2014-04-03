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
	bool readerMode;
	
	bool cacheBeingBuilt;
	
	MANGAS_DATA project;
	DATA_LECTURE data;
	
	int currentElem;
	int posElemInStructure;
	bool isTome;
	
	char texteTrad[SIZE_TRAD_ID_21][TRAD_LENGTH];
}

- (id) init : (Reader*)superView : (MANGAS_DATA) dataRequest : (int) elemRequest : (BOOL) isTomeRequest : (int) startPage;
- (NSString *) getContextToGTFO;

- (void) initialPositionning : (BOOL) canIHazSuperview : (NSRect) frameWindow;
- (void) leaveReaderMode;
- (void) startReaderMode;

- (void) failure;

- (void) nextPage;
- (void) prevPage;
- (void) nextChapter;
- (void) prevChapter;
- (void) moveSliderX : (int) move;
- (void) moveSliderY : (int) move;
- (void) setSliderPos : (NSPoint) newPos;

- (BOOL) initialLoading : (MANGAS_DATA) dataRequest : (int) elemRequest : (BOOL) isTomeRequest : (int) startPage;
- (void) buildCache;
- (void) changePage : (byte) switchType;
- (void) changeChapter : (bool) goToNext;
- (void) changeProject : (MANGAS_DATA) projectRequest : (int) elemRequest : (bool) isTomeRequest : (int) startPage;
- (void) updateContext;
- (BOOL) craftPageAndSetupEnv : (Reader *) superView : (byte) switchType;
- (void) addPageToView;

- (void) flushCache;
- (void) getTheFuckOut;

@end
