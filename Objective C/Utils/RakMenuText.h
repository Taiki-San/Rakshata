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

@interface RakMenuText : RakText

- (void) additionalDrawing;

- (NSColor *) getTextColor;
- (NSColor *) getBarColor;
- (NSColor *) getBackgroundColor;
- (CGFloat) getFontSize;
- (NSFont *) getFont;

- (CGFloat) getTextHeight;
- (NSRect) getMenuFrame : (NSRect) superViewSize;
- (id) initWithText:(NSRect)frame :(NSString *)text;
- (void) defineBackgroundColor;

- (void) resizeAnimation : (NSRect) frameRect;

@end
