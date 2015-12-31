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
 ********************************************************************************************/

@implementation RakReaderCore

- (void) sharedInit
{
	[RakDBUpdate registerForUpdate:self :@selector(DBUpdated:)];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(resumeReading:) name:NOTIFICATION_RESUME_READING object:nil];

	pthread_mutex_init(&cacheMutex, NULL);
}

- (void) deallocProcessing
{
	[RakDBUpdate unRegister : self];
	[self flushCache];
	MUTEX_DESTROY(cacheMutex);
}

- (void) flushCache
{
	
}

#pragma mark - Context update

- (void) restoreProject : (PROJECT_DATA) project withInsertionPoint : (NSDictionary *) insertionPoint
{
	STATE_DUMP savedState;
 
	if(insertionPoint != nil && [insertionPoint isKindOfClass:[NSDictionary class]] && [[insertionPoint objectForKey:@"isTome"] isKindOfClass:[NSNumber class]])
	{
		savedState = _recoverStateForProject(project, true, [[insertionPoint objectForKey:@"isTome"] boolValue]);
		
		NSNumber * ID = [insertionPoint objectForKey:@"ID"];
		if(ID != nil && [ID isKindOfClass:[NSNumber class]])
		{
			uint insertionPointID = [ID unsignedIntValue];
			
			if(!savedState.isInitialized || savedState.CTID != insertionPointID)
			{
				bool oldIsTome = savedState.isTome;
				
				savedState = getEmptyRecoverState();
				savedState.isInitialized = true;
				savedState.isTome = oldIsTome;
				savedState.CTID = insertionPointID;
				savedState.zoom = 1.0;
				savedState.scrollerX = CGFLOAT_MAX;
			}
		}
	}
	else
		savedState = recoverStateForProject(project);
	
	if(savedState.isInitialized)
	{
		[self preProcessStateRestoration:savedState project:project];
		
		[self startReading: project: savedState.CTID: savedState.isTome : savedState.page];

		[self postProcessStateRestoration:savedState];
	}
}

- (BOOL) startReading : (PROJECT_DATA) project : (uint) elemToRead : (BOOL) isTome : (uint) startPage
{
	BOOL shouldNotifyBottomBarInitialized = NO;
	
	initialized = YES;
	
	if(self.initWithNoContent)
	{
		self.initWithNoContent = NO;
		
		if(![self initPage: project: elemToRead: isTome : startPage])	//Failed at initializing, most probably because of unreadable data
		{
			self.initWithNoContent = YES;
			return shouldNotifyBottomBarInitialized;
		}
		
		shouldNotifyBottomBarInitialized = YES;
	}
	else
		[self changeProject : project : elemToRead : isTome : startPage];

	return shouldNotifyBottomBarInitialized;
}

- (void) preProcessStateRestoration : (STATE_DUMP) savedState project : (PROJECT_DATA) project
{
	
}

- (void) postProcessStateRestoration : (STATE_DUMP) savedState
{
	
}

- (BOOL) initPage : (PROJECT_DATA) dataRequest : (uint) elemRequest : (BOOL) isTomeRequest : (uint) startPage
{
	if(![self initialLoading:dataRequest :elemRequest :isTomeRequest : startPage])
		return NO;
	
	addRecentEntry(dataRequest, false);
	return YES;
}

- (BOOL) initialLoading : (PROJECT_DATA) dataRequest : (uint) elemRequest : (BOOL) isTomeRequest : (uint) startPage
{
	_project = getCopyOfProjectData(dataRequest);
	_currentElem = elemRequest;
	self.isTome = isTomeRequest;
	
	_posElemInStructure = reader_getPosIntoContentIndex(_project, _currentElem, self.isTome);
	if(_posElemInStructure == INVALID_VALUE)
		return NO;
	
	return YES;
}

- (void) changeProject : (PROJECT_DATA) projectRequest : (uint) elemRequest : (BOOL) isTomeRequest : (uint) startPage
{
	
}

- (void) DBUpdated : (NSNotification*) notification
{
	if([RakDBUpdate analyseNeedUpdateProject:notification.userInfo :_project])
	{
		PROJECT_DATA project = getProjectByID(_project.cacheDBID);
		if(project.isInitialized)
		{
			releaseCTData(_project);
			_project = project;
			_posElemInStructure = reader_getPosIntoContentIndex(_project, _currentElem, self.isTome);
			
			[self postProcessingDBUpdated];
		}
		else if(!checkProjectStillExist(_project.cacheDBID))
		{
			releaseCTData(_project);
			_project = getEmptyProject();
			self.initWithNoContent = YES;
		}
	}
}

- (void) postProcessingDBUpdated
{
	
}

#pragma mark - Page loading

