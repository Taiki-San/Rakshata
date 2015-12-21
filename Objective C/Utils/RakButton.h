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
#import "RakStatusButton.h"
#import "RakPopUpButton.h"

@interface RakButton: NSButton
{
	BOOL haveBackground, hasRegisteredThemeUpdates;
}

@property BOOL textButton;
@property BOOL hasBorder;

+ (instancetype) allocForReader : (NSView*) superview : (NSString*) imageName : (CGFloat) posX : (BOOL) posXFromLeftSide : (id) target : (SEL) selectorToCall;

+ (instancetype) allocImageWithBackground : (NSString*) imageName : (id) target : (SEL) selectorToCall;
+ (instancetype) allocImageWithoutBackground : (NSString*) imageName : (id) target : (SEL) selectorToCall;
+ (instancetype) allocWithText : (NSString*) string : (NSRect) frame;
+ (instancetype) allocWithText : (NSString*) string;

- (void) triggerBackground;

@end

@interface RakButtonCell : NSButtonCell
{
	BOOL notAvailable, hasRegisteredThemeUpdates;
	
	NSString * _imageName;
	
	RakImage *nonClicked;
	RakImage *clicked;
	RakImage *unAvailable;
	
	RakCenteredTextFieldCell * textCell;
}

@property RakColor * customBackgroundColor;
@property (nonatomic) CGFloat borderWidth;
@property BOOL forceHighlight;
@property BOOL hasBorder;
@property (nonatomic, getter=isActiveAllowed) BOOL activeAllowed;

- (instancetype) initWithPage : (NSString*) imageName;

- (instancetype) initWithText : (NSString *) text;
- (void) reloadFontColor;
- (NSSize) sizeOfTextCell;
- (RakColor*) getBorderColor;
- (RakColor*) getBackgroundColor;
- (RakColor *) getFontColor;

@end