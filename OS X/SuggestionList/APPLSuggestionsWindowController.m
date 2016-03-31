/*
 File: SuggestionsWindowController.m
 Abstract: The controller for the suggestions popup window. This class handles creating, displaying, and event tracking of the suggestion popup window.
 Version: 1.4
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
 Inc. ("Apple") in consideration of your agreement to the following
 terms, and your use, installation, modification or redistribution of
 this Apple software constitutes acceptance of these terms.  If you do
 not agree with these terms, please do not use, install, modify or
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following
 text and disclaimers in all such redistributions of the Apple Software.
 Neither the name, trademarks, service marks or logos of Apple Inc. may
 be used to endorse or promote products derived from the Apple Software
 without specific prior written permission from Apple.  Except as
 expressly stated in this notice, no other rights or licenses, express or
 implied, are granted by Apple herein, including but not limited to any
 patent rights that may be infringed by your derivative works or by other
 works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
 MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
 OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
 AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 
 Copyright (C) 2012 Apple Inc. All Rights Reserved.
 
 */

#define kTrackerKey @"whichImageView"

#define THUMBNAIL_WIDTH 24.0f

enum
{
	OFFSET = 22,
	HEIGHT = 44,
	INTERLINE = 3,
	BORDER = 5,
	IMAGE_WITDH = 34
};

@interface SuggestionsWindow : NSWindow

@property id parentElement;

@end

@interface APPLSuggestionsWindowController()

//declare the these properties in an anonymous category since they are private
@property BOOL needsLayoutUpdate;
@property (nonatomic) APPLHighlightingView * selectedView;

// private helper methods
-(void)layoutSuggestions;

@end


@implementation APPLSuggestionsWindowController

- (instancetype) init
{
	NSRect contentRect = {{0, 0}, {20, 20}};
	NSWindow *window = [[SuggestionsWindow alloc] initWithContentRect:contentRect styleMask:NSFullSizeContentViewWindowMask backing:NSBackingStoreBuffered defer:YES];
	NSView * contentView;
	BOOL hasYosemite = floor(NSAppKitVersionNumber) >= NSAppKitVersionNumber10_10;
 
	if(hasYosemite)
	{
		NSVisualEffectView * _contentView = [[NSVisualEffectView alloc] initWithFrame:contentRect];
		if(contentView != nil)
		{
			window.contentView = contentView = _contentView;
			window.titlebarAppearsTransparent = YES;
			_contentView.material = NSVisualEffectMaterialAppearanceBased;
			_contentView.blendingMode = NSVisualEffectBlendingModeBehindWindow;
			_contentView.state = NSVisualEffectStateActive;
		}
		else
		{
			hasYosemite = NO;
			contentView = window.contentView;
		}
	}
	else
		contentView = window.contentView;
	
	if(window == nil)
		return nil;
	
	self = [super initWithWindow:window];
	if (self != nil)
	{
		self.window = window;
		contentView.wantsLayer = YES;
		
		if(hasYosemite)
		{
			self.window.appearance = [NSAppearance appearanceNamed:NSAppearanceNameVibrantDark];
		}
		else
		{
			self.window.backgroundColor = [[NSColor blackColor] colorWithAlphaComponent:0.7f];
			self.window.opaque = NO;
		}
	}
	
	
	return self;
}

/* Custom selectedView property setter so that we can set the highlighted property of the old and new selected views.
 */
- (void) setSelectedView : (APPLHighlightingView *) view
{
	if (_selectedView != view)
	{
		_selectedView.highlighted = NO;
		_selectedView = view;
	}

	_selectedView.highlighted = YES;
}

/* Set selected view and send action
 */
- (void)userSetSelectedView : (APPLHighlightingView *) view
{
	self.selectedView = view;
	
	[NSApp sendAction:self.action to:self.target from:self];
}


/* Position and lay out the suggestions window, set up auto cancelling tracking, and wires up the logical relationship for accessibility.
 */
