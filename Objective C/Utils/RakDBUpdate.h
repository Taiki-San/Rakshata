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

@interface RakDBUpdate : NSObject

+ (void) registerForUpdate : (id) instance : (SEL) selector;
+ (void) unRegister : (id) instance;

+ (void) postNotificationFullUpdate;
+ (void) postNotificationTeamUpdate : (TEAMS_DATA) team;
+ (void) postNotificationProjectUpdate : (PROJECT_DATA) project;
+ (void) postNotification : (uint) teamID : (uint) projectID;

+ (BOOL) analyseNeedUpdateProject : (NSDictionary*) notification : (PROJECT_DATA) project;
+ (BOOL) getIDUpdated : (NSDictionary*) notification : (uint*) ID;
+ (BOOL) isPluralUpdate : (NSDictionary *) notification;

@end
