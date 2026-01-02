

#include <lib9.h>
#include <isa.h>
#include <interp.h>

#include "../e.h"
//#include "../emod.h"

#undef Unknown
#include "../../emu/port/dat.h"
#include "../../emu/port/fns.h"
#include "kernel.h"

#ifdef _MSC_VER 
  //not #if defined(_WIN32) || defined(_WIN64) because we have strncasecmp in mingw
  #define strncasecmp _strnicmp
  #define strcasecmp _stricmp
#else
  #include <strings.h>
#endif

#include "graph.h"

#include "cairo-features.h"
//#include <cairo.h>
//#include <svg-cairo.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include <ft2build.h>
#include FT_FREETYPE_H

//#include "Imlib2.h"
#define CAIRO_HAS_FT_FONT 1
#include "cairo.h"
#include <cairo-ft.h>

//#include <harfbuzz/hb.h>
//#include <harfbuzz/hb-ft.h>
//#include <harfbuzz/hb-icu.h>
#include <hb.h>
#include <hb-ft.h>
//#include <hb-icu.h>


//#include <fontconfig.h>

//#ifdef X
//#include "cairo-xlib.h"
//#endif

//#include "bspline.h"

#include "utf8.h"


#ifndef M_PI
# define M_PI  3.14159265358979323846  /* pi */
#endif

#ifndef DEFAULT_FONT
  #define DEFAULT_FONT "/fonts/ttf/default/Arial.ttf"
#endif



#define screenW Xsize_
#define screenH Ysize_




static FT_Library ft_lib = NULL;
static cairo_font_face_t *cairo_ft_face = NULL;

static cairo_user_data_key_t ukey;

static FT_Face ftface = NULL;

static hb_font_t *hb_ft_font = NULL;
static hb_face_t *hb_ft_face = NULL;

static hb_buffer_t *hb_buf   = NULL;

static cairo_glyph_t       *cairo_glyphs = NULL;
static unsigned int        glyph_count   = 0;



double _cur_alpha2draw = 0.; //1;
double _nxt_alpha2draw = 0.; //1;





typedef struct _font_list_item_t font_list_item_t;

struct _font_list_item_t {
    uint32_t          h;
    char             *name;
    char             *path;

    font_list_item_t *next;
};

static font_list_item_t *font_list = NULL;







cairo_font_face_t *cairo_ft_font_face_create_for_ft_face(FT_Face face,  int load_flags);









typedef struct {
	float r, g, b, a;
} color_t;


typedef struct {
    char *stylename;
    char *fontname;
    char *fontname_b;
    char *fontname_i;
    char *fontname_bi;
    float  fontsize;

    color_t color;
    color_t border_color;
    color_t shadow_color;

    int  bold;
    int  italic;
    int  underline;
    int  strikeout;
    int  fontscalex;
    int  fontscaley;
    int  spacing;
    float  angle;

    int borderstyle;
    float outline;
    float shadow;
    int alignment;
    float marginleft;
    float marginright;
    float marginvertical;
    char  *encoding;
    float alphaLevel;

	uint block_flags;
} style_t;



static style_t _default_style = {
    .stylename        = "Default",
    .fontname         =
#ifdef DEFAULT_FONT
                      DEFAULT_FONT,
#else
                      "DejaVuSans",
#endif
    .fontname_b       =
#ifdef DEFAULT_FONT_B
                      DEFAULT_FONT_B,
#else
                      "DejaVuSans-Bold",
#endif
    .fontname_i       =
#ifdef DEFAULT_FONT_I
                      DEFAULT_FONT_I,
#else
                      "DejaVuSans-Oblique",
#endif
    .fontname_bi      =
#ifdef DEFAULT_FONT_BI
                      DEFAULT_FONT_BI,
#else
                      "DejaVuSans-BoldOblique",
#endif
    .fontsize         = 18.,
    .color            = {0.9, 0.4, 0.2, 0.},
    .border_color     = {0.4, 0.8, 0.1, 0.},
    .shadow_color     = {0.2, 0.1, 0.7, 0.5},

    .bold             = 200, //0, //200,
    .italic           = 0,
    .underline        = 0,
    .strikeout        = 0,
    .fontscalex       = 100,
    .fontscaley       = 100,
    .spacing          = 0,
    .angle            = 0.,
    .borderstyle      = 1,
    .outline          = 2,
    .shadow           = 1,
    .alignment        = 2,
    .marginleft       = 20.,
    .marginright      = 20.,
    .marginvertical   = 40.,

    .encoding         = NULL, //"utf-8",
    .alphaLevel       = 0.,

	.block_flags      = 0,
};


static void do_clean(cairo_t *cr);



style_t* new_style()
{
    style_t *st = malloc(sizeof(style_t));
    if (st) {
        memcpy(st, &_default_style, sizeof(style_t));
    }

    return st;
}

void del_style(style_t* st)
{
    if (!st)
        return;

    free(st->stylename);
    free(st->fontname);
    free(st->encoding);

    free(st);
}










typedef struct {
    style_t stl;
    float a;
    float w, h;
    float y;
} cr_block_size_t;



static cr_block_size_t tcr_block_size;





static float _X = 10.;
static float _Y = 10.;

static float _line_begin_Y = 0.;





static uint32_t str_calc_hash(char *str)
{
    uint32_t h = 0;

    if (!str)
        return 0;

    for (; *str != '\0'; str++) {
        char c = *str;
        h = ((h & 0xff000000) >> 24) ^ (h << 3) ^ (c - ' ');
    }
    return h;
}  









#if 0
static void make_available_fonts_list()
{
    FcPattern *pat;
    FcFontSet *fs;
    FcObjectSet *os;
    FcChar8 *file_path;
    FcConfig *config;
    FcBool result;
    int i;

    result = FcInit();
    config = FcConfigGetCurrent();
    FcConfigSetRescanInterval(config, 0);

    // show the fonts (debugging)
    pat = FcPatternCreate();
    os = FcObjectSetBuild (/*FC_FAMILY, FC_STYLE, FC_LANG,*/ FC_FILE, (char *) 0);
    fs = FcFontList(config, pat, os);
    //printf("!!!> Total fonts: %d", fs->nfont);
    for (i=0; fs && i < fs->nfont; i++) {
        FcPattern *font = fs->fonts[i];//FcFontSetFont(fs, i);
        if (FcPatternGetString(font, FC_FILE, 0, &file_path) == FcResultMatch) {
            //printf("!!!> Filename_path: %s\n", file_path);
            font_list_item_t *fli = malloc(sizeof(font_list_item_t));
            if (fli) {
                int ll = strlen(file_path);
                //printf("!!!> ll: %d\n", ll);
                char *pth = malloc(ll + 1);
                //printf("!!!> pth = %p: %s\n", pth);
                memcpy(pth, file_path, ll);
                pth[ll] = '\0';
                //printf("!!!> Filename_path 1: %s\n", pth);
                fli->path = pth; //strdup(file_path);
                //printf("!!!> Filename_path 2: %s\n", fli->path);
                fli->h = 0;
                int i, b, e;
                for (i = 0; file_path[i] != '\0'; i++) {
                    char c = file_path[i];
                    if (c == '/')
                        b = i + 1;
                    if (c == '.')
                        e = i;
                }
                if (b > e)
                    e = i;

                int l = e - b + 1;
                char *name = malloc(l);
                if (name) {
                    for (i = 0; i < l-1; i++)
                        name[i] = file_path[i + b];
                    name[i] = '\0';
                    fli->h = str_calc_hash(name);
                    fli->name = name;
                    //printf("!!!> File name: %s\n", name);

                    fli->next = font_list;
                    font_list = fli;
                } else {
                    free(fli->path);
                    free(fli);
                }
            } // if (fli)
        } // if (FcPatternGetString(font, FC_FILE, 0, &file_path) == FcResultMatch)
    } // for (i=0; fs && i < fs->nfont; i++)
    if (fs)
        FcFontSetDestroy(fs);
}
#endif




