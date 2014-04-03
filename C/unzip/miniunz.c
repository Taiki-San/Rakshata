/*
   miniunz.c
   Version 1.1, February 14h, 2010
   sample part of the MiniZip project - ( http://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2010 Gilles Vollant (minizip) ( http://www.winimage.com/zLibDll/minizip.html )

         Modifications of Unzip for Zip64
         Copyright (C) 2007-2008 Even Rouault

         Modifications for Zip64 support on both zip and unzip
         Copyright (C) 2009-2010 Mathias Svensson ( http://result42.com )
*/

/*
  Write info about the ZipFile in the *pglobal_info structure.
  No preparation of the structure is needed
  return UNZ_OK if there is no problem.
*/

#ifndef _WIN32
        #ifndef __USE_FILE_OFFSET64
                #define __USE_FILE_OFFSET64
        #endif
        #ifndef __USE_LARGEFILE64
                #define __USE_LARGEFILE64
        #endif
        #ifndef _LARGEFILE64_SOURCE
                #define _LARGEFILE64_SOURCE
        #endif
        #ifndef _FILE_OFFSET_BIT
                #define _FILE_OFFSET_BIT 64
        #endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#ifdef _WIN32
	#include <direct.h>
	#include <io.h>
#endif

#include "unzip.h"
#include "../crypto/crypto.h"

#define CASESENSITIVITY (0)
#define WRITEBUFFERSIZE (8192)
#define MAXFILENAME (256)

#ifdef _WIN32
	#define USEWIN32IOAPI

	/*iowin32.h*/
	#include <windows.h>

	void fill_win32_filefunc OF((zlib_filefunc_def* pzlib_filefunc_def));
	void fill_win32_filefunc64 OF((zlib_filefunc64_def* pzlib_filefunc_def));
	void fill_win32_filefunc64A OF((zlib_filefunc64_def* pzlib_filefunc_def));
	void fill_win32_filefunc64W OF((zlib_filefunc64_def* pzlib_filefunc_def));
	/*end*/
#endif

