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

/*****************************************
 **										**
 **				  GENERAL				**
 **										**
 *****************************************/

#define LAUNCH_CRASH_FILE "crashAtLaunch"

void createCrashFile()
{
	if(checkFileExist(LAUNCH_CRASH_FILE))
	{
		NSAlert * alert = [[NSAlert alloc] init];
		
		if(alert != nil)
		{
			alert.alertStyle = NSCriticalAlertStyle;
			alert.messageText = NSLocalizedString(@"CRASH-TITLE", nil);
			
			if(checkFileExist(CONTEXT_FILE) || checkFileExist(SETTINGS_FILE))
			{
				alert.informativeText = NSLocalizedString(@"CRASH-CONTENT", nil);
				[alert addButtonWithTitle:NSLocalizedString(@"YES", nil)];
				[alert addButtonWithTitle:NSLocalizedString(@"CRASH-NUKE", nil)];
				[alert addButtonWithTitle:NSLocalizedString(@"NO", nil)];
				
				NSModalResponse response = [alert runModal];
				
				//Only remove context file
				if(response == -NSModalResponseStop && checkFileExist(CONTEXT_FILE))
				{
					remove(CONTEXT_FILE".crashed");
					rename(CONTEXT_FILE, CONTEXT_FILE".crashed");
				}
				
				//Deeper flush, remove context but also settings
				else if(response == -NSModalResponseAbort)
				{
					if(checkFileExist(CONTEXT_FILE))
					{
						remove(CONTEXT_FILE".crashed");
						rename(CONTEXT_FILE, CONTEXT_FILE".crashed");
					}
					
					if(checkFileExist(SETTINGS_FILE))
					{
						remove(SETTINGS_FILE".crashed");
						rename(SETTINGS_FILE, SETTINGS_FILE".crashed");
					}
					
					removeFolder(IMAGE_CACHE_DIR);
				}
			}
			else
			{
				alert.informativeText = NSLocalizedString(@"CRASH-CONTENT-TOTAL", nil);
				[alert addButtonWithTitle:NSLocalizedString(@"OK", nil)];
				
				[alert runModal];
			}
		}
	}
	
	FILE * file = fopen(LAUNCH_CRASH_FILE, "w+");
	
	if(file != NULL)
		fclose(file);
}

void sendCrashedFilesHome()
{
	const byte nbEntries = 3, minValid = 2;
	char * filename[] = {CONTEXT_FILE".crashed", SETTINGS_FILE".crashed", SETTINGS_FILE};
	char * argumentName[] = {"context", "settings", "settings"};
	
	char * output = NULL;
	size_t outputLength = 150, currentPos = 0;
	
	for(uint i = 0, validCount = 0; i < nbEntries && validCount < minValid; i++)
	{
		if(checkFileExist(filename[i]))
		{
			size_t fileSize = getFileSize(filename[i]);
			byte dataField[fileSize];
			
			FILE * file = fopen(filename[i], "rb");
			if(file != NULL)
			{
				fileSize = fread(dataField, 1, fileSize, file);
				fclose(file);
				
				char encodedData[2 * fileSize + 1];
				decToHex(dataField, fileSize, encodedData);
				
				outputLength += 2 * fileSize + strlen(argumentName[i]) + 2;
				void * tmp = realloc(output, outputLength * sizeof(char));
				if(tmp != NULL)
				{
					output = tmp;
					
					int length = snprintf(&(output[currentPos]), 2 * fileSize + 150, "%s%s=%s", currentPos != 0 ? "&" : "", argumentName[i], encodedData);
					if(length >= 0)
						currentPos += (uint) length;
				}
				
				fclose(file);
				validCount++;
			}
		}
	}
	
	if(output != NULL)
	{
		char * bufferOut = NULL;
		size_t bufferLengthOut;
		
		download_mem(SERVEUR_URL"/crashRecovery.php", output, &bufferOut, &bufferLengthOut, true);
		
		if(bufferOut != NULL)
			free(bufferOut);
		
		free(output);
		
		for(uint i = 0; i < nbEntries; remove(filename[i++]));
	}
}

