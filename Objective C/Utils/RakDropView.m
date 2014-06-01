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

@implementation RakDropView

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];

    if (self != nil)
	{
		self.wantsLayer = YES;
		self.layer.backgroundColor = [NSColor clearColor].CGColor;
		
		[self registerForDraggedTypes:[NSArray arrayWithObjects: NSPasteboardTypeString, nil]];
	}
    
	return self;
}

- (void) setResponder : (id) responder
{
	_responder = responder;
}

- (void) notifyResponder : (MANGAS_DATA) data : (bool) isTome : (int) element
{
	if (_responder != nil && [_responder respondsToSelector:@selector(receiveDrop:::)])
	{
		[_responder receiveDrop : data : isTome : element];
	}
}

#pragma mark - Drop support

- (NSDragOperation) draggingEntered : (id) sender
{
	if (([sender draggingSourceOperationMask] & NSDragOperationGeneric) == NSDragOperationGeneric)
		return NSDragOperationGeneric;

	return NSDragOperationNone;
	
}

- (BOOL) prepareForDragOperation : (id) sender	{	return YES;		}

- (BOOL) performDragOperation : (id) sender
{
	NSPasteboard *pasteboard = [sender draggingPasteboard];
	NSString *acceptedTypes = [pasteboard availableTypeFromArray : [NSArray arrayWithObjects: NSPasteboardTypeTIFF, NSFilenamesPboardType, nil]];
	
    if ([acceptedTypes isEqualToString:NSFilenamesPboardType])
	{
		// the pasteboard contains a list of file names
		//Take the first one
		NSArray *pasteboardContent = [pasteboard propertyListForType:@"NSFilenamesPboardType"];
		
		if(pasteboardContent == nil)
			return NO;
		
		NSString *path = [pasteboardContent objectAtIndex:0];
		
		NSLog(@"Received the drop: %@", path);
		
		return YES;
	}
	
	return NO;
}

- (void) concludeDragOperation : (id) sender
{
	[self setNeedsDisplay:YES];
}

@end