- (void)beginForTextField:(NSTextField *)parentTextField
{
	NSWindow *suggestionWindow = self.window;
	NSWindow *parentWindow = parentTextField.window;
	NSRect parentFrame = parentTextField.frame;
	NSRect frame = suggestionWindow.frame;
	
	frame.size.width = parentFrame.size.width - 2 * OFFSET;
	parentFrame.origin.x += OFFSET;
	
	if(parentTextField.isFlipped)
		parentFrame.origin.y += parentFrame.size.height - 3;
	
	// Place the suggestion window just underneath the text field and make it the same width as th text field.
	NSPoint location = [parentTextField.superview convertPoint:parentFrame.origin toView:nil];
	location = [parentWindow convertRectToScreen:(NSRect) {location, NSZeroSize}].origin;
	location.y -= 2.0f; // nudge the suggestion window down so it doesn't overlapp the parent view
	[suggestionWindow setFrame:frame display:NO];
	[suggestionWindow setFrameTopLeftPoint:location];
	[self layoutSuggestions]; // The height of the window will be adjusted in -layoutSuggestions.
	
	// add the suggestion window as a child window so that it plays nice with Expose
	[parentWindow addChildWindow:suggestionWindow ordered:NSWindowAbove];
	
	// keep track of the parent text field in case we need to commit or abort editing.
	_parentTextField = parentTextField;
	
	// The window must know its accessibility parent, the control must know the window one of its accessibility children
	// Note that views (controls especially) are often ignored, so we want the unignored descendant - usually a cell
	// Finally, post that we have created the unignored decendant of the suggestions window
	[(SuggestionsWindow *)suggestionWindow setParentElement:NSAccessibilityUnignoredDescendant(parentTextField)];
	NSAccessibilityPostNotification(NSAccessibilityUnignoredDescendant(suggestionWindow),  NSAccessibilityCreatedNotification);
	
	// setup auto cancellation if the user clicks outside the suggestion window and parent text field. Note: this is a local event monitor and will only catch clicks in windows that belong to this application. We use another technique below to catch clicks in other application windows.
	_localMouseDownEventMonitor = [NSEvent addLocalMonitorForEventsMatchingMask:NSLeftMouseDownMask|NSRightMouseDownMask|NSOtherMouseDown handler:^(NSEvent *event)
								   {
									   // If the mouse event is in the suggestion window, then there is nothing to do.
									   if ([event window] != suggestionWindow)
									   {
										   if ([event window] == parentWindow)
										   {
											   /* Clicks in the parent window should either be in the parent text field or dismiss the suggestions window. We want clicks to occur in the parent text field so that the user can move the caret or select the search text.
												
												Use hit testing to determine if the click is in the parent text field. Note: when editing an NSTextField, there is a field editor that covers the text field that is performing the actual editing. Therefore, we need to check for the field editor when doing hit testing.
												*/
											   NSView *contentView = [parentWindow contentView];
											   NSPoint locationTest = [contentView convertPoint:[event locationInWindow] fromView:nil];
											   NSView *hitView = [contentView hitTest:locationTest];
											   NSText *fieldEditor = [parentTextField currentEditor];
											   if (hitView != parentTextField && (fieldEditor && hitView != fieldEditor) )
											   {
												   // Since the click is not in the parent text field, return nil, so the parent window does not try to process it, and cancel the suggestion window.
												   event = nil;
												   [self cancelSuggestions];
											   }
										   }
										   else
										   {
											   // Not in the suggestion window, and not in the parent window. This must be another window or palette for this application.
											   [self cancelSuggestions];
										   }
									   }
									   
									   return event;
								   }];
	// as per the documentation, do not retain event monitors.
	
	// We also need to auto cancel when the window loses key status. This may be done via a mouse click in another window, or via the keyboard (cmd-~ or cmd-tab), or a notificaiton. Observing NSWindowDidResignKeyNotification catches all of these cases and the mouse down event monitor catches the other cases.
	_lostFocusObserver = [[NSNotificationCenter defaultCenter] addObserverForName:NSWindowDidResignKeyNotification object:parentWindow queue:nil usingBlock:^(NSNotification *arg1)
						  {
							  // lost key status, cancel the suggestion window
							  [self cancelSuggestions];
						  }];
}

