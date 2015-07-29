/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                          **
 **		Source code and assets are property of Taiki, distribution is stricly forbidden		**
 **                                                                                          **
 *********************************************************************************************/

ARCHIVE * openArchiveFromFile(const char * path)
{
	ARCHIVE * output = calloc(1, sizeof(ARCHIVE));
	if(output == NULL)
	{
		memoryError(sizeof(ARCHIVE));
		return NULL;
	}

	//Create the libarchive archive
	output->archive = archive_read_new();

	//Add support for the format we want to support
	archive_read_support_format_rar(output->archive);
	archive_read_support_format_7zip(output->archive);
	archive_read_support_filter_all(output->archive);

	//Open the actual file
	if(archive_read_open_filename(output->archive, path, 1024 * 1024) != ARCHIVE_OK)
	{
		logR("Couldn't open the file!");
		logR(archive_error_string(output->archive));
		closeArchive(output);
		return NULL;
	}

	//Okay, we can now crawl filenames
	//However, archive_file_count contains garbage until we crawled at least once the archive

	uint predictedNumberOfFiles = 127, step = 128, currentPos = 0;
	char ** filename = malloc(predictedNumberOfFiles * sizeof(char *));
	if(filename == NULL)
	{
		memoryError(predictedNumberOfFiles * sizeof(char *));
		closeArchive(output);
		return NULL;
	}

	int errCode;
	struct archive_entry * entry;
	while((errCode = archive_read_next_header(output->archive, &entry)) != ARCHIVE_EOF)
	{
		if(errCode == ARCHIVE_RETRY)
		{
			continue;
		}
		else if(errCode == ARCHIVE_FATAL)
		{
			logR("Error while reading the file");
			logR(archive_error_string(output->archive));

			output->fileList = filename;
			output->nbFiles = currentPos;
			closeArchive(output);
			return NULL;
		}

		//If we need more memory
		if(currentPos == predictedNumberOfFiles)
		{
			predictedNumberOfFiles += step;
			if(predictedNumberOfFiles < currentPos)	//Overflow
			{
				logR("Over 4 billions file in archive, a bit too much for us to cope, sorry!");

				output->fileList = filename;
				output->nbFiles = currentPos;
				closeArchive(output);
				return NULL;
			}

			void * tmp = realloc(filename, predictedNumberOfFiles * sizeof(char *));
			if(tmp == NULL)
			{
				memoryError(predictedNumberOfFiles * sizeof(char *));

				output->fileList = filename;
				output->nbFiles = currentPos;
				closeArchive(output);
				return NULL;
			}

			filename = tmp;
		}

		//We copy the metadata
		filename[currentPos] = strdup(archive_entry_pathname(entry));
		if(filename[currentPos] == NULL)
		{
			logR("Memory error, couldn't analyse the file");

			output->fileList = filename;
			output->nbFiles = currentPos;
			closeArchive(output);
			return NULL;
		}
		else
			currentPos++;
	}

	//Reduce our memory use
	if(currentPos > predictedNumberOfFiles)
	{
		void * tmp = realloc(filename, currentPos * sizeof(char *));
		if(tmp != NULL)
			filename = tmp;
	}

	//Sort the index
	qsort(filename, currentPos, sizeof(char *), strnatcmp);

	output->fileList = filename;
	output->nbFiles = currentPos;

	return output;
}

void closeArchive(ARCHIVE * archive)
{
	if(archive == NULL)
		return;

	if(archive->nbFiles && archive->fileList != NULL)
	{
		while(archive->nbFiles-- > 0)
			free(archive->fileList[archive->nbFiles]);
	}

	free(archive->fileList);

	if(archive->archive != NULL)
		archive_read_free(archive->archive);

	free(archive);
}

#pragma mark - High level utils

bool fileExistInArchive(ARCHIVE * archive, const char * filename)
{
	if(archive == NULL || archive->fileList == NULL)
		return false;

	for(uint i = 0, length = archive->nbFiles; i < length; ++i)
	{
		if(!strcmp(archive->fileList[i], filename))
			return true;
	}

	return false;
}
