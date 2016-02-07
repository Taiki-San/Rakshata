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

struct archive * getArchive()
{
	//Create the libarchive archive
	struct archive * archive = archive_read_new();

	//Add support for the format we want to support
	archive_read_support_format_rar(archive);
	archive_read_support_format_7zip(archive);
	archive_read_support_filter_all(archive);

	return archive;
}

ARCHIVE * openArchiveFromFile(const char * path)
{
	ARCHIVE * output = calloc(1, sizeof(ARCHIVE));
	if(output == NULL)
	{
		memoryError(sizeof(ARCHIVE));
		return NULL;
	}

	output->fileHandle = fopen(path, "rb");
	if(output->fileHandle == NULL)
	{
		logR("Couldn't open the file!");
		free(output);
		return NULL;
	}

	//Create the libarchive archive
	output->archive = getArchive();

	//Open the actual file
	if(archive_read_open_FILE(output->archive, output->fileHandle) != ARCHIVE_OK)
	{
		logR("Couldn't open the archive!");
		logR(archive_error_string(output->archive));
		closeArchive(output);
		fclose(output->fileHandle);
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
		if(errCode != ARCHIVE_OK)
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
		filename[currentPos] = strdup(archive_entry_pathname(entry));
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

		if(archive_entry_size(entry) == 0)
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

	rarJumpBackAtBegining(output);

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

void rarJumpBackAtBegining(ARCHIVE * archive)
{
	archive_read_free(archive->archive);
	archive->cachedEntry = NULL;

	rewind(archive->fileHandle);
	archive->archive = getArchive();

	if(archive_read_open_FILE(archive->archive, archive->fileHandle) != ARCHIVE_OK)
	{
		logR("Couldn't rewind the archive!");
		logR(archive_error_string(archive->archive));
		archive->archive = NULL;
	}
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

	fclose(archive->fileHandle);
	free(archive);
}

#pragma mark - Extraction

bool rarLocateFile(ARCHIVE * archive, void ** entryBackup, const char * filename)
{
	if(archive == NULL || filename == NULL)
		return false;

	//Don't locate the file if we receive a hint this is already the active header
	if(archive->cachedEntry != NULL)
	{
		const char * currentFile = archive_entry_pathname(archive->cachedEntry);
		if(currentFile != NULL && !strcmp(currentFile, filename))
		{
			if(entryBackup != NULL)
				*entryBackup = archive->cachedEntry;

			return true;
		}
	}

	int err;
	bool firstPass = true;
	struct archive_entry * entry;
	do
	{
		err = archive_read_next_header(archive->archive, &entry);

		if(err == ARCHIVE_EOF)
		{
			rarJumpBackAtBegining(archive);
			if(firstPass)
			{
				err = archive_read_next_header(archive->archive, &entry);
				firstPass = false;
			}
			else
			{
				logR("Couldn't find file in RAR archive :(");
				break;
			}
		}
		else if(err != ARCHIVE_OK)
		{
			logR("Error while reading the file");
			logR(archive_error_string(archive->archive));
			break;
		}

		const char * currentFile = archive_entry_pathname(entry);
		if(currentFile != NULL && !strcmp(currentFile, filename))
		{
			if(entryBackup != NULL)
				*entryBackup = entry;

			return true;
		}

	} while (err == ARCHIVE_OK);

	return false;
}

bool rarExtractOnefile(ARCHIVE * archive, const char* filename, const char* outputPath)
{
	FILE * output = fopen(outputPath, "wb");
	if(output == NULL)
		return false;

	struct archive_entry * entry;

	//Locate the file
	if(!rarLocateFile(archive, (void **) &entry, filename))
	{
		fclose(output);
		return false;
	}

	//Check this is really a file
	if(archive_entry_size(entry) == 0)
	{
		fclose(output);
		return false;
	}

	int64_t size = 0;
	byte buffer[8192];

	while(1)
	{
		//Grab a small chunk of the file
		size = archive_read_data(archive->archive, &buffer, sizeof(buffer));

		//Write it to the disk
		if(size > 0)
			fwrite(buffer, (uint64_t) size, 1, output);
		else
		{
			if(size < 0)
			{
				const char * errorString = archive_error_string(archive->archive);
				char logMessage[100 + strlen(filename) + strlen(errorString)];
				
				snprintf(logMessage, sizeof(logMessage), "libarchive failure, couldn't decompress file %s because of code %lld: %s", filename, size, errorString);
				logR(logMessage);
			}
			
			break;
		}
	}

	return true;
}

bool rarExtractToMem(ARCHIVE * archive, const char* filename, byte ** data, uint64_t * length)
{
	if(archive == NULL || filename == NULL || data == NULL || length == NULL)
		return false;

	struct archive_entry * entry;

	if(!rarLocateFile(archive, (void **) &entry, filename))
		return false;

	if(archive_entry_size(entry) <= 0)
		return false;

	//Allocate the expected bufferSize
	uint64_t expectedLength = (uint64_t) archive_entry_size(entry), currentPos = 0;
	byte * internalBuffer = malloc(expectedLength * sizeof(byte));
	if(internalBuffer == NULL)
		return false;

	int64_t bufferSize = 0;
	byte buffer[8192];
	while(1)
	{
		bufferSize = archive_read_data(archive->archive, &buffer, sizeof(buffer));

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
					archive_read_data_skip(archive->archive);
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
