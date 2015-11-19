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

@interface RakStatusButton : NSView
{
	NSColor * cachedColor,  * cachedBackgroundColor;
	RakText * text;

	RakAnimationController * _animation, * _backgroundAnimation;
	NSTrackingArea * trackingArea;

	BOOL cursorOver, clickingInside;
	CGFloat textWidth;
}

@property (nonatomic) byte status;
@property id target;
@property SEL action;

@property (nonatomic) NSString * stringValue;

- (instancetype) initWithStatus : (byte) status;

@end

enum
{
	STATUS_BUTTON_OK,
	STATUS_BUTTON_WARN,
	STATUS_BUTTON_ERROR,
};