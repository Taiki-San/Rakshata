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

#define AntiARCRetain(...) do { void *retainedThing = (__bridge_retained void *)__VA_ARGS__; retainedThing = retainedThing; } while(0)
#define AntiARCRelease(...) do { void *retainedThing = (__bridge void *) __VA_ARGS__; id unretainedThing = (__bridge_transfer id)retainedThing; unretainedThing = nil; } while(0)

NSString * getStringForWchar(const charType * string);
NSString * getStringForCTID(int ID);
NSString * getStringForChapter(uint chapID);
NSString * getStringForVolume(int volID);
NSString * getStringForVolumeFull(META_TOME tome);
NSNumber * getNumberForString(NSString * string);
NSString * getRepoName(REPO_DATA * repo);

void registerDefaultForExtension(NSString * extension);

@interface NSApplication (contextSave)

@property NSArray * savedContext;

@end

@interface RakFlippedView : NSView
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

@interface NSView (RakUtils)

- (NSView *) findSubviewAtCoordinate : (NSPoint) coordinates;
- (NSImage *) imageOfView;

@end

@interface NSMenuItem (AutoLocalization)

- (void) setAutoLocalizedString : (NSString *) string;

@end

@interface NSMenu (AutoLocalization)

- (void) setAutoLocalizedString : (NSString *) string;

@end

#import "RakWindow.h"
#import "RakSheetView.h"
#import "RakAnimationController.h"

#import "NSImage+Tint.h"
#import "RakTextCell.h"
#import "RakText.h"
#import "RakButton.h"
#import "RakScroller.h"
#import "RakBorder.h"
#import "RakSegmentedButtonCell.h"
#import "RakMenuText.h"
#import "RakProgressBar.h"
#import "RakBackButton.h"
#import "RakPopover.h"
#import "RakProjectView.h"

@class RakProjectMenu;


#import "RakAuthForegroundView.h"
#import "RakTabView.h"
#import "RakTabContentTemplate.h"
#import "RakDragView.h"
#import "RakDragItem.h"
#import "RakDragResponder.h"
#import "RakImageDropArea.h"
#import "RakList.h"
#import "RakGradientView.h"

#import "RakDBUpdate.h"
#import "RakSuggestionEngine.h"

#import "RakCustomWindow.h"
#import "RakAboutWindow.h"
#import "RakPrefsWindow.h"
#import "RakAddRepoController.h"

#import "RakEIStatusController.h"

#define NSCenteredRect(base, content) NSMakePoint(base.origin.x + base.size.width / 2 - content.size.width / 2, base.origin.y + base.size.height / 2 - content.size.height / 2)
#define NSCenterPoint(base, content) NSMakePoint(base.size.width / 2 - content.size.width / 2, base.size.height / 2 - content.size.height / 2)
#define NSCenterSize(base, content) NSMakePoint(base.width / 2 - content.width / 2, base.height / 2 - content.height / 2)
#define NSCenteredViews(parent, subview) NSCenteredRect(parent.bounds, subview.bounds)

//Thumbnail update management
void registerThumbnailUpdate(id object, SEL selector, byte updateType);
void flushBundleCache(NSBundle *bundle);
void invalidateCacheForRepoID(uint64_t repoID);

void exportImageToPath(NSImage * image, NSSize size, NSSize pixelSize,  NSString * outputPath);

//Image loading
NSSize getThumbSize(NSImage * image);
NSImage * loadImageForRepo(BOOL isRoot, void* repo);
NSImage * loadCTHeader(const PROJECT_DATA project);
NSImage * loadCTThumb(const PROJECT_DATA project);
NSImage * loadDDThumbnail(const PROJECT_DATA project);
NSImage * loadImageGrid(const PROJECT_DATA project);

NSImage * loadCTHeaderWithoutDefault(const PROJECT_DATA project);
NSImage * loadDDThumbnailWithoutDefault(const PROJECT_DATA project);
NSImage * loadImageGridWithoutDefault(const PROJECT_DATA project);

NSSize thumbSizeForID(byte ID);
