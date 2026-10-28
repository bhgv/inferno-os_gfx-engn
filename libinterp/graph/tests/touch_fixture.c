/* Scalar-copy boundary regression; no live Inferno heap is accessed by these
 * adapters. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAXPOINTS 10  /* Public event coordinate count. */
#define H ((void *)1) /* Distinct VM nil sentinel. */
#define nil NULL      /* Host null spelling. */
#define H2D(type, value)                                                       \
    ((type)(value)) /* Test allocator returns its initialized payload          \
                       directly. */
#define D2H(value)                                                             \
    ((void *)(value)) /* Retain validation must reject the caller's native     \
                         array. */
#define E_ShapeEvent_TOUCH 1 /* Original generated event variant. */
typedef struct
{
    double x;
    double y;
} E_XY_coord; /* Copied native coordinate value. */
typedef struct
{
    unsigned char *data;
} Array; /* Owns only the test scalar copy. */
typedef struct
{
    int unused;
} E_Shape; /* Opaque graph capability. */
typedef struct
{
    int pick;
    E_Shape *shape;
    union
    {
        struct
        {
            Array *data;
        } TOUCH;
    } u;
} E_ShapeEvent; /* Fields used by the production function. */
typedef struct
{
    int unused;
} Heap; /* Fake retain carrier, never a real native input prefix. */
typedef struct
{
    void *prog;
} Receiver; /* Ready receiver observation. */
typedef struct
{
    Receiver *recv;
    void *buf;
} Channel; /* Typed channel boundary used by dispatch. */
typedef struct
{
    Channel *exec_mod_chan;
} graphic_el; /* The test graph retains its event channel. */
typedef struct
{
    void *s;
    void *d;
} REG; /* Only the two dispatch fields used by the production function. */
static void *TXY_coord; /* Symbolic scalar-array allocation descriptor. */
static void *TShapeEvent_TOUCH; /* Symbolic event allocation descriptor. */
static E_Shape
    shape; /* The adapter returns one stable borrowed test capability. */
static E_ShapeEvent
    *received;    /* Captured event, freed by the fixture after verification. */
static int locks; /* Entry/exit balance for the native VM lock adapter. */
static int bad_retain; /* Any retain of native input is a defect, not accepted
                          heap authority. */

/* Allocate an independent scalar array; no caller buffer is adopted by
 * ownership. */
static Array *heaparray(void *type, int count)
{
    Array *array = malloc(sizeof(*array));
    (void)type;
    if (!array)
    {
        exit(2);
    }
    array->data = calloc(count, sizeof(E_XY_coord));
    if (!array->data)
    {
        exit(2);
    }
    return array;
}

/* The event descriptor has no other user callback in this fixture. */
static void *heap(void *type)
{
    (void)type;
    return calloc(1, sizeof(E_ShapeEvent));
}
/* Enter the test VM-lock region; no heap collection is simulated. */
static void gclock(void)
{
    locks++;
}
/* Leave the test VM-lock region; main checks that entry and exit balance. */
static void gcunlock(void)
{
    locks--;
}
/* Borrow one stable test shape capability; the element address is not adopted.
 */
static E_Shape *shape_by_graphic_el(graphic_el *element)
{
    (void)element;
    return &shape;
}
/* Record any forbidden native-input retain so the production regression rejects
 * it. */
static void heap_ref_inc(void *value)
{
    (void)value;
    bad_retain++;
}
/* Supply the expected mover identity; synchronous dispatch captures its frame
 * separately. */
static void movp_(REG *registers)
{
    (void)registers;
}
/* Fail this fixture on an unexpected VM error; no successful result is
 * fabricated. */
static void error(char *message)
{
    fprintf(stderr, "%s\n", message);
    exit(2);
}

/* The caller's stack event pointer is borrowed only during synchronous
 * dispatch. */
static void isend_(REG *registers, void (*mover)(REG *))
{
    (void)mover;
    received = *(E_ShapeEvent **)registers->s;
}

TOUCH_IMPLEMENTATION

/* Pass a native coordinate array with no Heap prefix; verify independent scalar
 * storage. */
int main(void)
{
    Receiver receiver = {&shape};
    Channel channel = {&receiver, H};
    graphic_el element = {&channel};
    E_XY_coord points[MAXPOINTS];
    int index;
    for (index = 0; index < MAXPOINTS; index++)
    {
        points[index].x = index;
        points[index].y = -index;
    }
    if (!csend_touch_evt_(&element, points) || !received || locks ||
        bad_retain || received->u.TOUCH.data->data == (unsigned char *)points ||
        memcmp(received->u.TOUCH.data->data, points, sizeof(points)))
    {
        fprintf(
            stderr,
            "native touch array was retained as a heap object or not copied\n");
        return 1;
    }
    free(received->u.TOUCH.data->data);
    free(received->u.TOUCH.data);
    free(received);
    puts("PASS touch event: independent coordinate copy, balanced VM lock, no "
         "invalid Heap retain");
    return 0;
}