static text_line_sfc_el_t* del_text_line_sfc_el(text_line_sfc_el_t* el)
{
	if (!el)
		return NULL;

	if (el->cr)
		cairo_destroy(el->cr);

	if (el->sfc)
		cairo_surface_destroy(el->sfc);

	text_line_sfc_el_t* el_nxt = el->next;

	return el_nxt;
}


static int del_text_line_sfc_list(text_line_sfc_el_t* root_el)
{
//	if (!root_el)
//		return 0;

	while (root_el) {
		root_el = del_text_line_sfc_el(root_el);
	}
	return 0;
}


static text_line_t* del_text_line_el(text_line_t* el)
{
	if (!el)
		return NULL;

	if (el->sfc_lst_root)
		del_text_line_sfc_list(el->sfc_lst_root);

	text_line_t* el_nxt = el->next;

	return el_nxt;
}


static text_line_t* text_split_to_lines(char* text, int n, uint w_max, uint flags)
{
	if (!text)
		return NULL;

	int i;
	text_line_t* el_root = NULL;
	text_line_t** el_nest;
	
    char *b = text,
         *p = text;
    utf8_int32_t c = 0;
    while(1) {
        char *p_next = utf8codepoint(p, &c);
        if (c == '\r' || c == '\n' || c == '\0') {
            *p = '\0';
            if (p_next - b > 1) {
				text_line_t* el_new = malloc(sizeof(text_line_t));
				memset(el_new, 0, sizeof(text_line_t));
				for (el_nest = &el_root; *el_nest; el_nest = &((*el_nest)->next));
				*el_nest = el_new;

				el_new->flags = flags;
				el_new->n     = n++;

				el_new->text = b;

				el_new->w_max = w_max;

                if (c == '\0') {
                    break;
                }
            }
            b = p_next;
        }
        p = p_next;
    }

	return el_root;
}







#if 1 //simpler or faster functions to draw text

//simpler one
static void do_text_free_font()
{
    if (cairo_ft_face) {
        cairo_font_face_set_user_data(cairo_ft_face, &ukey, ftface, (cairo_destroy_func_t)FT_Done_Face);
        //int cnt = cairo_font_face_get_reference_count(cairo_ft_face);
        //printf("ref of cairo_ft_face = %d\n", cnt);
        cairo_font_face_destroy(cairo_ft_face);
        cairo_ft_face = NULL;
    }

    if (hb_ft_font) {
        hb_font_destroy(hb_ft_font);
        hb_ft_font = NULL;
    }
    if (hb_ft_face) {
        hb_face_destroy(hb_ft_face);
        hb_ft_face = NULL;
    }

    if (cairo_glyphs) {
        free(cairo_glyphs);
        cairo_glyphs = NULL;
    }

    if (hb_buf) {
        hb_buffer_destroy(hb_buf);
        hb_buf = NULL;
    }

    if (ftface) {
//        FT_Done_Face(ftface);
        ftface = NULL;
    }
}


static char *do_text_get_font_path(char *fontname, char fontstyle)
{
    if (!fontname)
        return NULL;

    font_list_item_t *fli = font_list;
    uint32_t h = str_calc_hash(fontname);

    for (; fli; fli = fli->next) {
        if (fli->h == h) {
            if (!strcasecmp(fli->name, fontname))
                return fli->path;
        }
    }

    switch (fontstyle) {
        default:
            fontname = DEFAULT_FONT;
            break;
/*
		case 'i':
            fontname = DEFAULT_FONT_I;
            break;
        case 'b':
            fontname = DEFAULT_FONT_B;
            break;
        case ('b' ^ 'i'):
            fontname = DEFAULT_FONT_BI;
            break;
*/
    };

	if (font_list) {
	    fli = font_list;
	    h = str_calc_hash(fontname);

	    for (; fli; fli = fli->next) {
	        if (fli->h == h) {
	            if (!strcasecmp(fli->name, fontname))
	                return fli->path;
	        }
	    }
	} else {
		return fontname;
	}

    return NULL;
}


static void do_text_set_font(cairo_t *cr, char* txt, double fontsize, style_t* stl)
{
    FT_Error err;

// harfbuzz specifics VVV
    /*unsigned int*/        glyph_count   = 0;
    hb_glyph_info_t     *glyph_info   = NULL;
    hb_glyph_position_t *glyph_pos    = NULL;
// harfbuzz specifics AAA

    if (!ft_lib) {
        err = FT_Init_FreeType(&ft_lib);
        if (err)
            ft_lib = NULL;
    }

    char *fontname = DEFAULT_FONT;
    char fontstyle = '\0';
/*//{}
    if (ft_lib && stl) {
        if (stl->bold > 0) {
            fontname = stl->fontname_b;
            fontstyle = 'b';
        }
        if (stl->italic > 0) {
            if (stl->bold > 0) {
                fontname = stl->fontname_bi;
                fontstyle = 'b' ^ 'i';
            } else {
                fontname = stl->fontname_i;
                fontstyle = 'i';
            }
        }
        if (fontname == NULL)
            fontname = stl->fontname;
    }
*/

    char *fn_buf = do_text_get_font_path(fontname, fontstyle);
    if (fn_buf) {
/*
        char fn_buf[256];
        snprintf(fn_buf, sizeof(fn_buf),
#ifdef X
            "/usr/share/fonts/truetype/ubuntu/%s.ttf",
#else
            "/usr/share/fonts/%s.ttf",
#endif
            fontname);
*/
        err = FT_New_Face(ft_lib, fn_buf, 0, &ftface);
        if (err /* == FT_Err_Unknown_File_Format*/) {
            if (ftface) {
                FT_Done_Face(ftface);
                ftface = NULL;
            }
#if 1 // other place to set the default font
            snprintf(fn_buf, sizeof(fn_buf), "/fonts/ttf/default/%s.ttf", DEFAULT_FONT);
            err = FT_New_Face(ft_lib, fn_buf, 0, &ftface);
            if (err /* == FT_Err_Unknown_File_Format*/) {
                if (ftface) {
                    FT_Done_Face(ftface);
                    ftface = NULL;
                }
            }
#endif
        }
    } else {
        ftface = NULL;
    }

    static cairo_user_data_key_t ukey;

    if (ftface) {
        FT_Set_Char_Size(ftface, (FT_F26Dot6)(fontsize * 64), (FT_F26Dot6)(fontsize * 64), 72, 72);
        cairo_ft_face = cairo_ft_font_face_create_for_ft_face(ftface, 0);

        hb_ft_font = hb_ft_font_create(ftface, NULL);
        hb_ft_face = hb_ft_face_create(ftface, NULL);

        hb_buf = hb_buffer_create();

        //hb_buffer_set_unicode_funcs(hb_buf, hb_icu_get_unicode_funcs());
        hb_buffer_set_direction(hb_buf, HB_DIRECTION_LTR);
        //hb_buffer_set_script(hb_buf, HB_SCRIPT_CYRILLIC); /* see hb-unicode.h */
        hb_buffer_set_language(hb_buf, hb_language_from_string("ru", -1));

        hb_buffer_add_utf8(hb_buf, txt, strlen(txt), 0, strlen(txt));
        hb_shape(hb_ft_font, hb_buf, NULL, 0);

        glyph_count = hb_buffer_get_length (hb_buf);

        glyph_info   = hb_buffer_get_glyph_infos(hb_buf, NULL);
        glyph_pos    = hb_buffer_get_glyph_positions(hb_buf, NULL);
        cairo_glyphs = malloc(sizeof(cairo_glyph_t) * glyph_count);

//        unsigned int string_width_in_pixels = 0;
//        for (int i = 0; i < glyph_count; i++) {
//            string_width_in_pixels += glyph_pos[i].x_advance / 64;
//        }

        int hb_x = 0, hb_y = 0;
        for (int i = 0; i < glyph_count; i++) {
            cairo_glyphs[i].index = glyph_info[i].codepoint;
            cairo_glyphs[i].x = hb_x + (glyph_pos[i].x_offset / 64.0);
            cairo_glyphs[i].y = hb_y - (glyph_pos[i].y_offset / 64.0);
            hb_x += glyph_pos[i].x_advance / 64.0;
            hb_y -= glyph_pos[i].y_advance / 64.0;
        }

        if (cairo_ft_face) {
            cairo_set_font_face(cr, cairo_ft_face);
        }
    }

    if (!cairo_ft_face) {
        cairo_select_font_face(cr, 
			DEFAULT_FONT,        //{}(stl && stl->fontname) ? stl->fontname : DEFAULT_FONT_INT,
            CAIRO_FONT_SLANT_NORMAL, //{} (stl && stl->italic) ? CAIRO_FONT_SLANT_ITALIC : CAIRO_FONT_SLANT_NORMAL,
            CAIRO_FONT_WEIGHT_NORMAL //{} (stl && stl->bold)   ? CAIRO_FONT_WEIGHT_BOLD  : CAIRO_FONT_WEIGHT_NORMAL
        );
    }
}


