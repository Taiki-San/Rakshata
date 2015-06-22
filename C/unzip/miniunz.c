/*
 miniunz.c
 Version 1.1, February 14h, 2010
 sample part of the MiniZip project - ( http://www.winimage.com/zLibDll/minizip.html )

 Copyright (C) 1998-2010 Gilles Vollant (minizip) ( http://www.winimage.com/zLibDll/minizip.html )

 Modifications of Unzip for Zip64
 Copyright (C) 2007-2008 Even Rouault

 Modifications for Zip64 support on both zip and unzip
 Copyright (C) 2009-2010 Mathias Svensson ( http://result42.com )

 Major modifications by Taiki
 Copyright (C) 2011-2015 Taiki ( http://www.taiki.us/ )

 */

#include "zlibAPI.h"

#define BUFFER_SIZE 0x4000

int doExtractCurrentfile(unzFile zipFile, char* filenameExpected, char* outputPath, const bool extractWithoutPath, unsigned char* passwordPageCrypted)
{
	int err = UNZ_OK;
	unz_file_info64 file_info;
	char filenameInZip[BUFFER_SIZE];

	//Load current file metadata
	if((err = unzGetCurrentFileInfo64(zipFile, &file_info, filenameInZip, sizeof(filenameInZip), NULL, 0, NULL, 0)) != UNZ_OK || (filenameExpected != NULL && strcmp(filenameInZip, filenameExpected)))
	{
#ifdef DEV_VERSION
		char temp[100];
		snprintf(temp, 100, "Invalid zip entry (%d)\n",err);
		logR(temp);
#endif
		return err;
	}

	//Find the last / in the path
	char * filenameWithoutPath = filenameInZip;
	for(char * p = filenameInZip; *p != '\0'; p++)
	{
		if(*p == '/' || *p == '\\')
			filenameWithoutPath = p + 1; //Restreint au nom seul
	}

	//If directory
	if(*filenameWithoutPath == 0) //Si on est au bout du nom du fichier (/ final), c'est un dossier
	{
		if(!extractWithoutPath)
			mkdirR(filenameInZip);

		return UNZ_OK;
	}

	char* outputFilename = NULL;
	uint32_t sizeOutputPath;

	//Craft output file
	const char * filenameToUse = extractWithoutPath ? filenameWithoutPath : filenameInZip;
	sizeOutputPath = strlen(outputPath) + strlen(filenameToUse) + 2;
	outputFilename = calloc(1, sizeOutputPath);

	if(outputFilename == NULL)
		return UNZ_INTERNALERROR;

	snprintf(outputFilename, sizeOutputPath, "%s/%s", outputPath, filenameToUse);

	//If there was a password, we open the file with it
	if((err = unzOpenCurrentFilePassword(zipFile)) != UNZ_OK)
	{
#ifdef DEV_VERSION
		char temp[100];
		snprintf(temp, 100, "Invalid password (%d)\n",err);
		logR(temp);
#endif
		return err;
	}

	//We open the output file, eventually creating the path if missing
	FILE * outputFile = fopen(outputFilename, "wb");
	if(outputFile == NULL)		//Hum, who knows, maybe the output path wasn't built
	{
		createPath(outputFilename);
		outputFile = fopen(outputFilename, "wb");

		if(outputFile == NULL)
		{
#ifdef DEV_VERSION
			char temp[200];
			snprintf(temp, 200, "Error creating path to %s\n", outputFilename);
			logR(temp);
#endif
			return UNZ_INTERNALERROR;
		}
	}
	free(outputFilename);

	//Main decrompression part
	rawData workingBuffer[BUFFER_SIZE];

	//We want to encrypt the file
	if(passwordPageCrypted != NULL && strcmp(filenameWithoutPath, CONFIGFILE))
	{
		uint posIV = UINT_MAX, i, j, posDebChunk;
		byte key[KEYLENGTH(KEYBITS)], ciphertext_iv[2][CRYPTO_BUFFER_SIZE], plaintext[CRYPTO_BUFFER_SIZE], ciphertext[CRYPTO_BUFFER_SIZE];
		rawData workingEncryption[BUFFER_SIZE];

		//Startup encryption engine
		SerpentInstance pSer;
		TwofishInstance pTwoF;

		generateRandomKey(passwordPageCrypted);
		memcpy(key, passwordPageCrypted, sizeof(key));

		TwofishSetKey(&pTwoF, (uint32_t*) key, KEYBITS);
		serpent_set_key((uint8_t*) key, KEYLENGTH(KEYBITS), &pSer);

		do
		{
			if((err = unzReadCurrentFile(zipFile, workingBuffer, BUFFER_SIZE)) < 0)
			{
#ifdef DEV_VERSION
				char temp[100];
				snprintf(temp, 100, "Decompression error (%d)",err);
				logR(temp);
#endif
				break;
			}

			i = posDebChunk = 0;
			while(i < (uint) err)
			{
				for (j = 0; j < CRYPTO_BUFFER_SIZE && i < (uint) err; plaintext[j++] = workingBuffer[i++]);
				for (; j < CRYPTO_BUFFER_SIZE; plaintext[j++] = 0);

				if(posIV != UINT_MAX) //Pas premier passage, IV existante
					for (posIV = j = 0; j < CRYPTO_BUFFER_SIZE; plaintext[j++] ^= ciphertext_iv[0][posIV++]);

				serpent_encrypt(&pSer, (uint8_t*) plaintext, (uint8_t*) ciphertext);
				memcpy(&workingEncryption[posDebChunk], ciphertext, CRYPTO_BUFFER_SIZE);
				memcpy(ciphertext_iv, ciphertext, CRYPTO_BUFFER_SIZE);

				for (j = 0; j < CRYPTO_BUFFER_SIZE && i < (uint) err; plaintext[j++] = workingBuffer[i++]);
				for (; j < CRYPTO_BUFFER_SIZE; plaintext[j++] = 0);

				if(posIV != UINT_MAX) //Pas premier passage, IV existante
					for (posIV = j = 0; j < CRYPTO_BUFFER_SIZE; plaintext[j++] ^= ciphertext_iv[1][posIV++]);
				else
					posIV = 0;

				TwofishEncrypt(&pTwoF, (uint32_t*) plaintext, (uint32_t*) ciphertext);
				memcpy(ciphertext_iv[1], ciphertext, CRYPTO_BUFFER_SIZE);
				memcpy(&workingEncryption[posDebChunk+CRYPTO_BUFFER_SIZE], ciphertext, CRYPTO_BUFFER_SIZE);

				posDebChunk += 2*CRYPTO_BUFFER_SIZE;
			}

			fwrite(workingEncryption, 1, posDebChunk, outputFile);

		} while (err > 0);
	}

	//Standard decompression
	else
	{
		do
		{
			if((err = unzReadCurrentFile(zipFile, workingBuffer, BUFFER_SIZE)) < 0)
			{
#ifdef DEV_VERSION
				char temp[100];
				snprintf(temp, 100, "Decompression error (%d)",err);
				logR(temp);
#endif
				break;
			}

			fwrite(workingBuffer, 1, (size_t) err, outputFile);

		} while (err > 0);
	}

	fclose(outputFile);

	//Everything went fine
	if(err == UNZ_OK)
		err = unzCloseCurrentFile(zipFile);

	//keep the most important error: the one that killed the proccess
	else
		unzCloseCurrentFile(zipFile);

	return err;
}

