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

IMPORT_NODE _importDataForFiles(char * dirName, char ** files, const uint nbFiles, void * IOController, uint depth)
{
	if(dirName == NULL || files == NULL || !nbFiles || IOController == NULL || depth >= MAX_DEPTH)
		return getEmptyImportNode();

	IMPORT_NODE output = getEmptyImportNode();

	output.nodeName = dirName;

	bool onlyImages = true, imagesAndFlatCT = true;
	const char * supportedFormat[] = {"png", "jpg", "dat", "jpeg", "pdf", "tiff", "gif"};
	const byte formatLengths[] = {3, 3, 3, 4, 3, 4, 3};
	uint nbImages = 0;

	for(uint pos = 0, length, cursor; pos < nbFiles; pos++)
	{
		if(files[pos] == NULL)
			continue;

		cursor = length = strlen(files[pos]);

		if(length <= 1)
			continue;

		//Directory, we recursively analyse it
		if(files[pos][cursor - 1] == '/')
		{
			uint basePosDir = pos;
			//We look for files in this directory. The filename have to be longer, not to
			for(; pos + 1 < nbFiles && strlen(files[pos + 1]) > length && !strncmp(files[basePosDir], files[pos + 1], length); pos++);

			//Empty path, we discard it
			if(basePosDir == pos)
				continue;

			IMPORT_NODE newNode = _importDataForFiles(files[basePosDir], &(files[basePosDir + 1]), pos - basePosDir, IOController, depth + 1);

			if(!newNode.isValid)
				continue;

			void * tmp = realloc(output.children, (output.nbChildren + 1) * sizeof(IMPORT_NODE));
			if(tmp != NULL)
			{
				output.children = tmp;
				output.children[output.nbChildren++] = newNode;

				onlyImages = false;

				if(imagesAndFlatCT && !newNode.isFlatCT)
					imagesAndFlatCT = false;
			}
			else
				freeImportNode(newNode);

			continue;
		}

		//Okay, we look for extensions
		while(cursor-- > 0 && files[pos][cursor] != '/' && files[pos][cursor] != '.');

		//File don't start with a dot
		if(cursor > 0 && files[pos][cursor] == '.' && files[pos][cursor - 1] != '/' && files[pos][cursor - 1] != '.')
		{
			if(length - cursor < MIN_EXT_LENGTH || length - cursor > MAX_EXT_LENGTH)
				continue;

			//At this point, we still include the dot
			cursor += 1;

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
				else
					++nbImages;
			}

			if(!foundOne)
			{
				onlyImages = false;
				continue;
			}
		}
		else
			imagesAndFlatCT = onlyImages = false;
	}

	//Empty shell around one dir
	if(nbImages == 0 && output.nbChildren == 1)
	{
		free(output.nodeName);
		IMPORT_NODE newNode = output.children[0];

		free(output.children);
		output = newNode;
	}
	else
	{
		output.isFlatCT = onlyImages;
		output.couldBeComplexT = !onlyImages && imagesAndFlatCT && output.nbChildren > 1;
		output.nbImages = nbImages;

		if(output.isFlatCT || output.nbChildren)
		{
			output.isValid = true;
			output.IOController = IOController;
		}
	}

	return output;
}

IMPORT_NODE importDataForFiles(char * dirName, char ** files, const uint nbFiles, void * IOController)
{
	return _importDataForFiles(dirName, files, nbFiles, IOController, 0);
}

IMPORT_NODE getEmptyImportNode()
{
	IMPORT_NODE node;

	memset(&node, 0, sizeof(node));

	return node;
}

void freeImportNode(IMPORT_NODE node)
{
	for(uint i = 0; i < node.nbChildren; i++)
	{
		freeImportNode(node.children[i]);
	}

	free(node.nodeName);
}