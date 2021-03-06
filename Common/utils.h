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
 *********************************************************************************************/

#define AntiARCRetain(...) do { void *retainedThing = (__bridge_retained void *)__VA_ARGS__; retainedThing = retainedThing; } while(0)
#define AntiARCRelease(...) do { void *retainedThing = (__bridge void *) __VA_ARGS__; id unretainedThing = (__bridge_transfer id)retainedThing; unretainedThing = nil; } while(0)

NSString * getStringForWchar(const charType * string);
NSString * getStringForCTID(int ID);
NSString * getStringForChapter(uint chapID);
NSString * getStringForVolume(int volID);
NSString * getStringForVolumeFull(META_TOME tome);
NSNumber * getNumberForString(NSString * string);
NSString * getStringForPrice(uint price);
NSString * getRepoName(REPO_DATA * repo);

void registerDefaultForExtension(NSString * extension);

@interface RakApplication (contextSave)

@property NSArray * savedContext;

@end

@interface RakFlippedView : RakView
@end

@interface NSArray (dataConversion)

- (NSData*) convertToData;
+ (NSArray*) arrayWithData:(NSData*) data;

- (NSArray *) revertArray;

@end

@interface NSString (comparaison)

- (BOOL) hasPrefix:(NSString *)str caseInsensitive : (BOOL) caseInsensitive;
- (BOOL) isDirectory;

@end

@interface PDFDocument (dumpData)

- (NSArray <PDFPage *> *) getPages;

@end

#if !TARGET_OS_IPHONE

@interface NSMenuItem (AutoLocalization)

- (void) setAutoLocalizedString : (NSString *) string;

@end

@interface NSMenu (AutoLocalization)

- (void) setAutoLocalizedString : (NSString *) string;

@end

#endif

#if !TARGET_OS_IPHONE
	#import "RakWindow.h"
	#import "RakSheetView.h"

	#import "RakTextComplete.h"
	#import "RakScroller.h"
	#import "RakBorder.h"
	#import "RakProgressBar.h"
	#import "RakBackButton.h"
	#import "RakPopover.h"
	#import "RakProjectView.h"

	@class RakProjectMenu;

	#import "RakAuthForegroundView.h"
#else
	#import "UIImage+Performance.h"
	#import "RakDownloadStatus.h"
#endif

#import "RakTabView.h"
#import "RakTabContentTemplate.h"

#if !TARGET_OS_IPHONE
	#import "RakDragView.h"
	#import "RakDragItem.h"
	#import "RakDragResponder.h"
	#import "RakImageDropArea.h"
	#import "RakList.h"
	#import "RakGradientView.h"
#endif

#import "RakFormatter.h"
#import "RakDBUpdate.h"
#import "RakSuggestionEngine.h"

#import "RakNotification.h"

#if !TARGET_OS_IPHONE
	#import "RakCustomWindow.h"
	#import "RakAboutWindow.h"
	#import "RakPrefsWindow.h"
	#import "RakAddRepoController.h"
#endif

#import "RakEIStatusController.h"

#define NSCenteredRect(base, content) NSMakePoint(base.origin.x + base.size.width / 2 - content.size.width / 2, base.origin.y + base.size.height / 2 - content.size.height / 2)
#define NSCenterPoint(base, content) NSMakePoint(base.size.width / 2 - content.size.width / 2, base.size.height / 2 - content.size.height / 2)
#define NSCenterSize(base, content) NSMakePoint(base.width / 2 - content.width / 2, base.height / 2 - content.height / 2)
#define NSCenteredViews(parent, subview) NSCenteredRect(parent.bounds, subview.bounds)

//Thumbnail update management
void registerThumbnailUpdate(id object, SEL selector, byte updateType);
void flushBundleCache(NSBundle *bundle);
void invalidateCacheForRepoID(uint64_t repoID);

void exportImageToPath(RakImage * image, NSSize size, NSSize pixelSize,  NSString * outputPath);

//Image loading
NSSize getThumbSize(RakImage * image);
RakImage * loadImageForRepo(BOOL isRoot, void* repo);
RakImage * loadCTHeader(const PROJECT_DATA project);
RakImage * loadCTThumb(const PROJECT_DATA project);
RakImage * loadDDThumbnail(const PROJECT_DATA project);
RakImage * loadImageGrid(const PROJECT_DATA project);

RakImage * loadCTHeaderWithoutDefault(const PROJECT_DATA project);
RakImage * loadDDThumbnailWithoutDefault(const PROJECT_DATA project);
RakImage * loadImageGridWithoutDefault(const PROJECT_DATA project);

NSSize thumbSizeForID(byte ID);
