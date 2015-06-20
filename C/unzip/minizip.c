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

#include "zlibAPI.h"
#include "ioapi.h"

#ifdef _WIN32
	#include "iowin32.h"
#endif

zipFile * createZip(const char * outfile)
{
	return zipOpen64(outfile, APPEND_STATUS_CREATE);
}

bool addFileToZip(zipFile * zipFile, const char * filename)
{
	if(zipFile == NULL || filename == NULL)
		return false;

	//We ensure the file exist
	FILE * file = fopen(filename, "rb");
	if(file == NULL)
		return false;

	bool ret_value = true;
	uint64_t fileSize = getFileSize64(filename);

	//Write metadata
	zip_fileinfo metadata;
	memset(&metadata, 0, sizeof(metadata));

	if(zipOpenNewFileInZip3_64(zipFile, filename, &metadata, NULL, 0, NULL, 0, NULL, Z_DEFLATED, 5, 0, -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, NULL, 0, fileSize >= 0xffffffff) == ZIP_OK)
	{
		if(fileSize != 0)	//Check if actual content
		{
			byte chunkData[0x4000];
			uint chunckSize;

			while((chunckSize = fread(chunkData, sizeof(byte), sizeof(chunkData), file)) != 0)
			{
				if(zipWriteInFileInZip(zipFile, chunkData, chunckSize) != ZIP_OK)
				{
					ret_value = false;
					break;
				}
			}
		}
	}
	else
		ret_value = false;

	zipCloseFileInZip(zipFile);
	fclose(file);

	return ret_value;
}

bool addMemToZip(zipFile * zipFile, const char * filename, const byte * memoryChunk, const uint64_t chunckSize)
{
	if(zipFile == NULL || filename == NULL || memoryChunk == NULL)
		return false;

	//Write metadata
	zip_fileinfo metadata;
	memset(&metadata, 0, sizeof(metadata));

	int err = zipOpenNewFileInZip3_64(zipFile, filename, &metadata, NULL, 0, NULL, 0, NULL, Z_DEFLATED, 5, 0, -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, NULL, 0, chunckSize >= 0xffffffff);
	if(err != ZIP_OK)
		return false;

	zipWriteInFileInZip(zipFile, memoryChunk, chunckSize);

	zipCloseFileInZip(zipFile);
	return true;
}

bool addDirToZip(zipFile * zipFile, const char * dirName)
{
	if(zipFile == NULL || dirName == NULL)
		return false;

	DIR *directory;           //Directory
	struct dirent *entry;     //Directory entry
	uint dirNameLength = strlen(dirName);

	directory = opendir(dirName);
	if(directory == NULL)
		return false;

	bool retValue = true;

	while(retValue && (entry = readdir(directory)) != NULL)
	{
		if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
			continue;

		char subdirName[dirNameLength + entry->d_namlen + 2];
		snprintf(subdirName, sizeof(subdirName), "%s/%s", dirName, entry->d_name);

		if(checkDirExist(subdirName))	//Is it a file or a directory
			retValue &= addDirToZip(zipFile, subdirName);
		else
			retValue &= addFileToZip(zipFile, subdirName);
	}

	closedir(directory);

	return retValue;
}

void closeZip(zipFile * zipFile)
{
	zipClose(zipFile, NULL);
}