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

@property MANGAS_DATA project;
@property BOOL isTome;
@property int selection;

- (void) setDataProject : (MANGAS_DATA) project isTome : (BOOL) isTome element : (int) element;
- (id) initWithData : (NSData *) data;

- (NSData *) getData;
- (BOOL) defineIsTomePriority : (MANGAS_DATA*) project  alreadyRefreshed : (BOOL) refreshed;

@end
