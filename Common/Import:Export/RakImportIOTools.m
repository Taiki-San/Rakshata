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

//	We assume the file list is sorted, so that:
//		- Dirs entry are the first of the item of the dir
//		- All files within a dir follow each other
//
//	What are we looking for:
//		- Directories
//		- Images
//
//	Depending of the mix, we decide whatever this could be a CT
//

enum
{
	MIN_EXT_LENGTH = 3,
	MAX_EXT_LENGTH = 4,
	NB_EXT = 7,
	CONFIG_INDEX = 2,
	MAX_DEPTH = 5
};

bool fileIsBundle(const char * filename, RakImportBaseController <RakImportIO> * IOController);

RakImportNode * _importDataForFiles(const char * dirName, char ** files, const uint nbFiles, RakImportBaseController <RakImportIO> * IOController, uint depth)
{
	RakImportNode * output = getEmptyImportNode();

	if(dirName == NULL || files == NULL || !nbFiles || IOController == NULL || depth >= MAX_DEPTH)
		return output;

	NSString * nodeName = [NSString stringWithUTF8String:dirName];
	
	if([nodeName hasPrefix:@"__MACOSX/"])
		return output;
	
	output.nodeName = nodeName;

	bool imagesAndFlatCT = true;
	const char * supportedFormat[] = {"png", "jpg", "dat", "jpeg", "pdf", "tiff", "gif"};
	const byte formatLengths[] = {3, 3, 3, 4, 3, 4, 3};
	uint nbImages = 0, pos = 0;
	
	//Is the first file the name of the directory we are processing?
	if([IOController class] == [RakImportDirController class] &&
	   [nodeName isEqualToString:[NSString stringWithUTF8String:files[0]]] &&
	   (nbFiles > 1 && !strncmp(files[0], files[1], strlen(files[0]))))
	{
		pos = 1;
	}

	for(uint length, cursor; pos < nbFiles; pos++)
	{
		if(files[pos] == NULL)
			continue;

		cursor = length = strlen(files[pos]);

		if(length <= 1 || files[pos][length - 1] == '.')
			continue;
		
		//We need to take a different code path if the file is a bundle, but in order to determine that, we need to locate the extension...
		//This make the code a bit convulated...
		bool isDir = files[pos][length - 1] == '/';
		
		//Okay, we look for extensions
		while(cursor-- > 0 && files[pos][cursor] != '/' && files[pos][cursor] != '.');
		
		//File start with a dot or is straight out invalid
		if(cursor == 0 || (files[pos][cursor] == '.' && files[pos][cursor - 1] == '/'))
			continue;
		
		//At this point, we still include the dot
		cursor += 1;

		//Directory, we recursively analyse it
		if(isDir || fileIsBundle(&files[pos][cursor], IOController))
		{
			uint basePosDir = pos;
			RakImportNode * newNode;
			
			if(isDir)
			{
				//We look for files in this directory. The filename have to be longer
				for(; pos + 1 < nbFiles && strlen(files[pos + 1]) > length && !strncmp(files[basePosDir], files[pos + 1], length); pos++);
				
				//Empty path, we discard it
				if(basePosDir == pos)
					continue;
				
				newNode = _importDataForFiles(files[basePosDir], &(files[basePosDir + 1]), pos - basePosDir, IOController, depth + 1);
			}
			else
			{
				newNode = [createIOForFilename([NSString stringWithUTF8String:files[pos]]) getNode];
			}

			if(!newNode.isValid)
				continue;

			id tmp = output.children == nil ? @[newNode] : [output.children arrayByAddingObject:newNode];
			if(tmp != nil)
			{
				output.children = tmp;

				if(imagesAndFlatCT && !newNode.isFlatCT)
					imagesAndFlatCT = false;
			}
			else
				freeImportNode(newNode);

			continue;
		}
		else	//Image
		{
			if(length - cursor < MIN_EXT_LENGTH || length - cursor > MAX_EXT_LENGTH)
				continue;

			//Check if image
			bool foundOne = false;

			for(byte i = 0; i < NB_EXT && !foundOne; i++)
			{
				foundOne = true;
				for(byte j = 0; j < formatLengths[i] && foundOne; j++)
				{
					if(files[pos][cursor + j] != supportedFormat[i][j])
						foundOne = false;
				}

				if(!foundOne)
					continue;

				//Is config.dat? Otherwise, drop the file
				if(i == CONFIG_INDEX)
				{
					//Two scenario: the file is within a dir (then, baseFileName > 0), and we expect a /, as this is the full name of the entry
					//				the file is in the root (then, baseFileName == 0), and we expect the exact filename
					uint baseFileName = cursor + formatLengths[i] - strlen(CONFIGFILE);
					if((baseFileName == 0 && strcmp(files[pos], CONFIGFILE)) || (baseFileName > 0 && strcmp(&(files[pos][baseFileName - 1]), "/"CONFIGFILE)))
					{
						foundOne = false;
						break;
					}
				}
			}

			if(foundOne)
				++nbImages;
		}
	}

	//Empty shell around one dir
	if(nbImages == 0 && [output.children count] == 1)
	{
		output = output.children[0];
	}
	else
	{
		output.isFlatCT = nbImages > 0 && [output.children count] == 0;
		output.nbImages = nbImages;

		if(output.isFlatCT || [output.children count])
		{
			output.isValid = true;
			output.IOController = IOController;
		}
	}

	return output;
}

RakImportNode * importDataForFiles(const char * dirName, char ** files, const uint nbFiles, RakImportBaseController <RakImportIO> * IOController)
{
	return _importDataForFiles(dirName, files, nbFiles, IOController, 0);
}

RakImportNode * getEmptyImportNode()
{
	return [[RakImportNode alloc] init];
}

bool fileIsBundle(const char * filename, RakImportBaseController <RakImportIO> * IOController)
{
	if(![IOController acceptPackageInPackage])
		return false;
	
	const char * supportedBundle[] = {"rak", "zip", "cbz", "rar", "cbr"};
	const byte bundleLength[] = {3, 3, 3, 3, 3};
	
	for(byte i = 0; i < sizeof(bundleLength); i++)
	{
		bool isValid = true;
		
		for(byte j = 0; j < bundleLength[i] && isValid; j++)
		{
			if(filename[j] != supportedBundle[i][j])
				isValid = false;
		}
		
		if(isValid)
			return true;
	}

	return false;
}

void freeImportNode(RakImportNode * node)
{
	node.children = nil;
}