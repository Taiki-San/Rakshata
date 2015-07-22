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
		self.wantsLayer = YES;
		self.layer.cornerRadius = 5;
		self.layer.backgroundColor = [Prefs getSystemColor:COLOR_BACKGROUND_DROP_AREA :self].CGColor;

		[self registerForDraggedTypes:[NSImage imagePasteboardTypes]];

		RakText * content = [[RakText alloc] initWithText:string :[self textColor]];
		if(content != nil)
		{
			content.alignment = NSTextAlignmentCenter;

			[content setFrameOrigin : NSCenterPoint(_bounds, content.bounds)];
			[self addSubview:content];
		}
	}

	return self;
}

- (void) dealloc
{
	[Prefs deRegisterForChanges:self];
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

- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender
{
	//set the image using the best representation we can get from the pasteboard

	if([NSImage canInitWithPasteboard: [sender draggingPasteboard]])
	{
		NSImage *newImage = [[NSImage alloc] initWithPasteboard: [sender draggingPasteboard]];
		[self setImage:newImage];
	}

	return YES;
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
