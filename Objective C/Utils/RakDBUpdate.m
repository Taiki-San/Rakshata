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

#define NOTIFICATION_NAME	@"RakDatabaseGotUpdated"
#define REPO_FIELD			@"repo"
#define PROJECT_FIELD		@"project"
#define UNUSED_FIELD		UINT_MAX

@implementation RakDBUpdate

#pragma mark - Manage registration

+ (void) registerForUpdate : (id) instance : (SEL) selector
{
	if(instance != nil)
		[[NSNotificationCenter defaultCenter] addObserver:instance selector:selector name:NOTIFICATION_NAME object:nil];
}

+ (void) unRegister : (id) instance
{
	if(instance != nil)
		[[NSNotificationCenter defaultCenter] removeObserver:instance];
}

#pragma mark - Post notification

+ (void) postNotificationFullUpdate
{
	[self postNotification:UNUSED_FIELD :UNUSED_FIELD];
}

+ (void) postNotificationRepoUpdate : (REPO_DATA) repo
{
	[self postNotification : getRepoID(&repo) :UNUSED_FIELD];
}

+ (void) postNotificationProjectUpdate : (PROJECT_DATA) project
{
	[self postNotification:UNUSED_FIELD :project.cacheDBID];
}

+ (void) postNotification : (uint64_t) repoID : (uint) projectID
{
	[[NSNotificationCenter defaultCenter] postNotificationName: NOTIFICATION_NAME object:nil userInfo: @{REPO_FIELD:@(repoID), PROJECT_FIELD:@(projectID)}];
}

#pragma mark - Analyse notification

+ (BOOL) analyseNeedUpdateProject : (NSDictionary*) notification : (PROJECT_DATA) project
{
	if(notification == nil)
		return NO;
	
	NSNumber * repoIDObj = [notification objectForKey:REPO_FIELD], * projectIDObj = [notification objectForKey:PROJECT_FIELD];

	if(repoIDObj == nil || projectIDObj == nil)
		return NO;
	
	uint64_t repoID = [repoIDObj unsignedLongLongValue], projectID = [projectIDObj unsignedIntValue];
	
	if(repoID == UNUSED_FIELD && projectID == UNUSED_FIELD)			//Full update
		return YES;
	
	if(projectID != UNUSED_FIELD && projectID == project.cacheDBID)	//Project update
		return YES;
	
	if(repoID != UNUSED_FIELD && repoID == getRepoID(project.repo))	//Team update
		return YES;
	
	return NO;
}

+ (BOOL) getIDUpdated : (NSDictionary*) notification : (uint*) ID
{
	if(notification == nil || ID == NULL)
		return NO;
	
	NSNumber * val = [notification objectForKey:PROJECT_FIELD];
	if(val == nil)
		return NO;

	uint localID = [val unsignedIntValue];
	if(localID == UNUSED_FIELD)
		return NO;
	
	*ID = localID;
	return YES;
}

+ (BOOL) isPluralUpdate : (NSDictionary *) notification
{
	if(notification == nil)
		return NO;
	
	NSNumber * val = [notification objectForKey:PROJECT_FIELD];
	if(val == nil)
		return NO;
	
	uint localID = [val unsignedIntValue];
	return localID == UNUSED_FIELD;
}

@end
