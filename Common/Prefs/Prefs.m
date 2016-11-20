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
 **			file, You can obtain one at https://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 ********************************************************************************************/

__strong Prefs * prefsCache;

#ifdef FLUSH_PREFS_PROPERLY
NSMutableDictionary * __strong garbageCollector;
#endif

@interface Prefs ()

- (NSObject <RakPrefsCustomized> *) getCustomPrefs;

@end

@implementation Prefs

+ (void) initCache
{
	if(prefsCache == nil)
	{
		NSLog(@"Trying to automatically create the preference object without a custom object");
		prefsCache = [[Prefs alloc] init];
	}
}

+ (void) initCacheWithProxyClass : (Class) customClass
{
	if(prefsCache != nil && [prefsCache getCustomPrefs] != nil)
		return;
	
	if(![customClass conformsToProtocol:@protocol(RakPrefsCustomized)])
		return;

	prefsCache = [[Prefs alloc] init];
	
	NSObject <RakPrefsCustomized> * customPrefs = [[customClass alloc] init];
	
	[customPrefs initializeContextWithProxy:prefsCache];
	[prefsCache setCustomPrefs:customPrefs];
}

+ (NSString *) dumpPrefs
{
	return prefsCache != nil ? [prefsCache dumpPrefs] : nil;
}

+ (void) deletePrefs
{
	prefsCache = nil;
}

+ (void) setCurrentTheme : (uint) newTheme
{
	if(prefsCache == nil)
		[self initCache];
	
	if(prefsCache.themeCode == newTheme)
		return;
	
	prefsCache.themeCode = newTheme;
}

+ (RakColor*) getSystemColor : (byte) context
{
	if(prefsCache == nil)
		[self initCache];

	NSArray <NSColor *> * themeData = [[prefsCache getCustomPrefs] getColorThemeWithID:[self getCurrentTheme]];

	if(themeData == nil || [themeData count] <= context)
		return nil;

	return [themeData objectAtIndex:context];
}


+ (NSFont *) getFont : (byte) context ofSize : (CGFloat) size
{
	return [[prefsCache getCustomPrefs] getFont:context ofSize:size];
}

+ (void) getPref : (uint) requestID : (void*) outputContainer
{
	[self getPref : requestID : outputContainer : NULL];
}

+ (void) getPref : (uint) requestID : (void*) outputContainer : (void*) additionalData
{
	if(prefsCache == NULL)
		[self initCache];
	
	[prefsCache getPrefInternal : requestID : outputContainer : additionalData];
}

- (void) getPrefInternal : (uint) requestID : (void*) outputContainer : (void*) additionalData
{
	[customPreference getPrefInternal:requestID :outputContainer :additionalData];
}

+ (BOOL) setPref : (uint) requestID atValue: (uint64_t) value
{
	if(prefsCache == nil)
		[self initCache];
	
	return [[prefsCache getCustomPrefs] setPref:requestID atValue:value];
}

+ (void) directQuery : (uint8_t) request : (uint8_t) subRequest : (uint) mainThreadLocal : (uint) stateTabsReaderLocal : (void*) outputContainer
{
	if(prefsCache == NULL)
		[self initCache];
	
	[[prefsCache getCustomPrefs] directQueryInternal:request :subRequest :mainThreadLocal :stateTabsReaderLocal :outputContainer];
}

#pragma mark - KVO

+ (uint) getCurrentTheme
{
	if(prefsCache == nil)
		[self initCache];
	
#ifdef FLUSH_PREFS_PROPERLY
	if(registerForChanges != nil)
	{
		if([garbageCollector objectForKey:[NSString stringWithFormat:@"%@", @((uint64_t) registerForChanges)]] != nil)
		{
			NSLog(@"WTFFF!!!!!\n%@", [NSThread callStackSymbols]);
		}
		[garbageCollector setValue:[NSThread callStackSymbols] forKey:[NSString stringWithFormat:@"%@", @((uint64_t) registerForChanges)]];
	}
#endif
	
	return prefsCache.themeCode;
}

+ (void) registerForChange : (id) object forType : (byte) code
{
	if(prefsCache == nil)
		[self initCache];
	
	if(object != nil)
		[prefsCache addObserver:object forKeyPath:[self getKeyPathForCode:code] options:NSKeyValueObservingOptionNew context:nil];
}

+ (void) deRegisterForChange : (id) object forType : (byte) code
{
	if(code == KVO_THEME)
	{
#ifdef FLUSH_PREFS_PROPERLY
		[garbageCollector removeObjectForKey:[NSString stringWithFormat:@"%@", @((uint64_t) object)]];
#endif
		if(object != nil)
			[prefsCache removeObserver:object forKeyPath:[self getKeyPathForCode:code]];
	}
	
	else if(prefsCache != nil && object != nil)
		[[prefsCache getCustomPrefs] removeObserver:object forKeyPath:[self getKeyPathForCode:code]];
}

+ (NSString *) getKeyPathForCode : (byte) code
{
	if(code == KVO_THEME)
		return @"themeCode";
	
	if(prefsCache == nil)
		[self initCache];
	
	return [[prefsCache getCustomPrefs] getKeyPathForCode:code];
}

/************		Private sections		************/

#ifdef FLUSH_PREFS_PROPERLY
- (instancetype) init
{
	garbageCollector = [NSMutableDictionary new];

	return  [super init];
}
#endif

- (void) setCustomPrefs : (NSObject <RakPrefsCustomized> *) proxy
{
	if(customPreference == nil)
		customPreference = proxy;
}

- (NSObject <RakPrefsCustomized> *) getCustomPrefs
{
	return customPreference;
}

- (NSString *) dumpPrefs
{
	return [customPreference dumpPrefs];
}

- (NSString *) dumpProxyPrefs
{
	return [NSString stringWithFormat:@"\n%d", _themeCode];
}

- (void) flushMemory : (BOOL) memoryError
{
	prefsCache = NULL;
	
	if(memoryError)
		[[NSException exceptionWithName:@"NotEnoughMemory"
								 reason:@"We didn't had enough memory to do the job, sorry =/" userInfo:nil] raise];
}

#ifdef FLUSH_PREFS_PROPERLY
- (void) dealloc
{
	NSLog(@"%@", garbageCollector);
}
#endif

@end
