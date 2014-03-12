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

@implementation RakReaderBottomBar

- (id)init: (BOOL) displayed
{
	
    return self;
}

- (void)drawRect:(NSRect)dirtyRect
{
	[super drawRect:dirtyRect];
	
    // Drawing code here.
}

- (CGFloat) getWidthTab
{
	CGFloat output = 0;
	[Prefs directQuery:QUERY_READER :QUERY_GET_WIDTH :GUI_THREAD_READER :-1 :-1 :&output];
	return output;
}

- (NSRect) getPosBar
{
	NSRect output = {{0, 0}, {0, 0}};
	CGFloat sizeWindow;

	return output;
}

@end
