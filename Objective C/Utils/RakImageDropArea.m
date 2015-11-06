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
	WIDTH = 80,
	HEIGHT = 50
};

@implementation RakImageDropArea

- (instancetype) initWithContentString : (NSString *) string
{
	self = [self initWithFrame:NSMakeRect(0, 0, WIDTH, HEIGHT)];

	if(self != nil)
	{
		self.editable = YES;
		self.imageScaling = NSImageScaleProportionallyUpOrDown;

		self.wantsLayer = YES;
		self.layer.cornerRadius = 5;
		self.layer.backgroundColor = [Prefs getSystemColor:COLOR_BACKGROUND_DROP_AREA :self].CGColor;

		content = [[RakText alloc] initWithText:string :[self textColor]];
		if(content != nil)
		{
			content.alignment = NSTextAlignmentCenter;

			//El Capitan
			if(floor(NSAppKitVersionNumber) > NSAppKitVersionNumber10_10_5)
				[content setFrameSize:NSMakeSize(WIDTH, content.bounds.size.height)];
			
			[content setFrameOrigin : NSCenterPoint(_bounds, content.bounds)];
			[self addSubview:content];
		}
	}

	return self;
}

- (void) dealloc
{
	[Prefs deRegisterForThemeChanges:self];
}

- (void) rightMouseDown:(nonnull NSEvent *)theEvent
{
	self.image = nil;
}

#pragma mark - D&D management

- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender
{
	// Check if the pasteboard contains image data and source/user wants it copied
	if([NSImage canInitWithPasteboard:[sender draggingPasteboard]] && [sender draggingSourceOperationMask] & NSDragOperationCopy)
	{
		highlighted = YES;
		[self setNeedsDisplay: YES];

		return NSDragOperationCopy;
	}

	return NSDragOperationNone;
}

- (void)draggingExited:(id <NSDraggingInfo>)sender
{
	highlighted = NO;
	[self setNeedsDisplay: YES];
}

- (BOOL)prepareForDragOperation:(id <NSDraggingInfo>)sender
{
	highlighted = NO;
	[self setNeedsDisplay: YES];

	return [NSImage canInitWithPasteboard: [sender draggingPasteboard]];
}

- (void) setImage:(NSImage * _Nullable)image
{
	if(image == nil)
		[super setImage:image];

	content.hidden = image != nil;

	if(image != nil)
	{
		[super setImage:image];
		_defaultImage = NO;
	}
}

#pragma mark - Color & drawing management

- (NSColor *) textColor
{
	return [Prefs getSystemColor:COLOR_CLICKABLE_TEXT :nil];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];

	self.layer.backgroundColor = [Prefs getSystemColor:COLOR_BACKGROUND_DROP_AREA :nil].CGColor;
}

- (void) drawRect:(NSRect)dirtyRect
{
	[super drawRect:dirtyRect];

	if(highlighted)
	{
		NSBezierPath * path = [NSBezierPath bezierPathWithRoundedRect:_bounds xRadius:5 yRadius:5];
		[[Prefs getSystemColor:COLOR_ACTIVE :nil] setStroke];

		[path setLineWidth:2];
		[path stroke];
	}
}

@end
