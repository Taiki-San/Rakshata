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
 *********************************************************************************************/

@interface RakText : NSTextField <NSTextFieldDelegate, NSTextViewDelegate>
{
	BOOL haveFixedWidth, autoCompleting;
	CGFloat _suggestedWidth;

	CGFloat _cachedMinHeight;
}

@property BOOL wantCustomBorder;
@property RakColor * customBorderColor;
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

- (instancetype)initWithText:(NSRect)frame : (NSString *) text : (RakColor *) color;
- (instancetype) initWithText : (NSString *) text : (RakColor *) color;

- (RakColor *) getBorderColor;
- (void) additionalDrawing;

- (void) updateMultilineHeight;

- (void) overrideMouseDown : (NSEvent *) theEvent;
- (void) overrideMouseUp : (NSEvent *) theEvent;

@end

@interface RakFormatterLength : NSFormatter

@property uint maxLength;

- (instancetype) init : (uint) length;

@end

#import <RakKit/RakClickableText.h>
#import <RakKit/RakTextCell.h>
#import <RakKit/RakMenuText.h>
