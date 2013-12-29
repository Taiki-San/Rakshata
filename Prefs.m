//
//  Prefs.m
//  Interface
//
//  Created by Taiki on 29/12/2013.
//  Copyright (c) 2013 Taiki. All rights reserved.
//

#import "Prefs.h"

@implementation Prefs

+ (void) initCache
{
	//We'll have to cache the old encrypted prefs /!\ prefs de crypto à protéger!!!
	//Also, need to get the open prefs including tabs size, theme and various stuffs
}

+ (void) rebuildCache
{
	
}

+ (void) clearCache
{
	
}

+ (void *) getPref : (int) request
{
	return (void *) 300;
}

@end
