/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

//This code was suggested on the StackOverflow topic: http://stackoverflow.com/questions/13961912/read-a-char-string-as-a-virtual-file

struct zmem_data
{
	byte *buf;
	byte *mask;
	size_t length;
};

static voidpf zmemopen(voidpf opaque, const char *filename, int mode)
{
	if((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER) != ZLIB_FILEFUNC_MODE_READ)
		return NULL;

	uint64_t *pos = malloc(sizeof(uint64_t));
	if(pos != NULL)
		*pos = 0;
	return pos;
}

static uint64_t zmemread(voidpf opaque, voidpf stream, void* buf, uint64_t size)
{
	struct zmem_data *data = (struct zmem_data*)opaque;
	uint64_t *pos = (uint64_t*)stream;
	uint64_t remaining = data->length - *pos;
	uint64_t i;


	size = size < remaining ? size : remaining;

	if(*pos > data->length)
		return 0;

	for(i = 0; i < size; i++)
	{
		((byte*) buf)[i] = ~(data->buf[*pos + i] ^ data->mask[*pos + i]);
	}

	*pos += i;

	return i;
}

static int zmemclose(voidpf opaque, voidpf stream)
{
	free(stream);
	return 0;
}

static int zmemerror(voidpf opaque, voidpf stream)
{
	return stream == NULL;
}

static long zmemtell(voidpf opaque, voidpf stream)
{
	return * (long *) stream;
}

static long zmemseek(voidpf opaque, voidpf stream, uint64_t offset, int origin)
{
	struct zmem_data *data = opaque;
	uint64_t * pos = stream;

	switch (origin)
	{
		case ZLIB_FILEFUNC_SEEK_SET:
			*pos = offset;
			break;

		case ZLIB_FILEFUNC_SEEK_CUR:
			*pos = *pos + offset;
			break;

		case ZLIB_FILEFUNC_SEEK_END:
			*pos = data->length + offset;
			break;

		default:
			return -1;
	}

	return 0;
}

static void init_zmemfile(zlib_filefunc_def *inst, char *bufZip, char* mask, size_t length)
{
	struct zmem_data *data = malloc(sizeof(struct zmem_data));

	data->buf = (byte *) bufZip;
	data->mask = (byte *) mask;
	data->length = length;

	inst->opaque = data;
	inst->zopen_file = zmemopen;
	inst->zread_file = zmemread;
	inst->zwrite_file = NULL;
	inst->ztell_file = zmemtell;
	inst->zseek_file = zmemseek;
	inst->zclose_file = zmemclose;
	inst->zerror_file = zmemerror;
}

static void destroy_zmemfile(zlib_filefunc_def *inst)
{
	free(inst->opaque);
	inst->opaque = NULL;
}