void deleteCrashFile()
{
	remove(LAUNCH_CRASH_FILE);
	
	if(checkFileExist(CONTEXT_FILE".crashed") || checkFileExist(SETTINGS_FILE".crashed"))
	{
		NSAlert * alert = [[NSAlert alloc] init];
		
		if(alert != nil)
		{
			alert.alertStyle = NSInformationalAlertStyle;
			alert.messageText = NSLocalizedString(@"CRASH-RECOVERED-TITLE", nil);
			
			alert.informativeText = NSLocalizedString(@"CRASH-RECOVERED-CONTENT", nil);
			[alert addButtonWithTitle:NSLocalizedString(@"YES", nil)];
			[alert addButtonWithTitle:NSLocalizedString(@"NO", nil)];
			
			if([alert runModal] == -NSModalResponseStop)
			{
				dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
					sendCrashedFilesHome();
				});
			}
		}
	}
}

/*****************************************
**										**
**				  PREFS					**
**										**
*****************************************/

uint32_t getMainThread()
{
	uint32_t output;
	
	[Prefs getPref:PREFS_GET_MAIN_THREAD :&output];
	
	return output;
}

bool shouldDownloadFavorite()
{
	bool output = true;
	
	[Prefs getPref:PREFS_GET_FAVORITE_AUTODL :&output];
	
	return output;
}

void sendToLog(const char * string)
{
	NSLog(@"%s", string);
}

#ifdef VERBOSE_DB_MANAGEMENT
void logStack(void * address)
{
	[[NSString stringWithFormat:@"%@", [NSThread callStackSymbols]] writeToFile:[NSString stringWithFormat:@"log/%p.txt", address] atomically:NO encoding:NSASCIIStringEncoding error:nil];
}
#endif

void notifyEmailUpdate()
{
	[[NSNotificationCenter defaultCenter] postNotificationName:NOTIFICATION_MAIL object:nil];
}

void restorePrefsFile()
{
	NSString * settings = [[NSBundle mainBundle] pathForResource:@"settings" ofType:@""];
	
	if(settings != nil)
		[[NSData dataWithContentsOfFile:settings] writeToFile:@""SETTINGS_FILE"" options:NSDataWritingAtomic error:nil];
}

bool removeProjectWithContent()
{
	return false;
}

/*****************************************
 **										**
 **				 DB update				**
 **										**
 *****************************************/

void notifyFullUpdate()
{
	[RakDBUpdate postNotificationFullUpdate];
}

void notifyUpdateRepo(REPO_DATA repo)
{
	[RakDBUpdate postNotificationRepoUpdate:getRepoID(&repo)];
}

void notifyUpdateRootRepo(ROOT_REPO_DATA root)
{
	[RakDBUpdate postNotificationRepoUpdate:root.repoID];
}

void notifyUpdateProject(PROJECT_DATA project)
{
	[RakDBUpdate postNotificationProjectUpdate:project];
}

void notifyFullUpdateRepo()
{
	[RakDBUpdate postNotificationFullRepoUpdate];
}

/*****************************************
 **										**
 **			  Thumbnail update			**
 **										**
 *****************************************/

NSString * thumbNotificationName(byte updateType)
{
	switch (updateType)
	{
		case THUMBID_SRGRID:
			return NOTIFICATION_THUMBNAIL_UPDATE_SRGRID;
			
		case THUMBID_HEAD:
			return NOTIFICATION_THUMBNAIL_UPDATE_HEAD;
			
		case THUMBID_CT:
			return NOTIFICATION_THUMBNAIL_UPDATE_CT;
	}
	
	return nil;
}

void notifyThumbnailUpdate(ICONS_UPDATE * payload)
{
	if(payload == NULL)
		return;
	
	if(payload->next != NULL)
	{
		ICONS_UPDATE * next = payload->next;
		
		//The next entry is exactly the @2x version
		if(next->updateType == payload->updateType && next->projectID == payload->projectID && next->repoID == payload->repoID && strcmp(next->crc32, payload->crc32))
			return;
	}
	
	NSString * notificationName = thumbNotificationName(payload->updateType);
	
	if(notificationName != nil)
	{
		uint projectID = payload->projectID;
		uint64_t repoID = payload->repoID;
		
		dispatch_async(dispatch_get_main_queue(), ^{
			
			invalidateCacheForRepoID(repoID);
			[[NSNotificationCenter defaultCenter] postNotificationName:notificationName object:nil userInfo:@{@"project" : @(projectID), @"source" : @(repoID)}];
		});
	}
}

