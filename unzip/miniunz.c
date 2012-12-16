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
#include "../main.h"
#include "../AES.h"

#define fopen64 fopen

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


/* mymkdir and change_file_date are not 100 % portable
   As I don't know well Unix, I wait feedback for the unix portion*/

int mymkdir(const char* dirname)
{
    int ret=0;
#ifdef _WIN32
    ret = _mkdir(dirname);
#else
    ret = mkdir (dirname,0775);
#endif
    return ret;
}

int makedir (char *newdir)
{
  char *buffer ;
  char *p;
  int  len = (int)strlen(newdir);

  if (len <= 0)
    return 0;

  buffer = (char*)malloc(len+1);
        if (buffer==NULL)
        {
                logR("Error allocating memory\n");
                return UNZ_INTERNALERROR;
        }
  strcpy(buffer,newdir);

  if (buffer[len-1] == '/') {
    buffer[len-1] = '\0';
  }
  if (mymkdir(buffer) == 0)
    {
      free(buffer);
      return 1;
    }

  p = buffer+1;
  while (1)
    {
      char hold;

      while(*p && *p != '\\' && *p != '/')
        p++;
      hold = *p;
      *p = 0;
      if ((mymkdir(buffer) == -1) && (errno == ENOENT))
        {
          logR("couldn't create directory");
          logR(buffer);
          logR("\n");
          free(buffer);
          return 0;
        }
      if (hold == 0)
        break;
      *p++ = hold;
    }
  free(buffer);
  return 1;
}

void Display64BitsSize(ZPOS64_T n, int size_char)
{
  /* to avoid compatibility problem , we do here the conversion */
  char number[21];
  int offset=19;
  int pos_string = 19;
  number[20]=0;
  for (;;) {
      number[offset]=(char)((n%10)+'0');
      if (number[offset] != '0')
          pos_string=offset;
      n/=10;
      if (offset==0)
          break;
      offset--;
  }
  {
      int size_display_string = 19-pos_string;
      while (size_char > size_display_string)
      {
          size_char--;
          printf(" ");
      }
  }

  printf("%s",&number[pos_string]);
}



/* change_file_date : change the date/time of a file
    filename : the filename of the file where date/time must be modified
    dosdate : the new date at the MSDos format (4 bytes)
    tmu_date : the SAME new date at the tm_unz format*/

void change_file_date(filename,dosdate,tmu_date)
    const char *filename;
    uLong dosdate;
    tm_unz tmu_date;
{
#ifdef _WIN32
  HANDLE hFile;
  FILETIME ftm,ftLocal,ftCreate,ftLastAcc,ftLastWrite;

  hFile = CreateFileA(filename,GENERIC_READ | GENERIC_WRITE,
                      0,NULL,OPEN_EXISTING,0,NULL);
  GetFileTime(hFile,&ftCreate,&ftLastAcc,&ftLastWrite);
  DosDateTimeToFileTime((WORD)(dosdate>>16),(WORD)dosdate,&ftLocal);
  LocalFileTimeToFileTime(&ftLocal,&ftm);
  SetFileTime(hFile,&ftm,&ftLastAcc,&ftm);
  CloseHandle(hFile);
#else
  struct utimbuf ut;
  struct tm newdate;
  newdate.tm_sec = tmu_date.tm_sec;
  newdate.tm_min=tmu_date.tm_min;
  newdate.tm_hour=tmu_date.tm_hour;
  newdate.tm_mday=tmu_date.tm_mday;
  newdate.tm_mon=tmu_date.tm_mon;
  if (tmu_date.tm_year > 1900)
      newdate.tm_year=tmu_date.tm_year - 1900;
  else
      newdate.tm_year=tmu_date.tm_year ;
  newdate.tm_isdst=-1;

  ut.actime=ut.modtime=mktime(&newdate);
  utime(filename,&ut);
#endif
}

