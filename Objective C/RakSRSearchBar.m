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

#define PLACEHOLDER @"Test - test - test - tesT"

@interface RakSRSearchBarCell : NSSearchFieldCell

@end

@implementation RakSRSearchBarCell

- (NSText*) setUpFieldEditorAttributes : (NSText*) textObj
{
	NSTextView * output = (NSTextView*) [super setUpFieldEditorAttributes:textObj];
	
	[output setInsertionPointColor:[Prefs getSystemColor:GET_COLOR_INSERTION_POINT :nil]];
	
	return output;
}

- (NSRect)drawingRectForBounds:(NSRect)theRect
{
	return [self centerCell : [super drawingRectForBounds:theRect]];
}

- (void) selectWithFrame:(NSRect)aRect inView:(NSView *)controlView editor:(NSTextView *)textObj delegate:(id)anObject start:(NSInteger)selStart length:(NSInteger)selLength
{
	if([controlView isKindOfClass:[RakSRSearchBar class]] && [[(RakSRSearchBar *) controlView stringValue] isEqualToString:@""])
	{
		[(RakSRSearchBar *) controlView updatePlaceholder:NO];
	}
	
	[super selectWithFrame:[self centerCell:aRect] inView:controlView editor:textObj delegate:anObject start:selStart length:selLength];
}

- (void) editWithFrame:(NSRect)aRect inView:(NSView *)controlView editor:(NSText *)textObj delegate:(id)anObject event:(NSEvent *)theEvent
{
	[super editWithFrame:[self centerCell:aRect] inView:controlView editor:textObj delegate:anObject event:theEvent];
}

#define OFFSET 22

- (NSRect) centerCell : (NSRect) originalRect
{
	NSSize textSize = [self cellSizeForBounds:originalRect];		// Get our ideal size for current text
	
	double heightDelta = originalRect.size.height - textSize.height;		// Center that in the proposed rect
	if (heightDelta > 0)
	{
		originalRect.size.height -= heightDelta;
		originalRect.origin.y += (heightDelta / 2);
	}
	
	originalRect.origin.x += OFFSET;
	originalRect.size.width -= 2 * OFFSET;
	
	return originalRect;
}

@end

@implementation RakSRSearchBar

- (instancetype) initWithFrame : (NSRect) frameRect
{
	self = [super initWithFrame:frameRect];
	
	if(self != nil)
	{
		[self initCell];
		
		[self setBezelStyle:NSRegularSquareBezelStyle];
		[self setBezeled: NO];

		[self setDrawsBackground:YES];
		[self setBackgroundColor:[self getBackgroundColor]];
		[self setTextColor:[self getTextColor]];

		[self updatePlaceholder:YES];
	}
	
	return self;
}

- (void) initCell
{
	NSSearchFieldCell * cell = self.cell;
	NSButtonCell * _oldButton;
	
	RakButtonCell * button = [[RakButtonCell alloc] initWithPage : @"loupe" : RB_STATE_STANDARD];
	if(button != nil)
	{
		[button setBordered:NO];
		[button setHighlightAllowed:NO];
		
		_oldButton = cell.searchButtonCell;
		button.target = _oldButton.target;
		button.action = _oldButton.action;

		[cell setSearchButtonCell:button];
	}
	
	button = [[RakButtonCell alloc] initWithPage:@"discard" :RB_STATE_STANDARD];
	if(button != nil)
	{
		[button setBordered:NO];
		[button setHighlightAllowed:NO];
		
		_oldButton = cell.cancelButtonCell;
		button.target = _oldButton.target;
		button.action = _oldButton.action;
		
		[cell setCancelButtonCell:button];
	}
}

+ (Class)cellClass
{
	return [RakSRSearchBarCell class];
}

#pragma mark - Interface

- (NSColor *) getBackgroundColor
{
	return [NSColor blackColor];
}

- (NSColor *) getPlaceholderTextColor
{
	return [[Prefs getSystemColor:GET_COLOR_CLICKABLE_TEXT :nil] colorWithAlphaComponent:0.5];
}

- (NSColor *) getTextColor
{
	return [Prefs getSystemColor:GET_COLOR_ACTIVE :nil];
}

- (void) updatePlaceholder : (BOOL) inactive
{
	_currentPlaceholderState = inactive;
	
	CGFloat offset = inactive ? -3 : 0;

	[self.cell setPlaceholderAttributedString:[[NSAttributedString alloc] initWithString:PLACEHOLDER attributes:
											   @{NSForegroundColorAttributeName : [self getPlaceholderTextColor],
												 NSBackgroundColorAttributeName : [self getBackgroundColor],
												 NSBaselineOffsetAttributeName : @(offset)
												 }]];
}

- (void) willLooseFocus
{
	[self updatePlaceholder:YES];
}

#pragma mark - View stuffs

- (void) resizeAnimation : (NSRect) frame
{
	[self.animator setFrame:frame];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if ([object class] != [Prefs class])
		return;
	
	[self initCell];
	[self.cell setBackgroundColor:[Prefs getSystemColor:GET_COLOR_BACKGROUND_TEXTFIELD:nil]];
	[self.cell setTextColor:[Prefs getSystemColor:GET_COLOR_ACTIVE :nil]];
	[self updatePlaceholder : _currentPlaceholderState];
}

@end