/* Regression OS adapters. Rendering bodies and the Cairo image implementation
 * are real. */
#include <cairo.h>
#include <limits.h>
#include <math.h>
#include <pthread.h>
#include <sched.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Sandbox /proc restrictions prevent LeakSanitizer; address/UB checks remain
 * active. */
int __lsan_is_turned_off(void)
{
    return 1;
}

typedef unsigned char uchar; /* Inferno byte spelling at this test boundary. */
typedef unsigned long
    ulong; /* Host pixel-storage spelling used by the original interface. */
typedef unsigned int uint; /* Native unsigned scalar, not a wire format. */
typedef long long vlong;   /* Signed file offset. */
typedef unsigned long long uvlong; /* Checked offset arithmetic. */
typedef uint32_t uint32;           /* Fixed ARGB pixel word. */
typedef struct Prog
    Prog; /* Opaque VM type; no VM heap is executed in this fixture. */
typedef struct Channel Channel; /* Opaque graph event capability. */
typedef struct
{
    int unused;
} Lock; /* Inferno lock API is adapted to a pthread publication mutex. */
typedef struct
{
    int unused;
} QLock; /* Frame-owner gate, separate from publication. */
typedef union
{
    float f;
    char c[sizeof(float)];
} floatToChars_t; /* Original native packed command representation. */
ELEMENT_TYPES

typedef struct
{
    graphic_el *groot;
} Proc; /* Stable graph root borrowed by rendering. */
static Proc
    process; /* One owner graph, kept immutable until all jobs complete. */
static pthread_mutex_t publication =
    PTHREAD_MUTEX_INITIALIZER; /* Protect completion publication. */
static pthread_mutex_t frame =
    PTHREAD_MUTEX_INITIALIZER; /* Serialize public frame calls. */
static pthread_mutex_t statistics =
    PTHREAD_MUTEX_INITIALIZER;  /* Protect callback concurrency counters. */
static pthread_t threads[4096]; /* Joinable host wrappers, retired by the test
                                   after each frame. */
static int thread_count;        /* Number of initialized wrapper identities. */
static int concurrent; /* Number of executing independent raster callbacks. */
static int peak;       /* Highest observed callback concurrency. */
static int
    text_workers;       /* Text calls wrongly dispatched away from the owner. */
static pthread_t owner; /* Only this owner may use shared text helper state. */
int Xsize_ = 32;        /* Current destination width in pixels. */
int Ysize_ = 24;        /* Current destination height in pixels. */
ulong *graph_scrn_data; /* Allocated with enough bytes for the largest tested
                           canvas. */
#define KPDUPPG 1       /* OS adapter ignores Inferno namespace duplication. */
#define KPDUPFDG                                                               \
    2 /* Native Cairo jobs still receive distinct callback storage. */
#define KPDUPENVG 4 /* No VM state is shared through these test flags. */
#define DD(...)                                                                \
    ((void)0) /* Original debug logging is irrelevant to pixel correctness. */
#define D()                                                                    \
    ((void)0)   /* Original debug logging is irrelevant to bounds correctness. \
                 */
#define OREAD 0 /* Read-only file open in the PNG adapter. */

/* Publication primitives serialize the production done handoff with native
 * memory barriers. */
static void lock(Lock *value)
{
    (void)value;
    pthread_mutex_lock(&publication);
}
/* Release the fixture publication gate; value is an ignored Inferno lock
 * identity. */
static void unlock(Lock *value)
{
    (void)value;
    pthread_mutex_unlock(&publication);
}
/* Acquire the separate frame-owner gate; value is borrowed and ignored by this
 * adapter. */
static void qlock(QLock *value)
{
    (void)value;
    pthread_mutex_lock(&frame);
}
/* Release the frame-owner gate after production composition/copy completes. */
static void qunlock(QLock *value)
{
    (void)value;
    pthread_mutex_unlock(&frame);
}
/* Yield this host thread without inspecting mutable node flags. */
static void osyield(void)
{
    sched_yield();
}
/* Borrow the stable owner graph; fixture code never transfers its storage. */
static Proc *getup(void)
{
    return &process;
}