static void do_text_size(cairo_t *cr, float x, float y, char* txt, style_t* stl)
{
    cairo_text_extents_t extents;  

    if (!txt || !stl)
        return;

    double fontsize = ((stl->fontsize) ? stl->fontsize : 46.);

    cairo_save(cr);

    do_text_set_font(cr, txt, fontsize, stl);

    cairo_set_font_size(cr, fontsize);

    //font XY scale
    cairo_matrix_t font_matrix;
    memset(&font_matrix, 0, sizeof(cairo_matrix_t));
    font_matrix.xx = (double)fontsize * (double)stl->fontscalex / 100.0;
    font_matrix.yy = (double)fontsize * (double)stl->fontscaley / 100.0;

    cairo_set_font_matrix(cr, &font_matrix);
//    cairo_scale(cr, (double)stl->fontscalex / 100.0, (double)stl->fontscaley / 100.0);

    cairo_text_extents(cr, txt, &extents);

    y += font_matrix.yy;

    // main text
    tcr_block_size.y = y;

    float tw = extents.x_advance, th = extents.y_advance;
    float tlen = sqrtf(tw * tw + th * th);

//    _X += tlen * cosf(-stl->angle * M_PI / 180.);
//    _Y += tlen * sinf(-stl->angle * M_PI / 180.);

//    th += font_matrix.yy;
    tlen = sqrtf(tw * tw + th * th);

    float a = ((stl->angle - tcr_block_size.a) * M_PI / 180.) + atan2(th, tw);

    tcr_block_size.w += tlen * cosf(-a);
    tcr_block_size.h += tlen * sinf(-a);

    do_text_free_font();

    cairo_restore(cr);
}


static void do_text(cairo_t *cr, float x, float y, char* txt, style_t* stl)
{
    cairo_text_extents_t extents;  

    if (!txt || !stl)
        return;

//DD("xy: %f, %f", x, y);

//    y += screenH;
//    x += screenW;

    double fontsize = ((stl->fontsize) ? stl->fontsize : 46.);

    double fgr = 0.1,
           fgg = 0.1,
           fgb = 0.1,
           fga = 0.1;

    double bdr = 0.1,
           bdg = 0.1,
           bdb = 0.1,
           bda = 0.1;

    double shr = 0.1,
           shg = 0.1,
           shb = 0.1,
           sha = 0.1;

    fgr = stl->color.r; // (double)((stl->color & 0x00ff0000) >> 16) / 255.0;
    fgg = stl->color.g; //(double)((stl->color & 0x0000ff00) >>  8) / 255.0;
    fgb = stl->color.b; //(double)((stl->color & 0x000000ff) >>  0) / 255.0;
    fga = 1.0 - stl->color.a; //_cur_alpha2draw * (1.0 - (double)((stl->color & 0xff000000) >> 24) / 255.0);

    bdr = stl->border_color.r; //(double)((stl->border_color & 0x00ff0000) >> 16) / 255.0;
    bdg = stl->border_color.g; //(double)((stl->border_color & 0x0000ff00) >>  8) / 255.0;
    bdb = stl->border_color.b; //(double)((stl->border_color & 0x000000ff) >>  0) / 255.0;
    bda = 1.0 - stl->border_color.a; //_cur_alpha2draw * (1.0 - (double)((stl->border_color & 0xff000000) >> 24) / 255.0);

    shr = stl->shadow_color.r; //(double)((stl->shadow_color & 0x00ff0000) >> 16) / 255.0;
    shg = stl->shadow_color.g; //(double)((stl->shadow_color & 0x0000ff00) >>  8) / 255.0;
    shb = stl->shadow_color.b; //(double)((stl->shadow_color & 0x000000ff) >>  0) / 255.0;
    sha = 1.0 - stl->shadow_color.a; //_cur_alpha2draw * (1.0 - (double)((stl->shadow_color & 0xff000000) >> 24) / 255.0);

//    _cur_alpha2draw = _nxt_alpha2draw;

    cairo_save(cr);

    do_text_set_font(cr, txt, fontsize, stl);

    cairo_set_font_size(cr, fontsize);

    cairo_set_antialias(cr, CAIRO_ANTIALIAS_BEST); //CAIRO_ANTIALIAS_SUBPIXEL); //

    //font XY scale
    cairo_matrix_t font_matrix;
    memset(&font_matrix, 0, sizeof(cairo_matrix_t));
    font_matrix.xx = (double)fontsize * (double)stl->fontscalex / 100.0;
    font_matrix.yy = (double)fontsize * (double)stl->fontscaley / 100.0;

    cairo_set_font_matrix(cr, &font_matrix);
//    cairo_scale(cr, (double)stl->fontscalex / 100.0, (double)stl->fontscaley / 100.0);

    if (!cairo_ft_face) {
        cairo_text_extents(cr, txt, &extents);
    } else {
        cairo_glyph_extents(cr, cairo_glyphs, glyph_count, &extents);

/*
DD("x_bearing: %f;\ny_bearing: %f;\nwidth: %f;\nheight: %f;\nx_advance: %f;\ny_advance %f;\n",
    extents.x_bearing, extents.y_bearing,
    extents.width, extents.height,
    extents.x_advance, extents.y_advance
);
*/
    }

	y -= extents.height + extents.y_bearing;
    y += extents.height; //font_matrix.yy;

    // Shadow
    if (stl->borderstyle == 1 && stl->shadow > 0.) {
        cairo_save(cr);
        cairo_set_source_rgba(cr,
                          shr,  //red
                          shg,  //green
                          shb,  //blue
                          sha   //alpha
                          ); 
        cairo_translate(cr, x + stl->shadow, y + stl->shadow);
        cairo_rotate(cr, -stl->angle * M_PI / 180.);
        cairo_move_to(cr, 0, 0);
        if (!cairo_ft_face) {
            cairo_text_path(cr, txt);
        } else {
//DD("cairo_glyphs: %p, glyph_count: %d", cairo_glyphs, glyph_count);
            cairo_glyph_path(cr, cairo_glyphs, glyph_count);
        }
        cairo_fill(cr);
        cairo_restore(cr);
    }

    // main text
    cairo_set_source_rgba(cr,
                          fgr,  //red
                          fgg,  //green
                          fgb,  //blue
                          fga   //alpha
                          );

    tcr_block_size.y = y;

    cairo_translate(cr, x, y);
    cairo_rotate(cr, -stl->angle * M_PI / 180.);
    cairo_move_to(cr, 0, 0);

    if (!cairo_ft_face) {
        cairo_text_path(cr, txt);
    } else {
        cairo_glyph_path(cr, cairo_glyphs, glyph_count);
    }

    cairo_fill_preserve(cr);
    
    if (stl->borderstyle == 1) {
        cairo_set_source_rgba(cr,
                          bdr,  //red
                          bdg,  //green
                          bdb,  //blue
                          bda   //alpha
                          ); 
        cairo_set_line_width(cr, 0.5*stl->outline);
        cairo_stroke(cr);
    }

    if (stl->underline != 0) {
        cairo_set_source_rgba(cr,
                          bdr,  //red
                          bdg,  //green
                          bdb,  //blue
                          bda   //alpha
                          );
        cairo_set_line_width(cr, stl->outline);

        cairo_new_path(cr);
        cairo_move_to(cr, 0., 0.14 * fontsize);
        cairo_rel_line_to(cr, extents.width, 0.);
        cairo_stroke(cr);
    }

    if (stl->strikeout != 0) {
        cairo_set_source_rgba(cr,
                          bdr,  //red
                          bdg,  //green
                          bdb,  //blue
                          bda   //alpha
                          );
        cairo_set_line_width(cr, stl->outline);

        cairo_new_path(cr);
        cairo_move_to(cr, 0., -.4 * extents.height);
        cairo_rel_line_to(cr, extents.width, 0.);
        cairo_stroke(cr);
    }

    float tw = extents.x_advance, th = extents.y_advance;
    float tlen = sqrtf(tw * tw + th * th);

    _X += tlen * cosf(-stl->angle * M_PI / 180.);
    _Y += tlen * sinf(-stl->angle * M_PI / 180.);

    th += font_matrix.yy;
    tlen = sqrtf(tw * tw + th * th);

    float a = ((stl->angle - tcr_block_size.a) * M_PI / 180.) + atan2(th, tw);

    tcr_block_size.w += tlen * cosf(-a);
    tcr_block_size.h += tlen * sinf(-a);

    do_text_free_font();

    cairo_restore(cr);
}