void registerThumbnailUpdate(id object, SEL selector, byte updateType)
{
	NSString * notificationName = thumbNotificationName(updateType);
	
	if(notificationName != nil)
		[[NSNotificationCenter defaultCenter] addObserver:object selector:selector name:notificationName object:nil];
}

/*****************************************
 **										**
 **			Restrictions update			**
 **										**
 *****************************************/

void notifyRestrictionChanged()
{
	[[NSNotificationCenter defaultCenter] postNotificationName: NOTIFICATION_SEARCH_UPDATED object:nil userInfo: nil];
}

/*****************************************
 **										**
 **				  Series				**
 **										**
 *****************************************/

void updateRecentSeries()
{
	[[NSNotificationCenter defaultCenter] postNotificationName: @"RakSeriesNeedUpdateRecent" object:nil userInfo: nil];
}

/*****************************************
 **										**
 **				    MDL					**
 **										**
 *****************************************/

bool checkIfElementAlreadyInMDL(PROJECT_DATA data, bool isTome, uint element)
{
	MDL * tabMDL = [(RakAppDelegate*) [NSApp delegate] MDL];

	return tabMDL == nil ? [tabMDL proxyCheckForCollision:data :isTome :element] : false;
}

void addElementToMDL(PROJECT_DATA data, bool isTome, uint element, bool partOfBatch)
{
	MDL * tabMDL = [(RakAppDelegate*) [NSApp delegate] MDL];
	
	if(tabMDL != nil)
		[tabMDL proxyAddElement:data isTome:isTome element:element partOfBatch:partOfBatch];
}

void notifyDownloadOver()
{
	if(((RakAppDelegate *) [NSApp delegate]).hasFocus)
		return;
	
	NSUserNotification *notification = [[NSUserNotification alloc] init];
	if(notification != nil)
	{
		notification.title = NSLocalizedString(@"MDL-DLOVER-NOTIF-NAME", nil);
		notification.informativeText = NSLocalizedString(@"MDL-DLOVER-NOTIF-CONTENT", nil);
		
		[[NSUserNotificationCenter defaultUserNotificationCenter] deliverNotification:notification];
	}
}

/*****************************************
 **										**
 **				   Proxy				**
 **										**
 *****************************************/

#import <SystemConfiguration/SystemConfiguration.h>

bool getSystemProxy(char ** _proxyAddress)
{
	CFDictionaryRef proxies = SCDynamicStoreCopyProxies(NULL);
	
	const void * isActivated;
	bool isActivatedValue;
	
	CFStringRef constants [4][2] = {{kSCPropNetProxiesSOCKSEnable, kSCPropNetProxiesHTTPSEnable},
									{kSCPropNetProxiesSOCKSProxy, kSCPropNetProxiesHTTPSProxy},
									{kSCPropNetProxiesSOCKSPort, kSCPropNetProxiesHTTPSPort},
									{CFSTR("socks5://%@:%@"), CFSTR("https://%@:%@")}};

	//SOCKS then HTTPS
	for(byte i = 0; i < 2; ++i)
	{
		//Check if
		if((isActivated = CFDictionaryGetValue(proxies, constants[0][i])) != NULL)
		{
			if(CFNumberGetValue(isActivated, kCFNumberCharType, &isActivatedValue) && isActivatedValue)
			{
				const CFStringRef * proxyAddress = CFDictionaryGetValue(proxies, constants[1][i]);
				const CFNumberRef * proxyPort = CFDictionaryGetValue(proxies, constants[2][i]);
				const char * tmpProxyAddress;
				
				if(proxyAddress != NULL && proxyPort != NULL)
				{
					CFStringRef proxy = CFStringCreateWithFormat(NULL, NULL, constants[3][i], proxyAddress, proxyPort);
					
					if(proxyAddress != NULL && (tmpProxyAddress = CFStringGetCStringPtr(proxy, kCFStringEncodingUTF8)) != NULL)
						*_proxyAddress = strdup(tmpProxyAddress);
					
					CFRelease(proxy);
					CFRelease(proxies);
					return true;
				}
			}
		}
	}

	CFRelease(proxies);
	return false;
}
