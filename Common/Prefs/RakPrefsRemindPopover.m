/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
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

+ (BOOL) haveAnyRemindedValue
{
	NSArray * data = @[PREFS_REMIND_DELETE, PREFS_REMIND_AUTODL, PREFS_REMIND_FAVS, PREFS_REMIND_SUGGESTION];
	NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
	
	if(defaults == nil)
		return NO;
	
	for(NSString * element in data)
	{
		NSNumber* object = [defaults objectForKey:element];
		
		if(object != nil && [object boolValue])
			return YES;
	}
	
	return NO;
}

+ (void) flushRemindedValues
{
	NSArray * data = @[PREFS_REMIND_DELETE, PREFS_REMIND_AUTODL, PREFS_REMIND_FAVS, PREFS_REMIND_SUGGESTION];
	NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
	
	if(defaults != nil)
	{
		for(NSString * element in data)
			[defaults removeObjectForKey:element];
	}
}

@end
