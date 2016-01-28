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
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 ********************************************************************************************/

enum
{
	BORDER_BUTTON = 20
};


@implementation RakMinimalSegmentedControl

#pragma mark - Init

+ (Class)cellClass
{
	return [RakSegmentedButtonCell class];
}

#pragma mark - Resizing

- (NSRect) getButtonFrame : (NSRect) superviewFrame
{
	return superviewFrame;
}

- (void) setFrame:(NSRect)frameRect
{
	NSRect newFrame = [self getButtonFrame:frameRect];
	[super setFrame: newFrame];
	
	if(newFrame.size.width == frameRect.size.width)
	{
		[self sizeToFit];
		newFrame = [self getButtonFrame:frameRect];
		[super setFrame: newFrame];
	}
}

- (void) resizeAnimation : (NSRect) frameRect
{
	[self setFrameAnimated : [self getButtonFrame:frameRect]];
}

#pragma mark - Workaround

- (void) didAddSubview:(RakView *)subview
{
	//Hide the text 10.11 added which broke our customization with undocumented classes, yay \o/
	if(floor(NSAppKitVersionNumber) > NSAppKitVersionNumber10_10_5)
		subview.hidden = YES;
}

@end

@implementation RakSegmentedControl

- (instancetype) initWithFrame : (NSRect) frame : (NSArray *) buttonMessages
{
	self = [super initWithFrame:[self getButtonFrame:frame]];
	
	if(self != nil)
	{
		//≥ 10.10.3
		if([self respondsToSelector:@selector(setSpringLoaded:)])
			self.springLoaded = YES;

		__block uint oldWidthButton = 0;
		
		[self setSegmentCount:(NSInteger)[buttonMessages count]];
		
		[buttonMessages enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
			
			[self setLabel:obj forSegment:(NSInteger)idx];
			[self setEnabled:NO forSegment:(NSInteger)idx];
			[self sizeToFit];
			
			CGFloat widthButton;
			
			if(idx == 0)
				widthButton = self.frame.size.width - BORDER_BUTTON;
			else
				widthButton = self.frame.size.width - oldWidthButton - 1;	//Hack to properly draw the buttons below
			
			[self setWidth:widthButton forSegment:(NSInteger)idx];
			
			oldWidthButton += widthButton;
			
		}];
		
		[self setFrameOrigin:[self getButtonFrame:frame].origin];
	}
	
	return self;
}

- (void) setLabel:(NSString *)label forSegment:(NSInteger)segment
{
	[super setLabel:label forSegment:segment];
	[self sizeToFit];
}

- (void) dealloc
{
	[animationController abortAnimation];
}

#pragma mark - Animation

- (void) updateSelectionWithoutAnimation : (NSInteger) newState
{
	[_cell setSelectedSegmentWithoutAnimation:newState];
}

- (BOOL) setupTransitionAnimation : (NSNumber *) oldValue : (NSNumber *) newValue
{
	NSInteger initialPos = [oldValue integerValue];
	
	if(animationController == nil)
	{
		animationController = [[RakCTAnimationController alloc] init: initialPos : [newValue integerValue] - initialPos : self.cell];
		
		if(animationController == nil)
			return NO;
	}
	else
		[animationController updateState : initialPos : [newValue integerValue] - initialPos];
	
	if([self.superview respondsToSelector:@selector(feedAnimationController:)])
		[(id) self.superview feedAnimationController:animationController];
	
	[animationController startAnimation];
	
	return YES;
}

@end
