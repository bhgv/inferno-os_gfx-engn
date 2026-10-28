# Experimental graph renderer: ownership/bounds regression

[Русский](README.ru.md) · [Renderer](../render.c) · [PNG adapter](../aux_png.c) ·
[Event boundary](../channel_wrk.c)

The experimental parallel renderer was located in `render.c`: it launched one
`kproc` per dirty node, then polled ordinary mutable element flags. The text helper
in `aux_text.c` also keeps shared FreeType state, so dispatching text nodes in
parallel was unsafe. This repair is deliberately local to this graphics repository;
it does not import this GUI engine into FlashVM's Dis core.

## Repaired defects

- Null element fallback dereferenced `el`; packed float loads were unaligned and
  truncated commands/text were scanned without bounds. Complete validation now
  precedes interpretation, and floats are copied into aligned locals.
- Text-op terminators are consumed. Unknown/truncated/nonfinite commands fail
  instead of reading beyond the stream. Image extents are checked before int
  conversion/allocation; each image is capped at 16M pixels with axes <=32768.
- Completion is published through a lock-protected job record. The submitter
  joins before reusing stack jobs or reading completed surface/bounds. It no longer
  uses an unsynchronized WAIT flag as a native memory-publication mechanism.
- Independent leaves run in batches of eight. Text, dependent masks and nested
  canvases run on the owner after previous leaves join. This avoids racing the
  shared text helper and keeps composition order. It is not a persistent pool.
- Every ordinary job path retires WAIT, including unsupported kinds and failed
  images. PNG nodes are dispatched; empty canvases are finite transparent images.
- Frame surface creation/resizing is serialized. The surface is flushed and copied
  using Cairo stride and fixed ARGB32 pixel width. Zero/nonfinite transforms are
  skipped rather than poisoning the entire Cairo context with a singular matrix.
- PNG reads handle short `kpread` calls until exactly the requested extent, reject
  EOF/error/offset overflow, and do not decode or close a failed open descriptor.
- Touch input is native coordinate storage. Retaining `D2H(arevt)` accessed a
  nonexistent Heap prefix; the new array owns a scalar copy and no such retain occurs.

## Executable checks

From the repository root, with a C compiler, pthreads and Cairo image library:

```sh
mkdir -p build/tmp
TMPDIR="$PWD/build/tmp" python3 libinterp/graph/tests/render_regression.py
# Explicit alternatives for another installed Cairo library/compiler:
# CC=clang GRAPH_CAIRO_LIBS='-L/explicit/cairo/lib -lcairo -lm -pthread' ...
```

The driver compiles the **production renderer and PNG bodies**, using the bundled
Cairo declarations and a real native Cairo image library. Inferno process/lock/file
boundaries are adapted to pthreads and controlled I/O; SVG/text callbacks return
real independent Cairo surfaces. It checks packed/truncated input, 300 node jobs,
bounded parallel callbacks, owner-only text, terminal flags, repeated resize,
short-read/EOF/error PNG behavior and the independent touch copy. It does not
emulate successful rendering results or build the entire OS/window system.

Default C flags enable AddressSanitizer and UBSan. LeakSanitizer is explicitly
disabled in the fixture because the development sandbox cannot expose `/proc`
thread metadata; no leak-sanitizer result is claimed. The original `7273ec7`
renderer reproduces a null-member-access UB failure under the same fixture.
`GRAPH_RENDER_SOURCE` can select a saved baseline file for reproduction.

## Remaining contract and limits

The caller must retain a stable **acyclic** graph, node payloads, display buffer
and screen configuration for the whole synchronous frame. The frame lock does not
magically serialize arbitrary graph mutation or OS resize/free outside this entry.
No AS/Dis instructions or VM allocation may execute from these raster callbacks.
Independent VM contexts require separate owners/heaps and copied jobs.

Join still uses cooperative `osyield` polling under a real completion lock; a
condition-waiting worker pool is future work. OS-thread teardown may lag a completed
callback, so eight is a bound on scheduled raster jobs per batch, not a promise
about every transient OS thread. Canvas scratch surfaces still have the existing
2000×2000 limitation; arbitrary large nested canvases, cycles, full text/resource
lifetime audit, and safe mutation during rendering remain open. The touch test
repairs one invalid retain, not the entire legacy channel/GC protocol. Native
Windows/macOS/Android display acceptance and a full OS build were not performed.

The unfinished `E.getLevelsList` export now raises an explicit unsupported error.
It no longer dereferences an uninitialized return or replaces VM Array data with
a native linked-list pointer. A real typed conversion remains future work.
