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

@interface RakGradientView : NSView
{
	//Cached drawing ressources
	NSColor * _startColor;
	NSColor * _endColor;
	NSGradient * gradient;
}

@property CGFloat gradientMaxWidth;
@property CGFloat gradientWidth;
@property CGFloat angle;

- (void) initGradient;

- (void) updateTheme;
- (void) updateGradient;

- (NSRect) grandientBounds;
- (NSColor *) startColor;
- (NSColor *) endColor : (NSColor *) startColor;

@end
