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
 ********************************************************************************************/

@interface RakDeleteButtonCell : RakButtonCell

@end

@implementation RakDeleteConfirm

- (instancetype) autoInit
{
	return [self initWithFrame: NSMakeRect(0, 0, 170, 173)];
}

//We return if the popover is opened or was aborted, because of the `remind` feature

- (BOOL) launchPopover : (RakView *) anchor : (Reader*) receiver
{
	//We check if the user asked not to be annoyed again
	BOOL alreadyAsked, answer = NO;
	alreadyAsked = [RakPrefsRemindPopover getValueReminded : PREFS_REMIND_DELETE : &answer];
	if(alreadyAsked && ![RakApp window].shiftPressed)
	{
		if(answer)
			[receiver deleteElement];
		
		return NO;
	}
	
	_remind = alreadyAsked;
	_isTome = receiver.isTome;
	_receiver = receiver;
	_anchor = anchor;
	
	[self internalInitWithAnchor:anchor
						 atFrame:NSMakeRect(0, 0, _anchor.frame.size.width, 0)
					wantCallback:YES
			closeOnContextChange:YES];
	
	return YES;
}

- (void) setupView
{
	NSString * string = NSLocalizedString(_isTome ? @"DELETION-VOL-CONFIRM" : @"DELETION-CHAP-CONFIRM", nil);
	
	RakText * contentText = [[RakText alloc] initWithText:self.frame :string :[Prefs getSystemColor : COLOR_DANGER_POPOVER_TEXT_COLOR]];
	if(contentText != nil)
	{
		[contentText setAlignment:NSTextAlignmentCenter];
		[contentText setFont:[Prefs getFont:FONT_RD_BUTTONS ofSize:13]];
		[contentText sizeToFit];
		[self addSubview : contentText];
		[contentText setFrameOrigin:NSMakePoint(self.frame.size.width / 2 - contentText.frame.size.width / 2 , self.frame.size.height - 10 - contentText.frame.size.height)];
	}
	
	RakDeleteSegmentedControl * button = [[RakDeleteSegmentedControl alloc] initWithData : NSMakeRect(0, 0, self.frame.size.width, contentText.frame.origin.y - 10) : NSLocalizedString(@"CONFIRM", nil) : NSLocalizedString(@"NO", nil)];
	if(button != nil)
	{
		[button setTarget:self];
		[button setAction:@selector(buttonClicked:)];
		[self addSubview:button];
	}
	
	RakDeleteButton * buttonRemind = [RakDeleteButton allocWithText:NSLocalizedString(@"REMIND", nil) :NSMakeRect(button.frame.origin.x, button.frame.origin.y - 5 - button.frame.size.height, button.frame.size.width, button.frame.size.height)];
	if(buttonRemind != nil)
	{
		[buttonRemind setTarget:self];
		[buttonRemind setAction:@selector(remindSwitched:)];
		
		((RakButtonCell*)buttonRemind.cell).forceHighlight = _remind;
		[((RakButtonCell*)buttonRemind.cell) reloadFontColor];
		
		[self addSubview:buttonRemind];
	}
}

- (void) configurePopover:(INPopoverController *)internalPopover
{
	[super configurePopover:internalPopover];
	
	internalPopover.closesWhenApplicationBecomesInactive = YES;
	internalPopover.closesWhenPopoverResignsKey = YES;
}

- (void) additionalUpdateOnThemeChange
{
	[super additionalUpdateOnThemeChange];
	
	for(RakText * view in self.subviews)
	{
		if([view class] == [RakText class])
		{
			[view setTextColor:[Prefs getSystemColor : COLOR_DANGER_POPOVER_TEXT_COLOR]];
			break;
		}
	}
}

//Colors

- (RakColor *) popoverBorderColor
{
	return [Prefs getSystemColor: COLOR_DANGER_POPOVER_BORDER];
}

//Toolbox

- (void) locationUpdated : (NSRect) frame : (BOOL) animated
{
	[self updateOrigin : frame.origin : animated];
}

//Payload

- (void) buttonClicked : (RakQuerySegmentedControl*) sender
{
	BOOL output = NO;
	if([sender selectedSegment] == 0)
	{
		[_receiver deleteElement];
		output = YES;
	}
	
	if(_remind)
		[RakPrefsRemindPopover setValueReminded : PREFS_REMIND_DELETE : output];
	else
		[RakPrefsRemindPopover removeValueReminded : PREFS_REMIND_DELETE];
	
	[popover closePopover];
}

- (void) remindSwitched : (RakButton*) sender
{
	_remind = !_remind;
	
	if(sender != nil && [sender class] == [RakDeleteButton class] && sender.cell != nil && [sender.cell class] == [RakDeleteButtonCell class])
		((RakButtonCell*)sender.cell).forceHighlight = _remind;
}

@end

@interface RakDeleteSegmentedControlCell : RakSegmentedButtonCell

@end

@implementation RakDeleteSegmentedControlCell

- (RakColor *) getFontColor : (uint) cellID
{
	if([self isSelectedForSegment:cellID])
		return [Prefs getSystemColor : COLOR_DANGER_POPOVER_TEXT_COLOR_SELECTED];
	else
		return [Prefs getSystemColor : COLOR_DANGER_POPOVER_TEXT_COLOR];
}

@end

@implementation RakDeleteSegmentedControl

+ (Class) cellClass
{
	return [RakDeleteSegmentedControlCell class];
}

@end

@implementation RakDeleteButtonCell

- (RakColor *) getFontColor
{
	if([self isHighlighted] || self.forceHighlight)
		return [Prefs getSystemColor : COLOR_DANGER_POPOVER_TEXT_COLOR_SELECTED];
	else
		return [Prefs getSystemColor : COLOR_DANGER_POPOVER_TEXT_COLOR];
}

@end

@implementation RakDeleteButton

+ (Class) cellClass
{
	return [RakDeleteButtonCell class];
}

@end
