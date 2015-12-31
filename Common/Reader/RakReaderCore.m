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

	if(imageView == nil)
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

@end