#else

//faster (??) one
static void _clusters_out(cairo_t *cr,
    cairo_scaled_font_t* scaled_face,
    cairo_glyph_t* glyphs, int glyph_count, 
    cairo_text_cluster_t* clusters, int cluster_count,
    float* w, float* h
    )
{
// draw each cluster
    int glyph_index = 0;
    int byte_index = 0;
    int i;

    if (!cr || !scaled_face || !glyphs || glyph_count == 0 || !clusters || cluster_count == 0 || !w || !h)
        return;

    *w = *h = 0;

    for (i = 0; i < cluster_count; i++) {
        cairo_text_cluster_t* cluster = &clusters[i];
        cairo_glyph_t* clusterglyphs = &glyphs[glyph_index];

        // get extents for the glyphs in the cluster
        cairo_text_extents_t extents;
        cairo_scaled_font_glyph_extents(scaled_face, clusterglyphs, cluster->num_glyphs, &extents);
        // ... for later use

        *w += extents.x_bearing; //width; //x_advance / 2.;
        *h += extents.y_bearing; //height; //y_advance / 2.;

        // put paths for current cluster to context
        cairo_glyph_path(cr, clusterglyphs, cluster->num_glyphs);

        // draw black text with green stroke
        cairo_set_source_rgba(cr, 0.2, 0.2, 0.2, 1.0);
        cairo_fill_preserve(cr);
        cairo_set_source_rgba(cr, 0, 1, 0, 1.0);
        cairo_set_line_width(cr, 0.5);
        cairo_stroke(cr);

        // glyph/byte position
        glyph_index += cluster->num_glyphs;
        byte_index += cluster->num_bytes;
    }
}


