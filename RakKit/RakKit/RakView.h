//
//  RakView.h
//  RakKit
//
//  Created by Taiki on 20/11/2016.
//  Copyright © 2016 Taiki. All rights reserved.
//

@interface RakView (RakUtils)

- (void) setFrameAnimated : (NSRect) frame;
- (void) setFrameOriginAnimated : (NSPoint) origin;
- (void) setAlphaAnimated : (CGFloat) alpha;

- (RakView *) findSubviewAtCoordinate : (NSPoint) coordinates;
- (RakImage *) imageOfView;

@end

enum RAKBUTTON_STATES
{
	RB_STATE_STANDARD		= NSOffState,
	RB_STATE_HIGHLIGHTED	= NSOnState,
	RB_STATE_UNAVAILABLE	= NSMixedState
};
