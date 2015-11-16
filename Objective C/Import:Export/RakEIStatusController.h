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

@interface RakEIStatusController : NSObject
{
	CGFloat cachedPosInExport, cachedPosInEntry;

	RakSheetWindow * queryWindow;

	RakMenuText * title;
	NSProgressIndicator * progressBar;
	RakText * percentage;
	RakButton * cancel;
}

@property BOOL haveCanceled;

@property (nonatomic) CGFloat nbElementToExport, posInExport;
@property (nonatomic) CGFloat nbElementInEntry, posInEntry;

- (void) startUI;
- (void) refreshUI;
- (void) closeUI;

- (void) finishing;

- (NSString *) headerText;
- (NSColor *) backgroundColor;

@end
