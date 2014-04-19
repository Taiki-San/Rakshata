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

@implementation RakSRHeaderText

- (NSRect) getProjectNameSize : (NSRect) superViewSize
{
	NSRect frame = superViewSize;
	frame.size.height = CT_READERMODE_WIDTH_PROJECT_NAME;
	
	return frame;
}

- (CGFloat) getFontSize
{
	return 18;
}

@end

@implementation RakTableRowView

- (void) drawBackgroundInRect:(NSRect)dirtyRect
{
	
}

@end

@implementation RakTreeView

- (NSRect) frame
{
	if(_defaultFrame.size.height != 0)
		return _defaultFrame;
	else
		return [super frame];
}

- (void) setFrame:(NSRect)frameRect
{
	_defaultFrame = frameRect;
	[super setFrame:_defaultFrame];
}

@end

@implementation RakTableColumn

- (void) setFixedWidth : (CGFloat) fixedWidth
{
	if(_fixedWidth != fixedWidth)
		[super setWidth:fixedWidth];
	
	_fixedWidth = fixedWidth;
}

- (void) setWidth:(CGFloat)width
{
	if(!_fixedWidth)
		_fixedWidth = width;
	
	[super setWidth:_fixedWidth];
}

@end

@implementation RakSRSubMenu

- (CGFloat) getTextHeight
{
	return 21;
}

@end