static void do_text(cairo_t *cr, float x, float y, char* txt, style_t* stl)
{
    cairo_text_extents_t extents;
    float w, h;

    if (!txt || !stl)
        return;

    double fgr = 0.1,
           fgg = 0.1,
           fgb = 0.1,
           fga = 0.1;

    double bdr = 0.1,
           bdg = 0.1,
           bdb = 0.1,
           bda = 0.1;

    double shr = 0.1,
           shg = 0.1,
           shb = 0.1,
           sha = 0.1;

    fgr = (double)((stl->color1 & 0x00ff0000) >> 16) / 255.0;
    fgg = (double)((stl->color1 & 0x0000ff00) >>  8) / 255.0;
    fgb = (double)((stl->color1 & 0x000000ff) >>  0) / 255.0;
    fga = 1.0 - (double)((stl->color1 & 0xff000000) >> 24) / 255.0;

    bdr = (double)((stl->border_color & 0x00ff0000) >> 16) / 255.0;
    bdg = (double)((stl->border_color & 0x0000ff00) >>  8) / 255.0;
    bdb = (double)((stl->border_color & 0x000000ff) >>  0) / 255.0;
    bda = 1.0 - (double)((stl->border_color & 0xff000000) >> 24) / 255.0;

    shr = (double)((stl->shadow_color & 0x00ff0000) >> 16) / 255.0;
    shg = (double)((stl->shadow_color & 0x0000ff00) >>  8) / 255.0;
    shb = (double)((stl->shadow_color & 0x000000ff) >>  0) / 255.0;
    sha = 1.0 - (double)((stl->shadow_color & 0xff000000) >> 24) / 255.0;

    cairo_save(cr);

    cairo_select_font_face(cr, (stl && stl->fontname) ? stl->fontname : "Arial",
        (stl && stl->italic) ? CAIRO_FONT_SLANT_ITALIC : CAIRO_FONT_SLANT_NORMAL,
        (stl && stl->bold)   ? CAIRO_FONT_WEIGHT_BOLD  : CAIRO_FONT_WEIGHT_NORMAL
    );

    float fontsize = stl->fontsize > 0 ? stl->fontsize : 12;

    cairo_set_font_size(cr, fontsize);

    cairo_set_antialias(cr, CAIRO_ANTIALIAS_BEST); //CAIRO_ANTIALIAS_SUBPIXEL); //

    //font XY scale
    cairo_matrix_t font_matrix;
    memset(&font_matrix, 0, sizeof(cairo_matrix_t));
    font_matrix.xx = (double)fontsize * (double)stl->fontscalex / 100.0;
    font_matrix.yy = (double)fontsize * (double)stl->fontscaley / 100.0;

    cairo_set_font_matrix(cr, &font_matrix);
    //cairo_scale(cr, (double)stl->fontscalex / 100.0, (double)stl->fontscaley / 100.0);

    y += /*stl->marginvertical +*/ fontsize;

    cairo_translate(cr, x, y);
    cairo_rotate(cr, -stl->angle * M_PI / 180.);

    //cairo_font_face_t* cairo_ft_face = cairo_get_font_face(cr);

/*
    cairo_scaled_font_t* sc_fnt = cairo_scaled_font_create (cairo_ft_face,
                          const cairo_matrix_t *font_matrix,
                          const cairo_matrix_t *ctm,
                          const cairo_font_options_t *options);
*/

    cairo_scaled_font_t* scaled_face = cairo_get_scaled_font(cr);

    // get glyphs for the text
    cairo_glyph_t* glyphs = NULL;
    int glyph_count;
    cairo_text_cluster_t* clusters = NULL;
    int cluster_count;
    cairo_text_cluster_flags_t clusterflags;

    cairo_status_t stat = cairo_scaled_font_text_to_glyphs(scaled_face,
                                                 0., 0., 
                                                 //x, y,
                                                 txt, strlen(txt),
                                                 &glyphs, &glyph_count,
                                                 &clusters, &cluster_count,
                                                 &clusterflags
    );
    if (stat != CAIRO_STATUS_SUCCESS) {
        return;
    }

#if 0 // ex from doc
    cairo_show_text_glyphs (cr,
                            utf8, utf8_len,
                            glyphs, num_glyphs,
                            clusters, num_clusters, cluster_flags);

    cairo_glyph_free (glyphs);
    cairo_text_cluster_free (clusters);
#endif

    cairo_scaled_font_glyph_extents(scaled_face, glyphs, glyph_count, &extents);
    //cairo_scaled_font_text_extents(scaled_face, txt, &extents);
    //cairo_text_extents(cr, txt, &extents);

/*
    // Shadow
    if (stl->borderstyle == 1 && stl->shadow > 0.) {
        cairo_save(cr);
        cairo_set_source_rgba(cr,
                          shr,  //red
                          shg,  //green
                          shb,  //blue
                          sha   //alpha
                          ); 
        cairo_translate(cr, x + stl->shadow, y + stl->shadow);
        cairo_rotate(cr, stl->angle * M_PI / 180.);
        cairo_move_to(cr, 0, 0);
        cairo_text_path(cr, txt);
        cairo_fill(cr);
        cairo_restore(cr);
    }
*/

    // main text
    cairo_set_source_rgba(cr,
                          fgr,  //red
                          fgg,  //green
                          fgb,  //blue
                          fga   //alpha
                          );
//    cairo_translate(cr, x, y);
//    cairo_rotate(cr, stl->angle * M_PI / 180.);
//    cairo_move_to(cr, 0, 0);

/*
    cairo_show_text_glyphs (cr,
                            txt, strlen(txt),
                            glyphs, glyph_count,
                            clusters, cluster_count, clusterflags);
*/
    cairo_show_glyphs (cr,
                            //txt, strlen(txt),
                            glyphs, glyph_count);
                            //clusters, cluster_count, clusterflags);

//    cairo_text_path(cr, txt);

    //cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    //cairo_paint(cr);
    cairo_fill_preserve(cr);

//    if (stl->borderstyle == 1) {
        cairo_set_source_rgba(cr,
                          bdr,  //red
                          bdg,  //green
                          bdb,  //blue
                          bda   //alpha
                          ); 
        cairo_set_line_width(cr, 5.); //stl->outline);
        cairo_stroke(cr);
//    }

    if (stl->underline != 0) {
        cairo_set_source_rgba(cr,
                          bdr,  //red
                          bdg,  //green
                          bdb,  //blue
                          bda   //alpha
                          );
        cairo_set_line_width(cr, stl->outline);

        cairo_new_path(cr);
        cairo_move_to(cr, 0., 0.2 * stl->fontsize);
        cairo_rel_line_to(cr, extents.width, 0.);
        cairo_stroke(cr);
    }

    if (stl->strikeout != 0) {
        cairo_set_source_rgba(cr,
                          bdr,  //red
                          bdg,  //green
                          bdb,  //blue
                          bda   //alpha
                          );
        cairo_set_line_width(cr, stl->outline);

        cairo_new_path(cr);
        cairo_move_to(cr, 0., -.4 * extents.height);
        cairo_rel_line_to(cr, extents.width, 0.);
        cairo_stroke(cr);
    }

    float tw = extents.x_advance, th = extents.y_advance;
//    float tw = w, th = h;
    float tlen = sqrtf(tw * tw + th * th);

    _X += tlen * cosf(-stl->angle * M_PI / 180.);
    _Y += tlen * sinf(-stl->angle * M_PI / 180.);

    cairo_glyph_free (glyphs);
    cairo_text_cluster_free (clusters);

    cairo_restore(cr);
}
#endif




static cairo_surface_t* do_text_block_compose(text_line_t** sfc_stack_root, style_t* stl)
{
    cairo_surface_t* sfc = NULL;
	cairo_t*         cr  = NULL;

	if (!sfc_stack_root || !*sfc_stack_root)
		return NULL;

	text_line_t* tl;
	int w = 0, h = 0;

	for (tl = *sfc_stack_root; tl != NULL; tl = tl->next) {	
		int wtl = 0, htl = 0;
		for (text_line_sfc_el_t* sfc_el = tl->sfc_lst_root; sfc_el; sfc_el = sfc_el->next) {
			if (tl->flags & (1 << TEXT_LINE_BELOW_LINE)) {
				if (w < sfc_el->w)
					w = sfc_el->w;
				h += sfc_el->h;

				if (wtl < sfc_el->w)
					wtl = sfc_el->w;
				htl += sfc_el->h;
			}
		}
		tl->w = wtl;
		tl->h = htl;
	}

    sfc = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
	cr  = cairo_create(sfc);
	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

	double x = 0., y = 0.;
	uint tbock_flags = stl->block_flags;

	for (tl = *sfc_stack_root; tl != NULL; tl = tl->next) {
		for (text_line_sfc_el_t* sfc_el = tl->sfc_lst_root; sfc_el; sfc_el = sfc_el->next) {
#if 1
			if (tbock_flags & (1 << TEXT_BLOCK_ALIGN_LEFT)) {
				x = 0.;
			} else
			if (tbock_flags & (1 << TEXT_BLOCK_ALIGN_RIGHT)) {
				x = (double)(w - sfc_el->w);
			} else
			if (tbock_flags & (1 << TEXT_BLOCK_ALIGN_CENTER)) {
				x = (double)(w - sfc_el->w) / 2.;
			} else
			{
				x = 0.;
			}
//DD("tbock_flags: %d, x: %f", tbock_flags, x);
#endif

			if (tl->flags & (1 << TEXT_LINE_BELOW_LINE)) {

				cairo_translate(cr, x, y);

				cairo_set_source_surface(cr, sfc_el->sfc, 0., 0.); ///-y); //x, y);

				cairo_paint(cr);

				cairo_surface_flush(sfc);

				y += tl->h;
			}
		}
	}

	cairo_destroy(cr);

	return sfc;
}


