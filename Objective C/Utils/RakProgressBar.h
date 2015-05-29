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

@interface RakProgressBar : NSView
{
	NSSize currentSize;
	CGFloat _width;
	
	CGFloat _percentage;

	size_t _speed;
	RakText * _speedText;
	
	NSColor * slotColor;
	NSColor * progressColor;
	
	uint _posX;
	
	BOOL wasOffsetSet;
}

@property (nonatomic) CGFloat offsetYSpeed;

+ (CGFloat) getLeftBorder;
+ (CGFloat) getRightBorder;

- (void) updatePercentage : (CGFloat) percentage : (size_t) downloadSpeed;
- (void) updatePercentageProxy : (NSArray*) percentage;

- (void) setRightTextBorder : (uint) posX;

- (void) notifyNeedDisplay;
- (void) centerText;

@end
