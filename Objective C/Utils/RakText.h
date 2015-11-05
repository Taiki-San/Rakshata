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

@interface RakText : NSTextField <NSTextFieldDelegate, NSTextViewDelegate>
{
	BOOL haveFixedWidth, autoCompleting;
	CGFloat _suggestedWidth;

	CGFloat _cachedMinHeight;
}

@property BOOL wantCustomBorder;
@property CGFloat forcedOffsetY;

@property (nonatomic) BOOL enableMultiLine;
@property BOOL enableWraps;
@property (nonatomic) CGFloat fixedWidth;

@property BOOL discardHeight;
@property NSRect currentFrame;

@property (nonatomic) CGFloat maxLength;

@property (weak) id clicTarget;
@property SEL clicAction;
@property (copy) void (^callbackOnClic) (RakText * textField);

@property (copy) void (^callbackOnChange)(NSNotification * notification, BOOL didComplete);

//Completion
@property BOOL wantCompletion;
@property (copy) NSArray<NSString *> * (^callbackOnCompletion)();

- (instancetype)initWithText:(NSRect)frame : (NSString *) text : (NSColor *) color;
- (instancetype) initWithText : (NSString *) text : (NSColor *) color;

- (NSColor *) getBorderColor;
- (void) additionalDrawing;

- (void) overrideMouseDown : (NSEvent *) theEvent;
- (void) overrideMouseUp : (NSEvent *) theEvent;

@end

@interface RakFormatterLength : NSFormatter

@property uint maxLength;

- (instancetype) init : (uint) length;

@end

#import "RakClickableText.h"