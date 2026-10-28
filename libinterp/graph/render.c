

#include <interp.h>
#include <isa.h>
#include <lib9.h>

#include "../e.h"

#undef Unknown
#include "../../emu/port/dat.h"
#include "../../emu/port/fns.h"
#include "kernel.h"

#include "graph.h"

#include "cairo-features.h"
#include <cairo.h>
#include <svg-cairo.h>

#define _USE_MATH_DEFINES
#include <limits.h>
#include <math.h>

cairo_surface_t *sfc_main_screen = NULL; /* Owned image; frame gate protects publication/replacement. */
uint main_screen_flags = 0; /* Owner-side dirty state; not a worker completion primitive. */

static cairo_surface_t *render_canvas(graphic_el *el);

static void render_gr_stack_to_surface(cairo_surface_t *sfc, cairo_t *cr,
                                       graphic_el *gr_stack_root);

/* Frames share one display surface. Mutators must keep the graph stable until
 * this owner returns. */
static QLock render_frame_lock;

/* Validate a complete packed shape stream before allocating or reading float
 * payloads. data borrows length bytes; zero means malformed/truncated/unknown
 * input, not a partial path. */
static int shape_stream_valid(const char *data, int length)
{
    int position = 0;
    if (!data || length <= 0)
    {
        return 0;
    }
    while (position < length)
    {
        unsigned operation = (unsigned char)data[position++];
        int count;
        int index;
        switch (operation)
        {
        case SHP_OP_MOVE_TO:
        case SHP_OP_LINE_TO:
            count = 2;
            break;
        case SHP_OP_CURVE_TO:
            count = 6;
            break;
        case SHP_OP_ARC_CW:
        case SHP_OP_ARC_CCW:
            count = 5;
            break;
        case SHP_OP_CLOSE_PATH:
        case SHP_OP_toMASK:
        case SHP_OP_unMASK:
            count = 0;
            break;
        case SHP_OP_TEXT:
        {
            const char *end = memchr(data + position, 0, length - position);
            if (!end)
            {
                return 0;
            }
            position = (int)(end - data) + 1;
            continue;
        }
        default:
            return 0;
        }
        if (count > (length - position) / (int)sizeof(floatToChars_t))
        {
            return 0;
        }
        for (index = 0; index < count; index++)
        {
            float value;
            memmove(&value, data + position, sizeof(value));
            if (!isfinite(value) ||
                ((operation == SHP_OP_ARC_CW || operation == SHP_OP_ARC_CCW) &&
                 index == 2 && value < 0))
            {
                return 0;
            }
            position += sizeof(floatToChars_t);
        }
    }
    return 1;
}

/* Read an already validated packed float without an aligned union lvalue. */
static float shape_float(const char *bytes)
{
    float value;
    memmove(&value, bytes, sizeof(value));
    return value;
}

/* Bound image allocation and floating-to-int conversion; reject over-budget
 * geometry. Negative/nonfinite extents fail; zero becomes a transparent
 * one-pixel axis. */
static cairo_surface_t *render_image(double width, double height)
{
    cairo_surface_t *surface;
    if (!isfinite(width) || !isfinite(height) || width < 0 || height < 0 ||
        width > 32768 || height > 32768)
    {
        return NULL;
    }
    width = width < 1 ? 1 : ceil(width);
    height = height < 1 ? 1 : ceil(height);
    if (width * height > 16 * 1024 * 1024)
    {
        return NULL;
    }
    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, (int)width,
                                         (int)height);
    if (!surface || cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS)
    {
        if (surface)
        {
            cairo_surface_destroy(surface);
        }
        return NULL;
    }
    return surface;
}

/* Render one stable shape; return an owned surface or NULL on invalid
 * input/allocation failure. */
