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

@class RakPopoverView;

@interface RakPopoverWrapper : NSObject
{
	INPopoverController * _popover;
}

@property (weak) NSView * anchor;
@property NSRect anchorFrame;
@property INPopoverArrowDirection direction;

- (instancetype) init : (NSView *) contentView;
- (void) togglePopover : (NSRect) frame;

- (void) additionalConfiguration : (id) target;
- (void) justCallSuperConfigure : (RakPopoverView *) view;

- (void) updatePosition : (NSPoint) origin : (BOOL) animated;
- (void) setDelegate:(id <INPopoverControllerDelegate>) delegate;
- (void) closePopover;

- (void) clearMemory;

@end

@interface RakPopoverView : NSView <INPopoverControllerDelegate>
{
	BOOL prefsRegistered;
	BOOL additionalConfigRequired;
	
	NSView * _anchor;

	RakPopoverWrapper * popover;
}

- (void) internalInit : (id) anchor : (NSRect) baseFrame : (BOOL) wantAdditionalConfig;

- (void) setupView;
- (void) configurePopover : (INPopoverController*) internalPopover;
- (void) directConfiguration : (INPopoverController*) internalPopover;

- (NSColor *) popoverBorderColor;
- (NSColor *) popoverArrowColor;
- (NSColor *) borderColor;
- (NSColor *) backgroundColor;

- (void) updateOrigin : (NSPoint) origin : (BOOL) animated;
- (void) additionalUpdateOnThemeChange;
- (void) closePopover;

@end
