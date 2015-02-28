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

@interface RakAboutContent : NSView

@property BOOL haveAdditionalDrawing;
@property (nonatomic, copy) void (^additionalDrawing)(NSSize size);

@end

@interface RakAboutText : RakText
{
	NSTrackingRectTag tracking;
	
	NSColor * classicalTextColor;
}

@property NSString * URL;

@property id target;
@property SEL action;

- (void) setupArea;
- (instancetype) initWithText:(NSString *) text : (NSColor *)color responder : (RakAboutWindow *) responder;

@end