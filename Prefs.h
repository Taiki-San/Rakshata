//
//  Prefs.h
//  Interface
//
//  Created by Taiki on 29/12/2013.
//  Copyright (c) 2013 Taiki. All rights reserved.
//

#import <Foundation/Foundation.h>
#include "../../../Sources/graphics.h"

@interface Prefs : NSObject

+ (void) initCache;
+ (void) rebuildCache;
+ (void) clearCache;
+ (void *) getPref : (int) request;

@end

/*Codes servant à identifier les requêtes*/
#define PREFS_GET_TAB_SERIE_WIDTH 1

/*Divers constantes utilisées un peu partout mais renvoyés par Prefs*/
#define TAB_SERIE_INACTIVE_CT				200
#define TAB_SERIE_INACTIVE_LECTEUR			200
#define TAB_SERIE_INACTIVE_LECTEUR_REDUCED	50