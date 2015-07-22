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

#define PROJECT_PASTEBOARD_TYPE @"com.taiki.rakshataProject"

@interface RakDragItem : NSObject
{
	BOOL canDL;
}

@property PROJECT_DATA project;
@property BOOL isTome, fullProject;
@property int selection;
@property uint price;

- (void) setDataProject : (PROJECT_DATA) project fullProject : (BOOL) fullProject isTome : (BOOL) isTome element : (int) element;
- (instancetype) initWithData : (NSData *) data;

- (NSData *) getData;
- (BOOL) isFavorite;
+ (BOOL) isFavorite : (NSPasteboard*) pasteboard;
- (BOOL) canDL;
- (BOOL) canDLRefreshed;
+ (BOOL) canDL : (NSPasteboard*) pasteboard;
+ (BOOL) canDL : (PROJECT_DATA) project isTome : (BOOL) isTome element : (int) element;
+ (BOOL) defineIsTomePriority : (PROJECT_DATA*) project  alreadyRefreshed : (BOOL) refreshed;

@end