int doExtractFileInArchive(char * inputFile, char *outputPath, bool extractWithoutPath)
{
	unzFile * zipFile = unzOpen64(inputFile);
	if(zipFile == NULL)
		return false;

	unz_global_info64 metadata;
	int err;

	if((err = unzGetGlobalInfo64(zipFile, &metadata)) == UNZ_OK)
	{
		for (uint i = 0, nbEntry = metadata.number_entry; i < nbEntry; i++)
		{
			if(((err = doExtractCurrentfile(zipFile, NULL, outputPath, extractWithoutPath, NULL)) != UNZ_OK)		//Extract the current file
			   || (i + 1 < nbEntry && (err = unzGoToNextFile(zipFile)) != UNZ_OK))									//Jump to the next file
				break;
		}
	}

	unzClose(zipFile);

	return err;
}

bool doExtractOnefile(unzFile zipFile, char* filename, char* outputPath, bool extractWithoutPath, unsigned char* passwordPageCrypted)
{
	if(unzLocateFile(zipFile, filename, 0) != UNZ_OK)
	{
#ifdef DEV_VERSION
		char temp[256];
		snprintf(temp, 256, "File doesn't exist %s\n", filename);
		logR(temp);
#endif
		return false;
	}

	return doExtractCurrentfile(zipFile, filename, outputPath, extractWithoutPath, passwordPageCrypted) == UNZ_OK;
}

bool extractToMem(unzFile zipFile, byte ** output, uint64_t * sizeOutput)
{
	if(zipFile == NULL || output == NULL)
		return false;

	unz_file_info64 metadata;
	if(unzGetCurrentFileInfo64(zipFile, &metadata, NULL, 0, NULL, 0, NULL, 0) != UNZ_OK)
		return false;

	*output = malloc(metadata.uncompressed_size + 1);
	if(*output == NULL)
		return false;

	int err;
	uint64_t posOutput = 0;
	byte workingBuffer[BUFFER_SIZE];

	do
	{
		if((err = unzReadCurrentFile(zipFile, workingBuffer, BUFFER_SIZE)) < 0 || posOutput + (uint) err > metadata.uncompressed_size)
		{
			free(*output);
			*output = NULL;
			return false;
		}

		memcpy(&((*output)[posOutput]), workingBuffer, err);
		posOutput += (uint) err;

	} while(err > 0 && posOutput < metadata.uncompressed_size);

	(*output)[(*sizeOutput = posOutput)] = 0;

	return true;
}