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

#define PROJECT_PASTEBOARD_TYPE @"PasteboardTypeRakshataProject"

@interface RakDragItem : NSObject
{
	BOOL canDL;
}

@property PROJECT_DATA project;
@property BOOL isTome;
@property int selection;
@property uint price;

- (void) setDataProject : (PROJECT_DATA) project isTome : (BOOL) isTome element : (int) element;
- (id) initWithData : (NSData *) data;

- (NSData *) getData;
- (BOOL) canDL;
+ (BOOL) canDL : (NSPasteboard*) pasteboard;
+ (BOOL) canDL : (PROJECT_DATA) project isTome : (BOOL) isTome element : (int) element;
+ (BOOL) defineIsTomePriority : (PROJECT_DATA*) project  alreadyRefreshed : (BOOL) refreshed;

@end