#if 0
static void cr_stack_push()
{
//    if (!sfc || !tcr)
//        return;

/*
    cairo_surface_t* sf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, screenW * 2, screenH * 2);
    if (!sf)
        return;
    cairo_t* cr = cairo_create(sf);
    if (!cr) {
        cairo_surface_destroy(sf);
        return;
    }

    int l = cr_stack_num + 1;
    if (l > cr_stack_total) {
        cr_stack_total += 40;
        cr_stack = realloc(cr_stack, cr_stack_total * sizeof(cairo_surface_t*));

        cr_block_size_stack = realloc(cr_block_size_stack, cr_stack_total * sizeof(cr_block_size_t));
    }

    if (cr_stack_num > 0)
        cr_block_size_stack[cr_stack_num-1] = tcr_block_size;

    cr_stack[cr_stack_num] = sf;
    cr_stack_num = l;
*/

    tcr_block_size.w = 0.;
    tcr_block_size.h = 0.;

//    if (tcr)
//        cairo_destroy(tcr);
//    tcr = cr;
//    do_clean(tcr);
}
#endif



static text_line_sfc_el_t* text_new_line(style_t* tstl, text_line_t** sfc_stack_root, int w, int h)
{
	cairo_surface_t* sfc = NULL;

//    if (!tstl)
//        return;

//    if (_cur_effect && _cur_effect->effect == EFFECT_CODE_BANNER)
//        return;

//	text_line_t* tl = malloc(sizeof(text_line_t));
//	memset(tl, 0, sizeof(text_line_t));

	text_line_t* tl = *sfc_stack_root;

	text_line_t*         root_tl     = *sfc_stack_root;
	text_line_sfc_el_t** sfc_el_nest = &root_tl->sfc_lst_root;

	text_line_sfc_el_t* sfc_el = NULL;

	for (; *sfc_el_nest; sfc_el_nest = &(*sfc_el_nest)->next) {
		sfc_el = *sfc_el_nest;
	}

	if (sfc_el && sfc_el->sfc) {
		if (sfc_el->cr) {
			cairo_surface_flush(sfc_el->sfc);
			cairo_destroy(sfc_el->cr);
			sfc_el->cr = NULL;
		}
		
		sfc_el->w = w; //(int)tcr_block_size.w;
		sfc_el->h = h; //(int)tcr_block_size.h;
		
		sfc = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
		cairo_t* crt = cairo_create(sfc);
		
		cairo_set_operator(crt, CAIRO_OPERATOR_OVER);
		cairo_set_source_surface (crt, sfc_el->sfc, 0., 0. /*-tcr_block_size.h*/); // -x1, -y1);	//
		
		cairo_paint(crt);
		
		cairo_surface_flush(sfc);
		
		cairo_destroy(crt);
		
		cairo_surface_destroy(sfc_el->sfc);
		
		sfc_el->sfc = sfc;
		sfc = NULL;
	}

	sfc_el = malloc(sizeof(text_line_sfc_el_t));
	memset(sfc_el, 0, sizeof(text_line_sfc_el_t));

	*sfc_el_nest = sfc_el;

	sfc_el->w = tcr_block_size.w;
	sfc_el->h = tcr_block_size.h;

#if 0
	for (; sfc_el; sfc_el = sfc_el->next) {
		if (sfc_el->sfc == NULL) {
			sfc_el->w = tcr_block_size.w;
			sfc_el->h = tcr_block_size.h;

			*sfc_stack_root = tl;
		} else {
			if (root_tl->sfc) {
				cairo_surface_flush(root_tl->sfc);
				cairo_destroy(root_tl->cr);
				root_tl->cr = NULL;

				root_tl->w = w; //(int)tcr_block_size.w;
				root_tl->h = h; //(int)tcr_block_size.h;

				sfc = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
				cairo_t* crt = cairo_create(sfc);

				cairo_set_operator(crt, CAIRO_OPERATOR_OVER);
				cairo_set_source_surface (crt, root_tl->sfc, 0., 0. /*-tcr_block_size.h*/); // -x1, -y1);   //

				cairo_paint(crt);

				cairo_surface_flush(sfc);

				cairo_destroy(crt);

				cairo_surface_destroy(root_tl->sfc);

				root_tl->sfc = sfc;
				sfc = NULL;
			}

			root_tl->next = tl;
		}
	}
#endif

	sfc = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, screenW, screenH);

	sfc_el->cr = cairo_create(sfc);
	do_clean(sfc_el->cr);

	sfc_el->sfc = sfc;

	tl->flags = 1 << TEXT_LINE_BELOW_LINE;

//    cr_stack_push();

	tcr_block_size.w = 0.;
	tcr_block_size.h = 0.;

    _X = 0.;
    _Y = _line_begin_Y = 0.; //_line_begin_Y; //{} + 1. * ((tstl->fontsize > 0) ? tstl->fontsize : 46.) * (double)tstl->fontscaley / 100.0;

	return sfc_el;
}



static void text_end_line(style_t* tstl, text_line_t** sfc_stack_root, int w, int h)
{
	cairo_surface_t* sfc = NULL;

	text_line_t* tl = *sfc_stack_root;

	text_line_t*		 root_tl	 = *sfc_stack_root;
	text_line_sfc_el_t** sfc_el_nest = &root_tl->sfc_lst_root;

	text_line_sfc_el_t* sfc_el = NULL;

	for (; *sfc_el_nest; sfc_el_nest = &(*sfc_el_nest)->next) {
		sfc_el = *sfc_el_nest;
	}

	if (sfc_el && sfc_el->sfc) {
		if (sfc_el->cr) {
			cairo_surface_flush(sfc_el->sfc);
			cairo_destroy(sfc_el->cr);
			sfc_el->cr = NULL;
		}
		
		sfc_el->w = w; //(int)tcr_block_size.w;
		sfc_el->h = h; //(int)tcr_block_size.h;
		
		sfc = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
		cairo_t* crt = cairo_create(sfc);

		cairo_set_operator(crt, CAIRO_OPERATOR_OVER);
		cairo_set_source_surface (crt, sfc_el->sfc, 0., 0. /*-tcr_block_size.h*/); // -x1, -y1);	//
	
		cairo_paint(crt);
	
		cairo_surface_flush(sfc);
	
		cairo_destroy(crt);
	
		cairo_surface_destroy(sfc_el->sfc);

		sfc_el->sfc = sfc;
		sfc = NULL;
	}

#if 0
	sfc_el = malloc(sizeof(text_line_sfc_el_t));
	memset(sfc_el, 0, sizeof(text_line_sfc_el_t));

	*sfc_el_nest = sfc_el;

	sfc_el->w = tcr_block_size.w;
	sfc_el->h = tcr_block_size.h;

	sfc = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, screenW, screenH);

	sfc_el->cr = cairo_create(sfc);
	do_clean(sfc_el->cr);

	sfc_el->sfc = sfc;

	tl->flags = 1 << TEXT_LINE_BELOW_LINE;
#endif

	tcr_block_size.w = 0.;
	tcr_block_size.h = 0.;

	_X = 0.;
	_Y = _line_begin_Y = 0.; //_line_begin_Y; //{} + 1. * ((tstl->fontsize > 0) ? tstl->fontsize : 46.) * (double)tstl->fontscaley / 100.0;



#if 0
	cairo_surface_t* sfc = NULL;

	if (!tl)
		return;

	if (tl->sfc) {
		if (tl->cr) {
			cairo_surface_flush(tl->sfc);
			cairo_destroy(tl->cr);
			tl->cr = NULL;
		}

		tl->w = w; //tcr_block_size.w;
		tl->h = h; //tcr_block_size.h;

		sfc = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
		cairo_t* crt = cairo_create(sfc);

		cairo_set_operator(crt, CAIRO_OPERATOR_OVER);
		cairo_set_source_surface (crt, tl->sfc, 0., 0. /*-tcr_block_size.h*/); // -x1, -y1);   //

		cairo_paint(crt);

		cairo_surface_flush(sfc);

		cairo_destroy(crt);

		cairo_surface_destroy(tl->sfc);

		tl->sfc = sfc;
		sfc = NULL;
	}