int do_extract_currentfile(unzFile uf, char* filename_inzip, char* output_path, const int* popt_extract_without_path, int* popt_overwrite, const char* password, unsigned char* passwordPageCrypted)
{
    char* filename_withoutpath;
    char* p;
    int err=UNZ_OK;
    FILE *fout=NULL;
    uInt size_buf;

    unz_file_info64 file_info;
    //Si pas de fichier donné
    err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);

    if (err!=UNZ_OK)
    {
#ifdef DEV_VERSION
	    char temp[100];
		snprintf(temp, 100, "error %d with zipfile in 1\n",err);
		logR(temp);
#endif
        return err;
    }

    p = filename_withoutpath = filename_inzip;
    for(;(*p) != '\0'; p++)
    {
        if (((*p)=='/') || ((*p)=='\\'))
            filename_withoutpath = p+1; //Restreint au nom seul
    }

    if ((*filename_withoutpath)=='\0') //Si on est au bout du nom du fichier (/ final), c'est un dossier
    {
        if (!(*popt_extract_without_path))
            mkdirR(filename_inzip);
    }
    else
    {
        char* write_filename = NULL;
        int skip=0, size;

        if ((*popt_extract_without_path)==0)
        {
			size = strlen(filename_inzip) + strlen(output_path) + 10;
            write_filename = ralloc(size);
            snprintf(write_filename, size, "%s/%s", output_path, filename_inzip);
        }
        else
        {
			size = strlen(output_path) + strlen(filename_withoutpath) + 10;
            write_filename = ralloc(size);
            snprintf(write_filename, size, "%s/%s", output_path, filename_withoutpath);
        }
		err = unzOpenCurrentFilePassword(uf,password);

		if (err!=UNZ_OK)
        {
#ifdef DEV_VERSION
	    char temp[100];
		snprintf(temp, 100, "error %d with zipfile in 2\n",err);
		logR(temp);
#endif
        }

        if (((*popt_overwrite)==0) && (err==UNZ_OK))
            *popt_overwrite=1;

        if (skip == 0 && err == UNZ_OK)
        {
            fout = fopen(write_filename, "wb");

            //some zipfile don't contain directory alone before file
            if (fout == NULL && *popt_extract_without_path == 0 && strcmp(filename_withoutpath, filename_inzip))
            {
                char c = *(filename_withoutpath - 1);
                *(filename_withoutpath-1)='\0';
				
				mkdirR(write_filename);
				
				*(filename_withoutpath-1)=c;
                fout=fopen(write_filename,"wb");
            }

#ifdef DEV_VERSION
            if (fout==NULL)
            {
                char temp[200];
                snprintf(temp, 200, "error opening %s\n", write_filename);
                logR(temp);
            }
#endif
        }

		rawData *buf_char = malloc((size_buf = WRITEBUFFERSIZE) * sizeof(rawData));
		rawData *buf_enc = malloc(size_buf * sizeof(rawData));
        
		if (buf_char == NULL || buf_enc == NULL)
        {
			free(buf_char);
			free(buf_enc);
            logR("Error allocating memory\n");
            return UNZ_INTERNALERROR;
        }

        if (fout!=NULL && passwordPageCrypted != NULL && strcmp(filename_withoutpath, CONFIGFILE)) //Installation d'un chapitre: cryptage a la volée
        {
            int posIV;
            unsigned char key[KEYLENGTH(KEYBITS)], ciphertext_iv[2][CRYPTO_BUFFER_SIZE];
            SerpentInstance pSer;
			TwofishInstance pTwoF;

            generateRandomKey(passwordPageCrypted);
            for (posIV = 0; posIV < sizeof(key); posIV++)
                key[posIV] = *passwordPageCrypted!= 0 ? *passwordPageCrypted++ : 0;

            TwofishSetKey(&pTwoF, (u4byte*) key, KEYBITS);
            serpent_set_key((uint8_t*) key, KEYLENGTH(KEYBITS), &pSer);
            posIV = -1;

            do
            {
                err = unzReadCurrentFile(uf, buf_char, size_buf);
#ifdef DEV_VERSION
                if (err<0)
                {
                    char temp[100];
                    snprintf(temp, 100, "error %d with zipfile in 3\n",err);
                    logR(temp);
                    break;
                }
#endif
				if (err > 0)
                {
                    int i = 0, j, posDebChunk = 0;
                    while(i < err)
                    {
                        unsigned char plaintext[CRYPTO_BUFFER_SIZE], ciphertext[CRYPTO_BUFFER_SIZE];

                        for (j = 0; j < CRYPTO_BUFFER_SIZE && i < err; plaintext[j++] = buf_char[i++]);
                        for (; j < CRYPTO_BUFFER_SIZE; plaintext[j++] = 0);
                        if(posIV != -1) //Pas premier passage, IV existante
                            for (posIV = j = 0; j < CRYPTO_BUFFER_SIZE; plaintext[j++] ^= ciphertext_iv[0][posIV++]);
                        serpent_encrypt(&pSer, (uint8_t*) plaintext, (uint8_t*) ciphertext);
                        memcpy(&buf_enc[posDebChunk], ciphertext, CRYPTO_BUFFER_SIZE);
                        memcpy(ciphertext_iv, ciphertext, CRYPTO_BUFFER_SIZE);

                        for (j = 0; j < CRYPTO_BUFFER_SIZE && i < err; plaintext[j++] = buf_char[i++]);
                        for (; j < CRYPTO_BUFFER_SIZE; plaintext[j++] = 0);
                        if(posIV != -1) //Pas premier passage, IV existante
                            for (posIV = j = 0; j < CRYPTO_BUFFER_SIZE; plaintext[j++] ^= ciphertext_iv[1][posIV++]);
                        TwofishEncrypt(&pTwoF, (uint32_t*) plaintext, (uint32_t*) ciphertext);
                        memcpy(ciphertext_iv[1], ciphertext, CRYPTO_BUFFER_SIZE);
                        memcpy(&buf_enc[posDebChunk+CRYPTO_BUFFER_SIZE], ciphertext, CRYPTO_BUFFER_SIZE);
                        posIV = 0;
                        posDebChunk+=2*CRYPTO_BUFFER_SIZE;
                    }
                    fwrite(buf_enc, posDebChunk, 1, fout);
                }
            }while (err > 0);
        }

        else if (fout!=NULL) //DÈcompression normale
        {
            do
            {
                err = unzReadCurrentFile(uf, buf_char, size_buf);
                if (err<0)
                {
#ifdef DEV_VERSION
                    char temp[100];
                    snprintf(temp, 100, "error %d with zipfile in 4\n", err);
                    logR(temp);
#endif
                    break;
                }

                if (err>0 && fout != NULL && fwrite(buf_char, err, 1, fout)!=1)
                {
#ifdef DEV_VERSION
                    logR("error in writing extracted file\n");
#endif
                    err=UNZ_ERRNO;
                    break;
                }
            }while (err>0);
		}

        if (fout)
            fclose(fout);

		if (err == UNZ_OK)
        {
            err = unzCloseCurrentFile (uf);
#ifdef DEV_VERSION
            if (err != UNZ_OK)
            {
                char temp[100];
                snprintf(temp, 100, "error %d with zipfile in 5 \n",err);
                logR(temp);
            }
#endif
        }
        else
            unzCloseCurrentFile(uf); //don't lose the error

        free(buf_char);
		free(buf_enc);
    }
    return err;
}


int do_extract(unzFile uf, char *input, char *output_path, int opt_extract_without_path, int opt_overwrite, const char* password)
{
    uLong i;
    unz_global_info64 gi;
    int err;

    err = unzGetGlobalInfo64(uf,&gi);
#ifdef DEV_VERSION
    if (err!=UNZ_OK)
    {
        char temp[100];
        snprintf(temp, 100, "error %d with zipfile in do_extract-1\n",err);
        logR(temp);
    }
#endif

    for (i=0; i < gi.number_entry; i++)
    {
        if (do_extract_currentfile(uf,input,output_path, &opt_extract_without_path, &opt_overwrite, NULL, NULL) != UNZ_OK)
            break;

        if (i+1 < gi.number_entry)
        {
            err = unzGoToNextFile(uf);
            if (err != UNZ_OK)
            {
#ifdef DEV_VERSION
                char temp[100];
                snprintf(temp, 100, "error %d with zipfile in do_extract-2\n",err);
                logR(temp);
#endif
                break;
            }
        }
    }

    return 0;
}

int do_extract_onefile(unzFile uf, char* filename, char* output_path, int opt_extract_without_path, int opt_overwrite, const char* password, unsigned char* passwordPageCrypted)
{
    if (unzLocateFile(uf,filename,CASESENSITIVITY)!=UNZ_OK)
    {
#ifdef DEV_VERSION
		char temp[256];
		snprintf(temp, 256, "404 %s\n", filename); //File not found
		logR(temp);
#endif
        return 2;
    }

    return (do_extract_currentfile(uf,filename,output_path,&opt_extract_without_path, &opt_overwrite, password, passwordPageCrypted) != UNZ_OK);
}