/* Host launch closure owns only a callback and its borrowed,
 * publication-protected job. */
typedef struct
{
    void (*function)(void *);
    void *argument;
} Launch;

/* Invoke one actual production job; never touch its stack job after the
 * callback returns. */
static void *trampoline(void *pointer)
{
    Launch *launch = pointer;
    launch->function(launch->argument);
    free(launch);
    return NULL;
}

/* Start an actual pthread; failure is a test error, never a fake successful
 * submission. */
static void kproc(char *name, void (*function)(void *), void *argument,
                  int flags)
{
    Launch *launch = malloc(sizeof(*launch));
    (void)name;
    (void)flags;
    if (!launch || thread_count == 4096)
    {
        exit(2);
    }
    launch->function = function;
    launch->argument = argument;
    if (pthread_create(&threads[thread_count++], NULL, trampoline, launch))
    {
        exit(2);
    }
}

/* Native callback deliberately overlaps independent leaves and returns a real
 * Cairo surface. */
static cairo_surface_t *_draw_svg(char *data, double *width, double *height)
{
    cairo_surface_t *surface;
    (void)data;
    pthread_mutex_lock(&statistics);
    concurrent++;
    if (concurrent > peak)
    {
        peak = concurrent;
    }
    pthread_mutex_unlock(&statistics);
    usleep(1000);
    *width = *height = 3;
    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 3, 3);
    pthread_mutex_lock(&statistics);
    concurrent--;
    pthread_mutex_unlock(&statistics);
    return surface;
}

/* PNG routing test uses the same independent image callback; decoder I/O is
 * tested separately. */
static cairo_surface_t *_draw_png(char *data, double *width, double *height)
{
    return _draw_svg(data, width, height);
}

/* Model the non-thread-safe FreeType helper by requiring the frame owner
 * thread. */
static cairo_surface_t *_draw_text(graphic_el *element, double *width,
                                   double *height)
{
    (void)element;
    if (!pthread_equal(pthread_self(), owner))
    {
        text_workers++;
    }
    return _draw_svg(NULL, width, height);
}

static long read_limit =
    2; /* Force short positional reads instead of one successful full read. */
static int
    reads; /* Number of file calls, used to reject calls after failure. */
static int closes;       /* Successful opens must produce exactly one close. */
static int open_failure; /* Inject a missing file. */

/* Deterministic native file boundary: each byte equals its requested absolute
 * position. */
static long kpread(int fd, void *data, long count, vlong offset)
{
    long index;
    (void)fd;
    reads++;
    if (read_limit <= 0)
    {
        return read_limit;
    }
    if (count > read_limit)
    {
        count = read_limit;
    }
    for (index = 0; index < count; index++)
    {
        ((uchar *)data)[index] = (uchar)(offset + index);
    }
    return count;
}
/* Inject open failure or return the fixture descriptor; path/mode remain
 * borrowed. */
static int kopen(char *path, int mode)
{
    (void)path;
    (void)mode;
    return open_failure ? -1 : 3;
}
/* Validate exact descriptor closure; return zero on success and fail the
 * fixture otherwise. */
static int kclose(int fd)
{
    if (fd != 3)
    {
        exit(2);
    }
    closes++;
    return 0;
}

RENDER_IMPLEMENTATION
PNG_IMPLEMENTATION

/* Join host wrapper teardown after the production completion barrier has
 * already returned. */
static void retire_threads(void)
{
    int index;
    for (index = 0; index < thread_count; index++)
    {
        pthread_join(threads[index], NULL);
    }
    thread_count = 0;
}

/* Initialize an owner-held node; each test frees its published surface before
 * stack retirement. */
static void initialize(graphic_el *element, int type)
{
    memset(element, 0, sizeof(*element));
    element->type = type;
    element->sx = element->sy = 1;
    element->line_width = 1;
    element->line_a = element->fill_a = 1;
    element->flags = GR_EL_WAIT_TO_RENDER;
}

