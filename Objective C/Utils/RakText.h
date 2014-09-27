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

@interface RakText : NSTextField

@property BOOL wantCustomBorder;
@property CGFloat forcedOffset;

- (id)initWithText:(NSRect)frame : (NSString *) text : (NSColor *) color;

- (NSColor *) getBorderColor;
- (void) additionalDrawing;

@end

@interface RakFormatterLength : NSFormatter

@property uint maxLength;

- (instancetype) init : (uint) length;

@end

@interface RakTextClickable : NSView
{
	NSColor * backgroundColor;
}

@property (strong) NSString * URL;

- (id)initWithText:(NSRect)frame : (NSString *) text : (NSColor *) color;

@end