static cairo_surface_t *render_shape(graphic_el *el)
{
    int i;
    cairo_surface_t *sfc;
    cairo_t *cr;

    if (!el)
    {
        return NULL;
    }
    if (!el->data || el->data_len <= 0)
    {
        if (el->canvas)
        {
            return render_canvas(el);
        }

        return NULL;
    }

    if (!shape_stream_valid(el->data, el->data_len))
    {
        return NULL;
    }
    sfc = render_image(2000, 2000);
    if (!sfc)
    {
        return NULL;
    }

    cr = cairo_create(sfc);

    int isMask = 0;
    int isPaint = 1;

    cairo_new_path(cr);

    for (i = 0; i < el->data_len;)
    {
        char key = el->data[i++];

        switch (key)
        {
        case SHP_OP_MOVE_TO:
        {
            float x, y;

            x = shape_float(el->data + i);
            i += sizeof(floatToChars_t);
            y = shape_float(el->data + i);
            i += sizeof(floatToChars_t);

            cairo_move_to(cr, x, y);
        }
        break;

        case SHP_OP_LINE_TO:
        {
            float x, y;

            x = shape_float(el->data + i);
            i += sizeof(floatToChars_t);
            y = shape_float(el->data + i);
            i += sizeof(floatToChars_t);

            cairo_line_to(cr, x, y);
        }
        break;

        case SHP_OP_CURVE_TO:
        {
            float x1, y1, x2, y2, x3, y3;

            x1 = shape_float(el->data + i);
            i += sizeof(floatToChars_t);
            y1 = shape_float(el->data + i);
            i += sizeof(floatToChars_t);

            x2 = shape_float(el->data + i);
            i += sizeof(floatToChars_t);
            y2 = shape_float(el->data + i);
            i += sizeof(floatToChars_t);

            x3 = shape_float(el->data + i);
            i += sizeof(floatToChars_t);
            y3 = shape_float(el->data + i);
            i += sizeof(floatToChars_t);

            cairo_curve_to(cr, x1, y1, x2, y2, x3, y3);
        }
        break;

        case SHP_OP_ARC_CW:
        {
            float x;
            float y;
            float r;
            float a_fr;
            float a_to;

            x = shape_float(el->data + i);
            i += sizeof(floatToChars_t);
            y = shape_float(el->data + i);
            i += sizeof(floatToChars_t);

            r = shape_float(el->data + i);
            i += sizeof(floatToChars_t);

            a_fr = shape_float(el->data + i);
            i += sizeof(floatToChars_t);
            a_to = shape_float(el->data + i);
            i += sizeof(floatToChars_t);

            cairo_arc(cr, x, y, r, a_fr, a_to);
        }
        break;

        case SHP_OP_ARC_CCW:
        {
            float x;
            float y;
            float r;
            float a_fr;
            float a_to;

            x = shape_float(el->data + i);
            i += sizeof(floatToChars_t);
            y = shape_float(el->data + i);
            i += sizeof(floatToChars_t);

            r = shape_float(el->data + i);
            i += sizeof(floatToChars_t);

            a_fr = shape_float(el->data + i);
            i += sizeof(floatToChars_t);
            a_to = shape_float(el->data + i);
            i += sizeof(floatToChars_t);

            cairo_arc_negative(cr, x, y, r, a_fr, a_to);
        }
        break;

        case SHP_OP_CLOSE_PATH:

            cairo_close_path(cr);
            break;

        case SHP_OP_TEXT:
        {

            char *txt = &(el->data[i]);
            while (el->data[i] != '\0')
            {
                i++;
            }
            i++; /* Consume the validated terminator, not another opcode. */

            isPaint = 0;
        }
        break;

        case SHP_OP_toMASK:

            isMask = 1;
            break;

        case SHP_OP_unMASK:

            isMask = 0;
            break;
        }
    }

    cairo_path_t *path = cairo_copy_path(cr);
    if (path->status != CAIRO_STATUS_SUCCESS)
    {
        cairo_path_destroy(path);
        cairo_destroy(cr);
        cairo_surface_destroy(sfc);
        return NULL;
    }

    cairo_set_line_width(cr, el->line_width);
    cairo_stroke_preserve(cr);

    cairo_stroke_extents(cr, &el->x_min, &el->y_min, &el->x_max, &el->y_max);

    el->w = el->x_max - el->x_min;
    el->h = el->y_max - el->y_min;

    cairo_destroy(cr);
    cairo_surface_destroy(sfc);

    sfc = render_image(el->w, el->h);
    if (!sfc)
    {
        cairo_path_destroy(path);
        return NULL;
    }
    cr = cairo_create(sfc);

    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint(cr);

    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

    double dx = el->x_min;
    double dy = el->y_min;

    cairo_translate(cr, -dx, -dy);

    cairo_append_path(cr, path);

    if (isMask)
    {

        cairo_move_to(cr, el->x_min, el->y_min);
        cairo_clip(cr);
        cairo_new_path(cr); /* path not consumed by clip()*/

        graphic_el *el2 = el->masked;
        for (; el2 != NULL; el2 = el2->next)
        {
            cairo_matrix_t matrix;

            if (el2->flags & GR_EL_WAIT_TO_RENDER)
            {

                if (el2->sfc)
                {
                    cairo_surface_destroy(el2->sfc);
                    el2->sfc = NULL;
                }

                if ((el2->type & 0xff) == GR_EL_SHAPE)
                {

                    el2->sfc = render_shape(el2);
                }
            }

            if (el2->flags & GR_EL_RENDERED && el2->sfc != NULL)
            {

                cairo_pattern_t *pat =
                    cairo_pattern_create_for_surface(el2->sfc);
                cairo_pattern_set_filter(pat, CAIRO_FILTER_GOOD);

                cairo_matrix_init_scale(&matrix, el2->sx, el2->sy);
                cairo_matrix_translate(&matrix, /*-el->x_min*/ -el2->x,
                                       /*-el->y_min*/ -el2->y);

                cairo_pattern_set_matrix(pat, &matrix);

                cairo_set_source(cr, pat);
                cairo_paint(cr);
                cairo_pattern_destroy(pat);
            }
        }
    }
    else if (isPaint)
    {

        cairo_set_source_rgba(cr, el->fill_r, el->fill_g, el->fill_b,
                              el->fill_a);
        cairo_fill_preserve(cr);
        cairo_set_source_rgba(cr, el->line_r, el->line_g, el->line_b,
                              el->line_a);
        cairo_set_line_width(cr, el->line_width);
        cairo_stroke(cr);
    }

    cairo_path_destroy(path);

    cairo_destroy(cr);

    el->flags &= ~GR_EL_WAIT_TO_RENDER;
    el->flags |= GR_EL_RENDERED;

    return sfc;
}