/* Exercise null/truncated/misaligned input, terminal jobs, bounded native
 * concurrency and PNG I/O. */
int main(void)
{
    graphic_el element;
    graphic_el leaves[25];
    char bytes[32];
    float coordinate = 8;
    cairo_surface_t *surface;
    png_closure_t file = {3, 0};
    unsigned char copied[7];
    double width = 9;
    double height = 9;
    int index;
    int cycle;
    owner = pthread_self();
    if (render_shape(NULL))
    {
        fprintf(stderr, "test failed at generated line %d\n", __LINE__);
        return 1;
    }
    initialize(&element, GR_EL_SHAPE);
    bytes[0] = SHP_OP_MOVE_TO;
    memmove(bytes + 1, &coordinate, sizeof(coordinate));
    memmove(bytes + 1 + sizeof(coordinate), &coordinate, sizeof(coordinate));
    element.data = bytes;
    for (index = 1; index < 1 + 2 * (int)sizeof(float); index++)
    {
        element.data_len = index;
        if (render_shape(&element))
        {
            fprintf(stderr, "truncated shape accepted\n");
            fprintf(stderr, "test failed at generated line %d\n", __LINE__);
            return 1;
        }
    }
    element.data_len = 1 + 2 * sizeof(float);
    surface = render_shape(&element);
    if (!surface)
    {
        fprintf(stderr, "valid packed shape rejected\n");
        fprintf(stderr, "test failed at generated line %d\n", __LINE__);
        return 1;
    }
    cairo_surface_destroy(surface);
    /* Exercise every fixed-width operation at every truncated payload boundary.
     */
    {
        const int operations[] = {SHP_OP_MOVE_TO, SHP_OP_LINE_TO,
                                  SHP_OP_CURVE_TO, SHP_OP_ARC_CW,
                                  SHP_OP_ARC_CCW};
        const int counts[] = {2, 2, 6, 5, 5};
        unsigned operation;
        for (operation = 0;
             operation < sizeof(operations) / sizeof(operations[0]);
             operation++)
        {
            int extent = 1 + counts[operation] * (int)sizeof(float);
            int component;
            bytes[0] = operations[operation];
            for (component = 0; component < counts[operation]; component++)
            {
                memmove(bytes + 1 + component * sizeof(float), &coordinate,
                        sizeof(coordinate));
            }
            for (index = 1; index < extent; index++)
            {
                element.data_len = index;
                if (render_shape(&element))
                {
                    fprintf(stderr, "truncated fixed operation accepted\n");
                    return 1;
                }
            }
            element.data_len = extent;
            surface = render_shape(&element);
            if (!surface)
            {
                fprintf(stderr, "valid fixed operation rejected\n");
                return 1;
            }
            cairo_surface_destroy(surface);
            {
                float invalid = INFINITY;
                memmove(bytes + 1, &invalid, sizeof(invalid));
                if (render_shape(&element))
                {
                    fprintf(stderr, "nonfinite shape accepted\n");
                    return 1;
                }
            }
        }
    }
    bytes[0] = SHP_OP_TEXT;
    bytes[1] = 'x';
    element.data_len = 2;
    if (render_shape(&element))
    {
        fprintf(stderr, "test failed at generated line %d\n", __LINE__);
        return 1;
    }
    bytes[2] = 0;
    element.data_len = 3;
    surface = render_shape(&element);
    if (!surface)
    {
        fprintf(stderr, "test failed at generated line %d\n", __LINE__);
        return 1;
    }
    cairo_surface_destroy(surface);
    graph_scrn_data = calloc(64 * 64, sizeof(uint32_t));
    if (!graph_scrn_data)
    {
        return 2;
    }
    for (cycle = 0; cycle < 12; cycle++)
    {
        for (index = 0; index < 25; index++)
        {
            initialize(&leaves[index], index == 8    ? GR_EL_TEXT
                                       : index == 12 ? GR_EL_PNG
                                                     : GR_EL_SVG);
            leaves[index].next = index == 24 ? NULL : &leaves[index + 1];
        }
        process.groot = leaves;
        Xsize_ = 32 + cycle;
        Ysize_ = 24 + cycle;
        render_gr_stack_to_main_screen();
        retire_threads();
        for (index = 0; index < 25; index++)
        {
            if (leaves[index].flags & GR_EL_WAIT_TO_RENDER ||
                !leaves[index].sfc)
            {
                fprintf(stderr, "test failed at generated line %d\n", __LINE__);
                return 1;
            }
            cairo_surface_destroy(leaves[index].sfc);
            leaves[index].sfc = NULL;
        }
    }
    /* A hidden zero-scale node must not poison the Cairo context for following
     * nodes. */
    {
        cairo_surface_t *target =
            cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 16, 16);
        cairo_t *context = cairo_create(target);
        initialize(&leaves[0], GR_EL_SVG);
        initialize(&leaves[1], GR_EL_SVG);
        leaves[0].sx = 0;
        leaves[0].next = &leaves[1];
        render_gr_stack_to_surface(target, context, &leaves[0]);
        retire_threads();
        if (cairo_status(context) != CAIRO_STATUS_SUCCESS)
        {
            fprintf(stderr, "singular node poisoned frame\n");
            return 1;
        }
        cairo_destroy(context);
        cairo_surface_destroy(target);
        cairo_surface_destroy(leaves[0].sfc);
        cairo_surface_destroy(leaves[1].sfc);
    }
    initialize(&element, 255);
    process.groot = &element;
    render_gr_stack_to_main_screen();
    retire_threads();
    if (element.flags & GR_EL_WAIT_TO_RENDER || element.sfc || text_workers ||
        peak > 8 || peak < 2)
    {
        fprintf(stderr, "test failed at generated line %d\n", __LINE__);
        return 1;
    }
    initialize(&element, GR_EL_CANVAS);
    process.groot = &element;
    render_gr_stack_to_main_screen();
    retire_threads();
    if (element.flags & GR_EL_WAIT_TO_RENDER || !element.sfc)
    {
        fprintf(stderr, "test failed at generated line %d\n", __LINE__);
        return 1;
    }
    cairo_surface_destroy(element.sfc);
    cairo_surface_destroy(sfc_main_screen);
    sfc_main_screen = NULL;
    free(graph_scrn_data);
    if (png_chank_read_cb(&file, copied, sizeof(copied)) !=
            CAIRO_STATUS_SUCCESS ||
        file.offs != 7 || reads != 4)
    {
        fprintf(stderr, "test failed at generated line %d\n", __LINE__);
        return 1;
    }
    for (index = 0; index < 7; index++)
    {
        if (copied[index] != index)
        {
            fprintf(stderr, "test failed at generated line %d\n", __LINE__);
            return 1;
        }
    }
    read_limit = 0;
    if (png_chank_read_cb(&file, copied, 1) != CAIRO_STATUS_READ_ERROR ||
        file.offs != 7)
    {
        fprintf(stderr, "test failed at generated line %d\n", __LINE__);
        return 1;
    }
    read_limit = -1;
    if (png_chank_read_cb(&file, copied, 1) != CAIRO_STATUS_READ_ERROR ||
        file.offs != 7)
    {
        fprintf(stderr, "test failed at generated line %d\n", __LINE__);
        return 1;
    }
    open_failure = 1;
    if (production_draw_png("missing", &width, &height) || width || height ||
        closes)
    {
        fprintf(stderr, "test failed at generated line %d\n", __LINE__);
        return 1;
    }
    open_failure = 0;
    if (production_draw_png("bad", &width, &height) || closes != 1)
    {
        fprintf(stderr, "test failed at generated line %d\n", __LINE__);
        return 1;
    }
    printf("PASS graph render: packed bounds, 300 jobs, terminal flags, "
           "resize/stride, owner-only text, peak %d, exact PNG reads\n",
           peak);
    return 0;
}
