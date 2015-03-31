#include <iostream>
#include <string>
#include <cstring>
#include <math.h>
#include <jpeglib.h>

#include "ImageData.h"


#define CAIRO_RGBA_TO_UINT32(red, green, blue, alpha)         \
  (((alpha) << 24) | ((red) << 16) | ((green) << 8) | (blue))

using namespace std;

static void jpgError(j_common_ptr cinfo) {
    cout << "ERROR" << endl;
}

int ImageData::Load(string filename) {
loaded = false;
fname = filename;

  if(pixbuf != NULL)
    free(pixbuf);

    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    FILE * infile;      /* source file */
    JSAMPARRAY buffer;      /* Output row buffer */
    int row_stride;     /* physical row width in output buffer */

    if ((infile = fopen(filename.c_str(), "rb")) == NULL) {
        fprintf(stderr, "can't open %s\n", filename.c_str());
        return 0;
    }

    /* Step 1: allocate and initialize JPEG decompression object */

    /* We set up the normal JPEG error routines, then override error_exit. */
    cinfo.err = jpeg_std_error(&jerr);
    jerr.error_exit = jpgError;
    /* Establish the setjmp return context for my_error_exit to use. */
//    if (setjmp(jerr.setjmp_buffer)) {
//        jpeg_destroy_decompress(&cinfo);
//        fclose(infile);
//        return 0;
//    }
    /* Now we can initialize the JPEG decompression object. */
    jpeg_create_decompress(&cinfo);

    /* Step 2: specify data source (eg, a file) */

    jpeg_stdio_src(&cinfo, infile);

    /* Step 3: read file parameters with jpeg_read_header() */

    (void) jpeg_read_header(&cinfo, TRUE);
    /* Step 4: set parameters for decompression */

    /* In this example, we don't need to change any of the defaults set by
     * jpeg_read_header(), so we do nothing here.
     */

  /* Step 5: Start decompressor */

  (void) jpeg_start_decompress(&cinfo);
  /* We can ignore the return value since suspension is not possible
   * with the stdio data source.
   */
width = cinfo.output_width;
height = cinfo.output_height;
  /* We may need to do some setup of our own at this point before reading
   * the data.  After jpeg_start_decompress() we have the correct scaled
   * output image dimensions available, as well as the output colormap
   * if we asked for color quantization.
   * In this example, we need to make an output work buffer of the right size.
   */ 
  /* JSAMPLEs per row in output buffer */
  row_stride = cinfo.output_width * (cinfo.output_components + 1);
  /* Make a one-row-high sample array that will go away when done with image */
  buffer = (*cinfo.mem->alloc_sarray)
        ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

    pixbuf = (unsigned char *)malloc(row_stride * cinfo.output_height);

  /* Step 6: while (scan lines remain to be read) */
  /*           jpeg_read_scanlines(...); */

  /* Here we use the library's state variable cinfo.output_scanline as the
   * loop counter, so that we don't have to keep track ourselves.
   */
   int count = 0;
  while (cinfo.output_scanline < cinfo.output_height) {
    /* jpeg_read_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could ask for
     * more than one scanline at a time if that's more convenient.
     */
    //unsigned char *rowp[1];
    //rowp[0] = (unsigned char *) pixbuf + count;
    jpeg_read_scanlines(&cinfo, buffer, 1);


      unsigned char *p_surface = pixbuf + count;
      unsigned char *p_buffer = buffer[0];


      for (int x = 0; x < cinfo.output_width; x++) {
        unsigned char r = p_buffer[0];
        unsigned char g = p_buffer[1];
        unsigned char b = p_buffer[2];
        uint32_t pixel = CAIRO_RGBA_TO_UINT32 (r, g, b, 0xff);
        memcpy (p_surface, &pixel, sizeof (uint32_t));

        p_surface += 4;
        p_buffer += 3 /*srcinfo.output_components*/;
      }

    count += row_stride;
  }
       /* Step 7: Finish decompression */

    (void) jpeg_finish_decompress(&cinfo);
    /* Step 8: Release JPEG decompression object */

    /* This is an important step since it will release a good deal of memory. */
    jpeg_destroy_decompress(&cinfo);

    fclose(infile);
    /* And we're done! */
loaded = true;
    return row_stride;

}
