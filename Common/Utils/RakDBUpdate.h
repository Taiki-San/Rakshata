/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 ********************************************************************************************/

@interface RakDBUpdate : NSObject

+ (void) registerForUpdate : (id) instance : (SEL) selector;
+ (void) unRegister : (id) instance;

+ (void) postNotificationFullUpdate;
+ (void) postNotificationRepoUpdate : (uint64_t) repoID;
+ (void) postNotificationFullRepoUpdate;
+ (void) postNotificationProjectUpdate : (PROJECT_DATA) project;
+ (void) postNotification : (uint64_t) repoID : (uint) projectID;

+ (BOOL) analyseNeedUpdateProject : (NSDictionary*) notification : (PROJECT_DATA) project;
+ (BOOL) getIDUpdated : (NSDictionary*) notification : (uint*) ID;
+ (BOOL) isPluralUpdate : (NSDictionary *) notification;
+ (BOOL) isProjectUpdate : (NSDictionary *) notification;

+ (BOOL) getUpdatedRepo : (NSDictionary *) notification : (uint64_t *) ID;
+ (BOOL) isFullRepoUpdate : (NSDictionary *) notification;

@end
