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

@implementation RakSegmentedAnimationController

- (instancetype) initWithBasePos : (NSInteger) initialPos stepping: (CGFloat) diff cell: (RakSegmentedButtonCell*) cell
{
	self = [super init];
	
	if(self != nil)
	{
		_initialState = initialPos;
		_animationDiff = diff;
		_cell = cell;
		self.stage = self.animationFrame;
	}
	
	return self;
}

#pragma mark - Animation Work

- (void) animation:(NSAnimation *)animation didReachProgressMark:(NSAnimationProgress)progress
{
	[_cell updateAnimationStatus: YES : _initialState + _animationDiff * progress];
	[_cell.controlView setNeedsDisplay:YES];
	
	[self customAnimationUpdate:animation didReachProgressMark:progress];
	
	[super animation:animation didReachProgressMark:progress];
}

- (void) customAnimationUpdate:(NSAnimation *)animation didReachProgressMark:(NSAnimationProgress) progress
{
	
}

- (void) animationDidEnd:(NSAnimation *)animation
{
	[super animationDidEnd:animation];
	
	[_cell.controlView setNeedsDisplay:YES];
}

- (void) postProcessingBeforeAction
{
	[_cell updateAnimationStatus:NO :1];
}

@end
