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
	superviewSize.size.height = CT_READERMODE_WIDTH_PROJECT_NAME;
	
	return superviewSize;
}

@end

@implementation RakTableRowView

- (void) drawBackgroundInRect:(NSRect)dirtyRect
{
	
}

@end

@implementation RakTreeView

- (id)makeViewWithIdentifier:(NSString *)identifier owner:(id)owner
{
	id view = [super makeViewWithIdentifier:identifier owner:owner];
	
	if ([identifier isEqualToString:@"NSOutlineViewDisclosureButtonKey"])	//NSOutlineViewDisclosureButtonKey isn't exported before 10.9
	{
		uint themeID = [Prefs getCurrentTheme:nil];
		[(NSButton *)view setImage:[RakResPath craftResNameFromContext:@"TD->" :NO :YES :themeID]];
		[(NSButton *)view setAlternateImage:[RakResPath craftResNameFromContext:@"TD-v" :NO :YES :themeID]];
	}
	
	return view;
}

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

- (void) setDefaultFrame : (NSRect) frame
{
	_defaultFrame = frame;
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