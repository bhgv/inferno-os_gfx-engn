#include "../e.h"
#include <interp.h>
#include <isa.h>
#include <lib9.h>
#undef Unknown
#include "../../emu/port/dat.h"
#include "../../emu/port/fns.h"
#include "graph.h"
#include "kernel.h"
#include <cairo.h>
#include <limits.h>

/* One decoder invocation owns its file and position; no global cursor is shared
 * by workers. */
typedef struct png_closure_t
{
    int fd;     /* Owned open descriptor, closed after decoder completion. */
    vlong offs; /* Next absolute byte offset; advanced only by successful reads.
                 */
} png_closure_t;

/* Supply exactly length bytes to Cairo, handling ordinary short native reads.
 * closure borrows a live file owner; failure rejects the PNG, never
 * uninitialized data. */
static cairo_status_t png_chank_read_cb(void *closure, unsigned char *data,
                                        unsigned int length)
{
    png_closure_t *owner = closure;
    unsigned int total = 0;
    while (total < length)
    {
        unsigned int wanted = length - total;
        long received;
        if (wanted > INT_MAX)
        {
            wanted = INT_MAX;
        }
        received = kpread(owner->fd, data + total, wanted, owner->offs);
        if (received <= 0 || (unsigned long)received > wanted ||
            owner->offs < 0 ||
            (uvlong)owner->offs > ((uvlong)~0 >> 1) - (uvlong)received)
        {
            return CAIRO_STATUS_READ_ERROR;
        }
        total += (unsigned int)received;
        owner->offs += received;
    }
    return CAIRO_STATUS_SUCCESS;
}

/* Decode a path into an owned Cairo image, or NULL on open/read/decode failure.
 * Optional dimensions are reset on entry. Every successfully opened descriptor
 * is closed once. */
cairo_surface_t *_draw_png(char *path, double *width, double *height)
{
    cairo_surface_t *surface;
    png_closure_t owner;
    if (width)
    {
        *width = 0;
    }
    if (height)
    {
        *height = 0;
    }
    if (!path)
    {
        return NULL;
    }
    owner.fd = kopen(path, OREAD);
    owner.offs = 0;
    if (owner.fd < 0)
    {
        return NULL;
    }
    surface =
        cairo_image_surface_create_from_png_stream(png_chank_read_cb, &owner);
    kclose(owner.fd);
    if (!surface || cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS)
    {
        if (surface)
        {
            cairo_surface_destroy(surface);
        }
        return NULL;
    }
    if (width)
    {
        *width = cairo_image_surface_get_width(surface);
    }
    if (height)
    {
        *height = cairo_image_surface_get_height(surface);
    }
    return surface;
}
