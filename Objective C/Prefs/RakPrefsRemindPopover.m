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

@implementation RakPrefsRemindPopover

+ (void) setValueReminded : (NSString *) element : (BOOL) value
{
	NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
	
	[defaults setBool:value forKey:element];
}

+ (BOOL) getValueReminded : (NSString *) element : (BOOL *) value
{
	if(value == NULL)
		return NO;
	
	NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];

	NSNumber* object = [defaults objectForKey:element];
	
	if(object == nil)
		return NO;
	
	*value = [object boolValue];
	return YES;
}

+ (void) removeValueReminded : (NSString *) element
{
	NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
	[defaults removeObjectForKey:element];
}

@end
