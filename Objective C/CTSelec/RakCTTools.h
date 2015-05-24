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

@interface RakCTAnimationController : RakAnimationController
{
	RakSegmentedButtonCell * _cell;
	RakCTSelectionListContainer * _chapter;
	RakCTSelectionListContainer * _volume;
	
	CGFloat distanceToCoverPerMark;
	NSPoint chapOrigin;
	NSPoint volOrigin;
}

- (instancetype) init : (NSInteger) initialPos : (CGFloat) diff : (RakSegmentedButtonCell*) cell;

- (void) addCTContent : (RakCTSelectionListContainer*) chapter : (RakCTSelectionListContainer*) volume;

@end

@interface RakCTProjectImageView : NSImageView

- (instancetype) initWithImageName : (PROJECT_DATA) project : (NSRect) superviewFrame;
- (void) updateProject : (NSString *) imageName;

- (NSRect) getProjectImageSize : (NSRect) superviewFrame : (NSSize) imageSize;

- (void) resizeAnimation : (NSRect) frameRect;

@end

@interface RakCTCoreViewButtons : RakSegmentedControl

@end

NSString * priceString(uint price);