int do_extract_currentfile(uf,filename_inzip,popt_extract_without_path,popt_overwrite,password, passwordPageCrypted)
    unzFile uf;
    char* filename_inzip;
    const int* popt_extract_without_path;
    int* popt_overwrite;
    const char* password;
    unsigned char* passwordPageCrypted;
{
    char* filename_withoutpath;
    char* p;
    int err=UNZ_OK;
    void* buf;
    FILE *fout=NULL;
    uInt size_buf;

    unz_file_info64 file_info;
    //Si pas de fichier donn�
    err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);

    if (err!=UNZ_OK)
    {
        printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
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
        {
            printf("creating directory: %s\n",filename_inzip);
            mymkdir(filename_inzip);
        }
    }
    else
    {
        char* write_filename;
        int skip=0;

        if ((*popt_extract_without_path)==0)
            write_filename = filename_inzip;
        else
            write_filename = filename_withoutpath;

		err = unzOpenCurrentFilePassword(uf,password);

		if (err!=UNZ_OK)
        {
            printf("error %d with zipfile in unzOpenCurrentFilePassword\n",err);
        }

        if (((*popt_overwrite)==0) && (err==UNZ_OK))
            *popt_overwrite=1;

        if (skip==0 && err==UNZ_OK)
        {
            fout=fopen64(write_filename,"wb");

            //some zipfile don't contain directory alone before file
            if ((fout==NULL) && ((*popt_extract_without_path)==0) &&
                                (filename_withoutpath!=(char*)filename_inzip))
            {
                char c=*(filename_withoutpath-1);
                *(filename_withoutpath-1)='\0';
#ifdef _WIN32
				mkdir(write_filename);
#else
				mkdir(write_filename, 0775);
#endif
                *(filename_withoutpath-1)=c;
                fout=fopen64(write_filename,"wb");
            }

            if (fout==NULL)
            {
                printf("error opening %s\n",write_filename);
            }
        }

        size_buf = WRITEBUFFERSIZE;
        buf = (void*)malloc(size_buf);
        if (buf==NULL)
        {
            logR("Error allocating memory\n");
            return UNZ_INTERNALERROR;
        }

        if (fout!=NULL && passwordPageCrypted != NULL && strcmp(filename_withoutpath, CONFIGFILE)) //Installation d'un chapitre: cryptage a la vol�e
        {
            int nrounds;
            unsigned long rk[RKLENGTH(KEYBITS)];
            unsigned char *buf_char = NULL;
            unsigned char key[KEYLENGTH(KEYBITS)];

            generateKey(passwordPageCrypted);
            for (nrounds = 0; nrounds < sizeof(key); nrounds++)
                key[nrounds] = *passwordPageCrypted!= 0 ? *passwordPageCrypted++ : 0;
            nrounds = rijndaelSetupEncrypt(rk, key, 256);
            do
            {
                err = unzReadCurrentFile(uf,buf,size_buf);
                if (err<0)
                {
                    printf("error %d with zipfile in unzReadCurrentFile\n",err);
                    break;
                }
                if (err>0)
                {
                    int i;
                    for(i = 0; i<err;)
                    {
                        unsigned char plaintext[16];
                        unsigned char ciphertext[16];
                        buf_char = (unsigned char *) buf;
                        int j;
                        for (j = 0; j < sizeof(plaintext) && i < err;)
                            plaintext[j++] = buf_char[i++];
                        for (; j < sizeof(plaintext); plaintext[j++] = 0);
                        rijndaelEncrypt(rk, nrounds, plaintext, ciphertext);

                        if (fwrite(ciphertext, sizeof(ciphertext), 1, fout) != 1)
                        {
                            fclose(fout);
                            logR("File write error\n");
                            return 1;
                        }
                    }
                }
            }while (err>0);
        }

        else if (fout!=NULL) //D�compression normale
        {
            do
            {
                err = unzReadCurrentFile(uf,buf,size_buf);
                if (err<0)
                {
                    char temp[100];
                    sprintf(temp, "error %d with zipfile in unzReadCurrentFile\n", err);
                    logR(temp);
                    break;
                }
                if (err>0 && fout != NULL && fwrite(buf,err,1,fout)!=1)
                {
                    logR("error in writing extracted file\n");
                    err=UNZ_ERRNO;
                    break;
                }

            }while (err>0);
        }
        if (fout)
            fclose(fout);

        if (err==0)
            change_file_date(write_filename,file_info.dosDate,
                             file_info.tmu_date);

        if (err==UNZ_OK)
        {
            err = unzCloseCurrentFile (uf);
            if (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzCloseCurrentFile\n",err);
            }
        }
        else
            unzCloseCurrentFile(uf); //don't lose the error
        free(buf);
    }
    return err;
}


int do_extract(uf, input, opt_extract_without_path,opt_overwrite,password)
    unzFile uf;
    char *input;
    int opt_extract_without_path;
    int opt_overwrite;
    const char* password;
{
    uLong i;
    unz_global_info64 gi;
    int err;

    err = unzGetGlobalInfo64(uf,&gi);
    if (err!=UNZ_OK)
        printf("error %d with zipfile in unzGetGlobalInfo \n",err);

    for (i=0;i<gi.number_entry;i++)
    {
        if (do_extract_currentfile(uf,input, &opt_extract_without_path, &opt_overwrite, NULL, NULL) != UNZ_OK)
            break;

        if ((i+1)<gi.number_entry)
        {
            err = unzGoToNextFile(uf);
            if (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzGoToNextFile\n",err);
                break;
            }
        }
    }

    return 0;
}

int do_extract_onefile(uf,filename,opt_extract_without_path,opt_overwrite,password,passwordPageCrypted)
    unzFile uf;
    const char* filename;
    int opt_extract_without_path;
    int opt_overwrite;
    const char* password;
    unsigned char* passwordPageCrypted;
{
    if (unzLocateFile(uf,filename,CASESENSITIVITY)!=UNZ_OK)
    {
        printf("file %s not found in the zipfile\n",filename);
        return 2;
    }

    if (do_extract_currentfile(uf,filename,&opt_extract_without_path, &opt_overwrite, password, passwordPageCrypted) == UNZ_OK)
        return 0;
    else
        return 1;
}