/* Order out the suggestion window, disconnect the accessibility logical relationship and dismantle any observers for auto cancel.
 Note: It is safe to call this method even if the suggestions window is not currently visible.
 */
- (void)cancelSuggestions
{
	NSWindow *suggestionWindow = self.window;
	if ([suggestionWindow isVisible])
	{
		// Remove the suggestion window from parent window's child window collection before ordering out or the parent window will get ordered out with the suggestion window.
		[[suggestionWindow parentWindow] removeChildWindow:suggestionWindow];
		[suggestionWindow orderOut:nil];
		
		[self resetLayout];
		_views = _trackingAreas = nil;
		
		// Disconnect the accessibility parent/child relationship
		[(SuggestionsWindow *)suggestionWindow setParentElement:nil];
	}
	
	// dismantle any observers for auto cancel
	if (_lostFocusObserver)
	{
		[[NSNotificationCenter defaultCenter] removeObserver:_lostFocusObserver];
		_lostFocusObserver = nil;
	}
	
	if (_localMouseDownEventMonitor)
	{
		[NSEvent removeMonitor:_localMouseDownEventMonitor];
		_localMouseDownEventMonitor = nil;
	}
}

- (void) setSuggestions : (NSArray<NSDictionary *> *) suggestions
{
	_suggestions = suggestions;
	
	// We only need to update the layout if the window is currently visible.
	if ([self.window isVisible])
		[self layoutSuggestions];
}

/* Returns the dictionary of the currently selected suggestion.
 */
- (NSDictionary *) selectedSuggestion
{
	return self.selectedView.payload;
}

#pragma mark -
#pragma mark Mouse Tracking

/* Mouse tracking is easily accomplished via tracking areas. We setup a tracking area for suggestion view and watch as the mouse moves in and out of those tracking areas.
 */

/* Properly creates a tracking area for an image view.
 */
- (id) trackingAreaForView : (NSView *) view
{
	return [[NSTrackingArea alloc] initWithRect:[self.window.contentView convertRect:view.bounds fromView:view]
										options:NSTrackingEnabledDuringMouseDrag | NSTrackingMouseEnteredAndExited | NSTrackingActiveInActiveApp
										  owner:self
									   userInfo:@{kTrackerKey : view}];
}

- (void) resetLayout
{
	for(NSView * view in _views)
		[view removeFromSuperview];
	
	NSView * contentView = self.window.contentView;
	for (NSTrackingArea *trackingArea in _trackingAreas)
		[contentView removeTrackingArea:trackingArea];
}

- (void)layoutSuggestions
{
	NSWindow *window = self.window;
	NSView *contentView = window.contentView;
	
	// Remove any existing suggestion view and associated tracking area and set the selection to nil
	self.selectedView = nil;
	
	if (_trackingAreas)
		[self resetLayout];
	
	// Iterate througn each suggestion creating a view for each entry.

	NSRect contentFrame = contentView.frame;
	NSPoint movingOrigin = NSMakePoint(0, -1);
	NSMutableArray * workingTrackingAreas = @[].mutableCopy, * workingViews = @[].mutableCopy;
	
	for (NSDictionary *entry in [_suggestions reverseObjectEnumerator])
	{
		if(entry != nil)
		{
			APPLHighlightingView * view = [self getViewForEntry:entry forWitdth:contentFrame.size.width];
			if(view != nil)
			{
				view.frameOrigin = movingOrigin;
				
				NSTrackingArea *trackingArea = [self trackingAreaForView:view];
				[contentView addTrackingArea:trackingArea];
				
				[workingTrackingAreas addObject:trackingArea];
				[workingViews addObject:view];
				
				[contentView addSubview:view];
				movingOrigin.y += view.bounds.size.height;
			}
		}
	}
	
	_trackingAreas = [workingTrackingAreas count] != 0 ? [NSArray arrayWithArray:workingTrackingAreas] : nil;
	_views = [workingViews count] != 0 ? [NSArray arrayWithArray:workingViews] : nil;
	
	//We have added all of the suggestion to the window. Now set the size of the window.
	contentFrame.size.height = movingOrigin.y;
	
	NSRect winFrame = window.frame;
	winFrame.origin.y = NSMaxY(winFrame) - movingOrigin.y;
	winFrame.size.height = movingOrigin.y;
	[window setFrame:winFrame display:YES];
}

