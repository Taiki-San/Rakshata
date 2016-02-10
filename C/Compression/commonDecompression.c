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

ARCHIVE * openArchiveFromFile(const char * path, byte format)
{
	ARCHIVE * output = calloc(1, sizeof(ARCHIVE));
	if(output == NULL)
	{
		memoryError(sizeof(ARCHIVE));
		return NULL;
	}

	switch (format)
	{
		case LIB_UNARR:
		{
			break;
		}
			
		case LIB_LIBARCHIVE:
		{
			break;
		}
			
		default:
		{
			free(output);
			return NULL;
		}
	}
	
	output->utils.open_archive(output, path);
	
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
	
	while(output->utils.move_next_file(output))
	{
		//If we need more memory
		if(currentPos == predictedNumberOfFiles)
		{
			predictedNumberOfFiles += step;
			if(predictedNumberOfFiles < currentPos)	//Overflow
			{
				logR("Over 4 billion file in archive, a bit too much for us to cope, sorry!");
				
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
		filename[currentPos] = strdup(output->utils.get_name(output));
		if(filename[currentPos] == NULL)
		{
			logR("Memory error, couldn't analyse the file");
			
			output->fileList = filename;
			output->nbFiles = currentPos;
			closeArchive(output);
			return NULL;
		}
		
		//Dirs don't always have / at the end, but we assume they do later :(
		//Our heuristic is that empty entries are directories
		
		if(output->utils.get_size(output) == 0)
		{
			uint length = strlen(filename[currentPos]);
			void * tmp = realloc(filename[currentPos], length + 2);
			
			if(tmp != NULL)
			{
				filename[currentPos] = tmp;
				filename[currentPos][length] = '/';
				filename[currentPos][length + 1] = '\0';
			}
			else
			{
				logR("Memory error, couldn't analyse the file");
				
				output->fileList = filename;
				output->nbFiles = currentPos + 1;
				closeArchive(output);
				return NULL;
			}
		}
		
		currentPos++;
	}
	
	output->utils.jump_back_begining(output);
	
	//Reduce our memory use
	if(currentPos == 0)
	{
		free(filename);
		output->nbFiles = 0;
		closeArchive(output);
		return NULL;
	}
	else if(currentPos < predictedNumberOfFiles)
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
	
	archive->utils.close_archive(archive);

	free(archive);
}

#pragma mark - Extraction

bool archiveLocateFile(ARCHIVE * archive, const char * filename)
{
	if(archive == NULL || filename == NULL)
		return false;
	
	//Don't locate the file if we receive a hint this is already the active header
	if(archive->workingEntry != NULL)
	{
		const char * currentFile = archive->utils.get_name(archive);
		if(currentFile != NULL && !strcmp(currentFile, filename))
			return true;
	}
	
	if(archive->utils.cleanup_entry != NULL)
		archive->utils.cleanup_entry(archive);
	
	bool firstPass = true, success;
	do
	{
		success = archive->utils.move_next_file(archive->archive);
		
		if(success)
		{
			const char * currentFile = archive->utils.get_name(archive);
			if(currentFile != NULL && !strcmp(currentFile, filename))
			{
				return true;
			}
		}
		else if(archive->utils.is_EOF(archive))
		{
			archive->utils.jump_back_begining(archive);
			if(firstPass)
			{
				success = archive->utils.move_next_file(archive->archive);
				firstPass = false;
				success = true;
			}
			else
			{
				logR("Couldn't find file in 7Zip archive :(");
				break;
			}
		}
		else
		{
			logR("Error while reading the file");
			if(archive->utils.get_error_string != NULL)
				logR(archive->utils.get_error_string(archive->archive));
			break;
		}
		
		
	} while (success);
	
	return false;
}

bool archiveExtractOnefile(ARCHIVE * archive, const char* filename, const char* outputPath)
{
	FILE * output = fopen(outputPath, "wb");
	if(output == NULL)
		return false;
	
	//Locate the file
	if(!archiveLocateFile(archive, filename))
	{
		fclose(output);
		return false;
	}
	
	//Check this is really a file
	if(archive->utils.get_size(archive) == 0)
	{
		fclose(output);
		return false;
	}
	
	int64_t size = 0;
	rawData buffer[8192];
	
	while(1)
	{
		//Grab a small chunk of the file
		size = archive->utils.read_data(archive->archive, buffer, sizeof(buffer));
		
		//Write it to the disk
		if(size > 0)
			fwrite(buffer, (uint64_t) size, 1, output);
		else
		{
			if(size < 0 && archive->utils.get_error_string != NULL)
			{
				const char * errorString = archive->utils.get_error_string(archive->archive);
				char logMessage[100 + strlen(filename) + strlen(errorString)];
				
				snprintf(logMessage, sizeof(logMessage), "libarchive failure, couldn't decompress file %s because of code %lld: %s", filename, size, errorString);
				logR(logMessage);
			}
			
			break;
		}
	}
	
	return true;
}

bool archiveExtractToMem(ARCHIVE * archive, const char* filename, byte ** data, uint64_t * length)
{
	if(archive == NULL || filename == NULL || data == NULL || length == NULL)
		return false;
	
	//libarchive need to cache the entry at this point
	if(!archiveLocateFile(archive, filename))
		return false;
	
	uint64_t expectedLength = (uint64_t) archive->utils.get_size(archive), currentPos = 0;
	if((int64_t) expectedLength <= 0)
		return false;
	
	//Allocate the expected bufferSize
	byte * internalBuffer = malloc(expectedLength * sizeof(byte));
	if(internalBuffer == NULL)
		return false;
	
	int64_t bufferSize = 0;
	rawData buffer[8192];
	while(1)
	{
		bufferSize = archive->utils.read_data(archive->archive, buffer, sizeof(buffer));
		
		if(bufferSize > 0)
		{
			//Increase the buffer size
			if(currentPos + (uint64_t) bufferSize > expectedLength)
			{
				expectedLength = currentPos + (uint64_t) bufferSize;
				
				void * tmp = realloc(internalBuffer, expectedLength * sizeof(byte));
				if(tmp == NULL)
				{
					free(internalBuffer);
					archive->utils.move_next_file(archive);
					return false;
				}
				
				internalBuffer = tmp;
			}
			
			//Copy the memory
			memcpy(&(internalBuffer[currentPos]), buffer, ((uint64_t) bufferSize) * sizeof(byte));
			currentPos += (uint64_t) bufferSize;
		}
		else
			break;
	}
	
	//Reduce the buffer size to what is needed
	if(currentPos < expectedLength)
	{
		if(currentPos == 0)
		{
			free(internalBuffer);
			internalBuffer = NULL;
		}
		else
		{
			void * tmp = realloc(internalBuffer, currentPos * sizeof(byte));
			if(tmp != NULL)
				internalBuffer = tmp;
		}
	}
	
	*data = internalBuffer;
	*length = currentPos;
	
	if(archive->utils.cleanup_entry != NULL)
		archive->utils.cleanup_entry(archive);
	
	return true;
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
