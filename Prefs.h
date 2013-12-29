//
//  Prefs.h
//  Interface
//
//  Created by Taiki on 29/12/2013.
//  Copyright (c) 2013 Taiki. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface Prefs : NSObject

+ (void) initCache;
+ (void) rebuildCache;
+ (void) clearCache;
+ (void *) getPref : (int) request;

@end
