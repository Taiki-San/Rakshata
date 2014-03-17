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

#define READER_PAGE_TOP_BORDER	80
#define READER_PAGE_BORDERS_HIGH (RD_CONTROLBAR_HEIGHT + RD_CONTROLBAR_POSY + READER_PAGE_TOP_BORDER + 10)

@interface RakPage : NSScrollView
{
	NSImage *page;
	NSImageView * pageView;
	
	NSRect frameReader;
	NSRect selfFrame;
	BOOL pageTooLarge;
	BOOL pageTooHigh;
	BOOL areSlidersHidden;
}

- (id) init : (NSString*) path : (Reader*)superView;
- (void) releaseEverything;

- (void) initialPositionning : (BOOL) canIHazSuperview : (NSRect) frameWindow;

@end
