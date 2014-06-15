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

@interface RakButton: NSButton

+ (id) allocForSeries : (NSView*) superView : (NSString*) imageName : (NSPoint) origin : (id) target : (SEL) selectorToCall;
+ (id) allocForReader : (NSView*) superView : (NSString*) imageName : (short) stateAtStartup : (CGFloat) posX : (BOOL) posXFromLeftSide : (id) target : (SEL) selectorToCall;

@end

@interface RakButtonCell : NSButtonCell
{
	bool notAvailable;
	bool canHighlight;
	
	NSImage *nonClicked;
	NSImage *clicked;
	NSImage *unAvailable;
}

- (id) initWithPage : (NSString*) imageName : (short) state;
- (id) initWithRawData : (NSImage*) _clicked : (NSImage*) _nonClicked : (NSImage*) _unAvailable;
- (id) copyWithZone:(NSZone *)zone;

- (void) setHighlightAllowed : (BOOL) allowed;
- (bool) isHighlightAllowed;

@end