#endif
}



void* text_del_line(text_line_t* tl)
{
	if (!tl)
		return NULL;

	text_line_t* tl_next = tl->next;

	for (text_line_sfc_el_t* sfc_el = tl->sfc_lst_root; sfc_el; ) {
		text_line_sfc_el_t* sfc_el_next = sfc_el->next;
		if (sfc_el->cr)
			cairo_destroy(sfc_el->cr);
		if (sfc_el->sfc)
			cairo_surface_destroy(sfc_el->sfc);
		//free(sfc_el->text);

		free(sfc_el);

		sfc_el = sfc_el_next;
	}

	free(tl);

	return tl_next;
}



static void draw_text(/*cairo_t* tcr, char* txt,*/ style_t* tstl, text_line_t** sfc_stack_root)
{
    if (!sfc_stack_root || !*sfc_stack_root)
        return;

	text_line_t* tl = *sfc_stack_root;
	char* txt = NULL;

	cairo_t* tcr = NULL;
	text_line_sfc_el_t* sfc_el = NULL;

	sfc_el = tl->sfc_lst_root;
	if (!sfc_el || !sfc_el->cr)
		return;

//	for (; sfc_el && sfc_el->cr; sfc_el = sfc_el->next) {
//	}
	tcr = sfc_el->cr;
	txt = tl->text; //sfc_el->str;

//	if (!txt)
//		return;


    int i;
    size_t len_str = utf8len(txt);
    float char_w = 1.1 * tcr_block_size.stl.fontscalex * tcr_block_size.stl.fontsize / 100.;
    utf8_int32_t ch_utf8 = 0;

    float drawable_area_w =
				(float)((*sfc_stack_root)->w_max > 0 ? (*sfc_stack_root)->w_max : screenW)
				- tcr_block_size.stl.marginleft - tcr_block_size.stl.marginright;
    if (drawable_area_w < 2. * char_w)
        return;

/*//{}
    if (_cur_effect && _cur_effect->effect == EFFECT_CODE_BANNER) {
        if (flag_text_banner_style_is_calc_size)
            do_text_size(tcr, _X, _Y, txt, tstl);
        else
            do_text(tcr, _X, _Y, txt, tstl);
        return;
    }
*/

    int len_chunk = (int)((drawable_area_w - tcr_block_size.w) / char_w) - 1;
//    if (len_str < len_chunk)
//        len_chunk = len_str;

    int buf_len = len_chunk * 4 + 4;
    char *buf = malloc(buf_len);

    while (len_str > 0) {
        char *buf_p = buf;
        int buf_filled = 0;

        for (i = 0; i < len_chunk && len_str > 0 && buf_p != NULL; i++) {
            txt = utf8codepoint(txt, &ch_utf8);
            buf_p = utf8catcodepoint(buf_p, ch_utf8, buf_len - buf_filled - 1);
            buf_filled += utf8codepointsize(ch_utf8);
            len_str--;
        }

        if (buf_p != NULL) {
            utf8_int32_t ch_utf8_tmp = 0;
            buf_p = utf8catcodepoint(buf_p, ch_utf8_tmp, buf_len - buf_filled);
            do_text(tcr, _X, _Y, buf, tstl);
        }

        if (len_str > 1) {
            len_chunk = (int)((drawable_area_w - tcr_block_size.w) / char_w) - 1;
            if (len_chunk <= 0) {
#if 0 // hyphens
                utf8_int32_t c;
                utf8codepoint(txt, &c);
                if (
                    c != ' ' &&
                    c != '\t' &&
                    c != '.' &&
                    c != ',' &&
                    c != '-' &&
                    c != ':' &&
                    c != ';' &&
                    c != '\0' &&

                    ch_utf8 != ' ' &&
                    ch_utf8 != '\t' &&
                    ch_utf8 != '.' &&
                    ch_utf8 != ',' &&
                    ch_utf8 != '-' &&
                    ch_utf8 != ':' &&
                    ch_utf8 != ';'
                ) {
                    do_text(tcr, _X, _Y, "-", tstl);
                }

#endif
                //text_new_line(tstl);
//				text_new_line(tstl, sfc_stack_root, tcr_block_size.w, -tcr_block_size.h);
//				if ((*sfc_stack_root)->next) {
//					sfc_stack_root = &((*sfc_stack_root)->next);
//					tcr = (*sfc_stack_root)->cr;
//				}
				sfc_el = text_new_line(tstl, sfc_stack_root, tcr_block_size.w, -tcr_block_size.h);
				if (!sfc_el || !sfc_el->cr)
					return;
				tcr = sfc_el->cr;

                len_chunk = (int)(drawable_area_w / char_w) - 1;
            }
            buf_len = len_chunk * 4 + 4;
            buf = realloc(buf, buf_len);
        }
    }

    free(buf);
}



cairo_surface_t* _text_with_draw_cb(/*cairo_t* tcr, char *text,*/ style_t* stl, text_line_t** sfc_stack_root)
{
    char* font = NULL;

    if (/*!text ||*/ !stl || !sfc_stack_root)
        return NULL;

	text_line_t** root = sfc_stack_root;

//    tstl_name = NULL;
//    memcpy(&tstl_body, stl, sizeof(style_t));
//    tstl = &tstl_body;

    _cur_alpha2draw = 1.;
    _nxt_alpha2draw = 1.;

//    _karaoke_blend = 0.;
//    _karaoke_type = 0;
//    _karaoke_time_s = 0.;

//    _X = 0.; _Y = 0; //screenH / 2;
//    _line_begin_Y = _Y; //0.;

    tcr_block_size.a = -stl->angle;
    tcr_block_size.stl = *stl;

//    if (tstl_body.fontname)
//        tstl_body.fontname = utf8dup(tstl_body.fontname);
                //tstl_body.fontname = strdup(tstl_body.fontname);

//    cr_stack_push();

//    _line_begin_Y = _Y; //0.;

//    Parse(text, utf8size(text));

//	_line_begin_Y -= 1. * ((stl->fontsize > 0) ? stl->fontsize : 46.) * (double)stl->fontscaley / 100.0;

	text_line_t* tl = *sfc_stack_root;

	text_new_line(stl, sfc_stack_root, 0, 0);

//	do_text_size(tl->cr, 0, 0, text, stl);

#if 0
    char *b = text,
         *p = text;
    utf8_int32_t c = 0;
    while(1) {
        char *p_next = utf8codepoint(p, &c);
        if (c == '\r' || c == '\n' || c == '\0') {
            *p = '\0';
            if (p_next - b > 1) {
				(*sfc_stack_root)->text = b;
				//_X = 0.; _Y = 0.;
                draw_text(/*tl->cr, b,*/ stl, sfc_stack_root);
				tl = *sfc_stack_root;

                if (c == '\r' || c == '\n') {
                    text_new_line(stl, sfc_stack_root, tcr_block_size.w, -tcr_block_size.h);
					while ((*sfc_stack_root)->next) {
						sfc_stack_root = &((*sfc_stack_root)->next);
						tl = *sfc_stack_root;
					}
                } else {
					text_end_line(stl, sfc_stack_root, tcr_block_size.w, -tcr_block_size.h);
                    break;
                }
            }
            b = p_next;
        //} else if (p == b && (c == ' ' || c == '\t')) {
        //    b = p_next;
        }
        p = p_next;
    }
#else
	text_line_t** ln_nest = sfc_stack_root;
	utf8_int32_t c = 0;
	for(; ln_nest && *ln_nest; ln_nest = &((*ln_nest)->next)) {
		//_X = 0.; _Y = 0.;
		draw_text(/*tl->cr, b,*/ stl, ln_nest);

	
		text_end_line(stl, ln_nest, tcr_block_size.w, -tcr_block_size.h);
		if ((*ln_nest)->next) {
			text_new_line(stl, &((*ln_nest)->next), tcr_block_size.w, -tcr_block_size.h);
//		} else {
		}
	}
//	text_end_line(stl, sfc_stack_root, tcr_block_size.w, -tcr_block_size.h);
#endif

#if 0 //{}
	for (; tl; tl = tl->next) {
		draw_text(/*tl->cr, b,*/ stl, &tl);

		if (tl->next) {
			text_new_line(stl, &tl, tcr_block_size.w, -tcr_block_size.h);
		} else {
			text_end_line(stl, &tl, tcr_block_size.w, -tcr_block_size.h);
		}
	}
#endif //{}


//    do_text_block_compose(sfc_stack_root);

//    if (tstl_body.fontname)
//        free(tstl_body.fontname);

//    tstl = NULL;

    return do_text_block_compose(root, stl);
}


