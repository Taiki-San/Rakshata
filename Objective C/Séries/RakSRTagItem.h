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

@class RakSRTagRail;

@interface RakSRTagItem : NSView
{
	RakText * label;
	RakButton * close;
}

@property byte _dataType;
@property uint _dataID;

@property uint index;
@property (weak) RakSRTagRail * parent;

- (instancetype) initWithTag : (NSString *) tagName;
- (void) updateContent : (NSString *) newTagName;

@end

enum
{
	TAG_BUTTON_HEIGHT = 25,
	TAG_BUTTON_MIDDLE = 13,
	TAG_BUTTON_ARROW_DEPTH = 12,
	TAG_BORDER_LEFT_LABEL = 3,
	TAG_BORDER_RIGHT_LABEL = 5
};

