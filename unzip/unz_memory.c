struct zmem_data {
  char *buf;
  size_t length;
};

static voidpf zmemopen(voidpf opaque, const char *filename, int mode) {
    if ((mode&ZLIB_FILEFUNC_MODE_READWRITEFILTER) != ZLIB_FILEFUNC_MODE_READ)
        return NULL;

    uLong *pos = malloc(sizeof(uLong));    *pos = 0;    return pos;
}

static uLong zmemread(voidpf opaque, voidpf stream, void* buf, uLong size) {
  struct zmem_data *data = (struct zmem_data*)opaque;
  uLong *pos = (uLong*)stream;
  uLong remaining = data->length - *pos;
  uLong readlength = size < remaining ? size : remaining;
  if (*pos > data->length) return 0;
  memcpy(buf, data->buf+*pos, readlength);
  *pos += readlength;
  return readlength;
}

static uLong zmemwrite(voidpf opaque, voidpf stream, const void *buf, uLong size) {
  /* no write support for now */
  return 0;
}

static int zmemclose(voidpf opaque, voidpf stream) {
  free(stream);
  return 0;
}

static int zmemerror(voidpf opaque, voidpf stream) {
  if (stream == NULL) return 1;
  else return 0;
}

static long zmemtell(voidpf opaque, voidpf stream) {
  return *(uLong*)stream;
}

static long zmemseek(voidpf opaque, voidpf stream, uLong offset, int origin) {
  struct zmem_data *data = (struct zmem_data*)opaque;
  uLong *pos = (uLong*)stream;
  switch (origin) {
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

static void init_zmemfile(zlib_filefunc_def *inst, char *buf, size_t length) {
  struct zmem_data *data = malloc(sizeof(struct zmem_data));
  data->buf = buf;
  data->length = length;
  inst->opaque = data;
  inst->zopen_file = zmemopen;
  inst->zread_file = zmemread;
  inst->zwrite_file = zmemwrite;
  inst->ztell_file = zmemtell;
  inst->zseek_file = zmemseek;
  inst->zclose_file = zmemclose;
  inst->zerror_file = zmemerror;
}
static void destroy_zmemfile(zlib_filefunc_def *inst) {
  free(inst->opaque);
  inst->opaque = NULL;
}

/*void example() {
 zlib_filefunc_dec fileops;
 init_zmemfile(&fileops, buffer, buffer_length);
 unzFile zf = unzOpen2(NULL, &fileops);
  ... process zip file ...
 unzClose(zf);
 destroy_zmemfile(&fileops);
}*/
