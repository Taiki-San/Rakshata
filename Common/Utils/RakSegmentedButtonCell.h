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

@interface RakSegmentedButtonCell : NSSegmentedCell
{
	NSMutableArray * fields;
	
	//Animation
	BOOL animationRunning;
	
	CGFloat animationProgress;
	uint impactedCell;
	
	BOOL isNextCellImpacted;
	BOOL animationToTheLeft;
}

- (void) setSelectedSegmentWithoutAnimation:(NSInteger)selectedSegment;
- (void) updateAnimationStatus : (BOOL) stillRunning : (CGFloat) status;

- (void) createCellWithText : (NSString*) string forElem : (uint) cellID;

- (RakColor *) getFontColor : (uint) cellID;
- (NSTextFieldCell*) getCellForID : (uint) cellID;
- (void)drawCell:(uint)cellID inFrame:(NSRect)frame withView:(RakView *)controlView;

@end