static void do_clean(cairo_t *cr)
{
    cairo_set_source_rgba(cr, 1., 0., 1., 0.4);
    cairo_paint(cr);
//	cairo_fill(cr);
}


static cairo_surface_t* render_text(
							graphic_el* el,
							int *w, int *h,
                           char* fontname,
                           char* fontstyle,
                           int fontsize,
                           int fontcolor,
                           int backcolor
)
{
	if (!el || !el->data)
		return NULL;

	text_line_t** sfc_stack_root = &el->text_lines;
	char*         txt            = el->data;	
//	cairo_surface_t* sfc = NULL;

//    char *bmp = NULL;
//    int y = 0;

    *w = *h = 0;

//    ssa_player_init(screenW, screenH);
	
//    if (!font_list)
//        make_available_fonts_list();

#if 1 //{}
//{}{}    sfc = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, screenW, screenH);

//    if (sfc)
//{}{}    cairo_t* tcr = cairo_create(sfc);
//    else
//        tcr = NULL;

//    tcr_block_size.a = 1.;
//    tcr_block_size.w = 0.;
//    tcr_block_size.h = 0.;

//    _X = 0.; _Y = (*h)/2; //screenH / 2;

//{}{}    do_clean(tcr);

	style_t st; //{} = new_style();
//    memset(&st, 0, sizeof(style_t));
	memcpy(&st, &_default_style, sizeof(style_t));

//    if (fontname && fontname[0] != 0 && !!strcmp(fontname, "unknown"))
//        st.fontname = fontname; //strdup(fontname);
//    if (fontsize > 0)
//        st.fontsize = (float)fontsize;
//    if (fontcolor >= 0 && fontcolor != backcolor)
//        st.color = (uint32_t)el-;
//    if (backcolor >= 0 && fontcolor != backcolor)
//        st.shadow_color = (uint32_t)backcolor;

    if (fontstyle && fontstyle[0] != 0) {
        int i;
        for (i = 0; i < 10 && fontstyle[i] != 0; i++) {
            switch (fontstyle[i]) {
                case 'b':
                    st.bold = 600;
                    break;
                case 'i':
                    st.italic = 1;
                    break;
                case 'u':
                    st.underline = 1;
                    break;
            }
        }
    }

	st.block_flags = el->text_block_flags;

    cairo_surface_t* sfc_out = _text_with_draw_cb(/*tcr, txt,*/ &st, sfc_stack_root);

#if 0
//    ssa_player_surface_flush();

	cairo_surface_flush((*sfc_stack_root)->sfc);

	if ((*sfc_stack_root)->cr) {
		cairo_destroy((*sfc_stack_root)->cr);
	}

	cairo_surface_t* sfc_out = (*sfc_stack_root)->sfc;
#endif
#endif

#if 1
//tcr_block_size.w = 400.;
//tcr_block_size.h = -100.;

#if 0 //{}{}	
	sfc_out = cairo_image_surface_create(
										CAIRO_FORMAT_ARGB32,
										tcr_block_size.w,
										-tcr_block_size.h
									);
	cairo_t* cr = cairo_create(sfc_out);
	
	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
	cairo_set_source_surface (cr, sfc, 0., 0. /*-tcr_block_size.h*/); // -x1, -y1);   //

#if 0
	cairo_move_to(cr, 0., 0.);
	cairo_line_to(cr, 0., -tcr_block_size.h);
	cairo_line_to(cr, tcr_block_size.w, -tcr_block_size.h);
	cairo_line_to(cr, tcr_block_size.w, 0.);
	cairo_close_path(cr);	

//	do_clean(cr);
#endif

	cairo_paint(cr);

#if 0
	cairo_set_source_rgba(cr, 1., 0., 0., .7);
	cairo_fill_preserve  (cr);
	cairo_set_source_rgba(cr, 0., 1., 0., 1.);
	cairo_set_line_width (cr, 2.);
	cairo_stroke		 (cr);
#endif

	cairo_surface_flush(sfc_out);

	cairo_destroy(cr);

	cairo_surface_destroy(sfc);

//    *w = ssa_player_surface_get_width();
//    *h = ssa_player_surface_get_height();

//    bmp = ssa_player_surface_get_data();
//	*w = tcr_block_size.w; //cairo_image_surface_get_width(sfc_out);
//	*h = -tcr_block_size.h; //cairo_image_surface_get_height(sfc_out);
#endif
#endif //{}{}
	*w = cairo_image_surface_get_width(sfc_out);  //(*sfc_stack_root)->w; //
	*h = cairo_image_surface_get_height(sfc_out); //(*sfc_stack_root)->h; //

DD("sfc: %p, wh: (%d, %d)\n", sfc_out, *w, *h);

//{}    free(st);
//    ssa_player_destroy();
//    cairo_debug_reset_static_data();

    return sfc_out; //sfc_out;
}






cairo_surface_t *
_draw_text(graphic_el* el, double *w_out, double *h_out)
{
	cairo_surface_t* sfc = NULL;
//	png_closure_t d;

	if (!el || !el->data)
		return NULL;

//	text_line_t** sfc_stack_root = &el->text_lines;
//	char* text                   =  el->data;

/*
	d.fd = kopen(png_path, OREAD);
	d.offs = 0;

	sfc = cairo_image_surface_create_from_png_stream(png_chank_read_cb, &d);
	kclose(d.fd);
*/

	el->text_lines = text_split_to_lines(el->data, 0,
							el->text_vp_w,
							(1 << TEXT_LINE_BELOW_LINE) | (1 << TEXT_BLOCK_ALIGN_LEFT)
					);

	int w, h;

	sfc = render_text(el,
							&w, &h,
							DEFAULT_FONT,	//char* fontname,
							"",			//char* fontstyle,
							40,			//int fontsize,
							0x111100ff,	//int fontcolor,
							0x11aaffff	//int backcolor
						);
	if (!sfc)
		return NULL;

	if (w_out)
		*w_out = (double)w;
	if (h_out)
		*h_out = (double)h;

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
DD("sfc: %p, wh: (%f, %f)\n", sfc, *w_out, *h_out);

	}
	return sfc;
}



