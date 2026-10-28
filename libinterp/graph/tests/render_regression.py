"""Execute production graph rendering with real Cairo and pthread OS-boundary adapters."""
from pathlib import Path
import os
import re
import shlex
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[3]  # Repository containing the reviewed sources.


# Remove only unavailable Inferno include directives; all production renderer bodies are compiled intact.
def body(path):
    return re.sub(r'^\s*#include[^\n]*\n', '', Path(path).read_text(), flags=re.M)


# Build the real Cairo regression; SDK/library/compiler flags may be supplied explicitly.
# No generated binary is committed and a failing run always propagates its status.
def main():
    with tempfile.TemporaryDirectory(prefix='graph-render-') as temporary:
        work = Path(temporary)
        (work / 'cairo-features.h').write_text('#define CAIRO_HAS_IMAGE_SURFACE 1\n#define CAIRO_HAS_PNG_FUNCTIONS 1\n')
        dat = (ROOT / 'emu/port/dat.h').read_text()
        graph = (ROOT / 'libinterp/graph/graph.h').read_text()
        element = dat[dat.index('typedef struct graphic_el'):dat.index('\nstruct Proc\n', dat.index('typedef struct graphic_el'))]
        kinds = dat[dat.rfind('enum {', 0, dat.index('GR_EL_SHAPE')):dat.index('typedef struct graphic_el')]
        operations = graph[graph.rfind('enum {', 0, graph.index('SHP_OP_MOVE_TO')):graph.index('};', graph.index('SHP_OP_MOVE_TO'))+2]
        prefix = (ROOT / 'libinterp/graph/tests/render_fixture.c').read_text()
        source = Path(os.environ.get('GRAPH_RENDER_SOURCE', ROOT / 'libinterp/graph/render.c'))
        unit = prefix.replace('ELEMENT_TYPES', kinds+element+operations)
        unit = unit.replace('RENDER_IMPLEMENTATION', body(source))
        unit = unit.replace('PNG_IMPLEMENTATION', body(ROOT / 'libinterp/graph/aux_png.c').replace('_draw_png(', 'production_draw_png('))
        target = work / 'fixture.c'
        target.write_text(unit)
        if os.environ.get("GRAPH_UNIT_PATH"):
            Path(os.environ["GRAPH_UNIT_PATH"]).write_text(unit)
        command = shlex.split(os.environ.get('CC', 'cc'))
        command += shlex.split(os.environ.get('CFLAGS', '-O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all'))
        command += ['-std=gnu11', '-I'+str(work), '-I'+str(ROOT/'libgraph/cairo/src'), str(target)]
        command += shlex.split(os.environ.get('GRAPH_CAIRO_LIBS', '-Wl,-l:libcairo.so.2 -lm -pthread'))
        binary = work / 'regression'
        subprocess.run(command+['-o', str(binary)], check=True)
        subprocess.run([str(binary)], check=True, timeout=45)
        channel = (ROOT / "libinterp/graph/channel_wrk.c").read_text()
        start = channel.index("int\ncsend_touch_evt_(")
        brace = channel.index("{", start)
        depth = 0
        end = None
        for token in re.finditer(r'"(?:\\.|[^"\\])*"|/\*[\s\S]*?\*/|//[^\n]*|[{}]', channel[brace:]):
            if token.group() == "{":
                depth += 1
            elif token.group() == "}":
                depth -= 1
                if depth == 0:
                    end = brace+token.end()
                    break
        if end is None:
            raise ValueError("unterminated production touch function")
        touch = work / "touch.c"
        touch.write_text((ROOT / "libinterp/graph/tests/touch_fixture.c").read_text().replace(
            "TOUCH_IMPLEMENTATION", channel[start:end]))
        touch_binary = work / "touch"
        subprocess.run(shlex.split(os.environ.get("CC", "cc"))+["-O2", str(touch), "-o", str(touch_binary)], check=True)
        subprocess.run([str(touch_binary)], check=True)
        module = (ROOT / "libinterp/e.c").read_text()
        start = module.index("void E_getLevelsList(")
        end = module.index("\n}", start)+2
        rejected = work / "unsupported.c"
        rejected.write_text('#include <setjmp.h>\n#include <string.h>\n'
                            'static jmp_buf boundary; static char message[128];\n'
                            'static void error(char *text) { strcpy(message,text); longjmp(boundary,1); }\n'
                            +module[start:end]+
                            '\nint main(void) { if (!setjmp(boundary)) { E_getLevelsList((void*)1); return 1; } '
                            'return strcmp(message,"E.getLevelsList is not implemented"); }\n')
        rejected_binary = work / "unsupported"
        subprocess.run(shlex.split(os.environ.get("CC", "cc"))+["-O2", str(rejected), "-o", str(rejected_binary)], check=True)
        subprocess.run([str(rejected_binary)], check=True)
        print("PASS legacy E.getLevelsList: explicit failure, no forged Array or return-slot access")




if __name__ == '__main__':
    main()