/* Render a stable child list into an owned cropped image; empty canvases are
 * transparent. el is borrowed by the owner until all recursive jobs join;
 * invalid extents return NULL. */
static cairo_surface_t *render_canvas(graphic_el *el)
{
    cairo_surface_t *sfc2 = render_image(2000, 2000);
    if (!sfc2)
    {
        return NULL;
    }

    cairo_t *cr2 = cairo_create(sfc2);
    cairo_set_operator(cr2, CAIRO_OPERATOR_CLEAR);
    cairo_paint(cr2);
    cairo_destroy(cr2);

    double x1, y1, x2, y2;

    cr2 = cairo_create(sfc2);

    render_gr_stack_to_surface(sfc2, cr2, el->canvas);

    cairo_destroy(cr2);

    graphic_el *el_it = el->canvas;
    x1 = 10000000.;
    y1 = 10000000.;
    x2 = -10000000.;
    y2 = -10000000.;
    for (; el_it != NULL; el_it = el_it->next)
    {
        if (el_it->x_min < el_it->x_max && el_it->y_min < el_it->y_max)
        {
            if (el_it->x_min < x1)
                x1 = el_it->x_min;
            if (el_it->x_max > x2)
                x2 = el_it->x_max;
            if (el_it->y_min < y1)
                y1 = el_it->y_min;
            if (el_it->y_max > y2)
                y2 = el_it->y_max;
        }
    }

    el->x_min = x1;
    el->x_max = x2;
    el->y_min = y1;
    el->y_max = y2;

    double w = x2 - x1;
    double h = y2 - y1;
    el->w = w;
    el->h = h;

    if (x2 < x1 || y2 < y1)
    {
        x1 = y1 = x2 = y2 = 0;
        el->x_min = el->y_min = el->x_max = el->y_max = 0;
        w = h = 0;
    }
    el->w = w;
    el->h = h;
    cairo_surface_t *sfc3 = render_image(w, h);
    if (!sfc3)
    {
        cairo_surface_destroy(sfc2);
        return NULL;
    }
    cairo_t *cr3 = cairo_create(sfc3);

    cairo_set_source_surface(cr3, sfc2, -x1, -y1);
    cairo_paint(cr3);

    cairo_destroy(cr3);

    cairo_surface_destroy(sfc2);

    return sfc3;
}

