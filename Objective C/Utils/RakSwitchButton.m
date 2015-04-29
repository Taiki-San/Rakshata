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

@interface RakSwitchButtonCell : NSButtonCell
{
	NSColor * borderColor;
	NSColor *backgroundMixed, *backgroundOff, *backgroundOn;
}

@end

@implementation RakSwitchButton

- (instancetype) init
{
	self = [super init];

	if(self != nil)
	{
		[self setButtonType:NSSwitchButton];
		[self setImagePosition:NSImageOnly];
		[self sizeToFit];
	}
	
	return self;
}

+ (Class) cellClass
{
	return [RakSwitchButtonCell class];
}

@end

enum
{
	BORDER = 2,
	RADIUS_LARGE = 3,
	RADIUS_INTERNAL = 2
};

@implementation RakSwitchButtonCell

- (instancetype) init
{
	self = [super init];
	
	if(self != nil)
	{
		[self initColors];
		[Prefs getCurrentTheme:self];
	}
	
	return self;
}

- (void) dealloc
{
	[Prefs deRegisterForChanges:self];
}

#pragma mark - Color management

- (void) initColors
{
	borderColor = [Prefs getSystemColor:GET_COLOR_BORDER_SWITCH_BUTTON :nil];
	backgroundMixed = [Prefs getSystemColor:GET_COLOR_BACKGROUND_SWITCH_BUTTON_MIXED :nil];
	backgroundOff = [Prefs getSystemColor:GET_COLOR_BACKGROUND_SWITCH_BUTTON_OFF :nil];
	backgroundOn = [Prefs getSystemColor:GET_COLOR_BACKGROUND_SWITCH_BUTTON_ON :nil];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		[super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	
	[self initColors];
}

#pragma mark - Drawing

- (void) drawInteriorWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
	//Border
	[borderColor setFill];
	
	cellFrame.origin.x += BORDER;
	cellFrame.origin.y += BORDER;
	cellFrame.size.height -= 2 * BORDER;
	cellFrame.size.width -= 2 * BORDER;
	
	[[NSBezierPath bezierPathWithRoundedRect:cellFrame xRadius:RADIUS_LARGE yRadius:RADIUS_LARGE] fill];

	cellFrame.origin.x += 0.5;
	cellFrame.origin.y += 0.5;
	cellFrame.size.height -= 1;
	cellFrame.size.width -= 1;

	if(self.isHighlighted)
	{
		[backgroundMixed setFill];
	}
	else
	{
		if(self.state == NSOffState)
			[backgroundOff setFill];
		else
			[backgroundOn setFill];
	}

	[[NSBezierPath bezierPathWithRoundedRect:cellFrame xRadius:RADIUS_INTERNAL yRadius:RADIUS_INTERNAL] fill];
}

@end