- (RakImageView *) getImage : (uint) page : (DATA_LECTURE*) data : (BOOL *) isPDF
{
	if(data->path == NULL || page >= data->nbPage)
		return nil;
	
	NSData * imageData;
	BOOL _isPDF = haveSuffixCaseInsensitive(data->nomPages[page], ".pdf");
	//PDF
	if(_isPDF)
		imageData = [self getPDF:page :data];
	else
		imageData = [self getPage : page : data];
	
	if(imageData == nil || imageData.length == 0)
		return nil;
	
	RakImage * image = [[RakImage alloc] initWithData : imageData];
	
#ifdef EXTENSIVE_LOGGING
	if(image == nil)
		[imageData writeToFile:@"lol.png" atomically:NO];
#endif
	if(image == nil)
		return nil;
	
	RakImageView * imageView = [[RakImageView alloc] initWithFrame: (NSRect) {NSZeroPoint, image.size}];

	if(imageView != nil)
	{
		imageView.image = image;

		if(isPDF != NULL)
			*isPDF = _isPDF;
	}

	return imageView;
}

- (NSData *) getPage : (uint) posData : (DATA_LECTURE*) data
{
	IMG_DATA * dataPage = loadSecurePage(data->path[data->pathNumber[posData]], data->nomPages[posData], data->chapitreTomeCPT[data->pathNumber[posData]], data->pageCouranteDuChapitre[posData]);
	
	if(dataPage == IMGLOAD_INCORRECT_DECRYPTION)
		return nil;
	
	else if(dataPage == IMGLOAD_NEED_CREDENTIALS_MAIL || dataPage == IMGLOAD_NEED_CREDENTIALS_PASS)
	{
		//Incorrect account :X
		if(COMPTE_PRINCIPAL_MAIL != NULL && (!_needPassword || getPassFromCache(NULL)))
			return nil;
		
		if(dataPage == IMGLOAD_NEED_CREDENTIALS_PASS)
			_needPassword = YES;
		
		MUTEX_VAR * lock = [RakApp sharedLoginMutex : YES];
		
		[self performSelectorOnMainThread:@selector(setWaitingLoginWrapper:) withObject:@(YES) waitUntilDone:NO];
		
		while(COMPTE_PRINCIPAL_MAIL == NULL || (_needPassword && !getPassFromCache(NULL)))
		{
			pthread_cond_wait([RakApp sharedLoginLock], lock);
		}
		
		pthread_mutex_unlock(lock);
		
		[self performSelectorOnMainThread:@selector(setWaitingLoginWrapper:) withObject:@(NO) waitUntilDone:NO];
		
		return [self getPage : posData : data];
	}
	else if(dataPage == IMGLOAD_NODATA)
		return nil;
	
	NSData *output = [NSData dataWithBytes:dataPage->data length:dataPage->length];
	
	free(dataPage->data);
	free(dataPage);
	
	return output;
}

- (NSData *) getPDF : (uint) posData : (DATA_LECTURE *) data
{
	NSString * pageName = [NSString stringWithUTF8String:data->nomPages[posData]];
	
	NSArray * array;
	NSMutableDictionary * dict = data->PDFArrayForNames;
	if(dict == nil || (array = [dict objectForKey:pageName]))
	{
		//Enable us to support encrypted PDF cheapily
		NSData * pdfData = [self getPage:posData :data];
		if(pdfData == nil)
			return nil;
		
		PDFDocument * PDF = [[PDFDocument alloc] initWithData:pdfData];
		if(PDF == nil)
			return nil;
		
		array = [PDF getPages];
		if(array == nil || [array count] == 0)
			return nil;
		
		if(dict == nil)
		{
			dict = [NSMutableDictionary new];
			AntiARCRetain(dict);
			data->PDFArrayForNames = dict;
		}
		
		[dict setObject:array forKey:pageName];
	}
	
	if(data->nameID[posData] >= [array count])
		return nil;
	
	return [[array objectAtIndex:data->nameID[posData]] dataRepresentation];
}

#pragma mark - Notification work

- (PROJECT_DATA) activeProject
{
	return _project;
}

- (uint) currentElem
{
	return _currentElem;
}

- (void) updateContextNotification:(PROJECT_DATA)project :(BOOL)isTome :(uint)element
{
	if(element != INVALID_VALUE)
	{
		[self preProcessingUpdateContext : project : isTome];
		[self startReading : project : element : isTome : INVALID_VALUE];
		[self ownFocus];
	}
}

- (void) preProcessingUpdateContext : (PROJECT_DATA) project : (BOOL) isTome
{

}

- (void) resumeReading : (NSNotification *) notification
{
	uint cacheDBID = [notification.object unsignedIntValue];
	
	PROJECT_DATA project = getProjectByID(cacheDBID);
	if(!project.isInitialized)
		return;
	
	[self restoreProject:project withInsertionPoint:notification.userInfo];
	[self ownFocus];
	
	releaseCTData(project);
}

@end
