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

@implementation RakSRHeaderText

- (NSRect) getMenuFrame : (NSRect) superviewSize
{
	superviewSize.size.height = MENU_TEXT_WIDTH;
	
	return superviewSize;
}

@end

@implementation RakTableRowView

- (void) drawBackgroundInRect:(NSRect)dirtyRect
{
	
}

- (void) setForcedWidth:(CGFloat)forcedWidth
{
	haveForcedWidth = YES;
	_forcedWidth = forcedWidth;
	
	NSSize size = _frame.size;
	if(size.width != _forcedWidth)
	{
		[self setFrameSize:size];
	}
}

- (void) setFrameSize:(NSSize)newSize
{
	if(haveForcedWidth)
		newSize.width = _forcedWidth;
	
	[super setFrameSize:newSize];
}

@end

@implementation RakSRSubMenu

- (CGFloat) getFontSize
{
	return [NSFont systemFontSize];
}

- (CGFloat) getTextHeight
{
	return 21;
}

- (NSColor *) getBackgroundColor
{
	return nil;
}

@end
