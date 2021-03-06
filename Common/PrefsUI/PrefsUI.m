/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at https://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

enum
{
	LEFT_BORDER = 15,
	RIGHT_BORDER = 5,
	INTER_BORDER = 15,
	WIDTH = 150,
	HEIGHT = 200
};

@implementation RakPrefsPopover

#pragma mark - Core logic

- (instancetype) initWithFrame : (NSRect) frame
{
	self = [super initWithNibName:nil bundle:nil];
	if(self != nil)
	{
		mainView = [[RakView alloc] initWithFrame:frame];
		[self setView:mainView];
		
		[Prefs registerForChange:self forType:KVO_THEME];
	}
	return self;
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary<NSString *,id> *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	
	for(RakText * text in textFields)
		text.textColor = [self textColor];
}

- (void) dealloc
{
	[Prefs deRegisterForChange:self forType:KVO_THEME];
}

#pragma mark - Receive popover

- (void) setPopover:(NSPopover *)popover
{
	_popover = popover;
	[self configure];
}

- (void) popoverClosed
{
	[textFields removeAllObjects];
	
	NSArray * array = [mainView.subviews copy];
	
	for(RakView * view in array)
		[view removeFromSuperview];
}

- (void) configure
{
	[Prefs getPref:PREFS_GET_MAIN_THREAD : &mainThread];
	
	switch (mainThread)
	{
		case TAB_SERIES:
		case TAB_CT:
		{
			break;
		}
			
		case TAB_READER:
		{
			[self configureReader];
			break;
		}
	}
}

#pragma mark - Setting triggering responders

- (void) readerSaveMagnification : (RakSwitchButton *) button
{
	if(button != saveMagnification)
		return [saveMagnification performClick:nil];
	
	[Prefs setPref:PREFS_SET_SAVE_MAGNIFICATION atValue:saveMagnification.state == NSOnState];
}

- (void) readerColorPDFBackground : (RakSwitchButton *) button
{
	if(button != forcePDFBackground)
		return [forcePDFBackground performClick:nil];
	
	[Prefs setPref:PREFS_SET_HAVE_PDF_BACKGROUND atValue:forcePDFBackground.state == NSOnState];
}

- (void) readerOverrideDirection : (RakSwitchButton *) button
{
	if(button != overrideDirection)
		return [overrideDirection performClick:nil];

	[Prefs setPref:PREFS_SET_DIROVERRIDE atValue:overrideDirection.state == NSOnState];
}

#pragma mark - Colors

- (RakColor *) textColor
{
	return [Prefs getSystemColor:COLOR_INACTIVE];
}

#pragma mark - UI Configuration

- (void) configureReader
{
	CGFloat baseY = HEIGHT - INTER_BORDER;
	BOOL setting;
	
	RakSwitchButton * button;

	//Save magnification
	[Prefs getPref:PREFS_GET_SAVE_MAGNIFICATION :&setting];
	baseY = [self createSwitchWithText:@"READER-SETTING-KEEP-MAGNI" status:setting toButton:&button withSelector:@selector(readerSaveMagnification:) andBaseY:baseY];
	
	saveMagnification = button;
	
	//Have PDF background
	[Prefs getPref:PREFS_GET_HAVE_PDF_BACKGROUND :&setting];
	baseY = [self createSwitchWithText:@"READER-SETTING-FORCE-BKGD" status:setting toButton:&button withSelector:@selector(readerColorPDFBackground:) andBaseY:baseY];
	
	forcePDFBackground = button;
	
	//Should override the direction suggested by the project
	[Prefs getPref:PREFS_GET_DIROVERRIDE :&setting];
	[self createSwitchWithText:@"READER-SETTING-OVERRIDE-DIR" status:setting toButton:&button withSelector:@selector(readerOverrideDirection:) andBaseY:baseY];
	
	overrideDirection = button;
}

#pragma mark - UI Toolkit

#define WIDTH_TEXT (WIDTH - LEFT_BORDER - RIGHT_BORDER - (*button).bounds.size.width - INTER_BORDER)

- (CGFloat) createSwitchWithText : (NSString *) textString status : (BOOL) enabled toButton : (RakSwitchButton **) button withSelector : (SEL) selector andBaseY : (CGFloat) baseY
{
	*button = [[RakSwitchButton alloc] init];
	if(*button != nil)
	{
		RakText * text = [[RakText alloc] initWithText:NSLocalizedString(textString, nil) :[self textColor]];
		if(text != nil)
		{
			(*button).state = enabled ? NSOnState : NSOffState;
			(*button).target = self;
			(*button).action = selector;
			
			[mainView addSubview:*button];
			
			text.enableWraps = YES;
			text.fixedWidth = WIDTH_TEXT;
			text.clicTarget = self;
			text.clicAction = selector;
			
			const CGFloat heightButton = (*button).bounds.size.height, heightText = text.bounds.size.height, maxHeight = MAX(heightButton, heightText);
			
			baseY -= maxHeight;

			[*button setFrameOrigin:NSMakePoint(LEFT_BORDER, baseY + (maxHeight / 2 - heightButton / 2))];
			[text setFrameOrigin:NSMakePoint(LEFT_BORDER + (*button).bounds.size.width + INTER_BORDER, baseY + (maxHeight / 2 - heightText / 2))];
			[mainView addSubview:text];
			
			[textFields addObject:text];
			
			baseY -= INTER_BORDER;
		}
		else
			*button = nil;
	}
	
	return baseY;
}

@end

@implementation PrefsUI

- (instancetype) init
{
	self = [super init];
	if(self != nil)
	{
		viewControllerHUD = [[RakPrefsPopover alloc] initWithFrame : NSMakeRect(0, 0, WIDTH, HEIGHT)];
		[self setAnchor:nil];
	}
	return self;
}

- (void) setAnchor : (NSButton *) newAnchor
{
	anchor = newAnchor;
}

- (void)createPopover
{
	if(popover == nil && (popover = [[NSPopover alloc] init]) != nil)
	{
		popover.contentViewController = viewControllerHUD;
		popover.appearance = [Prefs getCurrentTheme] == THEME_CODE_LIGHT ? NSPopoverAppearanceMinimal : NSPopoverAppearanceHUD;
		
		popover.animates = YES;
		
		// AppKit will close the popover when the user interacts with a user interface element outside the popover.
		// note that interacting with menus or panels that become key only when needed will not cause a transient popover to close.
		popover.behavior = NSPopoverBehaviorTransient;
		
		// so we can be notified when the popover appears or closes
		popover.delegate = self;
		
		viewControllerHUD.popover = popover;
		
		[Prefs registerForChange:self forType:KVO_MAIN_THREAD];
	}
}

- (void)showPopover
{
	[self createPopover];
	[popover showRelativeToRect:[anchor bounds] ofView:anchor preferredEdge:NSMaxYEdge];
}

#pragma mark - NSPopoverDelegate

- (void)popoverDidClose:(NSNotification *)notification
{
	[viewControllerHUD popoverClosed];
	
	if(popover != nil)
	{
		popover = nil;
		[Prefs deRegisterForChange:self forType:KVO_MAIN_THREAD];
	}
}

#pragma mark - KVO and setting management

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	
	if(popover != nil)
		[popover close];
}

@end
