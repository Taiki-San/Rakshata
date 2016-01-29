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
 ********************************************************************************************/

enum
{
	MENU_TEXT_WIDTH = 28
};

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
	
	RakColor * _gradientBackgroundColor;
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

- (RakColor *) getTextColor;
- (RakColor *) getBarColor;
- (RakColor *) getBackgroundColor;
- (RakColor *) getGradientBackgroundColor;
- (CGFloat) getFontSize;

+ (NSFont *) getFont : (CGFloat) fontSize;

- (void) defineBackgroundColor;


@end
