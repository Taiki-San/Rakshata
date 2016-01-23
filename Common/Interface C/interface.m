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
 *********************************************************************************************/

/*****************************************
 **										**
 **				  GENERAL				**
 **										**
 *****************************************/

static bool wasSandboxed, checkedSandbox = false;

bool isSandboxed()
{
	if(!checkedSandbox)
	{
#if TARGET_OS_IPHONE
		wasSandboxed = true;
#else
		wasSandboxed = [[[[NSFileManager alloc] init] currentDirectoryPath] hasSuffix:[NSString stringWithFormat:@"/Library/Containers/%@/Data", [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleIdentifier"]]];
#endif
		checkedSandbox = true;
	}
	
	return wasSandboxed;
}

void configureSandbox()
{
#if !TARGET_OS_IPHONE
	if(isSandboxed())
		return;
	
	NSArray * searchPath = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
	if(searchPath == nil || [searchPath count] == 0)
		return;

	NSString * identifier = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleIdentifier"];
	NSString * container = [NSString stringWithFormat:@"%@/Containers/%@/Data/", [searchPath firstObject], identifier];
	BOOL isDir = NO;
		
	if(![[[NSFileManager alloc] init] fileExistsAtPath:container isDirectory:&isDir] || !isDir)
	{
		if(!isDir)
			remove([container UTF8String]);
		
		//We need to create the sandbox directory somehow
		createPath([container UTF8String]);
	}
	
	if(![[NSFileManager defaultManager] changeCurrentDirectoryPath:container])
		NSLog(@"Couldn't change directory to standard sandbox directory!");
#else
	[[NSFileManager defaultManager] changeCurrentDirectoryPath:[NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0]];
	
//	remove(SETTINGS_FILE);
#ifdef DEV_VERSION
	if(!checkFileExist(SETTINGS_FILE))
		[[NSData dataWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"settings" ofType:@"debug"]] writeToFile:@""SETTINGS_FILE"" options:NSDataWritingAtomic error:nil];
#endif
#endif
}

void registerExtensions()
{
#if !TARGET_OS_IPHONE
	if(isSandboxed())
		return;
	
	for(NSString * extension in DEFAULT_ARCHIVE_SUPPORT)
		registerDefaultForExtension(extension);
#endif
}

int getBuildID()
{
	return [[[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleVersion"] integerValue];
}

void openWebsite(const char * URL)
{
	NSURL * _URL = [NSURL URLWithString: [NSString stringWithUTF8String : URL]];

#if TARGET_OS_IPHONE
	[RakRealApp openURL:_URL];
#else
	[[NSWorkspace sharedWorkspace] openURL:_URL];
#endif
}

uint getActiveProjectForTab(uint32_t tabID)
{
	if(tabID == TAB_CT && RakApp.CT != nil)
		return [RakApp.CT activeProject].cacheDBID;
	
	if(tabID == TAB_READER && RakApp.reader != nil)
		return RakApp.reader.activeProject.cacheDBID;
	
	return INVALID_VALUE;
}

/*****************************************
 **										**
 **				   ERROR				**
 **										**
 *****************************************/

#define LAUNCH_CRASH_FILE "crashAtLaunch"
void createCrashFileCallBackWithResponse(BOOL haveResponse, NSInteger response);

void createCrashFile()
{
	if(checkFileExist(LAUNCH_CRASH_FILE))
	{
		BOOL partialFuckUp = checkFileExist(CONTEXT_FILE) || checkFileExist(SETTINGS_FILE);
		NSString * message;
		NSArray <NSString *> * buttons;
		
		if(partialFuckUp)
		{
			message = NSLocalizedString(@"CRASH-CONTENT", nil);
			buttons = @[NSLocalizedString(@"YES", nil), NSLocalizedString(@"CRASH-NUKE", nil), NSLocalizedString(@"NO", nil)];
		}
		else
		{
			message = NSLocalizedString(@"CRASH-CONTENT-TOTAL", nil);
			buttons = @[NSLocalizedString(@"OK", nil)];
		}

#if !TARGET_OS_IPHONE
		NSAlert * alert = [[NSAlert alloc] init];
		if(alert != nil)
		{
			alert.alertStyle = NSCriticalAlertStyle;
			alert.messageText = NSLocalizedString(@"CRASH-TITLE", nil);
			alert.informativeText = message;
			
			for(NSString * button in buttons)
			{
				[alert addButtonWithTitle:button];
			}
			
			return createCrashFileCallBackWithResponse(YES, [alert runModal]);
		}
#else
		UIAlertController * alert = [UIAlertController alertControllerWithTitle:NSLocalizedString(@"CRASH-TITLE", nil)
																		message:message
																 preferredStyle:UIAlertControllerStyleAlert];
		if(alert != nil)
		{
			uint count = [buttons count];
			UIAlertActionStyle _styles[3] = {UIAlertActionStyleDefault, UIAlertActionStyleDestructive, UIAlertActionStyleCancel}, *styles = _styles;
			const int _conversionTable[3] = {NSModalResponseStop, NSModalResponseAbort, NSModalResponseContinue}, *conversionTable = _conversionTable;
			
			[buttons enumerateObjectsUsingBlock:^(NSString * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop)
			{
				BOOL isLast = idx == count - 1;
				UIAlertAction * action = [UIAlertAction actionWithTitle:obj
																style:isLast ? styles[2] : styles[idx]
															  handler:^(UIAlertAction * _Nonnull _action)
				{
					for(uint i = 0; i < 3; ++i)
					{
						if(_action.style == styles[i])
						{
							createCrashFileCallBackWithResponse(YES, conversionTable[i]);
							break;
						}
					}
				}];
				
				[alert addAction:action];
			}];
			
			return [[RakRealApp.keyWindow rootViewController] presentViewController:alert animated:YES completion:^{}];
		}
#endif
	}
	
	createCrashFileCallBackWithResponse(NO, 0);
}

void createCrashFileCallBackWithResponse(BOOL haveResponse, NSInteger response)
{
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

	FILE * file = fopen(LAUNCH_CRASH_FILE, "w+");

	if(file != NULL)
		fclose(file);
	
#if !TARGET_OS_IPHONE
	[[BITHockeyManager sharedHockeyManager] configureWithIdentifier:@"68795b31d0a748d990db94ab4744c8c2"];
	[[BITHockeyManager sharedHockeyManager] startManager];
#endif
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
#if TARGET_OS_IPHONE
		UIAlertController * alert = [UIAlertController alertControllerWithTitle:NSLocalizedString(@"CRASH-RECOVERED-TITLE", nil)
																		message:NSLocalizedString(@"CRASH-RECOVERED-CONTENT", nil)
																 preferredStyle:UIAlertControllerStyleAlert];
		if(alert != nil)
		{
			[alert addAction:[UIAlertAction actionWithTitle:NSLocalizedString(@"YES", nil)
													   style:UIAlertActionStyleDefault
													 handler:^(UIAlertAction * _Nonnull action) {
														 dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
															 sendCrashedFilesHome();
														 });
													 }]];
			
			[alert addAction:[UIAlertAction actionWithTitle:NSLocalizedString(@"NO", nil)
													  style:UIAlertActionStyleCancel
													handler:^(UIAlertAction * _Nonnull action) {}
							  ]];
			
			[[RakRealApp.keyWindow rootViewController] presentViewController:alert animated:YES completion:^{}];
		}
#else
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
#endif
	}
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

void alertExit(const char * exitReason)
{
	if(![NSThread isMainThread])
		return dispatch_sync(dispatch_get_main_queue(), ^{	alertExit(exitReason);	});
	
	sendToLog(exitReason);
	
#if TARGET_OS_IPHONE
	UIAlertController * alert = [UIAlertController alertControllerWithTitle:NSLocalizedString(@"MAJOR-FAILURE-TITLE", nil)
																	message:[NSString localizedStringWithFormat:NSLocalizedString(@"MAJOR-FAILURE-MESSAGE-%s", nil), exitReason]
															 preferredStyle:UIAlertControllerStyleAlert];
	if(alert != nil)
	{
		[alert addAction:[UIAlertAction actionWithTitle:NSLocalizedString(@"OK", nil)
												  style:UIAlertActionStyleCancel
												handler:^(UIAlertAction * _Nonnull action) {}]];
		
		[[RakRealApp.keyWindow rootViewController] presentViewController:alert animated:YES completion:^{}];
	}
#else
	NSAlert * alert = [[NSAlert alloc] init];
	
	if(alert != nil)
	{
		alert.alertStyle = NSCriticalAlertStyle;
		alert.messageText = NSLocalizedString(@"MAJOR-FAILURE-TITLE", nil);
		alert.informativeText = [NSString localizedStringWithFormat:NSLocalizedString(@"MAJOR-FAILURE-MESSAGE-%s", nil), exitReason];
		[alert addButtonWithTitle:NSLocalizedString(@"OK", nil)];
		
		[alert runModal];
	}
#endif

	exit(EXIT_FAILURE);
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

bool shouldSuggestFromLastRead()
{
	bool output = false;
	
	[Prefs getPref:PREFS_GET_SUGGEST_FROM_LAST_READ :&output];

	return output;
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

static bool _lockNotifyRestrictionChanged = false;

void setLockStatusNotifyRestrictionChanged(bool lock)
{
	_lockNotifyRestrictionChanged = lock;
}

void notifyRestrictionChanged()
{
	if(!_lockNotifyRestrictionChanged)
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
	MDL * tabMDL = RakApp.MDL;

	return tabMDL == nil ? false : [tabMDL proxyCheckForCollision:data :isTome :element];
}

void addElementToMDL(PROJECT_DATA data, bool isTome, uint element, bool partOfBatch)
{
	if(!data.isInitialized)
		return;
	
	addElementWithIDToMDL(data.cacheDBID, isTome, element, partOfBatch);
}

void addElementWithIDToMDL(uint cacheDBID, bool isTome, uint element, bool partOfBatch)
{
	MDL * tabMDL = RakApp.MDL;
	
	if(tabMDL != nil)
		[tabMDL proxyAddElement:cacheDBID isTome:isTome element:element partOfBatch:partOfBatch];
}

void notifyDownloadOver()
{
	if(RakApp.hasFocus)
		return;
	
#if TARGET_OS_IPHONE
	UILocalNotification * notification = [[UILocalNotification alloc] init];
	if(notification != nil)
	{
		notification.repeatInterval = NSCalendarUnitDay;
		notification.alertTitle = NSLocalizedString(@"MDL-DLOVER-NOTIF-NAME", nil);
		notification.alertBody = NSLocalizedString(@"MDL-DLOVER-NOTIF-CONTENT", nil);
		[notification setFireDate:[NSDate dateWithTimeIntervalSinceNow:1]];
		[notification setTimeZone:[NSTimeZone  defaultTimeZone]];
		
		[RakRealApp scheduleLocalNotification:notification];
	}
#else
	NSUserNotification *notification = [[NSUserNotification alloc] init];
	if(notification != nil)
	{
		notification.title = NSLocalizedString(@"MDL-DLOVER-NOTIF-NAME", nil);
		notification.informativeText = NSLocalizedString(@"MDL-DLOVER-NOTIF-CONTENT", nil);
		
		[[NSUserNotificationCenter defaultUserNotificationCenter] deliverNotification:notification];
	}
#endif
}

/*****************************************
 **										**
 **				   Proxy				**
 **										**
 *****************************************/

#if !TARGET_OS_IPHONE
#import <SystemConfiguration/SystemConfiguration.h>
#endif

bool getSystemProxy(char ** _proxyAddress)
{
#if TARGET_OS_IPHONE
	CFDictionaryRef proxy = CFNetworkCopySystemProxySettings();
	
	const CFStringRef proxyCFstr =	(const CFStringRef) CFDictionaryGetValue(proxy, (const void*) kCFNetworkProxiesHTTPProxy);
	const CFNumberRef portCFnum =	(const CFNumberRef) CFDictionaryGetValue(proxy, (const void*) kCFNetworkProxiesHTTPPort);
	
	if(portCFnum == NULL || portCFnum == NULL)
	{
		CFRelease(proxy);
		return false;
	}

	const uint bufferLength = 2 * (uint) CFStringGetLength(proxyCFstr) + 1;
	char buffer[bufferLength];
	SInt32 port;
	
	if(!CFStringGetCString(proxyCFstr, buffer, (long) bufferLength, kCFStringEncodingUTF8)
	   || !CFNumberGetValue(portCFnum, kCFNumberSInt32Type, &port))
	{
		CFRelease(proxy);
		return false;
	}
	
	CFRelease(proxy);
	
	uint length = strlen(buffer) + 50;
	
	char * output = malloc(length * sizeof(char));
	if(output == NULL)
		return false;
	
	snprintf(output, length, "http://%s:%d", buffer, (uint) port);
	*_proxyAddress = output;
	return true;
#else
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
#endif
}
