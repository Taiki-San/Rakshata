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
 ********************************************************************************************/

@implementation RakListDragTextView

- (id) initWithText:(NSRect)frame :(NSString *)text :(NSColor *)color
{
	self = [super initWithText:frame :text :color];
	
	if(self != nil)
	{
		[self createApercu];
		if(self.image == nil)
			NSLog(@"Wut?");
	}
	
	return self;
}

#pragma mark - Image generation

- (NSView *) generateRepresentation
{
	NSView * view = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 100, 100)];
	
	if(view != nil)
	{
		[view setWantsLayer:YES];
		[view.layer setBackgroundColor:[NSColor redColor].CGColor];
	}
	
	return view;
}

- (void) createApercu
{
	NSView * view = [self generateRepresentation];
	
	if(view != nil)
	{
		self.image = [[NSImage alloc] initWithData:[view dataWithPDFInsideRect:[view bounds]]];
		[view release];
	}
}

#pragma mark - Draging source code

- (NSDragOperation)draggingSession:(NSDraggingSession *)session sourceOperationMaskForDraggingContext:(NSDraggingContext)context
{
	switch (context)
	{
		case NSDraggingContextOutsideApplication:
		{
			return NSDragOperationEvery;
		}
			
		case NSDraggingContextWithinApplication:
        default:
			return NSDragOperationEvery;
	}
}

#pragma mark NSPasteboardWriting support

- (NSArray *)writableTypesForPasteboard:(NSPasteboard *)pasteboard
{
    return [NSArray arrayWithObjects:NSPasteboardTypeString, nil];
}

- (NSPasteboardWritingOptions) writingOptionsForType : (NSString *) type pasteboard : (NSPasteboard *) pasteboard
{
    return 0;
}

- (id) pasteboardPropertyListForType : (NSString *) type
{
    if ([type isEqualToString:NSPasteboardTypeString])
        return [NSString stringWithFormat:@"%d-%d-%d", project.cacheDBID, isTome, selection];

	return nil;
}

#pragma mark  NSPasteboardReading support

+ (NSArray *) readableTypesForPasteboard : (NSPasteboard *) pasteboard
{
    // We allow creation from URLs so Finder items can be dragged to us
    return [NSArray arrayWithObjects:(id)kUTTypeURL, NSPasteboardTypeString, nil];
}

+ (NSPasteboardReadingOptions)readingOptionsForType:(NSString *)type pasteboard:(NSPasteboard *)pasteboard
{
    if ([type isEqualToString:NSPasteboardTypeString] || UTTypeConformsTo((CFStringRef)type, kUTTypeURL))
		return NSPasteboardReadingAsString;
	
	return NSPasteboardReadingAsData;
}

@end
