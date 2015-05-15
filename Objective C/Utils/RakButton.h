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

#import "RakCustomButton.h"
#import "RakSlideshowButton.h"
#import "RakSwitchButton.h"
#import "RakSegmentedControl.h"

@interface RakButton: NSButton
{
	BOOL haveBackground;
}

@property BOOL textButton;
@property BOOL hasBorder;

+ (instancetype) allocForReader : (NSView*) superview : (NSString*) imageName : (short) stateAtStartup : (CGFloat) posX : (BOOL) posXFromLeftSide : (id) target : (SEL) selectorToCall;

+ (instancetype) allocImageWithBackground : (NSString*) imageName : (short) stateAtStartup : (id) target : (SEL) selectorToCall;
+ (instancetype) allocImageWithoutBackground : (NSString*) imageName : (short) stateAtStartup : (id) target : (SEL) selectorToCall;
+ (instancetype) allocWithText : (NSString*) string : (NSRect) frame;
+ (instancetype) allocWithText : (NSString*) string;

- (void) triggerBackground;

@end

@interface RakButtonCell : NSButtonCell
{
	bool notAvailable;
	
	NSString * _imageName;
	
	NSImage *nonClicked;
	NSImage *clicked;
	NSImage *unAvailable;
	
	RakCenteredTextFieldCell * textCell;
}

@property NSColor * customBackgroundColor;
@property (nonatomic) CGFloat borderWidth;
@property bool forceHighlight;
@property BOOL hasBorder;
@property (nonatomic, getter=isHighlightAllowed) BOOL highlightAllowed;

- (instancetype) initWithPage : (NSString*) imageName : (short) state;

- (instancetype) initWithText : (NSString *) text;
- (void) reloadFontColor;
- (NSSize) sizeOfTextCell;
- (NSColor*) getBorderColor;
- (NSColor*) getBackgroundColor;
- (NSColor *) getFontColor;

@end