/* Native work item: completion protects publication of the element's surface
 * and bounds. The submitting owner keeps both job and element alive and
 * immutable until join. */
typedef struct RenderJob RenderJob;
struct RenderJob
{
    Lock completion;     /* Synchronizes done and publication; never held while
                            rendering. */
    int done;            /* Only read/written under completion. */
    graphic_el *element; /* Borrowed stable graph leaf; no VM instruction
                            executes here. */
};

/* Render one owner-exclusive node and retire its pending flag on every normal
 * path. Text/canvas/mask nodes are dispatched synchronously by the frame owner.
 */
static void draw_gr_stack_el(graphic_el *el)
{
    cairo_surface_t *surface = NULL;
    double width = 0;
    double height = 0;
    if (!el)
    {
        return;
    }
    switch (el->type & 0xff)
    {
    case GR_EL_SHAPE:
        surface = render_shape(el);
        break;
    case GR_EL_SVG:
        surface = _draw_svg(el->data, &width, &height);
        break;
    case GR_EL_PNG:
        surface = _draw_png(el->data, &width, &height);
        break;
    case GR_EL_TEXT:
        surface = _draw_text(el, &width, &height);
        break;
    case GR_EL_CANVAS:
        surface = render_canvas(el);
        break;
    }
    if (surface && cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS)
    {
        cairo_surface_destroy(surface);
        surface = NULL;
    }
    if (el->sfc)
    {
        cairo_surface_destroy(el->sfc);
    }
    el->sfc = surface;
    if (surface &&
        ((el->type & 0xff) == GR_EL_TEXT || (el->type & 0xff) == GR_EL_SVG ||
         (el->type & 0xff) == GR_EL_PNG))
    {
        el->w = width;
        el->h = height;
        el->x_min = el->y_min = 0;
        el->x_max = width;
        el->y_max = height;
    }
    el->flags &= ~(GR_EL_WAIT_TO_RENDER | GR_EL_RENDERED);
    if (surface)
    {
        el->flags |= GR_EL_RENDERED;
    }
}

/* Render a stable leaf, then publish completion as the final access to borrowed
 * job storage. */
static void thread_draw_gr_stack_el(void *parameter)
{
    RenderJob *job = parameter;
    draw_gr_stack_el(job->element);
    lock(&job->completion);
    job->done = 1;
    unlock(&job->completion);
}

/* Join all submitted jobs before stack job storage, node data or Cairo surfaces
 * can be reused. Native osyield remains cooperative polling, but no
 * unsynchronized element flag is polled. */
static void join_render_jobs(RenderJob *jobs, int count)
{
    int index;
    for (index = 0; index < count; index++)
    {
        for (;;)
        {
            int done;
            lock(&jobs[index].completion);
            done = jobs[index].done;
            unlock(&jobs[index].completion);
            if (done)
            {
                break;
            }
            osyield();
        }
    }
}

/* Rasterize stable independent leaves, join dependencies, then composite in
 * list order. Surface/context/root are borrowed synchronously; workers execute
 * no VM instructions. */
