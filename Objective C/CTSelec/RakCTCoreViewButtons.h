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

@interface RakCTCoreViewButtons : NSSegmentedControl

- (void) resizeAnimation : (NSRect) frameRect;

@end

@interface RakCTCoreViewButtonsCell : NSSegmentedCell
{
	NSTextFieldCell * firstField;
	NSTextFieldCell * secondField;
}

- (void) createCellWithText : (NSString*) string forElem : (uint) cellID;

- (NSColor *) getFontColor : (uint) cellID;
- (NSTextFieldCell*) getCellForID : (uint) cellID;
- (void)drawCell:(uint)cellID inFrame:(NSRect)frame withView:(NSView *)controlView;

@end