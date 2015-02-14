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
{
	BOOL _haveBackgroundColor;
	
	BOOL _barWidthInitialized;
	CGFloat _barWidth;
	
	//Gradient stuffs
	BOOL _drawGradient;
	CGFloat _widthGradient;
	
	CGFloat _gradientXOrigin;
	CGFloat _gradientAngle;
	
	NSColor * _gradientBackgroundColor;
	NSGradient * _gradient;
}

@property CGFloat barWidth;
@property BOOL ignoreInternalFrameMagic;

@property BOOL drawGradient;
@property CGFloat widthGradient;

@property BOOL haveBackgroundColor;

- (instancetype) initWithText : (NSRect)frame : (NSString *)text;
- (void) resizeAnimation : (NSRect) frameRect;
- (NSRect) getMenuFrame : (NSRect) superviewSize;
- (CGFloat) getTextHeight;

- (void) additionalDrawing;

- (NSColor *) getTextColor;
- (NSColor *) getBarColor;
- (NSColor *) getBackgroundColor;
- (NSColor *) getGradientBackgroundColor;
- (CGFloat) getFontSize;

+ (NSFont *) getFont : (CGFloat) fontSize;

- (void) defineBackgroundColor;


@end