- (APPLHighlightingView *) getViewForEntry : (NSDictionary *) entry forWitdth : (CGFloat) width
{
	__block APPLHighlightingView * output = nil;
	
	[_views enumerateObjectsUsingBlock:^(APPLHighlightingView * obj, NSUInteger idx, BOOL * stop)
	{
		if([obj.payload isEqualToDictionary:entry])
		{
			output = obj;
			*stop = YES;
		}
	}];
	
	if(output == nil)
	{
		BOOL isAuthor = [(NSNumber *) [entry objectForKey:kSuggestionType] unsignedCharValue] == RDB_FTS_CODE_AUTHOR;

		CGFloat baseY = BORDER + 1;
		NSRect frame = NSMakeRect(0, 0, width, HEIGHT);
		output = [[APPLHighlightingView alloc] initWithFrame:frame];
		if(output != nil)
		{
			output.payload = entry;
			
			RakText * prime, * secondary;
			uint cacheID = [[entry objectForKey:kSuggestionID] unsignedIntValue];
			
			//First, create the views so we can have a real idea of the height of the view
			//Create a suggestion for an author
			if(isAuthor)
			{
				prime = [[RakText alloc] initWithText:[entry objectForKey:kSuggestionString] :[Prefs getSystemColor:COLOR_ACTIVE]];
				
				uint nbSeries = getNbSeriesForAuthorOfID(cacheID);
				NSString * detailString;
				
				if(nbSeries == 1)
					detailString = NSLocalizedString(@"PROJ-SEARCH-AUTHOR-PROJECT", nil);
				else
					detailString = [NSString localizedStringWithFormat:NSLocalizedString(@"PROJ-SEARCH-AUTHOR-%d-PROJECTS", nil), nbSeries];
				
				secondary = [[RakText alloc] initWithText:detailString :[Prefs getSystemColor:COLOR_CLICKABLE_TEXT]];
				
			}
			//Create a suggestion for a serie
			else
			{
				byte type = [[entry objectForKey:kSuggestionType] unsignedCharValue];
				PROJECT_DATA project = getProjectByIDHelper(cacheID, false, false, false).project;
				
				prime = [[RakText alloc] initWithText:[entry objectForKey:kSuggestionString]
													 : (type & RDB_FTS_CODE_NAME) != 0 ?	[Prefs getSystemColor:COLOR_ACTIVE]
																						:	[Prefs getSystemColor:COLOR_CLICKABLE_TEXT]];

				secondary = [[RakText alloc] initWithText:getStringForWchar(project.authorName)
														 : (type & RDB_FTS_CODE_AUTHOR) != 0 ?	[Prefs getSystemColor:COLOR_ACTIVE]
																							 :	[Prefs getSystemColor:COLOR_CLICKABLE_TEXT]];
				
				output.thumbnail = loadDDThumbnail(project);
				output.thumbnailFrame = NSMakeRect(BORDER, BORDER + 1, IMAGE_WITDH, IMAGE_WITDH);
			}
			
			//Shared layout work
			CGFloat requiredHeight = 0;
			NSFont * oldFont = prime.font;
			if(prime != nil)
			{
				prime.enableWraps = YES;
				prime.fixedWidth = width - IMAGE_WITDH - 3 * BORDER;
				prime.font = [NSFont fontWithName:oldFont.fontName size:round(oldFont.pointSize * 1.05)];
				[prime sizeToFit];

				requiredHeight += prime.bounds.size.height;
			}

			if(secondary != nil)
			{
				secondary.enableWraps = YES;
				secondary.fixedWidth = width - IMAGE_WITDH - 3 * BORDER;
				secondary.font = [NSFont fontWithName:oldFont.fontName size:round(oldFont.pointSize * 0.95)];
				[secondary sizeToFit];
				
				if(requiredHeight != 0)
					requiredHeight += INTERLINE;
				
				requiredHeight += secondary.bounds.size.height;
			}
			
			if(requiredHeight != 0)	//Not empty row
			{
				requiredHeight += 2 * BORDER;
				
				if(requiredHeight != HEIGHT)
				{
					frame.size.height = requiredHeight;
					output.frame = frame;
				}
				
				//Now, layout work
				secondary.frameOrigin = NSMakePoint(IMAGE_WITDH + 2 * BORDER, baseY);
				prime.frameOrigin = NSMakePoint(IMAGE_WITDH + 2 * BORDER, secondary != nil ? (NSMaxY(secondary.frame) + INTERLINE) : baseY);
				
				[output addSubview:prime];
				[output addSubview:secondary];
			}
		}
	}
	
	return output;
}

