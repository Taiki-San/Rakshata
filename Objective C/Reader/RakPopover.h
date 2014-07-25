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

@interface RakPopoverWrapper : NSObject
{
	INPopoverController * _popover;
}

@property (assign) NSView * anchor;
@property NSRect anchorFrame;
@property INPopoverArrowDirection direction;

- (id) init : (NSView *) contentView;
- (void) togglePopover : (NSRect) frame;
- (void) additionalConfiguration : (id) target : (SEL) selector;
- (void) updatePosition : (NSPoint) origin : (BOOL) animated;
- (void) setDelegate:(id <INPopoverControllerDelegate>) delegate;
- (void) closePopover;

- (void) clearMemory;

@end

@interface RakPopoverView : NSView <INPopoverControllerDelegate>
{
	bool additionalConfigRequired;
	
	NSView * _anchor;

	RakPopoverWrapper * popover;
}

- (void) internalInit : (id) anchor : (NSRect) baseFrame : (BOOL) wantAdditionalConfig;

- (void) setupView;
- (void) configurePopover : (INPopoverController*) internalPopover;

- (NSColor *) popoverBorderColor;
- (NSColor *) popoverArrowColor;
- (NSColor *) borderColor;
- (NSColor *) backgroundColor;

- (void) updateOrigin : (NSPoint) origin : (BOOL) animated;
- (void) additionalUpdateOnThemeChange;
- (void) closePopover;

@end
