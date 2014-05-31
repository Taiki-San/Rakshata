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

@interface RakProgressCircle : NSView
{
	CGFloat _radius;
	CGFloat _width;
	
	CGFloat _percentage;
	
	RakText * _percText;
	
	NSColor * slotColor;
	NSColor * progressColor;
}

- (id)initWithRadius:(CGFloat) radius : (NSPoint) origin;
- (void) setWidth : (CGFloat) width;
- (RakText *) getText;
- (void) updatePercentage : (CGFloat) percentage;
- (void) updatePercentageProxy : (NSNumber*) percentage;

- (void) notifyNeedDisplay;

@end
