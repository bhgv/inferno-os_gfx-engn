

#include <lib9.h>
#include <isa.h>
#include <interp.h>

#include "../e.h"
//#include "../emod.h"

#undef Unknown
#include "../../emu/port/dat.h"
#include "../../emu/port/fns.h"
#include "kernel.h"

#include "graph.h"

#include "cairo-features.h"
#include <cairo.h>
#include <svg-cairo.h>

#define _USE_MATH_DEFINES
#include <math.h>



typedef struct png_closure_t {
    int fd;

	vlong offs;
} png_closure_t;



static cairo_status_t
png_chank_read_cb(void *closure, unsigned char *data, unsigned int length)
{
	png_closure_t* d = (png_closure_t*)closure;

	d->offs += kpread(d->fd, data, length, d->offs);

//    if (bytes_read != length)
//		return CAIRO_STATUS_READ_ERROR;

    return CAIRO_STATUS_SUCCESS;
}



cairo_surface_t *
_draw_png(char* png_path, double *w_out, double *h_out)
{
	cairo_surface_t* sfc;
	png_closure_t d;

	if (!png_path)
		return NULL;

	d.fd = kopen(png_path, OREAD);
	d.offs = 0;

	sfc = cairo_image_surface_create_from_png_stream(png_chank_read_cb, &d);
	kclose(d.fd);

	cairo_status_t status;

	status = cairo_surface_status(sfc);
	if (status) {
//		cairo_test_status_t ret;

		cairo_surface_destroy (sfc);
		sfc = NULL;

	//	ret = cairo_test_status_from_status (ctx, status);
//		if (ret != CAIRO_TEST_NO_MEMORY) {
//			cairo_test_log (ctx,
//					"Error: failed to create surface from PNG: %s - %s\n",
//					filename,
//					cairo_status_to_string (status));
//		}
	} else {
		if (w_out)
			*w_out = (double)cairo_image_surface_get_width(sfc);

		if (h_out)
			*h_out = (double)cairo_image_surface_get_height(sfc);
	}
	return sfc;
}



