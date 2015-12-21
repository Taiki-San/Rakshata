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

enum
{
	CELL_RADIUS = 4,
	TEXT_SPACE_HEIGHT = 20
};

@interface RakPrefsSelectionButtonCell : RakButtonCell

- (BOOL) wantBackground;

@end

@implementation RakPrefsSelectionButton

+ (instancetype) allocImageWithoutBackground:(NSString *)imageName :(id)target :(SEL)selectorToCall
{
	RakPrefsSelectionButton * button = [super allocImageWithoutBackground:imageName :target :selectorToCall];
	
	if(button != nil)
		[button setFrameSize:[button intrinsicContentSize]];
	
	return button;
}

+ (Class)cellClass
{
	return [RakPrefsSelectionButtonCell class];
}

- (NSSize) intrinsicContentSize
{
	NSSize size = [super intrinsicContentSize];
	
	size.height += TEXT_SPACE_HEIGHT;
	size.width += 10;
	
	return size;
}

- (void) mouseDown:(NSEvent *)theEvent
{
	if(![self.cell wantBackground])
		[super mouseDown:theEvent];
}

@end

@implementation RakPrefsSelectionButtonCell

- (void) drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
	if(self.image == clicked && clicked != nil)
	{
		NSRect patchedFrame = cellFrame;
		
		patchedFrame.size.height += CELL_RADIUS * 2;
		
		[[self activeBackground] setFill];
		[[NSBezierPath bezierPathWithRoundedRect:patchedFrame xRadius:CELL_RADIUS yRadius:CELL_RADIUS] fill];
	}
	
	cellFrame.origin.y += 5;
	cellFrame.size.height -= TEXT_SPACE_HEIGHT;
	[self drawImage:self.image withFrame:cellFrame inView:controlView];
	
	cellFrame.origin.y = cellFrame.size.height;
	cellFrame.size.height = TEXT_SPACE_HEIGHT;
	[self drawTitle:self.attributedTitle withFrame:[self titleRectForBounds:cellFrame] inView:controlView];
}

- (RakColor *) activeBackground
{
	return [Prefs getSystemColor:COLOR_PREFS_BUTTON_FOCUS];
}

- (void) highlight:(BOOL)flag withFrame:(NSRect)cellFrame inView:(NSView*)controlView
{
	if(!flag || self.image != clicked)
		[super highlight:flag withFrame:cellFrame inView:controlView];
}

#pragma mark - Interface

- (BOOL) wantBackground
{
	return clicked != nil && self.image == clicked;
}

@end