static void render_gr_stack_to_surface(cairo_surface_t *sfc, cairo_t *cr,
                                       graphic_el *gr_stack_root)
{

    graphic_el *el = gr_stack_root;

    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint(cr);

    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

    /* Text helpers share FreeType globals; dependent canvases/masks need
     * completed children. Parallel work is restricted to stable independent
     * leaves, in batches of at most eight. */
    RenderJob jobs[8];
    int submitted = 0;
    for (; el != NULL; el = el->next)
    {
        if (!(el->flags & GR_EL_WAIT_TO_RENDER))
        {
            continue;
        }
        if ((el->type & 0xff) == GR_EL_TEXT ||
            (el->type & 0xff) == GR_EL_CANVAS || el->canvas || el->masked)
        {
            join_render_jobs(jobs, submitted);
            submitted = 0;
            draw_gr_stack_el(el);
        }
        else
        {
            memset(&jobs[submitted], 0, sizeof(jobs[submitted]));
            jobs[submitted].element = el;
            kproc("graph leaf", thread_draw_gr_stack_el, &jobs[submitted],
                  KPDUPPG | KPDUPFDG | KPDUPENVG);
            submitted++;
            if (submitted == 8)
            {
                join_render_jobs(jobs, submitted);
                submitted = 0;
            }
        }
    }
    join_render_jobs(jobs, submitted);

    el = gr_stack_root;

    for (; el != NULL; el = el->next)
    {
        cairo_matrix_t matrix;

        if (el->flags & GR_EL_RENDERED && el->sfc != NULL)
        {
            /* A zero scale hides a node; never poison the whole Cairo context
             * with a singular matrix. */
            if (!isfinite(el->sx) || !isfinite(el->sy) ||
                !isfinite(el->angle) || !isfinite(el->x) || !isfinite(el->y) ||
                el->sx == 0 || el->sy == 0)
            {
                continue;
            }

            if ((el->type & 0xff) == GR_EL_TEXT)
            {
                DD("(el->type & 0xff): %d, el->sfc: %p", (el->type & 0xff),
                   el->sfc);
            }

            double whlf =
                /*el->x_min +*/ (double)cairo_image_surface_get_width(el->sfc) /
                2.;
            double hhlf =
                /*el->y_min +*/ (double)cairo_image_surface_get_height(
                    el->sfc) /
                2.;

            double xc, yc;

            xc = -el->x_min - whlf /**/ - el->x;
            yc = -el->y_min - hhlf /**/ - el->y;

            cairo_pattern_t *pat = cairo_pattern_create_for_surface(el->sfc);
            cairo_pattern_set_filter(pat, CAIRO_FILTER_GOOD);

            cairo_matrix_init_translate(&matrix, whlf, hhlf);
            cairo_matrix_scale(&matrix, el->sx, el->sy);

            cairo_matrix_rotate(&matrix, el->angle);

            cairo_matrix_translate(&matrix, xc, yc);

            cairo_pattern_set_matrix(pat, &matrix);

            cairo_set_source(cr, pat);
            cairo_pattern_destroy(pat);
            cairo_paint(cr);
        }
    }
}

/* Render one stable owner graph to the shared display, with serialized frame
 * publication. Invalid/zero screen extents and allocation failures return
 * without copying pixels. */
void render_gr_stack_to_main_screen(void)
{
    Proc *process;
    cairo_t *context;
    unsigned char *bytes;
    int stride;
    int row;
    if (Xsize_ <= 0 || Ysize_ <= 0 || !graph_scrn_data)
    {
        return;
    }
    qlock(&render_frame_lock);
    if (sfc_main_screen &&
        (cairo_image_surface_get_width(sfc_main_screen) != Xsize_ ||
         cairo_image_surface_get_height(sfc_main_screen) != Ysize_))
    {
        cairo_surface_destroy(sfc_main_screen);
        sfc_main_screen = NULL;
    }
    if (!sfc_main_screen)
    {
        sfc_main_screen = render_image(Xsize_, Ysize_);
    }
    if (!sfc_main_screen)
    {
        qunlock(&render_frame_lock);
        return;
    }
    process = getup();
    context = cairo_create(sfc_main_screen);
    render_gr_stack_to_surface(sfc_main_screen, context, process->groot);
    cairo_destroy(context);
    cairo_surface_flush(sfc_main_screen);
    bytes = cairo_image_surface_get_data(sfc_main_screen);
    stride = cairo_image_surface_get_stride(sfc_main_screen);
    for (row = 0; row < Ysize_; row++)
    {
        memmove(
            (uchar *)graph_scrn_data + (size_t)row * Xsize_ * sizeof(uint32_t),
            bytes + (size_t)row * stride, (size_t)Xsize_ * sizeof(uint32_t));
    }
    qunlock(&render_frame_lock);
}

/* Invalidate an owner-held node and its ancestors; parent links must form an
 * acyclic graph. Call only outside an active render of this graph; no worker
 * may race these mutable flags. */
void reset_rendered_flag(graphic_el **el)
{
    if (el && *el)
    {
        (*el)->flags &= ~GR_EL_RENDERED;
        (*el)->flags |= GR_EL_WAIT_TO_RENDER;

        graphic_el **e = (*el)->root;
        for (; e && *e; e = (*e)->root)
        {
            (*e)->flags &= ~GR_EL_RENDERED;
            (*e)->flags |= GR_EL_WAIT_TO_RENDER;
        }
    }

    main_screen_flags &= ~GR_EL_RENDERED;
    main_screen_flags |= GR_EL_WAIT_TO_RENDER;
}