/* The mouse is now over one of our child image views. Update selection and send action.
 */
- (void)mouseEntered:(NSEvent*)event
{
	[self userSetSelectedView:[(NSDictionary *) event.userData objectForKey:kTrackerKey]];
}

/* The mouse has left one of our child image views. Set the selection to no selection and send action
 */
- (void)mouseExited:(NSEvent*)event
{
	[self userSetSelectedView:nil];
}

/* The user released the mouse button. Force the parent text field to send its return action. Notice that there is no mouseDown: implementation. That is because the user may hold the mouse down and drag into another view.
 */
- (void)mouseUp:(NSEvent *)theEvent
{
	[_parentTextField validateEditing];
	[_parentTextField abortEditing];
	[_parentTextField sendAction:_parentTextField.action to:_parentTextField.target];
	[self cancelSuggestions];
}

#pragma mark -
#pragma mark Keyboard Tracking

/* In addition to tracking the mouse, we want to allow changing our selection via the keyboard. However, the suggestion window never gets key focus as the key focus remains on te text field. Therefore we need to route move up and move down action commands from the text field and this controller. See CustomMenuAppDelegate.m -control:textView:doCommandBySelector: to see how that is done.
 */

// Move the selection up and send action.
- (void) moveUp : (id) sender
{
	NSUInteger index = [_views indexOfObject:self.selectedView];
	if(index != NSNotFound && index != [_views count] - 1)
		index += 1;
	else
		index = 0;

	[self userSetSelectedView:[_views objectAtIndex:index]];
}

// Move the selection down and send action.
- (void) moveDown : (id) sender
{
	NSUInteger index = [_views indexOfObject:self.selectedView];
	if(index != NSNotFound && index != 0)
		index -= 1;
	else
		index = [_views count] - 1;
	
	[self userSetSelectedView:[_views objectAtIndex:index]];
}
@end

@implementation SuggestionsWindow

#pragma mark -
#pragma mark Accessibility

/* This window is acting as a popup menu of sorts.  Since this isn't semantically a window, we ignore it for accessibility purposes.  Similarly, the parent of this window is its logical parent in the parent window.  In this code sample, the text field, but essentially any UI element that is the logical 'parent' of the window.
 */
- (BOOL) accessibilityIsIgnored	{	return YES;		}
- (BOOL) isAccessibilityElement {	return NO;		}

// If we are asked for our AXParent, return the unignored anscestor of our parent element
- (id) accessibilityAttributeValue:(NSString *)attribute
{
	if ([attribute isEqualToString:NSAccessibilityParentAttribute])
		return NSAccessibilityUnignoredAncestor(_parentElement);
	
	return [super accessibilityAttributeValue:attribute];
}

@end
