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

@interface RakButton: NSButton
{
	BOOL haveBackground;
}

@property BOOL textButton;
@property BOOL hasBorder;

+ (id) allocForReader : (NSView*) superview : (NSString*) imageName : (short) stateAtStartup : (CGFloat) posX : (BOOL) posXFromLeftSide : (id) target : (SEL) selectorToCall;

+ (instancetype) allocImageWithBackground : (NSString*) imageName : (short) stateAtStartup : (id) target : (SEL) selectorToCall;
+ (instancetype) allocImageWithoutBackground : (NSString*) imageName : (short) stateAtStartup : (id) target : (SEL) selectorToCall;
+ (id) allocWithText : (NSString*) string : (NSRect) frame;

- (void) triggerBackground;

@end

@interface RakButtonCell : NSButtonCell
{
	bool notAvailable;
	bool canHighlight;
	
	NSString * _imageName;
	
	NSImage *nonClicked;
	NSImage *clicked;
	NSImage *unAvailable;
	
	RakCenteredTextFieldCell * textCell;
}

@property bool forceHighlight;
@property BOOL hasBorder;

- (id) initWithPage : (NSString*) imageName : (short) state;
- (id) initWithRawData : (NSString *) imageName : (NSImage*) _clicked : (NSImage*) _nonClicked : (NSImage*) _unAvailable;

- (void) setHighlightAllowed : (BOOL) allowed;
- (bool) isHighlightAllowed;

- (id) initWithText : (NSString *) text;
- (void) reloadFontColor;
- (NSSize) sizeOfTextCell;
- (NSColor*) getBorderColor;
- (NSColor*) getBackgroundColor;
- (NSColor *) getFontColor;

@end