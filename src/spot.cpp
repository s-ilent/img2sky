/** this is an amalgamated file. do not edit.
 */

/* Handy pixel/color and texture/image classes. BOOST licensed.
 * - rlyeh ~~ listening to Twilightning - Painting the blue eyes

 *   Hue - Think of a color wheel. Around 0 and 255 are reds 85 are greens, 170 are blues. Use anything in between 0-255. Values above and below will be modulus 255.
 *   Saturation - 0 is completely denatured (grayscale). 255 is fully saturated (full color).
 *   Lightness - 0 is completely dark (black). 255 is completely light (white). 127 is average lightness.
 *   alpha - Opacity/Transparency value. 0 is fully transparent. 255 is fully opaque. 127 is 50% transparent.

 *   enum hue
 *   {
 *       hred1 = 0,
 *       hgreen = 85,
 *       hblue = 170,
 *       hred2 = 255,

 *       hpurple =  ( ( hblue + hred2 ) / 2 ) % 255,
 *       hcyan =  ( ( hgreen + hblue ) / 2 ) % 255,
 *       hyellow = ( ( hred1 + hgreen ) / 2 ) % 255
 *   };

 *   unsigned char make_hue( float reds, float greens, float blues )
 *   {
 *       return (unsigned char)( int( reds * 255 + greens * 85 + blues * 170 ) % 255 );
 *   }

 * valid color components in range [0..1]
 * components can handle higher values (useful for color algebra), but their values will be clamped when displayed

 * image = array of pixels[] in RGBA or HSLA format
 * texture = strict subset of an image { squared, rgba only }
 * so, image > texture

 * image @todo
 * vg {
 * image.glow( 1px, pixel::black )
 * image.glow( 2px, pixel::red )
 * image.mask(white);
 * image.mask(white).glow( 2x, pixel::blue ).replace(white,transp) --> L4D2 style
 * image.aabb(); -> return aabb for this frame
 * }
 * composition {
 * image.append( img[], align09 = 6 )
 * image.crop( columns/rows to remove, align09 = 2, method = fast/carving )
 * image.shrink( w, h )
 * }
 * image.noisex()
 * image.perlin()
 * image.mipmap()
 * image.upload()/unload()
 * image.download()/capture()
 * image.rotate(deg, bgcolor/transparent?)
 * image.premultiply()/unpremultiply()
 * image.gamma(1.22)
 * image.atlas( image[] )/combine(other)
 * image.blur()/sharpen()
 * image.scale(marquee_scale,content_scale,scale_method)
 * @todo: also image1 * / *= / + / += image2

 * pixel @Todo:
 *  replace float with float8 (microfloat 0.4.4 ; good idea?)
 *  to_rg() // computer vision: r = r/r+g+b, g = g/r+g+b
 *  munsell 5 hues: red, yellow, green, blue, purple
 *  hsl 6 hues: red, yellow, green, cyan, blue, purple
 *  rgy
 *  http://en.wikipedia.org/wiki/HSL_color_space#Disadvantages
 *  https://github.com/aras-p/miniexr/blob/master/miniexr.cpp
 *  http://cgit.haiku-os.org/haiku/plain/src/add-ons/translators/exr/openexr/half/half.h
 *  ftp://www.fox-toolkit.org/pub/fasthalffloatconversion.pdf
 *  std::string str() const { return std::string(); } //#ffffffff

*/

#include <cassert>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "spot.hpp"

#define NANOSVG_ALL_COLOR_KEYWORDS  // Include full list of color keywords.
#define NANOSVG_IMPLEMENTATION      // Expands implementation
#define NANOSVGRAST_IMPLEMENTATION  // Expands implementation

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

//#line 1 "nanosvg.h"
#ifndef NANOSVG_H
#define NANOSVG_H

#ifdef __cplusplus
extern "C" {
#endif

// NanoSVG is a simple stupid single-header-file SVG parse. The output of the parser is a list of cubic bezier shapes.
//
// The library suits well for anything from rendering scalable icons in your editor application to prototyping a game.
//
// NanoSVG supports a wide range of SVG features, but something may be missing, feel free to create a pull request!
//
// The shapes in the SVG images are transformed by the viewBox and converted to specified units.
// That is, you should get the same looking data as your designed in your favorite app.
//
// NanoSVG can return the paths in few different units. For example if you want to render an image, you may choose
// to get the paths in pixels, or if you are feeding the data into a CNC-cutter, you may want to use millimeters.
//
// The units passed to NanoVG should be one of: 'px', 'pt', 'pc' 'mm', 'cm', or 'in'.
// DPI (dots-per-inch) controls how the unit conversion is done.
//
// If you don't know or care about the units stuff, "px" and 96 should get you going.

/* Example Usage:
	// Load
	SNVGImage* image;
	image = nsvgParseFromFile("test.svg", "px", 96);
	printf("size: %f x %f\n", image->width, image->height);
	// Use...
	for (shape = image->shapes; shape != NULL; shape = shape->next) {
		for (path = shape->paths; path != NULL; path = path->next) {
			for (i = 0; i < path->npts-1; i += 3) {
				float* p = &path->pts[i*2];
				drawCubicBez(p[0],p[1], p[2],p[3], p[4],p[5], p[6],p[7]);
			}
		}
	}
	// Delete
	nsvgDelete(image);
*/

enum NSVGpaintType {
	NSVG_PAINT_NONE = 0,
	NSVG_PAINT_COLOR = 1,
	NSVG_PAINT_LINEAR_GRADIENT = 2,
	NSVG_PAINT_RADIAL_GRADIENT = 3,
};

enum NSVGspreadType {
	NSVG_SPREAD_PAD = 0,
	NSVG_SPREAD_REFLECT = 1,
	NSVG_SPREAD_REPEAT = 2,
};

enum NSVGlineJoin {
	NSVG_JOIN_MITER = 0,
	NSVG_JOIN_ROUND = 1,
	NSVG_JOIN_BEVEL = 2,
};

enum NSVGlineCap {
	NSVG_CAP_BUTT = 0,
	NSVG_CAP_ROUND = 1,
	NSVG_CAP_SQUARE = 2,
};

typedef struct NSVGgradientStop {
	unsigned int color;
	float offset;
} NSVGgradientStop;

typedef struct NSVGgradient {
	float xform[6];
	char spread;
	float fx, fy;
	int nstops;
	NSVGgradientStop stops[1];
} NSVGgradient;

typedef struct NSVGpaint {
	char type;
	union {
		unsigned int color;
		NSVGgradient* gradient;
	};
} NSVGpaint;

typedef struct NSVGpath
{
	float* pts;					// Cubic bezier points: x0,y0, [cpx1,cpx1,cpx2,cpy2,x1,y1], ...
	int npts;					// Total number of bezier points.
	char closed;				// Flag indicating if shapes should be treated as closed.
	float bounds[4];			// Tight bounding box of the shape [minx,miny,maxx,maxy].
	struct NSVGpath* next;		// Pointer to next path, or NULL if last element.
} NSVGpath;

typedef struct NSVGshape
{
	char id[64];				// Optional 'id' attr of the shape or its group
	NSVGpaint fill;				// Fill paint
	NSVGpaint stroke;			// Stroke paint
	float opacity;				// Opacity of the shape.
	float strokeWidth;			// Stroke width (scaled).
	char strokeLineJoin;		// Stroke join type.
	char strokeLineCap;			// Stroke cap type.
	float bounds[4];			// Tight bounding box of the shape [minx,miny,maxx,maxy].
	NSVGpath* paths;			// Linked list of paths in the image.
	struct NSVGshape* next;		// Pointer to next shape, or NULL if last element.
} NSVGshape;

typedef struct NSVGimage
{
	float width;				// Width of the image.
	float height;				// Height of the image.
	NSVGshape* shapes;			// Linked list of shapes in the image.
} NSVGimage;

// Parses SVG file from a file, returns SVG image as paths.
NSVGimage* nsvgParseFromFile(const char* filename, const char* units, float dpi);

// Parses SVG file from a null terminated string, returns SVG image as paths.
NSVGimage* nsvgParse(char* input, const char* units, float dpi);

// Deletes list of paths.
void nsvgDelete(NSVGimage* image);

#ifdef __cplusplus
};
#endif

#endif // NANOSVG_H

#ifdef NANOSVG_IMPLEMENTATION

#include <string.h>
#include <stdlib.h>
#include <math.h>

#define NSVG_PI (3.14159265358979323846264338327f)
#define NSVG_KAPPA90 (0.5522847493f)	// Lenght proportional to radius of a cubic bezier handle for 90deg arcs.

#define NSVG_ALIGN_MIN 0
#define NSVG_ALIGN_MID 1
#define NSVG_ALIGN_MAX 2
#define NSVG_ALIGN_NONE 0
#define NSVG_ALIGN_MEET 1
#define NSVG_ALIGN_SLICE 2

#define NSVG_NOTUSED(v) do { (void)(1 ? (void)0 : ( (void)(v) ) ); } while(0)
#define NSVG_RGB(r, g, b) (((unsigned int)r) | ((unsigned int)g << 8) | ((unsigned int)b << 16))

#ifdef _MSC_VER
	#pragma warning (disable: 4996) // Switch off security warnings
	#pragma warning (disable: 4100) // Switch off unreferenced formal parameter warnings
	#ifdef __cplusplus
	#define NSVG_INLINE inline
	#else
	#define NSVG_INLINE
	#endif
#else
	#define NSVG_INLINE inline
#endif

static int nsvg__isspace(char c)
{
	return strchr(" \t\n\v\f\r", c) != 0;
}

static int nsvg__isdigit(char c)
{
	return strchr("0123456789", c) != 0;
}

static int nsvg__isnum(char c)
{
	return strchr("0123456789+-.eE", c) != 0;
}

static NSVG_INLINE float nsvg__minf(float a, float b) { return a < b ? a : b; }
static NSVG_INLINE float nsvg__maxf(float a, float b) { return a > b ? a : b; }

// Simple XML parser

#define NSVG_XML_TAG 1
#define NSVG_XML_CONTENT 2
#define NSVG_XML_MAX_ATTRIBS 256

static void nsvg__parseContent(char* s,
							   void (*contentCb)(void* ud, const char* s),
							   void* ud)
{
	// Trim start white spaces
	while (*s && nsvg__isspace(*s)) s++;
	if (!*s) return;

	if (contentCb)
		(*contentCb)(ud, s);
}

static void nsvg__parseElement(char* s,
							   void (*startelCb)(void* ud, const char* el, const char** attr),
							   void (*endelCb)(void* ud, const char* el),
							   void* ud)
{
	const char* attr[NSVG_XML_MAX_ATTRIBS];
	int nattr = 0;
	char* name;
	int start = 0;
	int end = 0;
	char quote;

	// Skip white space after the '<'
	while (*s && nsvg__isspace(*s)) s++;

	// Check if the tag is end tag
	if (*s == '/') {
		s++;
		end = 1;
	} else {
		start = 1;
	}

	// Skip comments, data and preprocessor stuff.
	if (!*s || *s == '?' || *s == '!')
		return;

	// Get tag name
	name = s;
	while (*s && !nsvg__isspace(*s)) s++;
	if (*s) { *s++ = '\0'; }

	// Get attribs
	while (!end && *s && nattr < NSVG_XML_MAX_ATTRIBS-3) {
		// Skip white space before the attrib name
		while (*s && nsvg__isspace(*s)) s++;
		if (!*s) break;
		if (*s == '/') {
			end = 1;
			break;
		}
		attr[nattr++] = s;
		// Find end of the attrib name.
		while (*s && !nsvg__isspace(*s) && *s != '=') s++;
		if (*s) { *s++ = '\0'; }
		// Skip until the beginning of the value.
		while (*s && *s != '\"' && *s != '\'') s++;
		if (!*s) break;
		quote = *s;
		s++;
		// Store value and find the end of it.
		attr[nattr++] = s;
		while (*s && *s != quote) s++;
		if (*s) { *s++ = '\0'; }
	}

	// List terminator
	attr[nattr++] = 0;
	attr[nattr++] = 0;

	// Call callbacks.
	if (start && startelCb)
		(*startelCb)(ud, name, attr);
	if (end && endelCb)
		(*endelCb)(ud, name);
}

int nsvg__parseXML(char* input,
				   void (*startelCb)(void* ud, const char* el, const char** attr),
				   void (*endelCb)(void* ud, const char* el),
				   void (*contentCb)(void* ud, const char* s),
				   void* ud)
{
	char* s = input;
	char* mark = s;
	int state = NSVG_XML_CONTENT;
	while (*s) {
		if (*s == '<' && state == NSVG_XML_CONTENT) {
			// Start of a tag
			*s++ = '\0';
			nsvg__parseContent(mark, contentCb, ud);
			mark = s;
			state = NSVG_XML_TAG;
		} else if (*s == '>' && state == NSVG_XML_TAG) {
			// Start of a content or new tag.
			*s++ = '\0';
			nsvg__parseElement(mark, startelCb, endelCb, ud);
			mark = s;
			state = NSVG_XML_CONTENT;
		} else {
			s++;
		}
	}

	return 1;
}

/* Simple SVG parser. */

#define NSVG_MAX_ATTR 128

#define NSVG_USER_SPACE 0
#define NSVG_OBJECT_SPACE 1

typedef struct NSVGlinearData {
	float x1, y1, x2, y2;
} NSVGlinearData;

typedef struct NSVGradialData {
	float cx, cy, r, fx, fy;
} NSVGradialData;

typedef struct NSVGgradientData
{
	char id[64];
	char ref[64];
	char type;
	union {
		NSVGlinearData linear;
		NSVGradialData radial;
	};
	char spread;
	char units;
	float xform[6];
	int nstops;
	NSVGgradientStop* stops;
	struct NSVGgradientData* next;
} NSVGgradientData;

typedef struct NSVGattrib
{
	char id[64];
	float xform[6];
	unsigned int fillColor;
	unsigned int strokeColor;
	float opacity;
	float fillOpacity;
	float strokeOpacity;
	char fillGradient[64];
	char strokeGradient[64];
	float strokeWidth;
	char strokeLineJoin;
	char strokeLineCap;
	float fontSize;
	unsigned int stopColor;
	float stopOpacity;
	float stopOffset;
	char hasFill;
	char hasStroke;
	char visible;
} NSVGattrib;

typedef struct NSVGparser
{
	NSVGattrib attr[NSVG_MAX_ATTR];
	int attrHead;
	float* pts;
	int npts;
	int cpts;
	NSVGpath* plist;
	NSVGimage* image;
	NSVGgradientData* gradients;
	float viewMinx, viewMiny, viewWidth, viewHeight;
	int alignX, alignY, alignType;
	float dpi;
	char pathFlag;
	char defsFlag;
} NSVGparser;

static void nsvg__xformIdentity(float* t)
{
	t[0] = 1.0f; t[1] = 0.0f;
	t[2] = 0.0f; t[3] = 1.0f;
	t[4] = 0.0f; t[5] = 0.0f;
}

static void nsvg__xformSetTranslation(float* t, float tx, float ty)
{
	t[0] = 1.0f; t[1] = 0.0f;
	t[2] = 0.0f; t[3] = 1.0f;
	t[4] = tx; t[5] = ty;
}

static void nsvg__xformSetScale(float* t, float sx, float sy)
{
	t[0] = sx; t[1] = 0.0f;
	t[2] = 0.0f; t[3] = sy;
	t[4] = 0.0f; t[5] = 0.0f;
}

static void nsvg__xformSetSkewX(float* t, float a)
{
	t[0] = 1.0f; t[1] = 0.0f;
	t[2] = tanf(a); t[3] = 1.0f;
	t[4] = 0.0f; t[5] = 0.0f;
}

static void nsvg__xformSetSkewY(float* t, float a)
{
	t[0] = 1.0f; t[1] = tanf(a);
	t[2] = 0.0f; t[3] = 1.0f;
	t[4] = 0.0f; t[5] = 0.0f;
}

static void nsvg__xformSetRotation(float* t, float a)
{
	float cs = cosf(a), sn = sinf(a);
	t[0] = cs; t[1] = sn;
	t[2] = -sn; t[3] = cs;
	t[4] = 0.0f; t[5] = 0.0f;
}

static void nsvg__xformMultiply(float* t, float* s)
{
	float t0 = t[0] * s[0] + t[1] * s[2];
	float t2 = t[2] * s[0] + t[3] * s[2];
	float t4 = t[4] * s[0] + t[5] * s[2] + s[4];
	t[1] = t[0] * s[1] + t[1] * s[3];
	t[3] = t[2] * s[1] + t[3] * s[3];
	t[5] = t[4] * s[1] + t[5] * s[3] + s[5];
	t[0] = t0;
	t[2] = t2;
	t[4] = t4;
}

static void nsvg__xformInverse(float* inv, float* t)
{
	double det = (double)t[0] * t[3] - (double)t[2] * t[1];
	double invdet = 1.0 / det;
	if (det > -1e-6 && det < -1e-6) {
		nsvg__xformIdentity(t);
		return;
	}
	inv[0] = (float)(t[3] * invdet);
	inv[2] = (float)(-t[2] * invdet);
	inv[4] = (float)(((double)t[2] * t[5] - (double)t[3] * t[4]) * invdet);
	inv[1] = (float)(-t[1] * invdet);
	inv[3] = (float)(t[0] * invdet);
	inv[5] = (float)(((double)t[1] * t[4] - (double)t[0] * t[5]) * invdet);
}

static void nsvg__xformPremultiply(float* t, float* s)
{
	float s2[6];
	memcpy(s2, s, sizeof(float)*6);
	nsvg__xformMultiply(s2, t);
	memcpy(t, s2, sizeof(float)*6);
}

static void nsvg__xformPoint(float* dx, float* dy, float x, float y, float* t)
{
	*dx = x*t[0] + y*t[2] + t[4];
	*dy = x*t[1] + y*t[3] + t[5];
}

static void nsvg__xformVec(float* dx, float* dy, float x, float y, float* t)
{
	*dx = x*t[0] + y*t[2];
	*dy = x*t[1] + y*t[3];
}

#define NSVG_EPSILON (1e-12)

static int nsvg__ptInBounds(float* pt, float* bounds)
{
	return pt[0] >= bounds[0] && pt[0] <= bounds[2] && pt[1] >= bounds[1] && pt[1] <= bounds[3];
}

static double nsvg__evalBezier(double t, double p0, double p1, double p2, double p3)
{
	double it = 1.0-t;
	return it*it*it*p0 + 3.0*it*it*t*p1 + 3.0*it*t*t*p2 + t*t*t*p3;
}

static void nsvg__curveBounds(float* bounds, float* curve)
{
	int i, j, count;
	double roots[2], a, b, c, b2ac, t, v;
	float* v0 = &curve[0];
	float* v1 = &curve[2];
	float* v2 = &curve[4];
	float* v3 = &curve[6];

	// Start the bounding box by end points
	bounds[0] = nsvg__minf(v0[0], v3[0]);
	bounds[1] = nsvg__minf(v0[1], v3[1]);
	bounds[2] = nsvg__maxf(v0[0], v3[0]);
	bounds[3] = nsvg__maxf(v0[1], v3[1]);

	// Bezier curve fits inside the convex hull of it's control points.
	// If control points are inside the bounds, we're done.
	if (nsvg__ptInBounds(v1, bounds) && nsvg__ptInBounds(v2, bounds))
		return;

	// Add bezier curve inflection points in X and Y.
	for (i = 0; i < 2; i++) {
		a = -3.0 * v0[i] + 9.0 * v1[i] - 9.0 * v2[i] + 3.0 * v3[i];
		b = 6.0 * v0[i] - 12.0 * v1[i] + 6.0 * v2[i];
		c = 3.0 * v1[i] - 3.0 * v0[i];
		count = 0;
		if (fabs(a) < NSVG_EPSILON) {
			if (fabs(b) > NSVG_EPSILON) {
				t = -c / b;
				if (t > NSVG_EPSILON && t < 1.0-NSVG_EPSILON)
					roots[count++] = t;
			}
		} else {
			b2ac = b*b - 4.0*c*a;
			if (b2ac > NSVG_EPSILON) {
				t = (-b + sqrt(b2ac)) / (2.0 * a);
				if (t > NSVG_EPSILON && t < 1.0-NSVG_EPSILON)
					roots[count++] = t;
				t = (-b - sqrt(b2ac)) / (2.0 * a);
				if (t > NSVG_EPSILON && t < 1.0-NSVG_EPSILON)
					roots[count++] = t;
			}
		}
		for (j = 0; j < count; j++) {
			v = nsvg__evalBezier(roots[j], v0[i], v1[i], v2[i], v3[i]);
			bounds[0+i] = nsvg__minf(bounds[0+i], (float)v);
			bounds[2+i] = nsvg__maxf(bounds[2+i], (float)v);
		}
	}
}

static NSVGparser* nsvg__createParser()
{
	NSVGparser* p;
	p = (NSVGparser*)malloc(sizeof(NSVGparser));
	if (p == NULL) goto error;
	memset(p, 0, sizeof(NSVGparser));

	p->image = (NSVGimage*)malloc(sizeof(NSVGimage));
	if (p->image == NULL) goto error;
	memset(p->image, 0, sizeof(NSVGimage));

	// Init style
	nsvg__xformIdentity(p->attr[0].xform);
	memset(p->attr[0].id, 0, sizeof p->attr[0].id);
	p->attr[0].fillColor = NSVG_RGB(0,0,0);
	p->attr[0].strokeColor = NSVG_RGB(0,0,0);
	p->attr[0].opacity = 1;
	p->attr[0].fillOpacity = 1;
	p->attr[0].strokeOpacity = 1;
	p->attr[0].stopOpacity = 1;
	p->attr[0].strokeWidth = 1;
	p->attr[0].strokeLineJoin = NSVG_JOIN_MITER;
	p->attr[0].strokeLineCap = NSVG_CAP_BUTT;
	p->attr[0].hasFill = 1;
	p->attr[0].hasStroke = 0;
	p->attr[0].visible = 1;

	return p;

error:
	if (p) {
		if (p->image) free(p->image);
		free(p);
	}
	return NULL;
}

static void nsvg__deletePaths(NSVGpath* path)
{
	while (path) {
		NSVGpath *next = path->next;
		if (path->pts != NULL)
			free(path->pts);
		free(path);
		path = next;
	}
}

static void nsvg__deletePaint(NSVGpaint* paint)
{
	if (paint->type == NSVG_PAINT_LINEAR_GRADIENT || paint->type == NSVG_PAINT_LINEAR_GRADIENT)
		free(paint->gradient);
}

static void nsvg__deleteGradientData(NSVGgradientData* grad)
{
	NSVGgradientData* next;
	while (grad != NULL) {
		next = grad->next;
		free(grad->stops);
		free(grad);
		grad = next;
	}
}

static void nsvg__deleteParser(NSVGparser* p)
{
	if (p != NULL) {
		nsvg__deletePaths(p->plist);
		nsvg__deleteGradientData(p->gradients);
		nsvgDelete(p->image);
		free(p->pts);
		free(p);
	}
}

static void nsvg__resetPath(NSVGparser* p)
{
	p->npts = 0;
}

static void nsvg__addPoint(NSVGparser* p, float x, float y)
{
	if (p->npts+1 > p->cpts) {
		p->cpts = p->cpts ? p->cpts*2 : 8;
		p->pts = (float*)realloc(p->pts, p->cpts*2*sizeof(float));
		if (!p->pts) return;
	}
	p->pts[p->npts*2+0] = x;
	p->pts[p->npts*2+1] = y;
	p->npts++;
}

static void nsvg__moveTo(NSVGparser* p, float x, float y)
{
	if (p->npts > 0) {
		p->pts[(p->npts-1)*2+0] = x;
		p->pts[(p->npts-1)*2+1] = y;
	} else {
		nsvg__addPoint(p, x, y);
	}
}

static void nsvg__lineTo(NSVGparser* p, float x, float y)
{
	float px,py, dx,dy;
	if (p->npts > 0) {
		px = p->pts[(p->npts-1)*2+0];
		py = p->pts[(p->npts-1)*2+1];
		dx = x - px;
		dy = y - py;
		nsvg__addPoint(p, px + dx/3.0f, py + dy/3.0f);
		nsvg__addPoint(p, x - dx/3.0f, y - dy/3.0f);
		nsvg__addPoint(p, x, y);
	}
}

static void nsvg__cubicBezTo(NSVGparser* p, float cpx1, float cpy1, float cpx2, float cpy2, float x, float y)
{
	nsvg__addPoint(p, cpx1, cpy1);
	nsvg__addPoint(p, cpx2, cpy2);
	nsvg__addPoint(p, x, y);
}

static NSVGattrib* nsvg__getAttr(NSVGparser* p)
{
	return &p->attr[p->attrHead];
}

static void nsvg__pushAttr(NSVGparser* p)
{
	if (p->attrHead < NSVG_MAX_ATTR-1) {
		p->attrHead++;
		memcpy(&p->attr[p->attrHead], &p->attr[p->attrHead-1], sizeof(NSVGattrib));
	}
}

static void nsvg__popAttr(NSVGparser* p)
{
	if (p->attrHead > 0)
		p->attrHead--;
}

static NSVGgradientData* nsvg__findGradientData(NSVGparser* p, const char* id)
{
	NSVGgradientData* grad = p->gradients;
	while (grad) {
		if (strcmp(grad->id, id) == 0)
			return grad;
		grad = grad->next;
	}
	return NULL;
}

static NSVGgradient* nsvg__createGradient(NSVGparser* p, const char* id, const float* bounds, char* paintType)
{
	NSVGattrib* attr = nsvg__getAttr(p);
	NSVGgradientData* data = NULL;
	NSVGgradientData* ref = NULL;
	NSVGgradientStop* stops = NULL;
	NSVGgradient* grad;
	float dx, dy, d;
	int nstops = 0;
	NSVG_NOTUSED(bounds);

	data = nsvg__findGradientData(p, id);
	if (data == NULL) return NULL;

	// TODO: use ref to fill in all unset values too.
	ref = data;
	while (ref != NULL) {
		if (ref->stops != NULL) {
			stops = ref->stops;
			nstops = ref->nstops;
			break;
		}
		ref = nsvg__findGradientData(p, ref->ref);
	}
	if (stops == NULL) return NULL;

	grad = (NSVGgradient*)malloc(sizeof(NSVGgradient) + sizeof(NSVGgradientStop)*(nstops-1));
	if (grad == NULL) return NULL;

	// TODO: handle data->units == NSVG_OBJECT_SPACE.

	if (data->type == NSVG_PAINT_LINEAR_GRADIENT) {
		// Calculate transform aligned to the line
		dx = data->linear.x2 - data->linear.x1;
		dy = data->linear.y2 - data->linear.y1;
		d = sqrtf(dx*dx + dy*dy);
		grad->xform[0] = dy; grad->xform[1] = -dx;
		grad->xform[2] = dx; grad->xform[3] = dy;
		grad->xform[4] = data->linear.x1; grad->xform[5] = data->linear.y1;
	} else {
		// Calculate transform aligned to the circle
		grad->xform[0] = data->radial.r; grad->xform[1] = 0;
		grad->xform[2] = 0; grad->xform[3] = data->radial.r;
		grad->xform[4] = data->radial.cx; grad->xform[5] = data->radial.cy;
		grad->fx = data->radial.fx / data->radial.r;
		grad->fy = data->radial.fy / data->radial.r;
	}

	nsvg__xformMultiply(grad->xform, attr->xform);
	nsvg__xformMultiply(grad->xform, data->xform);

	grad->spread = data->spread;
	memcpy(grad->stops, stops, nstops*sizeof(NSVGgradientStop));
	grad->nstops = nstops;

	*paintType = data->type;

	return grad;
}

static float nsvg__getAverageScale(float* t)
{
	float sx = sqrtf(t[0]*t[0] + t[2]*t[2]);
	float sy = sqrtf(t[1]*t[1] + t[3]*t[3]);
	return (sx + sy) * 0.5f;
}

static void nsvg__addShape(NSVGparser* p)
{
	NSVGattrib* attr = nsvg__getAttr(p);
	float scale = 1.0f;
	NSVGshape *shape, *cur, *prev;
	NSVGpath* path;

	if (p->plist == NULL)
		return;

	shape = (NSVGshape*)malloc(sizeof(NSVGshape));
	if (shape == NULL) goto error;
	memset(shape, 0, sizeof(NSVGshape));

	memcpy(shape->id, attr->id, sizeof shape->id);
	scale = nsvg__getAverageScale(attr->xform);
	shape->strokeWidth = attr->strokeWidth * scale;
	shape->strokeLineJoin = attr->strokeLineJoin;
	shape->strokeLineCap = attr->strokeLineCap;
	shape->opacity = attr->opacity;

	shape->paths = p->plist;
	p->plist = NULL;

	// Calculate shape bounds
	shape->bounds[0] = shape->paths->bounds[0];
	shape->bounds[1] = shape->paths->bounds[1];
	shape->bounds[2] = shape->paths->bounds[2];
	shape->bounds[3] = shape->paths->bounds[3];
	for (path = shape->paths->next; path != NULL; path = path->next) {
		shape->bounds[0] = nsvg__minf(shape->bounds[0], path->bounds[0]);
		shape->bounds[1] = nsvg__minf(shape->bounds[1], path->bounds[1]);
		shape->bounds[2] = nsvg__maxf(shape->bounds[2], path->bounds[2]);
		shape->bounds[3] = nsvg__maxf(shape->bounds[3], path->bounds[3]);
	}

	// Set fill
	if (attr->hasFill == 0) {
		shape->fill.type = NSVG_PAINT_NONE;
	} else if (attr->hasFill == 1) {
		shape->fill.type = NSVG_PAINT_COLOR;
		shape->fill.color = attr->fillColor;
		shape->fill.color |= (unsigned int)(attr->fillOpacity*255) << 24;
	} else if (attr->hasFill == 2) {
		shape->fill.gradient = nsvg__createGradient(p, attr->fillGradient, shape->bounds, &shape->fill.type);
		if (shape->fill.gradient == NULL) {
			shape->fill.type = NSVG_PAINT_NONE;
		}
	}

	// Set stroke
	if (attr->hasStroke == 0) {
		shape->stroke.type = NSVG_PAINT_NONE;
	} else if (attr->hasStroke == 1) {
		shape->stroke.type = NSVG_PAINT_COLOR;
		shape->stroke.color = attr->strokeColor;
		shape->stroke.color |= (unsigned int)(attr->strokeOpacity*255) << 24;
	} else if (attr->hasStroke == 2) {
		shape->stroke.gradient = nsvg__createGradient(p, attr->strokeGradient, shape->bounds, &shape->stroke.type);
		if (shape->stroke.gradient == NULL)
			shape->stroke.type = NSVG_PAINT_NONE;
	}

	// Add to tail
	prev = NULL;
	cur = p->image->shapes;
	while (cur != NULL) {
		prev = cur;
		cur = cur->next;
	}
	if (prev == NULL)
		p->image->shapes = shape;
	else
		prev->next = shape;

	return;

error:
	if (shape) free(shape);
}

static void nsvg__addPath(NSVGparser* p, char closed)
{
	NSVGattrib* attr = nsvg__getAttr(p);
	NSVGpath* path = NULL;
	float bounds[4];
	float* curve;
	int i;

	if (p->npts < 4)
		return;

	if (closed)
		nsvg__lineTo(p, p->pts[0], p->pts[1]);

	path = (NSVGpath*)malloc(sizeof(NSVGpath));
	if (path == NULL) goto error;
	memset(path, 0, sizeof(NSVGpath));

	path->pts = (float*)malloc(p->npts*2*sizeof(float));
	if (path->pts == NULL) goto error;
	path->closed = closed;
	path->npts = p->npts;

	// Transform path.
	for (i = 0; i < p->npts; ++i)
		nsvg__xformPoint(&path->pts[i*2], &path->pts[i*2+1], p->pts[i*2], p->pts[i*2+1], attr->xform);

	// Find bounds
	for (i = 0; i < path->npts-1; i += 3) {
		curve = &path->pts[i*2];
		nsvg__curveBounds(bounds, curve);
		if (i == 0) {
			path->bounds[0] = bounds[0];
			path->bounds[1] = bounds[1];
			path->bounds[2] = bounds[2];
			path->bounds[3] = bounds[3];
		} else {
			path->bounds[0] = nsvg__minf(path->bounds[0], bounds[0]);
			path->bounds[1] = nsvg__minf(path->bounds[1], bounds[1]);
			path->bounds[2] = nsvg__maxf(path->bounds[2], bounds[2]);
			path->bounds[3] = nsvg__maxf(path->bounds[3], bounds[3]);
		}
	}

	path->next = p->plist;
	p->plist = path;

	return;

error:
	if (path != NULL) {
		if (path->pts != NULL) free(path->pts);
		free(path);
	}
}

static const char* nsvg__parseNumber(const char* s, char* it, const int size)
{
	const int last = size-1;
	int i = 0;

	// sign
	if (*s == '-' || *s == '+') {
		if (i < last) it[i++] = *s;
		s++;
	}
	// integer part
	while (*s && nsvg__isdigit(*s)) {
		if (i < last) it[i++] = *s;
		s++;
	}
	if (*s == '.') {
		// decimal point
		if (i < last) it[i++] = *s;
		s++;
		// fraction part
		while (*s && nsvg__isdigit(*s)) {
			if (i < last) it[i++] = *s;
			s++;
		}
	}
	// exponent
	if (*s == 'e' || *s == 'E') {
		if (i < last) it[i++] = *s;
		s++;
		if (*s == '-' || *s == '+') {
			if (i < last) it[i++] = *s;
			s++;
		}
		while (*s && nsvg__isdigit(*s)) {
			if (i < last) it[i++] = *s;
			s++;
		}
	}
	it[i] = '\0';

	return s;
}

static const char* nsvg__getNextPathItem(const char* s, char* it)
{
	it[0] = '\0';
	// Skip white spaces and commas
	while (*s && (nsvg__isspace(*s) || *s == ',')) s++;
	if (!*s) return s;
	if (*s == '-' || *s == '+' || *s == '.' || nsvg__isdigit(*s)) {
		s = nsvg__parseNumber(s, it, 64);
	} else {
		// Parse command
		it[0] = *s++;
		it[1] = '\0';
		return s;
	}

	return s;
}

static float nsvg__actualWidth(NSVGparser* p)
{
	return p->viewWidth;
}

static float nsvg__actualHeight(NSVGparser* p)
{
	return p->viewHeight;
}

static float nsvg__actualLength(NSVGparser* p)
{
	float w = nsvg__actualWidth(p), h = nsvg__actualHeight(p);
	return sqrtf(w*w + h*h) / sqrtf(2.0f);
}

static unsigned int nsvg__parseColorHex(const char* str)
{
	unsigned int c = 0, r = 0, g = 0, b = 0;
	int n = 0;
	str++; // skip #
	// Calculate number of characters.
	while(str[n] && !nsvg__isspace(str[n]))
		n++;
	if (n == 6) {
		sscanf(str, "%x", &c);
	} else if (n == 3) {
		sscanf(str, "%x", &c);
		c = (c&0xf) | ((c&0xf0) << 4) | ((c&0xf00) << 8);
		c |= c<<4;
	}
	r = (c >> 16) & 0xff;
	g = (c >> 8) & 0xff;
	b = c & 0xff;
	return NSVG_RGB(r,g,b);
}

static unsigned int nsvg__parseColorRGB(const char* str)
{
	int r = -1, g = -1, b = -1;
	char s1[32]="", s2[32]="";
	sscanf(str + 4, "%d%[%%, \t]%d%[%%, \t]%d", &r, s1, &g, s2, &b);
	if (strchr(s1, '%')) {
		return NSVG_RGB((r*255)/100,(g*255)/100,(b*255)/100);
	} else {
		return NSVG_RGB(r,g,b);
	}
}

typedef struct NSVGNamedColor {
	const char* name;
	unsigned int color;
} NSVGNamedColor;

NSVGNamedColor nsvg__colors[] = {

	{ "red", NSVG_RGB(255, 0, 0) },
	{ "green", NSVG_RGB( 0, 128, 0) },
	{ "blue", NSVG_RGB( 0, 0, 255) },
	{ "yellow", NSVG_RGB(255, 255, 0) },
	{ "cyan", NSVG_RGB( 0, 255, 255) },
	{ "magenta", NSVG_RGB(255, 0, 255) },
	{ "black", NSVG_RGB( 0, 0, 0) },
	{ "grey", NSVG_RGB(128, 128, 128) },
	{ "gray", NSVG_RGB(128, 128, 128) },
	{ "white", NSVG_RGB(255, 255, 255) },

#ifdef NANOSVG_ALL_COLOR_KEYWORDS
	{ "aliceblue", NSVG_RGB(240, 248, 255) },
	{ "antiquewhite", NSVG_RGB(250, 235, 215) },
	{ "aqua", NSVG_RGB( 0, 255, 255) },
	{ "aquamarine", NSVG_RGB(127, 255, 212) },
	{ "azure", NSVG_RGB(240, 255, 255) },
	{ "beige", NSVG_RGB(245, 245, 220) },
	{ "bisque", NSVG_RGB(255, 228, 196) },
	{ "blanchedalmond", NSVG_RGB(255, 235, 205) },
	{ "blueviolet", NSVG_RGB(138, 43, 226) },
	{ "brown", NSVG_RGB(165, 42, 42) },
	{ "burlywood", NSVG_RGB(222, 184, 135) },
	{ "cadetblue", NSVG_RGB( 95, 158, 160) },
	{ "chartreuse", NSVG_RGB(127, 255, 0) },
	{ "chocolate", NSVG_RGB(210, 105, 30) },
	{ "coral", NSVG_RGB(255, 127, 80) },
	{ "cornflowerblue", NSVG_RGB(100, 149, 237) },
	{ "cornsilk", NSVG_RGB(255, 248, 220) },
	{ "crimson", NSVG_RGB(220, 20, 60) },
	{ "darkblue", NSVG_RGB( 0, 0, 139) },
	{ "darkcyan", NSVG_RGB( 0, 139, 139) },
	{ "darkgoldenrod", NSVG_RGB(184, 134, 11) },
	{ "darkgray", NSVG_RGB(169, 169, 169) },
	{ "darkgreen", NSVG_RGB( 0, 100, 0) },
	{ "darkgrey", NSVG_RGB(169, 169, 169) },
	{ "darkkhaki", NSVG_RGB(189, 183, 107) },
	{ "darkmagenta", NSVG_RGB(139, 0, 139) },
	{ "darkolivegreen", NSVG_RGB( 85, 107, 47) },
	{ "darkorange", NSVG_RGB(255, 140, 0) },
	{ "darkorchid", NSVG_RGB(153, 50, 204) },
	{ "darkred", NSVG_RGB(139, 0, 0) },
	{ "darksalmon", NSVG_RGB(233, 150, 122) },
	{ "darkseagreen", NSVG_RGB(143, 188, 143) },
	{ "darkslateblue", NSVG_RGB( 72, 61, 139) },
	{ "darkslategray", NSVG_RGB( 47, 79, 79) },
	{ "darkslategrey", NSVG_RGB( 47, 79, 79) },
	{ "darkturquoise", NSVG_RGB( 0, 206, 209) },
	{ "darkviolet", NSVG_RGB(148, 0, 211) },
	{ "deeppink", NSVG_RGB(255, 20, 147) },
	{ "deepskyblue", NSVG_RGB( 0, 191, 255) },
	{ "dimgray", NSVG_RGB(105, 105, 105) },
	{ "dimgrey", NSVG_RGB(105, 105, 105) },
	{ "dodgerblue", NSVG_RGB( 30, 144, 255) },
	{ "firebrick", NSVG_RGB(178, 34, 34) },
	{ "floralwhite", NSVG_RGB(255, 250, 240) },
	{ "forestgreen", NSVG_RGB( 34, 139, 34) },
	{ "fuchsia", NSVG_RGB(255, 0, 255) },
	{ "gainsboro", NSVG_RGB(220, 220, 220) },
	{ "ghostwhite", NSVG_RGB(248, 248, 255) },
	{ "gold", NSVG_RGB(255, 215, 0) },
	{ "goldenrod", NSVG_RGB(218, 165, 32) },
	{ "greenyellow", NSVG_RGB(173, 255, 47) },
	{ "honeydew", NSVG_RGB(240, 255, 240) },
	{ "hotpink", NSVG_RGB(255, 105, 180) },
	{ "indianred", NSVG_RGB(205, 92, 92) },
	{ "indigo", NSVG_RGB( 75, 0, 130) },
	{ "ivory", NSVG_RGB(255, 255, 240) },
	{ "khaki", NSVG_RGB(240, 230, 140) },
	{ "lavender", NSVG_RGB(230, 230, 250) },
	{ "lavenderblush", NSVG_RGB(255, 240, 245) },
	{ "lawngreen", NSVG_RGB(124, 252, 0) },
	{ "lemonchiffon", NSVG_RGB(255, 250, 205) },
	{ "lightblue", NSVG_RGB(173, 216, 230) },
	{ "lightcoral", NSVG_RGB(240, 128, 128) },
	{ "lightcyan", NSVG_RGB(224, 255, 255) },
	{ "lightgoldenrodyellow", NSVG_RGB(250, 250, 210) },
	{ "lightgray", NSVG_RGB(211, 211, 211) },
	{ "lightgreen", NSVG_RGB(144, 238, 144) },
	{ "lightgrey", NSVG_RGB(211, 211, 211) },
	{ "lightpink", NSVG_RGB(255, 182, 193) },
	{ "lightsalmon", NSVG_RGB(255, 160, 122) },
	{ "lightseagreen", NSVG_RGB( 32, 178, 170) },
	{ "lightskyblue", NSVG_RGB(135, 206, 250) },
	{ "lightslategray", NSVG_RGB(119, 136, 153) },
	{ "lightslategrey", NSVG_RGB(119, 136, 153) },
	{ "lightsteelblue", NSVG_RGB(176, 196, 222) },
	{ "lightyellow", NSVG_RGB(255, 255, 224) },
	{ "lime", NSVG_RGB( 0, 255, 0) },
	{ "limegreen", NSVG_RGB( 50, 205, 50) },
	{ "linen", NSVG_RGB(250, 240, 230) },
	{ "maroon", NSVG_RGB(128, 0, 0) },
	{ "mediumaquamarine", NSVG_RGB(102, 205, 170) },
	{ "mediumblue", NSVG_RGB( 0, 0, 205) },
	{ "mediumorchid", NSVG_RGB(186, 85, 211) },
	{ "mediumpurple", NSVG_RGB(147, 112, 219) },
	{ "mediumseagreen", NSVG_RGB( 60, 179, 113) },
	{ "mediumslateblue", NSVG_RGB(123, 104, 238) },
	{ "mediumspringgreen", NSVG_RGB( 0, 250, 154) },
	{ "mediumturquoise", NSVG_RGB( 72, 209, 204) },
	{ "mediumvioletred", NSVG_RGB(199, 21, 133) },
	{ "midnightblue", NSVG_RGB( 25, 25, 112) },
	{ "mintcream", NSVG_RGB(245, 255, 250) },
	{ "mistyrose", NSVG_RGB(255, 228, 225) },
	{ "moccasin", NSVG_RGB(255, 228, 181) },
	{ "navajowhite", NSVG_RGB(255, 222, 173) },
	{ "navy", NSVG_RGB( 0, 0, 128) },
	{ "oldlace", NSVG_RGB(253, 245, 230) },
	{ "olive", NSVG_RGB(128, 128, 0) },
	{ "olivedrab", NSVG_RGB(107, 142, 35) },
	{ "orange", NSVG_RGB(255, 165, 0) },
	{ "orangered", NSVG_RGB(255, 69, 0) },
	{ "orchid", NSVG_RGB(218, 112, 214) },
	{ "palegoldenrod", NSVG_RGB(238, 232, 170) },
	{ "palegreen", NSVG_RGB(152, 251, 152) },
	{ "paleturquoise", NSVG_RGB(175, 238, 238) },
	{ "palevioletred", NSVG_RGB(219, 112, 147) },
	{ "papayawhip", NSVG_RGB(255, 239, 213) },
	{ "peachpuff", NSVG_RGB(255, 218, 185) },
	{ "peru", NSVG_RGB(205, 133, 63) },
	{ "pink", NSVG_RGB(255, 192, 203) },
	{ "plum", NSVG_RGB(221, 160, 221) },
	{ "powderblue", NSVG_RGB(176, 224, 230) },
	{ "purple", NSVG_RGB(128, 0, 128) },
	{ "rosybrown", NSVG_RGB(188, 143, 143) },
	{ "royalblue", NSVG_RGB( 65, 105, 225) },
	{ "saddlebrown", NSVG_RGB(139, 69, 19) },
	{ "salmon", NSVG_RGB(250, 128, 114) },
	{ "sandybrown", NSVG_RGB(244, 164, 96) },
	{ "seagreen", NSVG_RGB( 46, 139, 87) },
	{ "seashell", NSVG_RGB(255, 245, 238) },
	{ "sienna", NSVG_RGB(160, 82, 45) },
	{ "silver", NSVG_RGB(192, 192, 192) },
	{ "skyblue", NSVG_RGB(135, 206, 235) },
	{ "slateblue", NSVG_RGB(106, 90, 205) },
	{ "slategray", NSVG_RGB(112, 128, 144) },
	{ "slategrey", NSVG_RGB(112, 128, 144) },
	{ "snow", NSVG_RGB(255, 250, 250) },
	{ "springgreen", NSVG_RGB( 0, 255, 127) },
	{ "steelblue", NSVG_RGB( 70, 130, 180) },
	{ "tan", NSVG_RGB(210, 180, 140) },
	{ "teal", NSVG_RGB( 0, 128, 128) },
	{ "thistle", NSVG_RGB(216, 191, 216) },
	{ "tomato", NSVG_RGB(255, 99, 71) },
	{ "turquoise", NSVG_RGB( 64, 224, 208) },
	{ "violet", NSVG_RGB(238, 130, 238) },
	{ "wheat", NSVG_RGB(245, 222, 179) },
	{ "whitesmoke", NSVG_RGB(245, 245, 245) },
	{ "yellowgreen", NSVG_RGB(154, 205, 50) },
#endif
};

static unsigned int nsvg__parseColorName(const char* str)
{
	int i, ncolors = sizeof(nsvg__colors) / sizeof(NSVGNamedColor);

	for (i = 0; i < ncolors; i++) {
		if (strcmp(nsvg__colors[i].name, str) == 0) {
			return nsvg__colors[i].color;
		}
	}

	return NSVG_RGB(128, 128, 128);
}

static unsigned int nsvg__parseColor(const char* str)
{
	size_t len = 0;
	while(*str == ' ') ++str;
	len = strlen(str);
	if (len >= 1 && *str == '#')
		return nsvg__parseColorHex(str);
	else if (len >= 4 && str[0] == 'r' && str[1] == 'g' && str[2] == 'b' && str[3] == '(')
		return nsvg__parseColorRGB(str);
	return nsvg__parseColorName(str);
}

static float nsvg__convertToPixels(NSVGparser* p, float val, const char* units, int dir)
{
	NSVGattrib* attr;

	if (p != NULL) {
		// Convert units to pixels.
		if (units[0] == '\0') {
			return val;
		} else if (units[0] == 'p' && units[1] == 'x') {
			return val;
		} else if (units[0] == 'p' && units[1] == 't') {
			return val / 72.0f * p->dpi;
		} else if (units[0] == 'p' && units[1] == 'c') {
			return val / 6.0f * p->dpi;
		} else if (units[0] == 'm' && units[1] == 'm') {
			return val / 25.4f * p->dpi;
		} else if (units[0] == 'c' && units[1] == 'm') {
			return val / 2.54f * p->dpi;
		} else if (units[0] == 'i' && units[1] == 'n') {
			return val * p->dpi;
		} else if (units[0] == '%') {
			if (p != NULL) {
				attr = nsvg__getAttr(p);
				if (dir == 0)
					return (val/100.0f) * nsvg__actualWidth(p);
				else if (dir == 1)
					return (val/100.0f) * nsvg__actualHeight(p);
				else if (dir == 2)
					return (val/100.0f) * nsvg__actualLength(p);
			} else {
				return (val/100.0f);
			}
		} else if (units[0] == 'e' && units[1] == 'm') {
			if (p != NULL) {
				attr = nsvg__getAttr(p);
				return val * attr->fontSize;
			}
		} else if (units[0] == 'e' && units[1] == 'x') {
			if (p != NULL) {
				attr = nsvg__getAttr(p);
				return val * attr->fontSize * 0.52f; // x-height of Helvetica.
			}
		}
	} else {
		// Convert units to pixels.
		if (units[0] == '\0') {
			return val;
		} else if (units[0] == 'p' && units[1] == 'x') {
			return val;
		} else if (units[0] == '%') {
			return (val/100.0f);
		}
	}
	return val;
}

static float nsvg__parseFloat(NSVGparser* p, const char* str, int dir)
{
	float val = 0;
	char units[32]="";
	sscanf(str, "%f%s", &val, units);
	return nsvg__convertToPixels(p, val, units, dir);
}

static int nsvg__parseTransformArgs(const char* str, float* args, int maxNa, int* na)
{
	const char* end;
	const char* ptr;
	char it[64];

	*na = 0;
	ptr = str;
	while (*ptr && *ptr != '(') ++ptr;
	if (*ptr == 0)
		return 1;
	end = ptr;
	while (*end && *end != ')') ++end;
	if (*end == 0)
		return 1;

	while (ptr < end) {
		if (*ptr == '-' || *ptr == '+' || *ptr == '.' || nsvg__isdigit(*ptr)) {
			if (*na >= maxNa) return 0;
			ptr = nsvg__parseNumber(ptr, it, 64);
			args[(*na)++] = (float)atof(it);
		} else {
			++ptr;
		}
	}
	return (int)(end - str);
}

static int nsvg__parseMatrix(float* xform, const char* str)
{
	float t[6];
	int na = 0;
	int len = nsvg__parseTransformArgs(str, t, 6, &na);
	if (na != 6) return len;
	memcpy(xform, t, sizeof(float)*6);
	return len;
}

static int nsvg__parseTranslate(float* xform, const char* str)
{
	float args[2];
	float t[6];
	int na = 0;
	int len = nsvg__parseTransformArgs(str, args, 2, &na);
	if (na == 1) args[1] = 0.0;

	nsvg__xformSetTranslation(t, args[0], args[1]);
	memcpy(xform, t, sizeof(float)*6);
	return len;
}

static int nsvg__parseScale(float* xform, const char* str)
{
	float args[2];
	int na = 0;
	float t[6];
	int len = nsvg__parseTransformArgs(str, args, 2, &na);
	if (na == 1) args[1] = args[0];
	nsvg__xformSetScale(t, args[0], args[1]);
	memcpy(xform, t, sizeof(float)*6);
	return len;
}

static int nsvg__parseSkewX(float* xform, const char* str)
{
	float args[1];
	int na = 0;
	float t[6];
	int len = nsvg__parseTransformArgs(str, args, 1, &na);
	nsvg__xformSetSkewX(t, args[0]/180.0f*NSVG_PI);
	memcpy(xform, t, sizeof(float)*6);
	return len;
}

static int nsvg__parseSkewY(float* xform, const char* str)
{
	float args[1];
	int na = 0;
	float t[6];
	int len = nsvg__parseTransformArgs(str, args, 1, &na);
	nsvg__xformSetSkewY(t, args[0]/180.0f*NSVG_PI);
	memcpy(xform, t, sizeof(float)*6);
	return len;
}

static int nsvg__parseRotate(float* xform, const char* str)
{
	float args[3];
	int na = 0;
	float m[6];
	float t[6];
	int len = nsvg__parseTransformArgs(str, args, 3, &na);
	if (na == 1)
		args[1] = args[2] = 0.0f;
	nsvg__xformIdentity(m);

	if (na > 1) {
		nsvg__xformSetTranslation(t, -args[1], -args[2]);
		nsvg__xformMultiply(m, t);
	}

	nsvg__xformSetRotation(t, args[0]/180.0f*NSVG_PI);
	nsvg__xformMultiply(m, t);

	if (na > 1) {
		nsvg__xformSetTranslation(t, args[1], args[2]);
		nsvg__xformMultiply(m, t);
	}

	memcpy(xform, m, sizeof(float)*6);

	return len;
}

static void nsvg__parseTransform(float* xform, const char* str)
{
	float t[6];
	nsvg__xformIdentity(xform);
	while (*str)
	{
		if (strncmp(str, "matrix", 6) == 0)
			str += nsvg__parseMatrix(t, str);
		else if (strncmp(str, "translate", 9) == 0)
			str += nsvg__parseTranslate(t, str);
		else if (strncmp(str, "scale", 5) == 0)
			str += nsvg__parseScale(t, str);
		else if (strncmp(str, "rotate", 6) == 0)
			str += nsvg__parseRotate(t, str);
		else if (strncmp(str, "skewX", 5) == 0)
			str += nsvg__parseSkewX(t, str);
		else if (strncmp(str, "skewY", 5) == 0)
			str += nsvg__parseSkewY(t, str);
		else{
			++str;
			continue;
		}

		nsvg__xformPremultiply(xform, t);
	}
}

static void nsvg__parseUrl(char* id, const char* str)
{
	int i = 0;
	str += 4; // "url(";
	if (*str == '#')
		str++;
	while (i < 63 && *str != ')') {
		id[i] = *str++;
		i++;
	}
	id[i] = '\0';
}

static char nsvg__parseLineCap(const char* str)
{
	if (strcmp(str, "butt") == 0)
		return NSVG_CAP_BUTT;
	else if (strcmp(str, "round") == 0)
		return NSVG_CAP_ROUND;
	else if (strcmp(str, "square") == 0)
		return NSVG_CAP_SQUARE;
	// TODO: handle inherit.
	return NSVG_CAP_BUTT;
}

static char nsvg__parseLineJoin(const char* str)
{
	if (strcmp(str, "miter") == 0)
		return NSVG_JOIN_MITER;
	else if (strcmp(str, "round") == 0)
		return NSVG_JOIN_ROUND;
	else if (strcmp(str, "bevel") == 0)
		return NSVG_JOIN_BEVEL;
	// TODO: handle inherit.
	return NSVG_CAP_BUTT;
}

static void nsvg__parseStyle(NSVGparser* p, const char* str);

static int nsvg__parseAttr(NSVGparser* p, const char* name, const char* value)
{
	float xform[6];
	NSVGattrib* attr = nsvg__getAttr(p);
	if (!attr) return 0;

	if (strcmp(name, "style") == 0) {
		nsvg__parseStyle(p, value);
	} else if (strcmp(name, "display") == 0) {
		if (strcmp(value, "none") == 0)
			attr->visible = 0;
		else
			attr->visible = 1;
	} else if (strcmp(name, "fill") == 0) {
		if (strcmp(value, "none") == 0) {
			attr->hasFill = 0;
		} else if (strncmp(value, "url(", 4) == 0) {
			attr->hasFill = 2;
			nsvg__parseUrl(attr->fillGradient, value);
		} else {
			attr->hasFill = 1;
			attr->fillColor = nsvg__parseColor(value);
		}
	} else if (strcmp(name, "opacity") == 0) {
		attr->opacity = nsvg__parseFloat(p, value, 2);
	} else if (strcmp(name, "fill-opacity") == 0) {
		attr->fillOpacity = nsvg__parseFloat(p, value, 2);
	} else if (strcmp(name, "stroke") == 0) {
		if (strcmp(value, "none") == 0) {
			attr->hasStroke = 0;
		} else if (strncmp(value, "url(", 4) == 0) {
			attr->hasStroke = 2;
			nsvg__parseUrl(attr->strokeGradient, value);
		} else {
			attr->hasStroke = 1;
			attr->strokeColor = nsvg__parseColor(value);
		}
	} else if (strcmp(name, "stroke-width") == 0) {
		attr->strokeWidth = nsvg__parseFloat(p, value, 2);
	} else if (strcmp(name, "stroke-opacity") == 0) {
		attr->strokeOpacity = nsvg__parseFloat(NULL, value, 2);
	} else if (strcmp(name, "stroke-linecap") == 0) {
		attr->strokeLineCap = nsvg__parseLineCap(value);
	} else if (strcmp(name, "stroke-linejoin") == 0) {
		attr->strokeLineJoin = nsvg__parseLineJoin(value);
	} else if (strcmp(name, "font-size") == 0) {
		attr->fontSize = nsvg__parseFloat(p, value, 2);
	} else if (strcmp(name, "transform") == 0) {
		nsvg__parseTransform(xform, value);
		nsvg__xformPremultiply(attr->xform, xform);
	} else if (strcmp(name, "stop-color") == 0) {
		attr->stopColor = nsvg__parseColor(value);
	} else if (strcmp(name, "stop-opacity") == 0) {
		attr->stopOpacity = nsvg__parseFloat(NULL, value, 2);
	} else if (strcmp(name, "offset") == 0) {
		attr->stopOffset = nsvg__parseFloat(NULL, value, 2);
	} else if (strcmp(name, "id") == 0) {
		strncpy(attr->id, value, 63);
		attr->id[63] = '\0';
	} else {
		return 0;
	}
	return 1;
}

static int nsvg__parseNameValue(NSVGparser* p, const char* start, const char* end)
{
	const char* str;
	const char* val;
	char name[512];
	char value[512];
	int n;

	str = start;
	while (str < end && *str != ':') ++str;

	val = str;

	// Right Trim
	while (str > start &&  (*str == ':' || nsvg__isspace(*str))) --str;
	++str;

	n = (int)(str - start);
	if (n > 511) n = 511;
	if (n) memcpy(name, start, n);
	name[n] = 0;

	while (val < end && (*val == ':' || nsvg__isspace(*val))) ++val;

	n = (int)(end - val);
	if (n > 511) n = 511;
	if (n) memcpy(value, val, n);
	value[n] = 0;

	return nsvg__parseAttr(p, name, value);
}

static void nsvg__parseStyle(NSVGparser* p, const char* str)
{
	const char* start;
	const char* end;

	while (*str) {
		// Left Trim
		while(*str && nsvg__isspace(*str)) ++str;
		start = str;
		while(*str && *str != ';') ++str;
		end = str;

		// Right Trim
		while (end > start &&  (*end == ';' || nsvg__isspace(*end))) --end;
		++end;

		nsvg__parseNameValue(p, start, end);
		if (*str) ++str;
	}
}

static void nsvg__parseAttribs(NSVGparser* p, const char** attr)
{
	int i;
	for (i = 0; attr[i]; i += 2)
	{
		if (strcmp(attr[i], "style") == 0)
			nsvg__parseStyle(p, attr[i + 1]);
		else
			nsvg__parseAttr(p, attr[i], attr[i + 1]);
	}
}

static int nsvg__getArgsPerElement(char cmd)
{
	switch (cmd) {
		case 'v':
		case 'V':
		case 'h':
		case 'H':
			return 1;
		case 'm':
		case 'M':
		case 'l':
		case 'L':
		case 't':
		case 'T':
			return 2;
		case 'q':
		case 'Q':
		case 's':
		case 'S':
			return 4;
		case 'c':
		case 'C':
			return 6;
		case 'a':
		case 'A':
			return 7;
	}
	return 0;
}

static void nsvg__pathMoveTo(NSVGparser* p, float* cpx, float* cpy, float* args, int rel)
{
	if (rel) {
		*cpx += args[0];
		*cpy += args[1];
	} else {
		*cpx = args[0];
		*cpy = args[1];
	}
	nsvg__moveTo(p, *cpx, *cpy);
}

static void nsvg__pathLineTo(NSVGparser* p, float* cpx, float* cpy, float* args, int rel)
{
	if (rel) {
		*cpx += args[0];
		*cpy += args[1];
	} else {
		*cpx = args[0];
		*cpy = args[1];
	}
	nsvg__lineTo(p, *cpx, *cpy);
}

static void nsvg__pathHLineTo(NSVGparser* p, float* cpx, float* cpy, float* args, int rel)
{
	if (rel)
		*cpx += args[0];
	else
		*cpx = args[0];
	nsvg__lineTo(p, *cpx, *cpy);
}

static void nsvg__pathVLineTo(NSVGparser* p, float* cpx, float* cpy, float* args, int rel)
{
	if (rel)
		*cpy += args[0];
	else
		*cpy = args[0];
	nsvg__lineTo(p, *cpx, *cpy);
}

static void nsvg__pathCubicBezTo(NSVGparser* p, float* cpx, float* cpy,
								 float* cpx2, float* cpy2, float* args, int rel)
{
	float x1, y1, x2, y2, cx1, cy1, cx2, cy2;

	x1 = *cpx;
	y1 = *cpy;
	if (rel) {
		cx1 = *cpx + args[0];
		cy1 = *cpy + args[1];
		cx2 = *cpx + args[2];
		cy2 = *cpy + args[3];
		x2 = *cpx + args[4];
		y2 = *cpy + args[5];
	} else {
		cx1 = args[0];
		cy1 = args[1];
		cx2 = args[2];
		cy2 = args[3];
		x2 = args[4];
		y2 = args[5];
	}

	nsvg__cubicBezTo(p, cx1,cy1, cx2,cy2, x2,y2);

	*cpx2 = cx2;
	*cpy2 = cy2;
	*cpx = x2;
	*cpy = y2;
}

static void nsvg__pathCubicBezShortTo(NSVGparser* p, float* cpx, float* cpy,
									  float* cpx2, float* cpy2, float* args, int rel)
{
	float x1, y1, x2, y2, cx1, cy1, cx2, cy2;

	x1 = *cpx;
	y1 = *cpy;
	if (rel) {
		cx2 = *cpx + args[0];
		cy2 = *cpy + args[1];
		x2 = *cpx + args[2];
		y2 = *cpy + args[3];
	} else {
		cx2 = args[0];
		cy2 = args[1];
		x2 = args[2];
		y2 = args[3];
	}

	cx1 = 2*x1 - *cpx2;
	cy1 = 2*y1 - *cpy2;

	nsvg__cubicBezTo(p, cx1,cy1, cx2,cy2, x2,y2);

	*cpx2 = cx2;
	*cpy2 = cy2;
	*cpx = x2;
	*cpy = y2;
}

static void nsvg__pathQuadBezTo(NSVGparser* p, float* cpx, float* cpy,
								float* cpx2, float* cpy2, float* args, int rel)
{
	float x1, y1, x2, y2, cx, cy;
	float cx1, cy1, cx2, cy2;

	x1 = *cpx;
	y1 = *cpy;
	if (rel) {
		cx = *cpx + args[0];
		cy = *cpy + args[1];
		x2 = *cpx + args[2];
		y2 = *cpy + args[3];
	} else {
		cx = args[0];
		cy = args[1];
		x2 = args[2];
		y2 = args[3];
	}

	// Convert to cubic bezier
	cx1 = x1 + 2.0f/3.0f*(cx - x1);
	cy1 = y1 + 2.0f/3.0f*(cy - y1);
	cx2 = x2 + 2.0f/3.0f*(cx - x2);
	cy2 = y2 + 2.0f/3.0f*(cy - y2);

	nsvg__cubicBezTo(p, cx1,cy1, cx2,cy2, x2,y2);

	*cpx2 = cx;
	*cpy2 = cy;
	*cpx = x2;
	*cpy = y2;
}

static void nsvg__pathQuadBezShortTo(NSVGparser* p, float* cpx, float* cpy,
									 float* cpx2, float* cpy2, float* args, int rel)
{
	float x1, y1, x2, y2, cx, cy;
	float cx1, cy1, cx2, cy2;

	x1 = *cpx;
	y1 = *cpy;
	if (rel) {
		x2 = *cpx + args[0];
		y2 = *cpy + args[1];
	} else {
		x2 = args[0];
		y2 = args[1];
	}

	cx = 2*x1 - *cpx2;
	cy = 2*y1 - *cpy2;

	// Convert to cubix bezier
	cx1 = x1 + 2.0f/3.0f*(cx - x1);
	cy1 = y1 + 2.0f/3.0f*(cy - y1);
	cx2 = x2 + 2.0f/3.0f*(cx - x2);
	cy2 = y2 + 2.0f/3.0f*(cy - y2);

	nsvg__cubicBezTo(p, cx1,cy1, cx2,cy2, x2,y2);

	*cpx2 = cx;
	*cpy2 = cy;
	*cpx = x2;
	*cpy = y2;
}

static float nsvg__sqr(float x) { return x*x; }
static float nsvg__vmag(float x, float y) { return sqrtf(x*x + y*y); }

static float nsvg__vecrat(float ux, float uy, float vx, float vy)
{
	return (ux*vx + uy*vy) / (nsvg__vmag(ux,uy) * nsvg__vmag(vx,vy));
}

static float nsvg__vecang(float ux, float uy, float vx, float vy)
{
	float r = nsvg__vecrat(ux,uy, vx,vy);
	if (r < -1.0f) r = -1.0f;
	if (r > 1.0f) r = 1.0f;
	return ((ux*vy < uy*vx) ? -1.0f : 1.0f) * acosf(r);
}

static void nsvg__pathArcTo(NSVGparser* p, float* cpx, float* cpy, float* args, int rel)
{
	// Ported from canvg (https://code.google.com/p/canvg/)
	float rx, ry, rotx;
	float x1, y1, x2, y2, cx, cy, dx, dy, d;
	float x1p, y1p, cxp, cyp, s, sa, sb;
	float ux, uy, vx, vy, a1, da;
	float x, y, tanx, tany, a, px = 0, py = 0, ptanx = 0, ptany = 0, t[6];
	float sinrx, cosrx;
	int fa, fs;
	int i, ndivs;
	float hda, kappa;

	rx = fabsf(args[0]);				// y radius
	ry = fabsf(args[1]);				// x radius
	rotx = args[2] / 180.0f * NSVG_PI;		// x rotation engle
	fa = fabsf(args[3]) > 1e-6 ? 1 : 0;	// Large arc
	fs = fabsf(args[4]) > 1e-6 ? 1 : 0;	// Sweep direction
	x1 = *cpx;							// start point
	y1 = *cpy;
	if (rel) {							// end point
		x2 = *cpx + args[5];
		y2 = *cpy + args[6];
	} else {
		x2 = args[5];
		y2 = args[6];
	}

	dx = x1 - x2;
	dy = y1 - y2;
	d = sqrtf(dx*dx + dy*dy);
	if (d < 1e-6f || rx < 1e-6f || ry < 1e-6f) {
		// The arc degenerates to a line
		nsvg__lineTo(p, x2, y2);
		*cpx = x2;
		*cpy = y2;
		return;
	}

	sinrx = sinf(rotx);
	cosrx = cosf(rotx);

	// Convert to center point parameterization.
	// http://www.w3.org/TR/SVG11/implnote.html#ArcImplementationNotes
	// 1) Compute x1', y1'
	x1p = cosrx * dx / 2.0f + sinrx * dy / 2.0f;
	y1p = -sinrx * dx / 2.0f + cosrx * dy / 2.0f;
	d = nsvg__sqr(x1p)/nsvg__sqr(rx) + nsvg__sqr(y1p)/nsvg__sqr(ry);
	if (d > 1) {
		d = sqrtf(d);
		rx *= d;
		ry *= d;
	}
	// 2) Compute cx', cy'
	s = 0.0f;
	sa = nsvg__sqr(rx)*nsvg__sqr(ry) - nsvg__sqr(rx)*nsvg__sqr(y1p) - nsvg__sqr(ry)*nsvg__sqr(x1p);
	sb = nsvg__sqr(rx)*nsvg__sqr(y1p) + nsvg__sqr(ry)*nsvg__sqr(x1p);
	if (sa < 0.0f) sa = 0.0f;
	if (sb > 0.0f)
		s = sqrtf(sa / sb);
	if (fa == fs)
		s = -s;
	cxp = s * rx * y1p / ry;
	cyp = s * -ry * x1p / rx;

	// 3) Compute cx,cy from cx',cy'
	cx = (x1 + x2)/2.0f + cosrx*cxp - sinrx*cyp;
	cy = (y1 + y2)/2.0f + sinrx*cxp + cosrx*cyp;

	// 4) Calculate theta1, and delta theta.
	ux = (x1p - cxp) / rx;
	uy = (y1p - cyp) / ry;
	vx = (-x1p - cxp) / rx;
	vy = (-y1p - cyp) / ry;
	a1 = nsvg__vecang(1.0f,0.0f, ux,uy);	// Initial angle
	da = nsvg__vecang(ux,uy, vx,vy);		// Delta angle

//	if (vecrat(ux,uy,vx,vy) <= -1.0f) da = NSVG_PI;
//	if (vecrat(ux,uy,vx,vy) >= 1.0f) da = 0;

	if (fa) {
		// Choose large arc
		if (da > 0.0f)
			da = da - 2*NSVG_PI;
		else
			da = 2*NSVG_PI + da;
	}

	// Approximate the arc using cubic spline segments.
	t[0] = cosrx; t[1] = sinrx;
	t[2] = -sinrx; t[3] = cosrx;
	t[4] = cx; t[5] = cy;

	// Split arc into max 90 degree segments.
	// The loop assumes an iteration per end point (including start and end), this +1.
	ndivs = (int)(fabsf(da) / (NSVG_PI*0.5f) + 1.0f);
	hda = (da / (float)ndivs) / 2.0f;
	kappa = fabsf(4.0f / 3.0f * (1.0f - cosf(hda)) / sinf(hda));
	if (da < 0.0f)
		kappa = -kappa;

	for (i = 0; i <= ndivs; i++) {
		a = a1 + da * (i/(float)ndivs);
		dx = cosf(a);
		dy = sinf(a);
		nsvg__xformPoint(&x, &y, dx*rx, dy*ry, t); // position
		nsvg__xformVec(&tanx, &tany, -dy*rx * kappa, dx*ry * kappa, t); // tangent
		if (i > 0)
			nsvg__cubicBezTo(p, px+ptanx,py+ptany, x-tanx, y-tany, x, y);
		px = x;
		py = y;
		ptanx = tanx;
		ptany = tany;
	}

	*cpx = x2;
	*cpy = y2;
}

static void nsvg__parsePath(NSVGparser* p, const char** attr)
{
	const char* s = NULL;
	char cmd = '\0';
	float args[10];
	int nargs;
	int rargs = 0;
	float cpx, cpy, cpx2, cpy2;
	const char* tmp[4];
	char closedFlag;
	int i;
	char item[64];

	for (i = 0; attr[i]; i += 2) {
		if (strcmp(attr[i], "d") == 0) {
			s = attr[i + 1];
		} else {
			tmp[0] = attr[i];
			tmp[1] = attr[i + 1];
			tmp[2] = 0;
			tmp[3] = 0;
			nsvg__parseAttribs(p, tmp);
		}
	}

	if (s) {
		nsvg__resetPath(p);
		cpx = 0; cpy = 0;
		closedFlag = 0;
		nargs = 0;

		while (*s) {
			s = nsvg__getNextPathItem(s, item);
			if (!*item) break;
			if (nsvg__isnum(item[0])) {
				if (nargs < 10)
					args[nargs++] = (float)atof(item);
				if (nargs >= rargs) {
					switch (cmd) {
						case 'm':
						case 'M':
							nsvg__pathMoveTo(p, &cpx, &cpy, args, cmd == 'm' ? 1 : 0);
							// Moveto can be followed by multiple coordinate pairs,
							// which should be treated as linetos.
							cmd = (cmd == 'm') ? 'l' : 'L';
							rargs = nsvg__getArgsPerElement(cmd);
							cpx2 = cpx; cpy2 = cpy;
							break;
						case 'l':
						case 'L':
							nsvg__pathLineTo(p, &cpx, &cpy, args, cmd == 'l' ? 1 : 0);
							cpx2 = cpx; cpy2 = cpy;
							break;
						case 'H':
						case 'h':
							nsvg__pathHLineTo(p, &cpx, &cpy, args, cmd == 'h' ? 1 : 0);
							cpx2 = cpx; cpy2 = cpy;
							break;
						case 'V':
						case 'v':
							nsvg__pathVLineTo(p, &cpx, &cpy, args, cmd == 'v' ? 1 : 0);
							cpx2 = cpx; cpy2 = cpy;
							break;
						case 'C':
						case 'c':
							nsvg__pathCubicBezTo(p, &cpx, &cpy, &cpx2, &cpy2, args, cmd == 'c' ? 1 : 0);
							break;
						case 'S':
						case 's':
							nsvg__pathCubicBezShortTo(p, &cpx, &cpy, &cpx2, &cpy2, args, cmd == 's' ? 1 : 0);
							break;
						case 'Q':
						case 'q':
							nsvg__pathQuadBezTo(p, &cpx, &cpy, &cpx2, &cpy2, args, cmd == 'q' ? 1 : 0);
							break;
						case 'T':
						case 't':
							nsvg__pathQuadBezShortTo(p, &cpx, &cpy, &cpx2, &cpy2, args, cmd == 's' ? 1 : 0);
							break;
						case 'A':
						case 'a':
							nsvg__pathArcTo(p, &cpx, &cpy, args, cmd == 'a' ? 1 : 0);
							cpx2 = cpx; cpy2 = cpy;
							break;
						default:
							if (nargs >= 2) {
								cpx = args[nargs-2];
								cpy = args[nargs-1];
	                            cpx2 = cpx; cpy2 = cpy;
							}
							break;
					}
					nargs = 0;
				}
			} else {
				cmd = item[0];
				rargs = nsvg__getArgsPerElement(cmd);
				if (cmd == 'M' || cmd == 'm') {
					// Commit path.
					if (p->npts > 0)
						nsvg__addPath(p, closedFlag);
					// Start new subpath.
					nsvg__resetPath(p);
					closedFlag = 0;
					nargs = 0;
				} else if (cmd == 'Z' || cmd == 'z') {
					closedFlag = 1;
					// Commit path.
					if (p->npts > 0) {
						// Move current point to first point
						cpx = p->pts[0];
						cpy = p->pts[1];
						cpx2 = cpx; cpy2 = cpy;
						nsvg__addPath(p, closedFlag);
					}
					// Start new subpath.
					nsvg__resetPath(p);
					nsvg__moveTo(p, cpx, cpy);
					closedFlag = 0;
					nargs = 0;
				}
			}
		}
		// Commit path.
		if (p->npts)
			nsvg__addPath(p, closedFlag);
	}

	nsvg__addShape(p);
}

static void nsvg__parseRect(NSVGparser* p, const char** attr)
{
	float x = 0.0f;
	float y = 0.0f;
	float w = 0.0f;
	float h = 0.0f;
	float rx = -1.0f; // marks not set
	float ry = -1.0f;
	int i;

	for (i = 0; attr[i]; i += 2) {
		if (!nsvg__parseAttr(p, attr[i], attr[i + 1])) {
			if (strcmp(attr[i], "x") == 0) x = nsvg__parseFloat(p, attr[i+1], 0);
			if (strcmp(attr[i], "y") == 0) y = nsvg__parseFloat(p, attr[i+1], 1);
			if (strcmp(attr[i], "width") == 0) w = nsvg__parseFloat(p, attr[i+1], 0);
			if (strcmp(attr[i], "height") == 0) h = nsvg__parseFloat(p, attr[i+1], 1);
			if (strcmp(attr[i], "rx") == 0) rx = fabsf(nsvg__parseFloat(p, attr[i+1], 0));
			if (strcmp(attr[i], "ry") == 0) ry = fabsf(nsvg__parseFloat(p, attr[i+1], 1));
		}
	}

	if (rx < 0.0f && ry > 0.0f) rx = ry;
	if (ry < 0.0f && rx > 0.0f) ry = rx;
	if (rx < 0.0f) rx = 0.0f;
	if (ry < 0.0f) ry = 0.0f;
	if (rx > w/2.0f) rx = w/2.0f;
	if (ry > h/2.0f) ry = h/2.0f;

	if (w != 0.0f && h != 0.0f) {
		nsvg__resetPath(p);

		if (rx < 0.00001f || ry < 0.0001f) {
			nsvg__moveTo(p, x, y);
			nsvg__lineTo(p, x+w, y);
			nsvg__lineTo(p, x+w, y+h);
			nsvg__lineTo(p, x, y+h);
		} else {
			// Rounded rectangle
			nsvg__moveTo(p, x+rx, y);
			nsvg__lineTo(p, x+w-rx, y);
			nsvg__cubicBezTo(p, x+w-rx*(1-NSVG_KAPPA90), y, x+w, y+ry*(1-NSVG_KAPPA90), x+w, y+ry);
			nsvg__lineTo(p, x+w, y+h-ry);
			nsvg__cubicBezTo(p, x+w, y+h-ry*(1-NSVG_KAPPA90), x+w-rx*(1-NSVG_KAPPA90), y+h, x+w-rx, y+h);
			nsvg__lineTo(p, x+rx, y+h);
			nsvg__cubicBezTo(p, x+rx*(1-NSVG_KAPPA90), y+h, x, y+h-ry*(1-NSVG_KAPPA90), x, y+h-ry);
			nsvg__lineTo(p, x, y+ry);
			nsvg__cubicBezTo(p, x, y+ry*(1-NSVG_KAPPA90), x+rx*(1-NSVG_KAPPA90), y, x+rx, y);
		}

		nsvg__addPath(p, 1);

		nsvg__addShape(p);
	}
}

static void nsvg__parseCircle(NSVGparser* p, const char** attr)
{
	float cx = 0.0f;
	float cy = 0.0f;
	float r = 0.0f;
	int i;

	for (i = 0; attr[i]; i += 2) {
		if (!nsvg__parseAttr(p, attr[i], attr[i + 1])) {
			if (strcmp(attr[i], "cx") == 0) cx = nsvg__parseFloat(p, attr[i+1], 0);
			if (strcmp(attr[i], "cy") == 0) cy = nsvg__parseFloat(p, attr[i+1], 1);
			if (strcmp(attr[i], "r") == 0) r = fabsf(nsvg__parseFloat(p, attr[i+1], 2));
		}
	}

	if (r > 0.0f) {
		nsvg__resetPath(p);

		nsvg__moveTo(p, cx+r, cy);
		nsvg__cubicBezTo(p, cx+r, cy+r*NSVG_KAPPA90, cx+r*NSVG_KAPPA90, cy+r, cx, cy+r);
		nsvg__cubicBezTo(p, cx-r*NSVG_KAPPA90, cy+r, cx-r, cy+r*NSVG_KAPPA90, cx-r, cy);
		nsvg__cubicBezTo(p, cx-r, cy-r*NSVG_KAPPA90, cx-r*NSVG_KAPPA90, cy-r, cx, cy-r);
		nsvg__cubicBezTo(p, cx+r*NSVG_KAPPA90, cy-r, cx+r, cy-r*NSVG_KAPPA90, cx+r, cy);

		nsvg__addPath(p, 1);

		nsvg__addShape(p);
	}
}

static void nsvg__parseEllipse(NSVGparser* p, const char** attr)
{
	float cx = 0.0f;
	float cy = 0.0f;
	float rx = 0.0f;
	float ry = 0.0f;
	int i;

	for (i = 0; attr[i]; i += 2) {
		if (!nsvg__parseAttr(p, attr[i], attr[i + 1])) {
			if (strcmp(attr[i], "cx") == 0) cx = nsvg__parseFloat(p, attr[i+1], 0);
			if (strcmp(attr[i], "cy") == 0) cy = nsvg__parseFloat(p, attr[i+1], 1);
			if (strcmp(attr[i], "rx") == 0) rx = fabsf(nsvg__parseFloat(p, attr[i+1], 0));
			if (strcmp(attr[i], "ry") == 0) ry = fabsf(nsvg__parseFloat(p, attr[i+1], 1));
		}
	}

	if (rx > 0.0f && ry > 0.0f) {

		nsvg__resetPath(p);

		nsvg__moveTo(p, cx+rx, cy);
		nsvg__cubicBezTo(p, cx+rx, cy+ry*NSVG_KAPPA90, cx+rx*NSVG_KAPPA90, cy+ry, cx, cy+ry);
		nsvg__cubicBezTo(p, cx-rx*NSVG_KAPPA90, cy+ry, cx-rx, cy+ry*NSVG_KAPPA90, cx-rx, cy);
		nsvg__cubicBezTo(p, cx-rx, cy-ry*NSVG_KAPPA90, cx-rx*NSVG_KAPPA90, cy-ry, cx, cy-ry);
		nsvg__cubicBezTo(p, cx+rx*NSVG_KAPPA90, cy-ry, cx+rx, cy-ry*NSVG_KAPPA90, cx+rx, cy);

		nsvg__addPath(p, 1);

		nsvg__addShape(p);
	}
}

static void nsvg__parseLine(NSVGparser* p, const char** attr)
{
	float x1 = 0.0;
	float y1 = 0.0;
	float x2 = 0.0;
	float y2 = 0.0;
	int i;

	for (i = 0; attr[i]; i += 2) {
		if (!nsvg__parseAttr(p, attr[i], attr[i + 1])) {
			if (strcmp(attr[i], "x1") == 0) x1 = nsvg__parseFloat(p, attr[i + 1], 0);
			if (strcmp(attr[i], "y1") == 0) y1 = nsvg__parseFloat(p, attr[i + 1], 1);
			if (strcmp(attr[i], "x2") == 0) x2 = nsvg__parseFloat(p, attr[i + 1], 0);
			if (strcmp(attr[i], "y2") == 0) y2 = nsvg__parseFloat(p, attr[i + 1], 1);
		}
	}

	nsvg__resetPath(p);

	nsvg__moveTo(p, x1, y1);
	nsvg__lineTo(p, x2, y2);

	nsvg__addPath(p, 0);

	nsvg__addShape(p);
}

static void nsvg__parsePoly(NSVGparser* p, const char** attr, int closeFlag)
{
	int i;
	const char* s;
	float args[2];
	int nargs, npts = 0;
	char item[64];

	nsvg__resetPath(p);

	for (i = 0; attr[i]; i += 2) {
		if (!nsvg__parseAttr(p, attr[i], attr[i + 1])) {
			if (strcmp(attr[i], "points") == 0) {
				s = attr[i + 1];
				nargs = 0;
				while (*s) {
					s = nsvg__getNextPathItem(s, item);
					args[nargs++] = (float)atof(item);
					if (nargs >= 2) {
						if (npts == 0)
							nsvg__moveTo(p, args[0], args[1]);
						else
							nsvg__lineTo(p, args[0], args[1]);
						nargs = 0;
						npts++;
					}
				}
			}
		}
	}

	nsvg__addPath(p, (char)closeFlag);

	nsvg__addShape(p);
}

static void nsvg__parseSVG(NSVGparser* p, const char** attr)
{
	int i;
	for (i = 0; attr[i]; i += 2) {
		if (!nsvg__parseAttr(p, attr[i], attr[i + 1])) {
			if (strcmp(attr[i], "width") == 0) {
				p->image->width = nsvg__parseFloat(p, attr[i + 1], 0);
			} else if (strcmp(attr[i], "height") == 0) {
				p->image->height = nsvg__parseFloat(p, attr[i + 1], 1);
			} else if (strcmp(attr[i], "viewBox") == 0) {
				sscanf(attr[i + 1], "%f%*[%%, \t]%f%*[%%, \t]%f%*[%%, \t]%f", &p->viewMinx, &p->viewMiny, &p->viewWidth, &p->viewHeight);
			} else if (strcmp(attr[i], "preserveAspectRatio") == 0) {
				if (strstr(attr[i + 1], "none") != 0) {
					// No uniform scaling
					p->alignType = NSVG_ALIGN_NONE;
				} else {
					// Parse X align
					if (strstr(attr[i + 1], "xMin") != 0)
						p->alignX = NSVG_ALIGN_MIN;
					else if (strstr(attr[i + 1], "xMid") != 0)
						p->alignX = NSVG_ALIGN_MID;
					else if (strstr(attr[i + 1], "xMax") != 0)
						p->alignX = NSVG_ALIGN_MAX;
					// Parse X align
					if (strstr(attr[i + 1], "yMin") != 0)
						p->alignY = NSVG_ALIGN_MIN;
					else if (strstr(attr[i + 1], "yMid") != 0)
						p->alignY = NSVG_ALIGN_MID;
					else if (strstr(attr[i + 1], "yMax") != 0)
						p->alignY = NSVG_ALIGN_MAX;
					// Parse meet/slice
					p->alignType = NSVG_ALIGN_MEET;
					if (strstr(attr[i + 1], "slice") != 0)
						p->alignType = NSVG_ALIGN_SLICE;
				}
			}
		}
	}
}

static void nsvg__parseGradient(NSVGparser* p, const char** attr, char type)
{
	int i;
	NSVGgradientData* grad = (NSVGgradientData*)malloc(sizeof(NSVGgradientData));
	if (grad == NULL) return;
	memset(grad, 0, sizeof(NSVGgradientData));

	grad->type = type;
	nsvg__xformIdentity(grad->xform);

	// TODO: does not handle percent and objectBoundingBox correctly yet.
	for (i = 0; attr[i]; i += 2) {
		if (strcmp(attr[i], "id") == 0) {
			strncpy(grad->id, attr[i+1], 63);
			grad->id[63] = '\0';
		} else if (!nsvg__parseAttr(p, attr[i], attr[i + 1])) {
			if (strcmp(attr[i], "gradientUnits") == 0) {
				if (strcmp(attr[i+1], "objectBoundingBox") == 0)
					grad->units = NSVG_OBJECT_SPACE;
				else
					grad->units = NSVG_USER_SPACE;
			} else if (strcmp(attr[i], "gradientTransform") == 0) {
				nsvg__parseTransform(grad->xform, attr[i + 1]);
			} else if (strcmp(attr[i], "cx") == 0) {
				grad->radial.cx = nsvg__parseFloat(p, attr[i + 1], 0);
			} else if (strcmp(attr[i], "cy") == 0) {
				grad->radial.cy = nsvg__parseFloat(p, attr[i + 1], 1);
			} else if (strcmp(attr[i], "r") == 0) {
				grad->radial.r = nsvg__parseFloat(p, attr[i + 1], 2);
			} else if (strcmp(attr[i], "fx") == 0) {
				grad->radial.fx = nsvg__parseFloat(p, attr[i + 1], 0);
			} else if (strcmp(attr[i], "fy") == 0) {
				grad->radial.fy = nsvg__parseFloat(p, attr[i + 1], 1);
			} else if (strcmp(attr[i], "x1") == 0) {
				grad->linear.x1 = nsvg__parseFloat(p, attr[i + 1], 0);
			} else if (strcmp(attr[i], "y1") == 0) {
				grad->linear.y1 = nsvg__parseFloat(p, attr[i + 1], 1);
			} else if (strcmp(attr[i], "x2") == 0) {
				grad->linear.x2 = nsvg__parseFloat(p, attr[i + 1], 0);
			} else if (strcmp(attr[i], "y2") == 0) {
				grad->linear.y2 = nsvg__parseFloat(p, attr[i + 1], 1);
			} else if (strcmp(attr[i], "spreadMethod") == 0) {
				if (strcmp(attr[i+1], "pad") == 0)
					grad->spread = NSVG_SPREAD_PAD;
				else if (strcmp(attr[i+1], "reflect") == 0)
					grad->spread = NSVG_SPREAD_REFLECT;
				else if (strcmp(attr[i+1], "repeat") == 0)
					grad->spread = NSVG_SPREAD_REPEAT;
			} else if (strcmp(attr[i], "xlink:href") == 0) {
				strncpy(grad->ref, attr[i+1], 63);
				grad->ref[63] = '\0';
			}
		}
	}

	grad->next = p->gradients;
	p->gradients = grad;
}

static void nsvg__parseGradientStop(NSVGparser* p, const char** attr)
{
	NSVGattrib* curAttr = nsvg__getAttr(p);
	NSVGgradientData* grad;
	NSVGgradientStop* stop;
	int i, idx;

	curAttr->stopOffset = 0;
	curAttr->stopColor = 0;
	curAttr->stopOpacity = 1.0f;

	for (i = 0; attr[i]; i += 2) {
		nsvg__parseAttr(p, attr[i], attr[i + 1]);
	}

	// Add stop to the last gradient.
	grad = p->gradients;
	if (grad == NULL) return;

	grad->nstops++;
	grad->stops = (NSVGgradientStop*)realloc(grad->stops, sizeof(NSVGgradientStop)*grad->nstops);
	if (grad->stops == NULL) return;

	// Insert
	idx = grad->nstops-1;
	for (i = 0; i < grad->nstops-1; i++) {
		if (curAttr->stopOffset < grad->stops[i].offset) {
			idx = i;
			break;
		}
	}
	if (idx != grad->nstops-1) {
		for (i = grad->nstops-1; i > idx; i--)
			grad->stops[i] = grad->stops[i-1];
	}

	stop = &grad->stops[idx];
	stop->color = curAttr->stopColor;
	stop->color |= (unsigned int)(curAttr->stopOpacity*255) << 24;
	stop->offset = curAttr->stopOffset;
}

static void nsvg__startElement(void* ud, const char* el, const char** attr)
{
	NSVGparser* p = (NSVGparser*)ud;

	if (p->defsFlag) {
		// Skip everything but gradients in defs
		if (strcmp(el, "linearGradient") == 0) {
			nsvg__parseGradient(p, attr, NSVG_PAINT_LINEAR_GRADIENT);
		} else if (strcmp(el, "radialGradient") == 0) {
			nsvg__parseGradient(p, attr, NSVG_PAINT_RADIAL_GRADIENT);
		} else if (strcmp(el, "stop") == 0) {
			nsvg__parseGradientStop(p, attr);
		}
		return;
	}

	if (strcmp(el, "g") == 0) {
		nsvg__pushAttr(p);
		nsvg__parseAttribs(p, attr);
	} else if (strcmp(el, "path") == 0) {
		if (p->pathFlag)	// Do not allow nested paths.
			return;
		nsvg__pushAttr(p);
		nsvg__parsePath(p, attr);
		nsvg__popAttr(p);
	} else if (strcmp(el, "rect") == 0) {
		nsvg__pushAttr(p);
		nsvg__parseRect(p, attr);
		nsvg__popAttr(p);
	} else if (strcmp(el, "circle") == 0) {
		nsvg__pushAttr(p);
		nsvg__parseCircle(p, attr);
		nsvg__popAttr(p);
	} else if (strcmp(el, "ellipse") == 0) {
		nsvg__pushAttr(p);
		nsvg__parseEllipse(p, attr);
		nsvg__popAttr(p);
	} else if (strcmp(el, "line") == 0)  {
		nsvg__pushAttr(p);
		nsvg__parseLine(p, attr);
		nsvg__popAttr(p);
	} else if (strcmp(el, "polyline") == 0)  {
		nsvg__pushAttr(p);
		nsvg__parsePoly(p, attr, 0);
		nsvg__popAttr(p);
	} else if (strcmp(el, "polygon") == 0)  {
		nsvg__pushAttr(p);
		nsvg__parsePoly(p, attr, 1);
		nsvg__popAttr(p);
	} else  if (strcmp(el, "linearGradient") == 0) {
		nsvg__parseGradient(p, attr, NSVG_PAINT_LINEAR_GRADIENT);
	} else if (strcmp(el, "radialGradient") == 0) {
		nsvg__parseGradient(p, attr, NSVG_PAINT_RADIAL_GRADIENT);
	} else if (strcmp(el, "stop") == 0) {
		nsvg__parseGradientStop(p, attr);
	} else if (strcmp(el, "defs") == 0) {
		p->defsFlag = 1;
	} else if (strcmp(el, "svg") == 0) {
		nsvg__parseSVG(p, attr);
	}
}

static void nsvg__endElement(void* ud, const char* el)
{
	NSVGparser* p = (NSVGparser*)ud;

	if (strcmp(el, "g") == 0) {
		nsvg__popAttr(p);
	} else if (strcmp(el, "path") == 0) {
		p->pathFlag = 0;
	} else if (strcmp(el, "defs") == 0) {
		p->defsFlag = 0;
	}
}

static void nsvg__content(void* ud, const char* s)
{
	NSVG_NOTUSED(ud);
	NSVG_NOTUSED(s);
	// empty
}

static void nsvg__imageBounds(NSVGparser* p, float* bounds)
{
	NSVGshape* shape;
	shape = p->image->shapes;
	if (shape == NULL) return;
	bounds[0] = shape->bounds[0];
	bounds[1] = shape->bounds[1];
	bounds[2] = shape->bounds[2];
	bounds[3] = shape->bounds[3];
	for (shape = shape->next; shape != NULL; shape = shape->next) {
		bounds[0] = nsvg__minf(bounds[0], shape->bounds[0]);
		bounds[1] = nsvg__minf(bounds[1], shape->bounds[1]);
		bounds[2] = nsvg__maxf(bounds[2], shape->bounds[2]);
		bounds[3] = nsvg__maxf(bounds[3], shape->bounds[3]);
	}
}

static float nsvg__viewAlign(float content, float container, int type)
{
	if (type == NSVG_ALIGN_MIN)
		return 0;
	else if (type == NSVG_ALIGN_MAX)
		return container - content;
	// mid
	return (container - content) * 0.5f;
}

static void nsvg__scaleGradient(NSVGgradient* grad, float tx, float ty, float sx, float sy)
{
	grad->xform[0] *= sx;
	grad->xform[1] *= sx;
	grad->xform[2] *= sy;
	grad->xform[3] *= sy;
	grad->xform[4] += tx*sx;
	grad->xform[5] += ty*sx;
}

static void nsvg__scaleToViewbox(NSVGparser* p, const char* units)
{
	NSVGshape* shape;
	NSVGpath* path;
	float tx, ty, sx, sy, us, bounds[4], t[6];
	int i;
	float* pt;

	// Guess image size if not set completely.
	nsvg__imageBounds(p, bounds);
	if (p->viewWidth == 0) {
		if (p->image->width > 0)
			p->viewWidth = p->image->width;
		else
			p->viewWidth = bounds[2];
	}
	if (p->viewHeight == 0) {
		if (p->image->height > 0)
			p->viewHeight = p->image->height;
		else
			p->viewHeight = bounds[3];
	}
	if (p->image->width == 0)
		p->image->width = p->viewWidth;
	if (p->image->height == 0)
		p->image->height = p->viewHeight;

	tx = -p->viewMinx;
	ty = -p->viewMiny;
	sx = p->viewWidth > 0 ? p->image->width / p->viewWidth : 0;
	sy = p->viewHeight > 0 ? p->image->height / p->viewHeight : 0;
	us = 1.0f / nsvg__convertToPixels(p, 1.0f, units, 0);

	// Fix aspect ratio
	if (p->alignType == NSVG_ALIGN_MEET) {
		// fit whole image into viewbox
		sx = sy = nsvg__minf(sx, sy);
		tx += nsvg__viewAlign(p->viewWidth*sx, p->image->width, p->alignX) / sx;
		ty += nsvg__viewAlign(p->viewHeight*sy, p->image->height, p->alignY) / sy;
	} else if (p->alignType == NSVG_ALIGN_SLICE) {
		// fill whole viewbox with image
		sx = sy = nsvg__maxf(sx, sy);
		tx += nsvg__viewAlign(p->viewWidth*sx, p->image->width, p->alignX) / sx;
		ty += nsvg__viewAlign(p->viewHeight*sy, p->image->height, p->alignY) / sy;
	}

	// Transform
	sx *= us;
	sy *= us;
	for (shape = p->image->shapes; shape != NULL; shape = shape->next) {
		shape->bounds[0] = (shape->bounds[0] + tx) * sx;
		shape->bounds[1] = (shape->bounds[1] + ty) * sy;
		shape->bounds[2] = (shape->bounds[2] + tx) * sx;
		shape->bounds[3] = (shape->bounds[3] + ty) * sy;
		for (path = shape->paths; path != NULL; path = path->next) {
			path->bounds[0] = (path->bounds[0] + tx) * sx;
			path->bounds[1] = (path->bounds[1] + ty) * sy;
			path->bounds[2] = (path->bounds[2] + tx) * sx;
			path->bounds[3] = (path->bounds[3] + ty) * sy;
			for (i =0; i < path->npts; i++) {
				pt = &path->pts[i*2];
				pt[0] = (pt[0] + tx) * sx;
				pt[1] = (pt[1] + ty) * sy;
			}
		}

		if (shape->fill.type == NSVG_PAINT_LINEAR_GRADIENT || shape->fill.type == NSVG_PAINT_RADIAL_GRADIENT) {
			nsvg__scaleGradient(shape->fill.gradient, tx,ty, sx,sy);
			memcpy(t, shape->fill.gradient->xform, sizeof(float)*6);
			nsvg__xformInverse(shape->fill.gradient->xform, t);
		}
		if (shape->stroke.type == NSVG_PAINT_LINEAR_GRADIENT || shape->stroke.type == NSVG_PAINT_RADIAL_GRADIENT) {
			nsvg__scaleGradient(shape->stroke.gradient, tx,ty, sx,sy);
			memcpy(t, shape->stroke.gradient->xform, sizeof(float)*6);
			nsvg__xformInverse(shape->stroke.gradient->xform, t);
		}

	}

	sx *= us;
	sy *= us;
}

NSVGimage* nsvgParse(char* input, const char* units, float dpi)
{
	NSVGparser* p;
	NSVGimage* ret = 0;

	p = nsvg__createParser();
	if (p == NULL) {
		return NULL;
	}
	p->dpi = dpi;

	nsvg__parseXML(input, nsvg__startElement, nsvg__endElement, nsvg__content, p);

	// Scale to viewBox
	nsvg__scaleToViewbox(p, units);

	ret = p->image;
	p->image = NULL;

	nsvg__deleteParser(p);

	return ret;
}

NSVGimage* nsvgParseFromFile(const char* filename, const char* units, float dpi)
{
	FILE* fp = NULL;
	size_t size;
	char* data = NULL;
	NSVGimage* image = NULL;

	fp = fopen(filename, "rb");
	if (!fp) goto error;
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	data = (char*)malloc(size+1);
	if (data == NULL) goto error;
	size = fread(data, size, 1, fp);
	data[size] = '\0';	// Must be null terminated.
	fclose(fp);
	image = nsvgParse(data, units, dpi);
	free(data);

	return image;

error:
	if (fp) fclose(fp);
	if (data) free(data);
	if (image) nsvgDelete(image);
	return NULL;
}

void nsvgDelete(NSVGimage* image)
{
	NSVGshape *snext, *shape;
	if (image == NULL) return;
	shape = image->shapes;
	while (shape != NULL) {
		snext = shape->next;
		nsvg__deletePaths(shape->paths);
		nsvg__deletePaint(&shape->fill);
		nsvg__deletePaint(&shape->stroke);
		free(shape);
		shape = snext;
	}
	free(image);
}

#endif



//#line 1 "nanosvgrast.h"
#ifndef NANOSVGRAST_H
#define NANOSVGRAST_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NSVGrasterizer NSVGrasterizer;

/* Example Usage:
	// Load SVG
	struct SNVGImage* image = nsvgParseFromFile("test.svg.");

	// Create rasterizer (can be used to render multiple images).
	struct NSVGrasterizer* rast = nsvgCreateRasterizer();
	// Allocate memory for image
	unsigned char* img = malloc(w*h*4);
	// Rasterize
	nsvgRasterize(rast, image, 0,0,1, img, w, h, w*4);
*/

// Allocated rasterizer context.
NSVGrasterizer* nsvgCreateRasterizer();

// Rasterizes SVG image, returns RGBA image (non-premultiplied alpha)
//   r - pointer to rasterizer context
//   image - pointer to image to rasterize
//   tx,ty - image offset (applied after scaling)
//   scale - image scale
//   dst - pointer to destination image data, 4 bytes per pixel (RGBA)
//   w - width of the image to render
//   h - height of the image to render
//   stride - number of bytes per scaleline in the destination buffer
void nsvgRasterize(NSVGrasterizer* r,
				   NSVGimage* image, float tx, float ty, float scale,
				   unsigned char* dst, int w, int h, int stride);

// Deletes rasterizer context.
void nsvgDeleteRasterizer(NSVGrasterizer*);

#ifdef __cplusplus
};
#endif

#endif // NANOSVGRAST_H

#ifdef NANOSVGRAST_IMPLEMENTATION

#include <math.h>

#define NSVG__SUBSAMPLES	5
#define NSVG__FIXSHIFT		10
#define NSVG__FIX			(1 << NSVG__FIXSHIFT)
#define NSVG__FIXMASK		(NSVG__FIX-1)
#define NSVG__MEMPAGE_SIZE	1024

typedef struct NSVGedge {
	float x0,y0, x1,y1;
	int dir;
	struct NSVGedge* next;
} NSVGedge;

typedef struct NSVGpoint {
	float x, y;
	float dx, dy;
	float len;
	float dmx, dmy;
	unsigned char flags;
} NSVGpoint;

typedef struct NSVGactiveEdge {
	int x,dx;
	float ey;
	int dir;
	struct NSVGactiveEdge *next;
} NSVGactiveEdge;

typedef struct NSVGmemPage {
	unsigned char mem[NSVG__MEMPAGE_SIZE];
	int size;
	struct NSVGmemPage* next;
} NSVGmemPage;

typedef struct NSVGcachedPaint {
	char type;
	char spread;
	float xform[6];
	unsigned int colors[256];
} NSVGcachedPaint;

struct NSVGrasterizer
{
	float px, py;

	float tessTol;
	float distTol;

	NSVGedge* edges;
	int nedges;
	int cedges;

	NSVGpoint* points;
	int npoints;
	int cpoints;

	NSVGactiveEdge* freelist;
	NSVGmemPage* pages;
	NSVGmemPage* curpage;

	unsigned char* scanline;
	int cscanline;

	unsigned char* bitmap;
	int width, height, stride;
};

NSVGrasterizer* nsvgCreateRasterizer()
{
	NSVGrasterizer* r = (NSVGrasterizer*)malloc(sizeof(NSVGrasterizer));
	if (r == NULL) goto error;
	memset(r, 0, sizeof(NSVGrasterizer));

	r->tessTol = 0.25f;
	r->distTol = 0.01f;

	return r;

error:
	nsvgDeleteRasterizer(r);
	return NULL;
}

void nsvgDeleteRasterizer(NSVGrasterizer* r)
{
	NSVGmemPage* p;

	if (r == NULL) return;

	p = r->pages;
	while (p != NULL) {
		NSVGmemPage* next = p->next;
		free(p);
		p = next;
	}

	if (r->edges) free(r->edges);
	if (r->points) free(r->points);
	if (r->scanline) free(r->scanline);

	free(r);
}

static NSVGmemPage* nsvg__nextPage(NSVGrasterizer* r, NSVGmemPage* cur)
{
	NSVGmemPage *newp;

	// If using existing chain, return the next page in chain
	if (cur != NULL && cur->next != NULL) {
		return cur->next;
	}

	// Alloc new page
	newp = (NSVGmemPage*)malloc(sizeof(NSVGmemPage));
	if (newp == NULL) return NULL;
	memset(newp, 0, sizeof(NSVGmemPage));

	// Add to linked list
	if (cur != NULL)
		cur->next = newp;
	else
		r->pages = newp;

	return newp;
}

static void nsvg__resetPool(NSVGrasterizer* r)
{
	NSVGmemPage* p = r->pages;
	while (p != NULL) {
		p->size = 0;
		p = p->next;
	}
	r->curpage = r->pages;
}

static unsigned char* nsvg__alloc(NSVGrasterizer* r, int size)
{
	unsigned char* buf;
	if (size > NSVG__MEMPAGE_SIZE) return NULL;
	if (r->curpage == NULL || r->curpage->size+size > NSVG__MEMPAGE_SIZE) {
		r->curpage = nsvg__nextPage(r, r->curpage);
	}
	buf = &r->curpage->mem[r->curpage->size];
	r->curpage->size += size;
	return buf;
}

static int nsvg__ptEquals(float x1, float y1, float x2, float y2, float tol)
{
	float dx = x2 - x1;
	float dy = y2 - y1;
	return dx*dx + dy*dy < tol*tol;
}

static void nsvg__addPathPoint(NSVGrasterizer* r, float x, float y, int flags)
{
	NSVGpoint* pt;

	if (r->npoints > 0) {
		pt = &r->points[r->npoints-1];
		if (nsvg__ptEquals(pt->x,pt->y, x,y, r->distTol)) {
			pt->flags |= flags;
			return;
		}
	}

	if (r->npoints+1 > r->cpoints) {
		r->cpoints = r->cpoints > 0 ? r->cpoints * 2 : 64;
		r->points = (NSVGpoint*)realloc(r->points, sizeof(NSVGpoint) * r->cpoints);
		if (r->points == NULL) return;
	}

	pt = &r->points[r->npoints];
	pt->x = x;
	pt->y = y;
	pt->flags = flags;
	r->npoints++;
}

static void nsvg__addEdge(NSVGrasterizer* r, float x0, float y0, float x1, float y1)
{
	NSVGedge* e;

	// Skip horizontal edges
	if (y0 == y1)
		return;

	if (r->nedges+1 > r->cedges) {
		r->cedges = r->cedges > 0 ? r->cedges * 2 : 64;
		r->edges = (NSVGedge*)realloc(r->edges, sizeof(NSVGedge) * r->cedges);
		if (r->edges == NULL) return;
	}

	e = &r->edges[r->nedges];
	r->nedges++;

	if (y0 < y1) {
		e->x0 = x0;
		e->y0 = y0;
		e->x1 = x1;
		e->y1 = y1;
		e->dir = 1;
	} else {
		e->x0 = x1;
		e->y0 = y1;
		e->x1 = x0;
		e->y1 = y0;
		e->dir = -1;
	}
}

static float nsvg__normalize(float *x, float* y)
{
	float d = sqrtf((*x)*(*x) + (*y)*(*y));
	if (d > 1e-6f) {
		float id = 1.0f / d;
		*x *= id;
		*y *= id;
	}
	return d;
}

static float nsvg__absf(float x) { return x < 0 ? -x : x; }

static void nsvg__flattenCubicBez(NSVGrasterizer* r,
								  float x1, float y1, float x2, float y2,
								  float x3, float y3, float x4, float y4,
								  int level, int type)
{
	float x12,y12,x23,y23,x34,y34,x123,y123,x234,y234,x1234,y1234;
	float dx,dy,d2,d3;

	if (level > 10) return;

	x12 = (x1+x2)*0.5f;
	y12 = (y1+y2)*0.5f;
	x23 = (x2+x3)*0.5f;
	y23 = (y2+y3)*0.5f;
	x34 = (x3+x4)*0.5f;
	y34 = (y3+y4)*0.5f;
	x123 = (x12+x23)*0.5f;
	y123 = (y12+y23)*0.5f;

	dx = x4 - x1;
	dy = y4 - y1;
	d2 = nsvg__absf(((x2 - x4) * dy - (y2 - y4) * dx));
	d3 = nsvg__absf(((x3 - x4) * dy - (y3 - y4) * dx));

	if ((d2 + d3)*(d2 + d3) < r->tessTol * (dx*dx + dy*dy)) {
		nsvg__addPathPoint(r, x4, y4, type);
		return;
	}

	x234 = (x23+x34)*0.5f;
	y234 = (y23+y34)*0.5f;
	x1234 = (x123+x234)*0.5f;
	y1234 = (y123+y234)*0.5f;

	nsvg__flattenCubicBez(r, x1,y1, x12,y12, x123,y123, x1234,y1234, level+1, 0);
	nsvg__flattenCubicBez(r, x1234,y1234, x234,y234, x34,y34, x4,y4, level+1, type);
}

static void nsvg__flattenShape(NSVGrasterizer* r, NSVGshape* shape, float scale)
{
	int i, j;
	NSVGpath* path;

	for (path = shape->paths; path != NULL; path = path->next) {
		r->npoints = 0;
		// Flatten path
		nsvg__addPathPoint(r, path->pts[0]*scale, path->pts[1]*scale, 0);
		for (i = 0; i < path->npts-1; i += 3) {
			float* p = &path->pts[i*2];
			nsvg__flattenCubicBez(r, p[0]*scale,p[1]*scale, p[2]*scale,p[3]*scale, p[4]*scale,p[5]*scale, p[6]*scale,p[7]*scale, 0, 0);
		}
		// Close path
		nsvg__addPathPoint(r, path->pts[0]*scale, path->pts[1]*scale, 0);
		// Build edges
		for (i = 0, j = r->npoints-1; i < r->npoints; j = i++)
			nsvg__addEdge(r, r->points[j].x, r->points[j].y, r->points[i].x, r->points[i].y);
	}
}

enum NSVGpointFlags
{
	NSVG_PT_CORNER = 0x01,
	NSVG_PT_BEVEL = 0x02,
	NSVG_PT_LEFT = 0x04,
};

static void nsvg__initClosed(NSVGpoint* left, NSVGpoint* right, NSVGpoint* p0, NSVGpoint* p1, float lineWidth)
{
	float w = lineWidth * 0.5f;
	float dx = p1->x - p0->x;
	float dy = p1->y - p0->y;
	float len = nsvg__normalize(&dx, &dy);
	float px = p0->x + dx*len*0.5f, py = p0->y + dy*len*0.5f;
	float dlx = dy, dly = -dx;
	float lx = px - dlx*w, ly = py - dly*w;
	float rx = px + dlx*w, ry = py + dly*w;
	left->x = lx; left->y = ly;
	right->x = rx; right->y = ry;
}

static void nsvg__buttCap(NSVGrasterizer* r, NSVGpoint* left, NSVGpoint* right, NSVGpoint* p, float dx, float dy, float lineWidth, int connect)
{
	float w = lineWidth * 0.5f;
	float px = p->x, py = p->y;
	float dlx = dy, dly = -dx;
	float lx = px - dlx*w, ly = py - dly*w;
	float rx = px + dlx*w, ry = py + dly*w;

	nsvg__addEdge(r, lx, ly, rx, ry);

	if (connect) {
		nsvg__addEdge(r, left->x, left->y, lx, ly);
		nsvg__addEdge(r, rx, ry, right->x, right->y);
	}
	left->x = lx; left->y = ly;
	right->x = rx; right->y = ry;
}

static void nsvg__squareCap(NSVGrasterizer* r, NSVGpoint* left, NSVGpoint* right, NSVGpoint* p, float dx, float dy, float lineWidth, int connect)
{
	float w = lineWidth * 0.5f;
	float px = p->x - dx*w, py = p->y - dy*w;
	float dlx = dy, dly = -dx;
	float lx = px - dlx*w, ly = py - dly*w;
	float rx = px + dlx*w, ry = py + dly*w;

	nsvg__addEdge(r, lx, ly, rx, ry);

	if (connect) {
		nsvg__addEdge(r, left->x, left->y, lx, ly);
		nsvg__addEdge(r, rx, ry, right->x, right->y);
	}
	left->x = lx; left->y = ly;
	right->x = rx; right->y = ry;
}

#ifndef NSVG_PI
#define NSVG_PI (3.14159265358979323846264338327f)
#endif

static void nsvg__roundCap(NSVGrasterizer* r, NSVGpoint* left, NSVGpoint* right, NSVGpoint* p, float dx, float dy, float lineWidth, int ncap, int connect)
{
	int i;
	float w = lineWidth * 0.5f;
	float px = p->x, py = p->y;
	float dlx = dy, dly = -dx;
	float lx, ly, rx, ry, prevx, prevy;

	for (i = 0; i < ncap; i++) {
		float a = i/(float)(ncap-1)*NSVG_PI;
		float ax = cosf(a) * w, ay = sinf(a) * w;
		float x = px - dlx*ax - dx*ay;
		float y = py - dly*ax - dy*ay;

		if (i > 0)
			nsvg__addEdge(r, prevx, prevy, x, y);

		prevx = x;
		prevy = y;

		if (i == 0) {
			lx = x; ly = y;
		} else if (i == ncap-1) {
			rx = x; ry = y;
		}
	}

	if (connect) {
		nsvg__addEdge(r, left->x, left->y, lx, ly);
		nsvg__addEdge(r, rx, ry, right->x, right->y);
	}

	left->x = lx; left->y = ly;
	right->x = rx; right->y = ry;
}

static void nsvg__bevelJoin(NSVGrasterizer* r, NSVGpoint* left, NSVGpoint* right, NSVGpoint* p0, NSVGpoint* p1, float lineWidth)
{
	float w = lineWidth * 0.5f;
	float dlx0 = p0->dy, dly0 = -p0->dx;
	float dlx1 = p1->dy, dly1 = -p1->dx;
	float lx0 = p1->x - (dlx0 * w), ly0 = p1->y - (dly0 * w);
	float rx0 = p1->x + (dlx0 * w), ry0 = p1->y + (dly0 * w);
	float lx1 = p1->x - (dlx1 * w), ly1 = p1->y - (dly1 * w);
	float rx1 = p1->x + (dlx1 * w), ry1 = p1->y + (dly1 * w);

	nsvg__addEdge(r, lx0, ly0, left->x, left->y);
	nsvg__addEdge(r, lx1, ly1, lx0, ly0);

	nsvg__addEdge(r, right->x, right->y, rx0, ry0);
	nsvg__addEdge(r, rx0, ry0, rx1, ry1);

	left->x = lx1; left->y = ly1;
	right->x = rx1; right->y = ry1;
}

static void nsvg__miterJoin(NSVGrasterizer* r, NSVGpoint* left, NSVGpoint* right, NSVGpoint* p0, NSVGpoint* p1, float lineWidth)
{
	float w = lineWidth * 0.5f;
	float dlx0 = p0->dy, dly0 = -p0->dx;
	float dlx1 = p1->dy, dly1 = -p1->dx;
	float lx0, rx0, lx1, rx1;
	float ly0, ry0, ly1, ry1;

	if (p1->flags & NSVG_PT_LEFT) {
		lx0 = lx1 = p1->x - p1->dmx * w;
		ly0 = ly1 = p1->y - p1->dmy * w;
		nsvg__addEdge(r, lx1, ly1, left->x, left->y);

		rx0 = p1->x + (dlx0 * w);
		ry0 = p1->y + (dly0 * w);
		rx1 = p1->x + (dlx1 * w);
		ry1 = p1->y + (dly1 * w);
		nsvg__addEdge(r, right->x, right->y, rx0, ry0);
		nsvg__addEdge(r, rx0, ry0, rx1, ry1);
	} else {
		lx0 = p1->x - (dlx0 * w);
		ly0 = p1->y - (dly0 * w);
		lx1 = p1->x - (dlx1 * w);
		ly1 = p1->y - (dly1 * w);
		nsvg__addEdge(r, lx0, ly0, left->x, left->y);
		nsvg__addEdge(r, lx1, ly1, lx0, ly0);

		rx0 = rx1 = p1->x + p1->dmx * w;
		ry0 = ry1 = p1->y + p1->dmy * w;
		nsvg__addEdge(r, right->x, right->y, rx1, ry1);
	}

	left->x = lx1; left->y = ly1;
	right->x = rx1; right->y = ry1;
}

static void nsvg__roundJoin(NSVGrasterizer* r, NSVGpoint* left, NSVGpoint* right, NSVGpoint* p0, NSVGpoint* p1, float lineWidth, int ncap)
{
	int i, n;
	float w = lineWidth * 0.5f;
	float dlx0 = p0->dy, dly0 = -p0->dx;
	float dlx1 = p1->dy, dly1 = -p1->dx;
	float a0 = atan2f(dly0, dlx0);
	float a1 = atan2f(dly1, dlx1);
	float da = a1 - a0;
	float lx, ly, rx, ry;

	if (da < NSVG_PI) da += NSVG_PI*2;
	if (da > NSVG_PI) da -= NSVG_PI*2;

	n = (int)ceilf((nsvg__absf(da) / NSVG_PI) * ncap);
	if (n < 2) n = 2;
	if (n > ncap) n = ncap;

	lx = left->x;
	ly = left->y;
	rx = right->x;
	ry = right->y;

	for (i = 0; i < n; i++) {
		float u = i/(float)(n-1);
		float a = a0 + u*da;
		float ax = cosf(a) * w, ay = sinf(a) * w;
		float lx1 = p1->x - ax, ly1 = p1->y - ay;
		float rx1 = p1->x + ax, ry1 = p1->y + ay;

		nsvg__addEdge(r, lx1, ly1, lx, ly);
		nsvg__addEdge(r, rx, ry, rx1, ry1);

		lx = lx1; ly = ly1;
		rx = rx1; ry = ry1;
	}

	left->x = lx; left->y = ly;
	right->x = rx; right->y = ry;
}

static void nsvg__straightJoin(NSVGrasterizer* r, NSVGpoint* left, NSVGpoint* right, NSVGpoint* p1, float lineWidth)
{
	float w = lineWidth * 0.5f;
	float lx = p1->x - (p1->dmx * w), ly = p1->y - (p1->dmy * w);
	float rx = p1->x + (p1->dmx * w), ry = p1->y + (p1->dmy * w);

	nsvg__addEdge(r, lx, ly, left->x, left->y);
	nsvg__addEdge(r, right->x, right->y, rx, ry);

	left->x = lx; left->y = ly;
	right->x = rx; right->y = ry;
}

static int nsvg__curveDivs(float r, float arc, float tol)
{
	float da = acosf(r / (r + tol)) * 2.0f;
	int divs = (int)ceilf(arc / da);
	if (divs < 2) divs = 2;
	return divs;
}

static void nsvg__flattenShapeStroke(NSVGrasterizer* r, NSVGshape* shape, float scale)
{
	int i, j, closed;
	int s, e;
	NSVGpath* path;
	NSVGpoint* p0, *p1;
	float miterLimit = 4;
	int lineJoin = shape->strokeLineJoin;
	int lineCap = shape->strokeLineCap;
	float lineWidth = shape->strokeWidth * scale;
	int ncap = nsvg__curveDivs(lineWidth*0.5f, NSVG_PI, r->tessTol);	// Calculate divisions per half circle.
	NSVGpoint left, right, firstLeft, firstRight;

	for (path = shape->paths; path != NULL; path = path->next) {
		r->npoints = 0;
		// Flatten path
		nsvg__addPathPoint(r, path->pts[0]*scale, path->pts[1]*scale, NSVG_PT_CORNER);
		for (i = 0; i < path->npts-1; i += 3) {
			float* p = &path->pts[i*2];
			nsvg__flattenCubicBez(r, p[0]*scale,p[1]*scale, p[2]*scale,p[3]*scale, p[4]*scale,p[5]*scale, p[6]*scale,p[7]*scale, 0, NSVG_PT_CORNER);
		}
		if (r->npoints < 2)
			continue;

		closed = path->closed;

		// If the first and last points are the same, remove the last, mark as closed path.
		p0 = &r->points[r->npoints-1];
		p1 = &r->points[0];
		if (nsvg__ptEquals(p0->x,p0->y, p1->x,p1->y, r->distTol)) {
			r->npoints--;
			p0 = &r->points[r->npoints-1];
			closed = 1;
		}

		for (i = 0; i < r->npoints; i++) {
			// Calculate segment direction and length
			p0->dx = p1->x - p0->x;
			p0->dy = p1->y - p0->y;
			p0->len = nsvg__normalize(&p0->dx, &p0->dy);
			// Advance
			p0 = p1++;
		}

		// calculate joins
		p0 = &r->points[r->npoints-1];
		p1 = &r->points[0];
		for (j = 0; j < r->npoints; j++) {
			float dlx0, dly0, dlx1, dly1, dmr2, cross;
			dlx0 = p0->dy;
			dly0 = -p0->dx;
			dlx1 = p1->dy;
			dly1 = -p1->dx;
			// Calculate extrusions
			p1->dmx = (dlx0 + dlx1) * 0.5f;
			p1->dmy = (dly0 + dly1) * 0.5f;
			dmr2 = p1->dmx*p1->dmx + p1->dmy*p1->dmy;
			if (dmr2 > 0.000001f) {
				float s = 1.0f / dmr2;
				if (s > 600.0f) {
					s = 600.0f;
				}
				p1->dmx *= s;
				p1->dmy *= s;
			}

			// Clear flags, but keep the corner.
			p1->flags = (p1->flags & NSVG_PT_CORNER) ? NSVG_PT_CORNER : 0;

			// Keep track of left turns.
			cross = p1->dx * p0->dy - p0->dx * p1->dy;
			if (cross > 0.0f)
				p1->flags |= NSVG_PT_LEFT;

			// Check to see if the corner needs to be beveled.
			if (p1->flags & NSVG_PT_CORNER) {
				if ((dmr2 * miterLimit*miterLimit) < 1.0f || lineJoin == NSVG_JOIN_BEVEL || lineJoin == NSVG_JOIN_ROUND) {
					p1->flags |= NSVG_PT_BEVEL;
				}
			}

			p0 = p1++;
		}

		// Build stroke edges
		if (closed) {
			// Looping
			p0 = &r->points[r->npoints-1];
			p1 = &r->points[0];
			s = 0;
			e = r->npoints;
		} else {
			// Add cap
			p0 = &r->points[0];
			p1 = &r->points[1];
			s = 1;
			e = r->npoints-1;
		}

		if (closed) {
			nsvg__initClosed(&left, &right, p0, p1, lineWidth);
			firstLeft = left;
			firstRight = right;
		} else {
			// Add cap
			float dx = p1->x - p0->x;
			float dy = p1->y - p0->y;
			nsvg__normalize(&dx, &dy);
			if (lineCap == NSVG_CAP_BUTT)
				nsvg__buttCap(r, &left, &right, p0, dx, dy, lineWidth, 0);
			else if (lineCap == NSVG_CAP_SQUARE)
				nsvg__squareCap(r, &left, &right, p0, dx, dy, lineWidth, 0);
			else if (lineCap == NSVG_CAP_ROUND)
				nsvg__roundCap(r, &left, &right, p0, dx, dy, lineWidth, ncap, 0);
		}

		for (j = s; j < e; ++j) {
//			if (p1->flags & NSVG_PT_BEVEL) {
			if (p1->flags & NSVG_PT_CORNER) {
				if (lineJoin == NSVG_JOIN_ROUND)
					nsvg__roundJoin(r, &left, &right, p0, p1, lineWidth, ncap);
				else if (lineJoin == NSVG_JOIN_BEVEL || (p1->flags & NSVG_PT_BEVEL))
					nsvg__bevelJoin(r, &left, &right, p0, p1, lineWidth);
				else
					nsvg__miterJoin(r, &left, &right, p0, p1, lineWidth);
			} else {
				nsvg__straightJoin(r, &left, &right, p1, lineWidth);
			}
			p0 = p1++;
		}

		if (closed) {
			// Loop it
			nsvg__addEdge(r, firstLeft.x, firstLeft.y, left.x, left.y);
			nsvg__addEdge(r, right.x, right.y, firstRight.x, firstRight.y);
		} else {
			// Add cap
			float dx = p1->x - p0->x;
			float dy = p1->y - p0->y;
			nsvg__normalize(&dx, &dy);
			if (lineCap == NSVG_CAP_BUTT)
				nsvg__buttCap(r, &right, &left, p1, -dx, -dy, lineWidth, 1);
			else if (lineCap == NSVG_CAP_SQUARE)
				nsvg__squareCap(r, &right, &left, p1, -dx, -dy, lineWidth, 1);
			else if (lineCap == NSVG_CAP_ROUND)
				nsvg__roundCap(r, &right, &left, p1, -dx, -dy, lineWidth, ncap, 1);
		}
	}
}

static int nsvg__cmpEdge(const void *p, const void *q)
{
	NSVGedge* a = (NSVGedge*)p;
	NSVGedge* b = (NSVGedge*)q;

	if (a->y0 < b->y0) return -1;
	if (a->y0 > b->y0) return  1;
	return 0;
}

static NSVGactiveEdge* nsvg__addActive(NSVGrasterizer* r, NSVGedge* e, float startPoint)
{
	 NSVGactiveEdge* z;

	if (r->freelist != NULL) {
		// Restore from freelist.
		z = r->freelist;
		r->freelist = z->next;
	} else {
		// Alloc new edge.
		z = (NSVGactiveEdge*)nsvg__alloc(r, sizeof(NSVGactiveEdge));
		if (z == NULL) return NULL;
	}

	float dxdy = (e->x1 - e->x0) / (e->y1 - e->y0);
//	STBTT_assert(e->y0 <= start_point);
	// round dx down to avoid going too far
	if (dxdy < 0)
		z->dx = -floorf(NSVG__FIX * -dxdy);
	else
		z->dx = floorf(NSVG__FIX * dxdy);
	z->x = floorf(NSVG__FIX * (e->x0 + dxdy * (startPoint - e->y0)));
//	z->x -= off_x * FIX;
	z->ey = e->y1;
	z->next = 0;
	z->dir = e->dir;

	return z;
}

static void nsvg__freeActive(NSVGrasterizer* r, NSVGactiveEdge* z)
{
	z->next = r->freelist;
	r->freelist = z;
}

// note: this routine clips fills that extend off the edges... ideally this
// wouldn't happen, but it could happen if the truetype glyph bounding boxes
// are wrong, or if the user supplies a too-small bitmap
static void nsvg__fillActiveEdges(unsigned char* scanline, int len, NSVGactiveEdge* e, int maxWeight, int* xmin, int* xmax)
{
	// non-zero winding fill
	int x0 = 0, w = 0;

	while (e != NULL) {
		if (w == 0) {
			// if we're currently at zero, we need to record the edge start point
			x0 = e->x; w += e->dir;
		} else {
			int x1 = e->x; w += e->dir;
			// if we went to zero, we need to draw
			if (w == 0) {
				int i = x0 >> NSVG__FIXSHIFT;
				int j = x1 >> NSVG__FIXSHIFT;
				if (i < *xmin) *xmin = i;
				if (j > *xmax) *xmax = j;
				if (i < len && j >= 0) {
					if (i == j) {
						// x0,x1 are the same pixel, so compute combined coverage
						scanline[i] += (unsigned char)((x1 - x0) * maxWeight >> NSVG__FIXSHIFT);
					} else {
						if (i >= 0) // add antialiasing for x0
							scanline[i] += (unsigned char)(((NSVG__FIX - (x0 & NSVG__FIXMASK)) * maxWeight) >> NSVG__FIXSHIFT);
						else
							i = -1; // clip

						if (j < len) // add antialiasing for x1
							scanline[j] += (unsigned char)(((x1 & NSVG__FIXMASK) * maxWeight) >> NSVG__FIXSHIFT);
						else
							j = len; // clip

						for (++i; i < j; ++i) // fill pixels between x0 and x1
							scanline[i] += (unsigned char)maxWeight;
					}
				}
			}
		}
		e = e->next;
	}
}

static float nsvg__clampf(float a, float mn, float mx) { return a < mn ? mn : (a > mx ? mx : a); }

static unsigned int nsvg__RGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	return (r) | (g << 8) | (b << 16) | (a << 24);
}

static unsigned int nsvg__lerpRGBA(unsigned int c0, unsigned int c1, float u)
{
	int iu = (float)(nsvg__clampf(u, 0.0f, 1.0f) * 256.0f);
	int r = (((c0) & 0xff)*(256-iu) + (((c1) & 0xff)*iu)) >> 8;
	int g = (((c0>>8) & 0xff)*(256-iu) + (((c1>>8) & 0xff)*iu)) >> 8;
	int b = (((c0>>16) & 0xff)*(256-iu) + (((c1>>16) & 0xff)*iu)) >> 8;
	int a = (((c0>>24) & 0xff)*(256-iu) + (((c1>>24) & 0xff)*iu)) >> 8;
	return nsvg__RGBA(r,g,b,a);
}

static unsigned int nsvg__applyOpacity(unsigned int c, float u)
{
	int iu = (float)(nsvg__clampf(u, 0.0f, 1.0f) * 256.0f);
	int r = (c) & 0xff;
	int g = (c>>8) & 0xff;
	int b = (c>>16) & 0xff;
	int a = (((c>>24) & 0xff)*iu) >> 8;
	return nsvg__RGBA(r,g,b,a);
}

static void nsvg__scanlineSolid(unsigned char* dst, int count, unsigned char* cover, int x, int y,
								float tx, float ty, float scale, NSVGcachedPaint* cache)
{

	if (cache->type == NSVG_PAINT_COLOR) {
		int i, cr, cg, cb, ca;
		cr = cache->colors[0] & 0xff;
		cg = (cache->colors[0] >> 8) & 0xff;
		cb = (cache->colors[0] >> 16) & 0xff;
		ca = (cache->colors[0] >> 24) & 0xff;

		for (i = 0; i < count; i++) {
			int r,g,b;
			int a = ((int)cover[0] * ca) >> 8;
			int ia = 255 - a;
			// Premultiply
			r = (cr * a) >> 8;
			g = (cg * a) >> 8;
			b = (cb * a) >> 8;

			// Blend over
			r += ((ia * (int)dst[0]) >> 8);
			g += ((ia * (int)dst[1]) >> 8);
			b += ((ia * (int)dst[2]) >> 8);
			a += ((ia * (int)dst[3]) >> 8);

			dst[0] = (unsigned char)r;
			dst[1] = (unsigned char)g;
			dst[2] = (unsigned char)b;
			dst[3] = (unsigned char)a;

			cover++;
			dst += 4;
		}
	} else if (cache->type == NSVG_PAINT_LINEAR_GRADIENT) {
		// TODO: spread modes.
		// TODO: plenty of opportunities to optimize.
		float fx, fy, dx, gy;
		float* t = cache->xform;
		int i, cr, cg, cb, ca;
		unsigned int c;

		fx = (x - tx) / scale;
		fy = (y - ty) / scale;
		dx = 1.0f / scale;

		for (i = 0; i < count; i++) {
			int r,g,b,a,ia;
			gy = fx*t[1] + fy*t[3] + t[5];
			c = cache->colors[(int)nsvg__clampf(gy*255.0f, 0, 255.0f)];
			cr = (c) & 0xff;
			cg = (c >> 8) & 0xff;
			cb = (c >> 16) & 0xff;
			ca = (c >> 24) & 0xff;

			a = ((int)cover[0] * ca) >> 8;
			ia = 255 - a;

			// Premultiply
			r = (cr * a) >> 8;
			g = (cg * a) >> 8;
			b = (cb * a) >> 8;

			// Blend over
			r += ((ia * (int)dst[0]) >> 8);
			g += ((ia * (int)dst[1]) >> 8);
			b += ((ia * (int)dst[2]) >> 8);
			a += ((ia * (int)dst[3]) >> 8);

			dst[0] = (unsigned char)r;
			dst[1] = (unsigned char)g;
			dst[2] = (unsigned char)b;
			dst[3] = (unsigned char)a;

			cover++;
			dst += 4;
			fx += dx;
		}
	} else if (cache->type == NSVG_PAINT_RADIAL_GRADIENT) {
		// TODO: spread modes.
		// TODO: plenty of opportunities to optimize.
		// TODO: focus (fx,fy)
		float fx, fy, dx, gx, gy, gd;
		float* t = cache->xform;
		int i, cr, cg, cb, ca;
		unsigned int c;

		fx = (x - tx) / scale;
		fy = (y - ty) / scale;
		dx = 1.0f / scale;

		for (i = 0; i < count; i++) {
			int r,g,b,a,ia;
			gx = fx*t[0] + fy*t[2] + t[4];
			gy = fx*t[1] + fy*t[3] + t[5];
			gd = sqrtf(gx*gx + gy*gy);
			c = cache->colors[(int)nsvg__clampf(gd*255.0f, 0, 255.0f)];
			cr = (c) & 0xff;
			cg = (c >> 8) & 0xff;
			cb = (c >> 16) & 0xff;
			ca = (c >> 24) & 0xff;

			a = ((int)cover[0] * ca) >> 8;
			ia = 255 - a;

			// Premultiply
			r = (cr * a) >> 8;
			g = (cg * a) >> 8;
			b = (cb * a) >> 8;

			// Blend over
			r += ((ia * (int)dst[0]) >> 8);
			g += ((ia * (int)dst[1]) >> 8);
			b += ((ia * (int)dst[2]) >> 8);
			a += ((ia * (int)dst[3]) >> 8);

			dst[0] = (unsigned char)r;
			dst[1] = (unsigned char)g;
			dst[2] = (unsigned char)b;
			dst[3] = (unsigned char)a;

			cover++;
			dst += 4;
			fx += dx;
		}
	}
}

static void nsvg__rasterizeSortedEdges(NSVGrasterizer *r, float tx, float ty, float scale, NSVGcachedPaint* cache)
{
	NSVGactiveEdge *active = NULL;
	int y, s;
	int e = 0;
	int maxWeight = (255 / NSVG__SUBSAMPLES);  // weight per vertical scanline
	int xmin, xmax;

	for (y = 0; y < r->height; y++) {
		memset(r->scanline, 0, r->width);
		xmin = r->width;
		xmax = 0;
		for (s = 0; s < NSVG__SUBSAMPLES; ++s) {
			// find center of pixel for this scanline
			float scany = y*NSVG__SUBSAMPLES + s + 0.5f;
			NSVGactiveEdge **step = &active;

			// update all active edges;
			// remove all active edges that terminate before the center of this scanline
			while (*step) {
				NSVGactiveEdge *z = *step;
				if (z->ey <= scany) {
					*step = z->next; // delete from list
//					NSVG__assert(z->valid);
					nsvg__freeActive(r, z);
				} else {
					z->x += z->dx; // advance to position for current scanline
					step = &((*step)->next); // advance through list
				}
			}

			// resort the list if needed
			for (;;) {
				int changed = 0;
				step = &active;
				while (*step && (*step)->next) {
					if ((*step)->x > (*step)->next->x) {
						NSVGactiveEdge* t = *step;
						NSVGactiveEdge* q = t->next;
						t->next = q->next;
						q->next = t;
						*step = q;
						changed = 1;
					}
					step = &(*step)->next;
				}
				if (!changed) break;
			}

			// insert all edges that start before the center of this scanline -- omit ones that also end on this scanline
			while (e < r->nedges && r->edges[e].y0 <= scany) {
				if (r->edges[e].y1 > scany) {
					NSVGactiveEdge* z = nsvg__addActive(r, &r->edges[e], scany);
					if (z == NULL) break;
					// find insertion point
					if (active == NULL) {
						active = z;
					} else if (z->x < active->x) {
						// insert at front
						z->next = active;
						active = z;
					} else {
						// find thing to insert AFTER
						NSVGactiveEdge* p = active;
						while (p->next && p->next->x < z->x)
							p = p->next;
						// at this point, p->next->x is NOT < z->x
						z->next = p->next;
						p->next = z;
					}
				}
				e++;
			}

			// now process all active edges in non-zero fashion
			if (active != NULL)
				nsvg__fillActiveEdges(r->scanline, r->width, active, maxWeight, &xmin, &xmax);
		}
		// Blit
		if (xmin < 0) xmin = 0;
		if (xmax > r->width-1) xmax = r->width-1;
		if (xmin <= xmax) {
			nsvg__scanlineSolid(&r->bitmap[y * r->stride] + xmin*4, xmax-xmin+1, &r->scanline[xmin], xmin, y, tx,ty,scale,cache);
		}
	}

}

static void nsvg__unpremultiplyAlpha(unsigned char* image, int w, int h, int stride)
{
	int x,y;

	// Unpremultiply
	for (y = 0; y < h; y++) {
		unsigned char *row = &image[y*stride];
		for (x = 0; x < w; x++) {
			int r = row[0], g = row[1], b = row[2], a = row[3];
			if (a != 0) {
				row[0] = (int)(r*255/a);
				row[1] = (int)(g*255/a);
				row[2] = (int)(b*255/a);
			}
			row += 4;
		}
	}

	// Defringe
	for (y = 0; y < h; y++) {
		unsigned char *row = &image[y*stride];
		for (x = 0; x < w; x++) {
			int r = 0, g = 0, b = 0, a = row[3], n = 0;
			if (a == 0) {
				if (x-1 > 0 && row[-1] != 0) {
					r += row[-4];
					g += row[-3];
					b += row[-2];
					n++;
				}
				if (x+1 < w && row[7] != 0) {
					r += row[4];
					g += row[5];
					b += row[6];
					n++;
				}
				if (y-1 > 0 && row[-stride+3] != 0) {
					r += row[-stride];
					g += row[-stride+1];
					b += row[-stride+2];
					n++;
				}
				if (y+1 < h && row[stride+3] != 0) {
					r += row[stride];
					g += row[stride+1];
					b += row[stride+2];
					n++;
				}
				if (n > 0) {
					row[0] = r/n;
					row[1] = g/n;
					row[2] = b/n;
				}
			}
			row += 4;
		}
	}
}

static void nsvg__initPaint(NSVGcachedPaint* cache, NSVGpaint* paint, float opacity)
{
	int i, j;
	NSVGgradient* grad;

	cache->type = paint->type;

	if (paint->type == NSVG_PAINT_COLOR) {
		cache->colors[0] = nsvg__applyOpacity(paint->color, opacity);
		return;
	}

	grad = paint->gradient;

	cache->spread = grad->spread;
	memcpy(cache->xform, grad->xform, sizeof(float)*6);

	if (grad->nstops == 0) {
		for (i = 0; i < 256; i++)
			cache->colors[i] = 0;
	} if (grad->nstops == 1) {
		for (i = 0; i < 256; i++)
			cache->colors[i] = nsvg__applyOpacity(grad->stops[i].color, opacity);
	} else {
		unsigned int ca, cb;
		float ua, ub, du, u;
		int ia, ib, count;

		ca = nsvg__applyOpacity(grad->stops[0].color, opacity);
		ua = nsvg__clampf(grad->stops[0].offset, 0, 1);
		ub = nsvg__clampf(grad->stops[grad->nstops-1].offset, ua, 1);
		ia = ua * 255.0f;
		ib = ub * 255.0f;
		for (i = 0; i < ia; i++) {
			cache->colors[i] = ca;
		}

		for (i = 0; i < grad->nstops-1; i++) {
			ca = nsvg__applyOpacity(grad->stops[i].color, opacity);
			cb = nsvg__applyOpacity(grad->stops[i+1].color, opacity);
			ua = nsvg__clampf(grad->stops[i].offset, 0, 1);
			ub = nsvg__clampf(grad->stops[i+1].offset, 0, 1);
			ia = ua * 255.0f;
			ib = ub * 255.0f;
			count = ib - ia;
			if (count <= 0) continue;
			u = 0;
			du = 1.0f / (float)count;
			for (j = 0; j < count; j++) {
				cache->colors[ia+j] = nsvg__lerpRGBA(ca,cb,u);
				u += du;
			}
		}

		for (i = ib; i < 256; i++)
			cache->colors[i] = cb;
	}

}

/*
static void dumpEdges(NSVGrasterizer* r, const char* name)
{
	float xmin = 0, xmax = 0, ymin = 0, ymax = 0;
	NSVGedge *e = NULL;
	int i;
	if (r->nedges == 0) return;
	FILE* fp = fopen(name, "w");
	if (fp == NULL) return;

	xmin = xmax = r->edges[0].x0;
	ymin = ymax = r->edges[0].y0;
	for (i = 0; i < r->nedges; i++) {
		e = &r->edges[i];
		xmin = nsvg__minf(xmin, e->x0);
		xmin = nsvg__minf(xmin, e->x1);
		xmax = nsvg__maxf(xmax, e->x0);
		xmax = nsvg__maxf(xmax, e->x1);
		ymin = nsvg__minf(ymin, e->y0);
		ymin = nsvg__minf(ymin, e->y1);
		ymax = nsvg__maxf(ymax, e->y0);
		ymax = nsvg__maxf(ymax, e->y1);
	}

	fprintf(fp, "<svg viewBox=\"%f %f %f %f\" xmlns=\"http://www.w3.org/2000/svg\">", xmin, ymin, (xmax - xmin), (ymax - ymin));

	for (i = 0; i < r->nedges; i++) {
		e = &r->edges[i];
		fprintf(fp ,"<line x1=\"%f\" y1=\"%f\" x2=\"%f\" y2=\"%f\" style=\"stroke:#000;\" />", e->x0,e->y0, e->x1,e->y1);
	}

	for (i = 0; i < r->npoints; i++) {
		if (i+1 < r->npoints)
			fprintf(fp ,"<line x1=\"%f\" y1=\"%f\" x2=\"%f\" y2=\"%f\" style=\"stroke:#f00;\" />", r->points[i].x, r->points[i].y, r->points[i+1].x, r->points[i+1].y);
		fprintf(fp ,"<circle cx=\"%f\" cy=\"%f\" r=\"1\" style=\"fill:%s;\" />", r->points[i].x, r->points[i].y, r->points[i].flags == 0 ? "#f00" : "#0f0");
	}

	fprintf(fp, "</svg>");
	fclose(fp);
}
*/

void nsvgRasterize(NSVGrasterizer* r,
				   NSVGimage* image, float tx, float ty, float scale,
				   unsigned char* dst, int w, int h, int stride)
{
	NSVGshape *shape = NULL;
	NSVGedge *e = NULL;
	NSVGcachedPaint cache;
	int i;

	r->bitmap = dst;
	r->width = w;
	r->height = h;
	r->stride = stride;

	if (w > r->cscanline) {
		r->cscanline = w;
		r->scanline = (unsigned char*)realloc(r->scanline, w);
		if (r->scanline == NULL) return;
	}

	for (i = 0; i < h; i++)
		memset(&dst[i*stride], 0, w*4);

	for (shape = image->shapes; shape != NULL; shape = shape->next) {
		if (shape->fill.type != NSVG_PAINT_NONE) {
			nsvg__resetPool(r);
			r->freelist = NULL;
			r->nedges = 0;

			nsvg__flattenShape(r, shape, scale);

			// Scale and translate edges
			for (i = 0; i < r->nedges; i++) {
				e = &r->edges[i];
				e->x0 = tx + e->x0;
				e->y0 = (ty + e->y0) * NSVG__SUBSAMPLES;
				e->x1 = tx + e->x1;
				e->y1 = (ty + e->y1) * NSVG__SUBSAMPLES;
			}

			// Rasterize edges
			qsort(r->edges, r->nedges, sizeof(NSVGedge), nsvg__cmpEdge);

			// now, traverse the scanlines and find the intersections on each scanline, use non-zero rule
			nsvg__initPaint(&cache, &shape->fill, shape->opacity);

			nsvg__rasterizeSortedEdges(r, tx,ty,scale, &cache);
		}
		if (shape->stroke.type != NSVG_PAINT_NONE && (shape->strokeWidth * scale) > 0.01f) {
			nsvg__resetPool(r);
			r->freelist = NULL;
			r->nedges = 0;

			nsvg__flattenShapeStroke(r, shape, scale);

//			dumpEdges(r, "edge.svg");

			// Scale and translate edges
			for (i = 0; i < r->nedges; i++) {
				e = &r->edges[i];
				e->x0 = tx + e->x0;
				e->y0 = (ty + e->y0) * NSVG__SUBSAMPLES;
				e->x1 = tx + e->x1;
				e->y1 = (ty + e->y1) * NSVG__SUBSAMPLES;
			}

			// Rasterize edges
			qsort(r->edges, r->nedges, sizeof(NSVGedge), nsvg__cmpEdge);

			// now, traverse the scanlines and find the intersections on each scanline, use non-zero rule
			nsvg__initPaint(&cache, &shape->stroke, shape->opacity);

			nsvg__rasterizeSortedEdges(r, tx,ty,scale, &cache);
		}
	}

	nsvg__unpremultiplyAlpha(dst, w, h, stride);

	r->bitmap = NULL;
	r->width = 0;
	r->height = 0;
	r->stride = 0;
}

#endif


//#line 1 "stb_image.c"

//#line 1 "stb_image.h"
#ifndef STBI_INCLUDE_STB_IMAGE_H
#define STBI_INCLUDE_STB_IMAGE_H

// DOCUMENTATION
//
// Limitations:
//    - no 16-bit-per-channel PNG
//    - no 12-bit-per-channel JPEG
//    - no JPEGs with arithmetic coding
//    - no 1-bit BMP
//    - GIF always returns *comp=4
//
// Basic usage (see HDR discussion below for HDR usage):
//    int x,y,n;
//    unsigned char *data = stbi_load(filename, &x, &y, &n, 0);
//    // ... process data if not NULL ...
//    // ... x = width, y = height, n = # 8-bit components per pixel ...
//    // ... replace '0' with '1'..'4' to force that many components per pixel
//    // ... but 'n' will always be the number that it would have been if you said 0
//    stbi_image_free(data)
//
// Standard parameters:
//    int *x       -- outputs image width in pixels
//    int *y       -- outputs image height in pixels
//    int *comp    -- outputs # of image components in image file
//    int req_comp -- if non-zero, # of image components requested in result
//
// The return value from an image loader is an 'unsigned char *' which points
// to the pixel data, or NULL on an allocation failure or if the image is
// corrupt or invalid. The pixel data consists of *y scanlines of *x pixels,
// with each pixel consisting of N interleaved 8-bit components; the first
// pixel pointed to is top-left-most in the image. There is no padding between
// image scanlines or between pixels, regardless of format. The number of
// components N is 'req_comp' if req_comp is non-zero, or *comp otherwise.
// If req_comp is non-zero, *comp has the number of components that _would_
// have been output otherwise. E.g. if you set req_comp to 4, you will always
// get RGBA output, but you can check *comp to see if it's trivially opaque
// because e.g. there were only 3 channels in the source image.
//
// An output image with N components has the following components interleaved
// in this order in each pixel:
//
//     N=#comp     components
//       1           grey
//       2           grey, alpha
//       3           red, green, blue
//       4           red, green, blue, alpha
//
// If image loading fails for any reason, the return value will be NULL,
// and *x, *y, *comp will be unchanged. The function stbi_failure_reason()
// can be queried for an extremely brief, end-user unfriendly explanation
// of why the load failed. Define STBI_NO_FAILURE_STRINGS to avoid
// compiling these strings at all, and STBI_FAILURE_USERMSG to get slightly
// more user-friendly ones.
//
// Paletted PNG, BMP, GIF, and PIC images are automatically depalettized.
//
// ===========================================================================
//
// Philosophy
//
// stb libraries are designed with the following priorities:
//
//    1. easy to use
//    2. easy to maintain
//    3. good performance
//
// Sometimes I let "good performance" creep up in priority over "easy to maintain",
// and for best performance I may provide less-easy-to-use APIs that give higher
// performance, in addition to the easy to use ones. Nevertheless, it's important
// to keep in mind that from the standpoint of you, a client of this library,
// all you care about is #1 and #3, and stb libraries do not emphasize #3 above all.
//
// Some secondary priorities arise directly from the first two, some of which
// make more explicit reasons why performance can't be emphasized.
//
//    - Portable ("ease of use")
//    - Small footprint ("easy to maintain")
//    - No dependencies ("ease of use")
//
// ===========================================================================
//
// I/O callbacks
//
// I/O callbacks allow you to read from arbitrary sources, like packaged
// files or some other source. Data read from callbacks are processed
// through a small internal buffer (currently 128 bytes) to try to reduce
// overhead.
//
// The three functions you must define are "read" (reads some bytes of data),
// "skip" (skips some bytes of data), "eof" (reports if the stream is at the end).
//
// ===========================================================================
//
// SIMD support
//
// The JPEG decoder will try to automatically use SIMD kernels on x86 when
// supported by the compiler. For ARM Neon support, you must explicitly
// request it.
//
// (The old do-it-yourself SIMD API is no longer supported in the current
// code.)
//
// On x86, SSE2 will automatically be used when available based on a run-time
// test; if not, the generic C versions are used as a fall-back. On ARM targets,
// the typical path is to have separate builds for NEON and non-NEON devices
// (at least this is true for iOS and Android). Therefore, the NEON support is
// toggled by a build flag: define STBI_NEON to get NEON loops.
//
// The output of the JPEG decoder is slightly different from versions where
// SIMD support was introduced (that is, for versions before 1.49). The
// difference is only +-1 in the 8-bit RGB channels, and only on a small
// fraction of pixels. You can force the pre-1.49 behavior by defining
// STBI_JPEG_OLD, but this will disable some of the SIMD decoding path
// and hence cost some performance.
//
// If for some reason you do not want to use any of SIMD code, or if
// you have issues compiling it, you can disable it entirely by
// defining STBI_NO_SIMD.
//
// ===========================================================================
//
// HDR image support   (disable by defining STBI_NO_HDR)
//
// stb_image now supports loading HDR images in general, and currently
// the Radiance .HDR file format, although the support is provided
// generically. You can still load any file through the existing interface;
// if you attempt to load an HDR file, it will be automatically remapped to
// LDR, assuming gamma 2.2 and an arbitrary scale factor defaulting to 1;
// both of these constants can be reconfigured through this interface:
//
//     stbi_hdr_to_ldr_gamma(2.2f);
//     stbi_hdr_to_ldr_scale(1.0f);
//
// (note, do not use _inverse_ constants; stbi_image will invert them
// appropriately).
//
// Additionally, there is a new, parallel interface for loading files as
// (linear) floats to preserve the full dynamic range:
//
//    float *data = stbi_loadf(filename, &x, &y, &n, 0);
//
// If you load LDR images through this interface, those images will
// be promoted to floating point values, run through the inverse of
// constants corresponding to the above:
//
//     stbi_ldr_to_hdr_scale(1.0f);
//     stbi_ldr_to_hdr_gamma(2.2f);
//
// Finally, given a filename (or an open file or memory block--see header
// file for details) containing image data, you can query for the "most
// appropriate" interface to use (that is, whether the image is HDR or
// not), using:
//
//     stbi_is_hdr(char *filename);
//
// ===========================================================================
//
// iPhone PNG support:
//
// By default we convert iphone-formatted PNGs back to RGB, even though
// they are internally encoded differently. You can disable this conversion
// by by calling stbi_convert_iphone_png_to_rgb(0), in which case
// you will always just get the native iphone "format" through (which
// is BGR stored in RGB).
//
// Call stbi_set_unpremultiply_on_load(1) as well to force a divide per
// pixel to remove any premultiplied alpha *only* if the image file explicitly
// says there's premultiplied data (currently only happens in iPhone images,
// and only if iPhone convert-to-rgb processing is on).
//

#ifndef STBI_NO_STDIO
#include <stdio.h>
#endif // STBI_NO_STDIO

#define STBI_VERSION 1

enum
{
   STBI_default = 0, // only used for req_comp

   STBI_grey       = 1,
   STBI_grey_alpha = 2,
   STBI_rgb        = 3,
   STBI_rgb_alpha  = 4
};

typedef unsigned char stbi_uc;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef STB_IMAGE_STATIC
#define STBIDEF static
#else
#define STBIDEF extern
#endif

//////////////////////////////////////////////////////////////////////////////
//
// PRIMARY API - works on images of any type
//

//
// load image by filename, open file, or memory buffer
//

typedef struct
{
   int      (*read)  (void *user,char *data,int size);   // fill 'data' with 'size' bytes.  return number of bytes actually read
   void     (*skip)  (void *user,int n);                 // skip the next 'n' bytes, or 'unget' the last -n bytes if negative
   int      (*eof)   (void *user);                       // returns nonzero if we are at end of file/data
} stbi_io_callbacks;

STBIDEF stbi_uc *stbi_load               (char              const *filename,           int *x, int *y, int *comp, int req_comp);
STBIDEF stbi_uc *stbi_load_from_memory   (stbi_uc           const *buffer, int len   , int *x, int *y, int *comp, int req_comp);
STBIDEF stbi_uc *stbi_load_from_callbacks(stbi_io_callbacks const *clbk  , void *user, int *x, int *y, int *comp, int req_comp);

#ifndef STBI_NO_STDIO
STBIDEF stbi_uc *stbi_load_from_file  (FILE *f,                  int *x, int *y, int *comp, int req_comp);
// for stbi_load_from_file, file pointer is left pointing immediately after image
#endif

#ifndef STBI_NO_LINEAR
   STBIDEF float *stbi_loadf                 (char const *filename,           int *x, int *y, int *comp, int req_comp);
   STBIDEF float *stbi_loadf_from_memory     (stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp);
   STBIDEF float *stbi_loadf_from_callbacks  (stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp, int req_comp);

   #ifndef STBI_NO_STDIO
   STBIDEF float *stbi_loadf_from_file  (FILE *f,                int *x, int *y, int *comp, int req_comp);
   #endif
#endif

#ifndef STBI_NO_HDR
   STBIDEF void   stbi_hdr_to_ldr_gamma(float gamma);
   STBIDEF void   stbi_hdr_to_ldr_scale(float scale);
#endif

#ifndef STBI_NO_LINEAR
   STBIDEF void   stbi_ldr_to_hdr_gamma(float gamma);
   STBIDEF void   stbi_ldr_to_hdr_scale(float scale);
#endif // STBI_NO_HDR

// stbi_is_hdr is always defined, but always returns false if STBI_NO_HDR
STBIDEF int    stbi_is_hdr_from_callbacks(stbi_io_callbacks const *clbk, void *user);
STBIDEF int    stbi_is_hdr_from_memory(stbi_uc const *buffer, int len);
#ifndef STBI_NO_STDIO
STBIDEF int      stbi_is_hdr          (char const *filename);
STBIDEF int      stbi_is_hdr_from_file(FILE *f);
#endif // STBI_NO_STDIO

// get a VERY brief reason for failure
// NOT THREADSAFE
STBIDEF const char *stbi_failure_reason  (void);

// free the loaded image -- this is just free()
STBIDEF void     stbi_image_free      (void *retval_from_stbi_load);

// get image dimensions & components without fully decoding
STBIDEF int      stbi_info_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp);
STBIDEF int      stbi_info_from_callbacks(stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp);

#ifndef STBI_NO_STDIO
STBIDEF int      stbi_info            (char const *filename,     int *x, int *y, int *comp);
STBIDEF int      stbi_info_from_file  (FILE *f,                  int *x, int *y, int *comp);

#endif

// for image formats that explicitly notate that they have premultiplied alpha,
// we just return the colors as stored in the file. set this flag to force
// unpremultiplication. results are undefined if the unpremultiply overflow.
STBIDEF void stbi_set_unpremultiply_on_load(int flag_true_if_should_unpremultiply);

// indicate whether we should process iphone images back to canonical format,
// or just pass them through "as-is"
STBIDEF void stbi_convert_iphone_png_to_rgb(int flag_true_if_should_convert);

// ZLIB client - used by PNG, available for other purposes

STBIDEF char *stbi_zlib_decode_malloc_guesssize(const char *buffer, int len, int initial_size, int *outlen);
STBIDEF char *stbi_zlib_decode_malloc_guesssize_headerflag(const char *buffer, int len, int initial_size, int *outlen, int parse_header);
STBIDEF char *stbi_zlib_decode_malloc(const char *buffer, int len, int *outlen);
STBIDEF int   stbi_zlib_decode_buffer(char *obuffer, int olen, const char *ibuffer, int ilen);

STBIDEF char *stbi_zlib_decode_noheader_malloc(const char *buffer, int len, int *outlen);
STBIDEF int   stbi_zlib_decode_noheader_buffer(char *obuffer, int olen, const char *ibuffer, int ilen);

#ifndef STBI_NO_DDS

//#line 1 "stbi_DDS.h"
#ifndef HEADER_STB_IMAGE_DDS_AUGMENTATION
#define HEADER_STB_IMAGE_DDS_AUGMENTATION

/*	is it a DDS file? */
extern int      stbi__dds_test_memory      (stbi_uc const *buffer, int len);
extern int      stbi__dds_test_callbacks   (stbi_io_callbacks const *clbk, void *user);

extern stbi_uc *stbi__dds_load_from_path   (const char *filename,           int *x, int *y, int *comp, int req_comp);
extern stbi_uc *stbi__dds_load_from_memory (stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp);
extern stbi_uc *stbi__dds_load_from_callbacks (stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp, int req_comp);

#ifndef STBI_NO_STDIO
extern int      stbi__dds_test_filename    (char const *filename);
extern int      stbi__dds_test_file        (FILE *f);
extern stbi_uc *stbi__dds_load_from_file   (FILE *f,                  int *x, int *y, int *comp, int req_comp);
#endif

extern int      stbi__dds_info_from_memory (stbi_uc const *buffer, int len, int *x, int *y, int *comp, int *iscompressed);
extern int      stbi__dds_info_from_callbacks (stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp, int *iscompressed);

#ifndef STBI_NO_STDIO
extern int      stbi__dds_info_from_path   (char const *filename,     int *x, int *y, int *comp, int *iscompressed);
extern int      stbi__dds_info_from_file   (FILE *f,                  int *x, int *y, int *comp, int *iscompressed);
#endif

/*
//
////   end header file   /////////////////////////////////////////////////////*/
#endif /* HEADER_STB_IMAGE_DDS_AUGMENTATION */


#endif

#ifndef STBI_NO_PVR

//#line 1 "stbi_pvr.h"
#ifndef HEADER_STB_IMAGE_PVR_AUGMENTATION
#define HEADER_STB_IMAGE_PVR_AUGMENTATION

/*	is it a PVR file? */
extern int      stbi__pvr_test_memory      (stbi_uc const *buffer, int len);
extern int      stbi__pvr_test_callbacks   (stbi_io_callbacks const *clbk, void *user);

extern stbi_uc *stbi__pvr_load_from_path   (char const *filename,           int *x, int *y, int *comp, int req_comp);
extern stbi_uc *stbi__pvr_load_from_memory (stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp);
extern stbi_uc *stbi__pvr_load_from_callbacks (stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp, int req_comp);

#ifndef STBI_NO_STDIO
extern int      stbi__pvr_test_filename    (char const *filename);
extern int      stbi__pvr_test_file        (FILE *f);
extern stbi_uc *stbi__pvr_load_from_file   (FILE *f,                  int *x, int *y, int *comp, int req_comp);
#endif

extern int      stbi__pvr_info_from_memory (stbi_uc const *buffer, int len, int *x, int *y, int *comp, int *iscompressed);
extern int      stbi__pvr_info_from_callbacks (stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp, int *iscompressed);

#ifndef STBI_NO_STDIO
extern int      stbi__pvr_info_from_path   (char const *filename,     int *x, int *y, int *comp, int *iscompressed);
extern int      stbi__pvr_info_from_file   (FILE *f,                  int *x, int *y, int *comp, int *iscompressed);
#endif

/*
//
////   end header file   /////////////////////////////////////////////////////*/
#endif /* HEADER_STB_IMAGE_PVR_AUGMENTATION */


#endif

#ifndef STBI_NO_PKM

//#line 1 "stbi_pkm.h"
#ifndef HEADER_STB_IMAGE_PKM_AUGMENTATION
#define HEADER_STB_IMAGE_PKM_AUGMENTATION

/*	is it a PKM file? */
extern int      stbi__pkm_test_memory      (stbi_uc const *buffer, int len);
extern int      stbi__pkm_test_callbacks   (stbi_io_callbacks const *clbk, void *user);

extern stbi_uc *stbi__pkm_load_from_path   (char const *filename,           int *x, int *y, int *comp, int req_comp);
extern stbi_uc *stbi__pkm_load_from_memory (stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp);
extern stbi_uc *stbi__pkm_load_from_callbacks (stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp, int req_comp);

#ifndef STBI_NO_STDIO
extern int      stbi__pkm_test_filename    (char const *filename);
extern int      stbi__pkm_test_file        (FILE *f);
extern stbi_uc *stbi__pkm_load_from_file   (FILE *f,                  int *x, int *y, int *comp, int req_comp);
#endif

extern int      stbi__pkm_info_from_memory (stbi_uc const *buffer, int len, int *x, int *y, int *comp);
extern int      stbi__pkm_info_from_callbacks (stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp);

#ifndef STBI_NO_STDIO
extern int      stbi__pkm_info_from_path   (char const *filename,     int *x, int *y, int *comp);
extern int      stbi__pkm_info_from_file   (FILE *f,                  int *x, int *y, int *comp);
#endif

/*
//
////   end header file   /////////////////////////////////////////////////////*/
#endif /* HEADER_STB_IMAGE_PKM_AUGMENTATION */


#endif

#ifndef STBI_NO_EXT

//#line 1 "stbi_ext.h"
#ifndef HEADER_STB_IMAGE_EXT
#define HEADER_STB_IMAGE_EXT

enum {
	STBI_unknown= 0,
	STBI_jpeg	= 1,
	STBI_png	= 2,
	STBI_bmp	= 3,
	STBI_gif	= 4,
	STBI_tga	= 5,
	STBI_psd	= 6,
	STBI_pic	= 7,
	STBI_pnm	= 8,
	STBI_dds	= 9,
	STBI_pvr	= 10,
	STBI_pkm	= 11,
	STBI_hdr	= 12
};

extern int      stbi_test_from_memory      (stbi_uc const *buffer, int len);
extern int      stbi_test_from_callbacks   (stbi_io_callbacks const *clbk, void *user);

#ifndef STBI_NO_STDIO
extern int      stbi_test					(char const *filename);
extern int      stbi_test_from_file        (FILE *f);
#endif

#endif /* HEADER_STB_IMAGE_EXT */


#endif

#ifdef __cplusplus
}
#endif

//
//
////   end header file   /////////////////////////////////////////////////////
#endif // STBI_INCLUDE_STB_IMAGE_H

/*
   revision history:
	  2.02  (2015-01-19) fix incorrect assert, fix warning
	  2.01  (2015-01-17) fix various warnings; suppress SIMD on gcc 32-bit without -msse2
	  2.00b (2014-12-25) fix STBI_MALLOC in progressive JPEG
	  2.00  (2014-12-25) optimize JPG, including x86 SSE2 & NEON SIMD (ryg)
						 progressive JPEG (stb)
						 PGM/PPM support (Ken Miller)
						 STBI_MALLOC,STBI_REALLOC,STBI_FREE
						 GIF bugfix -- seemingly never worked
						 STBI_NO_*, STBI_ONLY_*
	  1.48  (2014-12-14) fix incorrectly-named assert()
	  1.47  (2014-12-14) 1/2/4-bit PNG support, both direct and paletted (Omar Cornut & stb)
						 optimize PNG (ryg)
						 fix bug in interlaced PNG with user-specified channel count (stb)
	  1.46  (2014-08-26)
			  fix broken tRNS chunk (colorkey-style transparency) in non-paletted PNG
	  1.45  (2014-08-16)
			  fix MSVC-ARM internal compiler error by wrapping malloc
	  1.44  (2014-08-07)
			  various warning fixes from Ronny Chevalier
	  1.43  (2014-07-15)
			  fix MSVC-only compiler problem in code changed in 1.42
	  1.42  (2014-07-09)
			  don't define _CRT_SECURE_NO_WARNINGS (affects user code)
			  fixes to stbi__cleanup_jpeg path
			  added STBI_ASSERT to avoid requiring assert.h
	  1.41  (2014-06-25)
			  fix search&replace from 1.36 that messed up comments/error messages
	  1.40  (2014-06-22)
			  fix gcc struct-initialization warning
	  1.39  (2014-06-15)
			  fix to TGA optimization when req_comp != number of components in TGA;
			  fix to GIF loading because BMP wasn't rewinding (whoops, no GIFs in my test suite)
			  add support for BMP version 5 (more ignored fields)
	  1.38  (2014-06-06)
			  suppress MSVC warnings on integer casts truncating values
			  fix accidental rename of 'skip' field of I/O
	  1.37  (2014-06-04)
			  remove duplicate typedef
	  1.36  (2014-06-03)
			  convert to header file single-file library
			  if de-iphone isn't set, load iphone images color-swapped instead of returning NULL
	  1.35  (2014-05-27)
			  various warnings
			  fix broken STBI_SIMD path
			  fix bug where stbi_load_from_file no longer left file pointer in correct place
			  fix broken non-easy path for 32-bit BMP (possibly never used)
			  TGA optimization by Arseny Kapoulkine
	  1.34  (unknown)
			  use STBI_NOTUSED in stbi__resample_row_generic(), fix one more leak in tga failure case
	  1.33  (2011-07-14)
			  make stbi_is_hdr work in STBI_NO_HDR (as specified), minor compiler-friendly improvements
	  1.32  (2011-07-13)
			  support for "info" function for all supported filetypes (SpartanJ)
	  1.31  (2011-06-20)
			  a few more leak fixes, bug in PNG handling (SpartanJ)
	  1.30  (2011-06-11)
			  added ability to load files via callbacks to accomidate custom input streams (Ben Wenger)
			  removed deprecated format-specific test/load functions
			  removed support for installable file formats (stbi_loader) -- would have been broken for IO callbacks anyway
			  error cases in bmp and tga give messages and don't leak (Raymond Barbiero, grisha)
			  fix inefficiency in decoding 32-bit BMP (David Woo)
	  1.29  (2010-08-16)
			  various warning fixes from Aurelien Pocheville
	  1.28  (2010-08-01)
			  fix bug in GIF palette transparency (SpartanJ)
	  1.27  (2010-08-01)
			  cast-to-stbi_uc to fix warnings
	  1.26  (2010-07-24)
			  fix bug in file buffering for PNG reported by SpartanJ
	  1.25  (2010-07-17)
			  refix trans_data warning (Won Chun)
	  1.24  (2010-07-12)
			  perf improvements reading from files on platforms with lock-heavy fgetc()
			  minor perf improvements for jpeg
			  deprecated type-specific functions so we'll get feedback if they're needed
			  attempt to fix trans_data warning (Won Chun)
	  1.23    fixed bug in iPhone support
	  1.22  (2010-07-10)
			  removed image *writing* support
			  stbi_info support from Jetro Lauha
			  GIF support from Jean-Marc Lienher
			  iPhone PNG-extensions from James Brown
			  warning-fixes from Nicolas Schulz and Janez Zemva (i.stbi__err. Janez (U+017D)emva)
	  1.21    fix use of 'stbi_uc' in header (reported by jon blow)
	  1.20    added support for Softimage PIC, by Tom Seddon
	  1.19    bug in interlaced PNG corruption check (found by ryg)
	  1.18 2008-08-02
			  fix a threading bug (local mutable static)
	  1.17    support interlaced PNG
	  1.16    major bugfix - stbi__convert_format converted one too many pixels
	  1.15    initialize some fields for thread safety
	  1.14    fix threadsafe conversion bug
			  header-file-only version (#define STBI_HEADER_FILE_ONLY before including)
	  1.13    threadsafe
	  1.12    const qualifiers in the API
	  1.11    Support installable IDCT, colorspace conversion routines
	  1.10    Fixes for 64-bit (don't use "unsigned long")
			  optimized upsampling by Fabian "ryg" Giesen
	  1.09    Fix format-conversion for PSD code (bad global variables!)
	  1.08    Thatcher Ulrich's PSD code integrated by Nicolas Schulz
	  1.07    attempt to fix C++ warning/errors again
	  1.06    attempt to fix C++ warning/errors again
	  1.05    fix TGA loading to return correct *comp and use good luminance calc
	  1.04    default float alpha is 1, not 255; use 'void *' for stbi_image_free
	  1.03    bugfixes to STBI_NO_STDIO, STBI_NO_HDR
	  1.02    support for (subset of) HDR files, float interface for preferred access to them
	  1.01    fix bug: possible bug in handling right-side up bmps... not sure
			  fix bug: the stbi__bmp_load() and stbi__tga_load() functions didn't work at all
	  1.00    interface to zlib that skips zlib header
	  0.99    correct handling of alpha in palette
	  0.98    TGA loader by lonesock; dynamically add loaders (untested)
	  0.97    jpeg errors on too large a file; also catch another malloc failure
	  0.96    fix detection of invalid v value - particleman@mollyrocket forum
	  0.95    during header scan, seek to markers in case of padding
	  0.94    STBI_NO_STDIO to disable stdio usage; rename all #defines the same
	  0.93    handle jpegtran output; verbose errors
	  0.92    read 4,8,16,24,32-bit BMP files of several formats
	  0.91    output 24-bit Windows 3.0 BMP files
	  0.90    fix a few more warnings; bump version number to approach 1.0
	  0.61    bugfixes due to Marc LeBlanc, Christopher Lloyd
	  0.60    fix compiling as c++
	  0.59    fix warnings: merge Dave Moore's -Wall fixes
	  0.58    fix bug: zlib uncompressed mode len/nlen was wrong endian
	  0.57    fix bug: jpg last huffman symbol before marker was >9 bits but less than 16 available
	  0.56    fix bug: zlib uncompressed mode len vs. nlen
	  0.55    fix bug: restart_interval not initialized to 0
	  0.54    allow NULL for 'int *comp'
	  0.53    fix bug in png 3->4; speedup png decoding
	  0.52    png handles req_comp=3,4 directly; minor cleanup; jpeg comments
	  0.51    obey req_comp requests, 1-component jpegs return as 1-component,
			  on 'test' only check type, not whether we support this variant
	  0.50    first released version
*/

#if defined(STBI_ONLY_JPEG) || defined(STBI_ONLY_PNG) || defined(STBI_ONLY_BMP) \
  || defined(STBI_ONLY_TGA) || defined(STBI_ONLY_GIF) || defined(STBI_ONLY_PSD) \
  || defined(STBI_ONLY_HDR) || defined(STBI_ONLY_PIC) || defined(STBI_ONLY_PNM) \
  || defined(STBI_ONLY_ZLIB)
   #ifndef STBI_ONLY_JPEG
   #define STBI_NO_JPEG
   #endif
   #ifndef STBI_ONLY_PNG
   #define STBI_NO_PNG
   #endif
   #ifndef STBI_ONLY_BMP
   #define STBI_NO_BMP
   #endif
   #ifndef STBI_ONLY_PSD
   #define STBI_NO_PSD
   #endif
   #ifndef STBI_ONLY_TGA
   #define STBI_NO_TGA
   #endif
   #ifndef STBI_ONLY_GIF
   #define STBI_NO_GIF
   #endif
   #ifndef STBI_ONLY_HDR
   #define STBI_NO_HDR
   #endif
   #ifndef STBI_ONLY_PIC
   #define STBI_NO_PIC
   #endif
   #ifndef STBI_ONLY_PNM
   #define STBI_NO_PNM
   #endif
#endif

#if defined(STBI_NO_PNG) && !defined(STBI_SUPPORT_ZLIB) && !defined(STBI_NO_ZLIB)
#define STBI_NO_ZLIB
#endif

#include <stdarg.h>
#include <stddef.h> // ptrdiff_t on osx
#include <stdlib.h>
#include <string.h>

#if !defined(STBI_NO_LINEAR) || !defined(STBI_NO_HDR)
#include <math.h>  // ldexp
#endif

#ifndef STBI_NO_STDIO
#include <stdio.h>
#endif

#ifndef STBI_ASSERT
#include <assert.h>
#define STBI_ASSERT(x) assert(x)
#endif

#ifndef _MSC_VER
   #ifdef __cplusplus
   #define stbi_inline inline
   #else
   #define stbi_inline
   #endif
#else
   #define stbi_inline __forceinline
#endif

#ifdef _MSC_VER
typedef unsigned short stbi__uint16;
typedef   signed short stbi__int16;
typedef unsigned int   stbi__uint32;
typedef   signed int   stbi__int32;
#else
#include <stdint.h>
typedef uint16_t stbi__uint16;
typedef int16_t  stbi__int16;
typedef uint32_t stbi__uint32;
typedef int32_t  stbi__int32;
#endif

// should produce compiler error if size is wrong
typedef unsigned char validate_uint32[sizeof(stbi__uint32)==4 ? 1 : -1];

#ifdef _MSC_VER
#define STBI_NOTUSED(v)  (void)(v)
#else
#define STBI_NOTUSED(v)  (void)sizeof(v)
#endif

#ifdef _MSC_VER
#define STBI_HAS_LROTL
#endif

#ifdef STBI_HAS_LROTL
   #define stbi_lrot(x,y)  _lrotl(x,y)
#else
   #define stbi_lrot(x,y)  (((x) << (y)) | ((x) >> (32 - (y))))
#endif

#if defined(STBI_MALLOC) && defined(STBI_FREE) && defined(STBI_REALLOC)
// ok
#elif !defined(STBI_MALLOC) && !defined(STBI_FREE) && !defined(STBI_REALLOC)
// ok
#else
#error "Must define all or none of STBI_MALLOC, STBI_FREE, and STBI_REALLOC."
#endif

#ifndef STBI_MALLOC
#define STBI_MALLOC(sz)    malloc(sz)
#define STBI_REALLOC(p,sz) realloc(p,sz)
#define STBI_FREE(p)       free(p)
#endif

#if defined(__GNUC__) && !defined(__SSE2__) && !defined(STBI_NO_SIMD)
// gcc doesn't support sse2 intrinsics unless you compile with -msse2,
// (but compiling with -msse2 allows the compiler to use SSE2 everywhere;
// this is just broken and gcc are jerks for not fixing it properly
// http://www.virtualdub.org/blog/pivot/entry.php?id=363 )
#define STBI_NO_SIMD
#endif

#if !defined(STBI_NO_SIMD) && (defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86))
#define STBI_SSE2
#include <emmintrin.h>

#ifdef _MSC_VER

#if _MSC_VER >= 1400  // not VC6
#include <intrin.h> // __cpuid
static int stbi__cpuid3(void)
{
   int info[4];
   __cpuid(info,1);
   return info[3];
}
#else
static int stbi__cpuid3(void)
{
   int res;
   __asm {
	  mov  eax,1
	  cpuid
	  mov  res,edx
   }
   return res;
}
#endif

#define STBI_SIMD_ALIGN(type, name) __declspec(align(16)) type name

static int stbi__sse2_available()
{
   int info3 = stbi__cpuid3();
   return ((info3 >> 26) & 1) != 0;
}
#else // assume GCC-style if not VC++
#define STBI_SIMD_ALIGN(type, name) type name __attribute__((aligned(16)))

static int stbi__sse2_available()
{
#if defined(__GNUC__) && (__GNUC__ * 100 + __GNUC_MINOR__) >= 408 // GCC 4.8 or later
   // GCC 4.8+ has a nice way to do this
   return __builtin_cpu_supports("sse2");
#else
   // portable way to do this, preferably without using GCC inline ASM?
   // just bail for now.
   return 0;
#endif
}
#endif
#endif

// ARM NEON
#if defined(STBI_NO_SIMD) && defined(STBI_NEON)
#undef STBI_NEON
#endif

#ifdef STBI_NEON
#include <arm_neon.h>
// assume GCC or Clang on ARM targets
#define STBI_SIMD_ALIGN(type, name) type name __attribute__((aligned(16)))
#endif

#ifndef STBI_SIMD_ALIGN
#define STBI_SIMD_ALIGN(type, name) type name
#endif

///////////////////////////////////////////////
//
//  stbi__context struct and start_xxx functions

// stbi__context structure is our basic context used by all images, so it
// contains all the IO context, plus some basic image information
typedef struct
{
   stbi__uint32 img_x, img_y;
   int img_n, img_out_n;

   stbi_io_callbacks io;
   void *io_user_data;

   int read_from_callbacks;
   int buflen;
   stbi_uc buffer_start[128];

   stbi_uc *img_buffer, *img_buffer_end;
   stbi_uc *img_buffer_original;
} stbi__context;

static void stbi__refill_buffer(stbi__context *s);

// initialize a memory-decode context
static void stbi__start_mem(stbi__context *s, stbi_uc const *buffer, int len)
{
   s->io.read = NULL;
   s->read_from_callbacks = 0;
   s->img_buffer = s->img_buffer_original = (stbi_uc *) buffer;
   s->img_buffer_end = (stbi_uc *) buffer+len;
}

// initialize a callback-based context
static void stbi__start_callbacks(stbi__context *s, stbi_io_callbacks *c, void *user)
{
   s->io = *c;
   s->io_user_data = user;
   s->buflen = sizeof(s->buffer_start);
   s->read_from_callbacks = 1;
   s->img_buffer_original = s->buffer_start;
   stbi__refill_buffer(s);
}

#ifndef STBI_NO_STDIO

static int stbi__stdio_read(void *user, char *data, int size)
{
   return (int) fread(data,1,size,(FILE*) user);
}

static void stbi__stdio_skip(void *user, int n)
{
   fseek((FILE*) user, n, SEEK_CUR);
}

static int stbi__stdio_eof(void *user)
{
   return feof((FILE*) user);
}

static stbi_io_callbacks stbi__stdio_callbacks =
{
   stbi__stdio_read,
   stbi__stdio_skip,
   stbi__stdio_eof,
};

static void stbi__start_file(stbi__context *s, FILE *f)
{
   stbi__start_callbacks(s, &stbi__stdio_callbacks, (void *) f);
}

//static void stop_file(stbi__context *s) { }

#endif // !STBI_NO_STDIO

static void stbi__rewind(stbi__context *s)
{
   // conceptually rewind SHOULD rewind to the beginning of the stream,
   // but we just rewind to the beginning of the initial buffer, because
   // we only use it after doing 'test', which only ever looks at at most 92 bytes
   s->img_buffer = s->img_buffer_original;
}

#ifndef STBI_NO_JPEG
static int      stbi__jpeg_test(stbi__context *s);
static stbi_uc *stbi__jpeg_load(stbi__context *s, int *x, int *y, int *comp, int req_comp);
static int      stbi__jpeg_info(stbi__context *s, int *x, int *y, int *comp);
#endif

#ifndef STBI_NO_PNG
static int      stbi__png_test(stbi__context *s);
static stbi_uc *stbi__png_load(stbi__context *s, int *x, int *y, int *comp, int req_comp);
static int      stbi__png_info(stbi__context *s, int *x, int *y, int *comp);
#endif

#ifndef STBI_NO_BMP
static int      stbi__bmp_test(stbi__context *s);
static stbi_uc *stbi__bmp_load(stbi__context *s, int *x, int *y, int *comp, int req_comp);
static int      stbi__bmp_info(stbi__context *s, int *x, int *y, int *comp);
#endif

#ifndef STBI_NO_TGA
static int      stbi__tga_test(stbi__context *s);
static stbi_uc *stbi__tga_load(stbi__context *s, int *x, int *y, int *comp, int req_comp);
static int      stbi__tga_info(stbi__context *s, int *x, int *y, int *comp);
#endif

#ifndef STBI_NO_PSD
static int      stbi__psd_test(stbi__context *s);
static stbi_uc *stbi__psd_load(stbi__context *s, int *x, int *y, int *comp, int req_comp);
static int      stbi__psd_info(stbi__context *s, int *x, int *y, int *comp);
#endif

#ifndef STBI_NO_HDR
static int      stbi__hdr_test(stbi__context *s);
static float   *stbi__hdr_load(stbi__context *s, int *x, int *y, int *comp, int req_comp);
static int      stbi__hdr_info(stbi__context *s, int *x, int *y, int *comp);
#endif

#ifndef STBI_NO_PIC
static int      stbi__pic_test(stbi__context *s);
static stbi_uc *stbi__pic_load(stbi__context *s, int *x, int *y, int *comp, int req_comp);
static int      stbi__pic_info(stbi__context *s, int *x, int *y, int *comp);
#endif

#ifndef STBI_NO_GIF
static int      stbi__gif_test(stbi__context *s);
static stbi_uc *stbi__gif_load(stbi__context *s, int *x, int *y, int *comp, int req_comp);
static int      stbi__gif_info(stbi__context *s, int *x, int *y, int *comp);
#endif

#ifndef STBI_NO_PNM
static int      stbi__pnm_test(stbi__context *s);
static stbi_uc *stbi__pnm_load(stbi__context *s, int *x, int *y, int *comp, int req_comp);
static int      stbi__pnm_info(stbi__context *s, int *x, int *y, int *comp);
#endif

#ifndef STBI_NO_DDS

static int      stbi__dds_test(stbi__context *s);
static stbi_uc *stbi__dds_load(stbi__context *s, int *x, int *y, int *comp, int req_comp);
#endif

#ifndef STBI_NO_PVR

static int      stbi__pvr_test(stbi__context *s);
static stbi_uc *stbi__pvr_load(stbi__context *s, int *x, int *y, int *comp, int req_comp);
#endif

#ifndef STBI_NO_PKM

static int      stbi__pkm_test(stbi__context *s);
static stbi_uc *stbi__pkm_load(stbi__context *s, int *x, int *y, int *comp, int req_comp);
#endif

// this is not threadsafe
static const char *stbi__g_failure_reason;

STBIDEF const char *stbi_failure_reason(void)
{
   return stbi__g_failure_reason;
}

static int stbi__err(const char *str)
{
   stbi__g_failure_reason = str;
   return 0;
}

static void *stbi__malloc(size_t size)
{
	return STBI_MALLOC(size);
}

// stbi__err - error
// stbi__errpf - error returning pointer to float
// stbi__errpuc - error returning pointer to unsigned char

#ifdef STBI_NO_FAILURE_STRINGS
   #define stbi__err(x,y)  0
#elif defined(STBI_FAILURE_USERMSG)
   #define stbi__err(x,y)  stbi__err(y)
#else
   #define stbi__err(x,y)  stbi__err(x)
#endif

#define stbi__errpf(x,y)   ((float *) (stbi__err(x,y)?NULL:NULL))
#define stbi__errpuc(x,y)  ((unsigned char *) (stbi__err(x,y)?NULL:NULL))

STBIDEF void stbi_image_free(void *retval_from_stbi_load)
{
   STBI_FREE(retval_from_stbi_load);
}

#ifndef STBI_NO_LINEAR
static float   *stbi__ldr_to_hdr(stbi_uc *data, int x, int y, int comp);
#endif

#ifndef STBI_NO_HDR
static stbi_uc *stbi__hdr_to_ldr(float   *data, int x, int y, int comp);
#endif

static unsigned char *stbi_load_main(stbi__context *s, int *x, int *y, int *comp, int req_comp)
{
   #ifndef STBI_NO_JPEG
   if (stbi__jpeg_test(s)) return stbi__jpeg_load(s,x,y,comp,req_comp);
   #endif
   #ifndef STBI_NO_PNG
   if (stbi__png_test(s))  return stbi__png_load(s,x,y,comp,req_comp);
   #endif
   #ifndef STBI_NO_BMP
   if (stbi__bmp_test(s))  return stbi__bmp_load(s,x,y,comp,req_comp);
   #endif
   #ifndef STBI_NO_GIF
   if (stbi__gif_test(s))  return stbi__gif_load(s,x,y,comp,req_comp);
   #endif
   #ifndef STBI_NO_PSD
   if (stbi__psd_test(s))  return stbi__psd_load(s,x,y,comp,req_comp);
   #endif
   #ifndef STBI_NO_PIC
   if (stbi__pic_test(s))  return stbi__pic_load(s,x,y,comp,req_comp);
   #endif
   #ifndef STBI_NO_PNM
   if (stbi__pnm_test(s))  return stbi__pnm_load(s,x,y,comp,req_comp);
   #endif
   #ifndef STBI_NO_DDS
   if (stbi__dds_test(s))  return stbi__dds_load(s,x,y,comp,req_comp);
   #endif
   #ifndef STBI_NO_PVR
   if (stbi__pvr_test(s))  return stbi__pvr_load(s,x,y,comp,req_comp);
   #endif
   #ifndef STBI_NO_PKM
   if (stbi__pkm_test(s))  return stbi__pkm_load(s,x,y,comp,req_comp);
   #endif
   #ifndef STBI_NO_HDR
   if (stbi__hdr_test(s)) {
	  float *hdr = stbi__hdr_load(s, x,y,comp,req_comp);
	  return stbi__hdr_to_ldr(hdr, *x, *y, req_comp ? req_comp : *comp);
   }
   #endif

   #ifndef STBI_NO_TGA
   // test tga last because it's a crappy test!
   if (stbi__tga_test(s))
	  return stbi__tga_load(s,x,y,comp,req_comp);
   #endif

   return stbi__errpuc("unknown image type", "Image not of any known type, or corrupt");
}

#ifndef STBI_NO_STDIO

static FILE *stbi__fopen(char const *filename, char const *mode)
{
   FILE *f;
#if defined(_MSC_VER) && _MSC_VER >= 1400
   if (0 != fopen_s(&f, filename, mode))
	  f=0;
#else
   f = fopen(filename, mode);
#endif
   return f;
}

STBIDEF stbi_uc *stbi_load(char const *filename, int *x, int *y, int *comp, int req_comp)
{
   FILE *f = stbi__fopen(filename, "rb");
   unsigned char *result;
   if (!f) return stbi__errpuc("can't fopen", "Unable to open file");
   result = stbi_load_from_file(f,x,y,comp,req_comp);
   fclose(f);
   return result;
}

STBIDEF stbi_uc *stbi_load_from_file(FILE *f, int *x, int *y, int *comp, int req_comp)
{
   unsigned char *result;
   stbi__context s;
   stbi__start_file(&s,f);
   result = stbi_load_main(&s,x,y,comp,req_comp);
   if (result) {
	  // need to 'unget' all the characters in the IO buffer
	  fseek(f, - (int) (s.img_buffer_end - s.img_buffer), SEEK_CUR);
   }
   return result;
}
#endif //!STBI_NO_STDIO

STBIDEF stbi_uc *stbi_load_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp)
{
   stbi__context s;
   stbi__start_mem(&s,buffer,len);
   return stbi_load_main(&s,x,y,comp,req_comp);
}

STBIDEF stbi_uc *stbi_load_from_callbacks(stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp, int req_comp)
{
   stbi__context s;
   stbi__start_callbacks(&s, (stbi_io_callbacks *) clbk, user);
   return stbi_load_main(&s,x,y,comp,req_comp);
}

#ifndef STBI_NO_LINEAR
static float *stbi_loadf_main(stbi__context *s, int *x, int *y, int *comp, int req_comp)
{
   unsigned char *data;
   #ifndef STBI_NO_HDR
   if (stbi__hdr_test(s))
	  return stbi__hdr_load(s,x,y,comp,req_comp);
   #endif
   data = stbi_load_main(s, x, y, comp, req_comp);
   if (data)
	  return stbi__ldr_to_hdr(data, *x, *y, req_comp ? req_comp : *comp);
   return stbi__errpf("unknown image type", "Image not of any known type, or corrupt");
}

STBIDEF float *stbi_loadf_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp)
{
   stbi__context s;
   stbi__start_mem(&s,buffer,len);
   return stbi_loadf_main(&s,x,y,comp,req_comp);
}

STBIDEF float *stbi_loadf_from_callbacks(stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp, int req_comp)
{
   stbi__context s;
   stbi__start_callbacks(&s, (stbi_io_callbacks *) clbk, user);
   return stbi_loadf_main(&s,x,y,comp,req_comp);
}

#ifndef STBI_NO_STDIO
STBIDEF float *stbi_loadf(char const *filename, int *x, int *y, int *comp, int req_comp)
{
   float *result;
   FILE *f = stbi__fopen(filename, "rb");
   if (!f) return stbi__errpf("can't fopen", "Unable to open file");
   result = stbi_loadf_from_file(f,x,y,comp,req_comp);
   fclose(f);
   return result;
}

STBIDEF float *stbi_loadf_from_file(FILE *f, int *x, int *y, int *comp, int req_comp)
{
   stbi__context s;
   stbi__start_file(&s,f);
   return stbi_loadf_main(&s,x,y,comp,req_comp);
}
#endif // !STBI_NO_STDIO

#endif // !STBI_NO_LINEAR

// these is-hdr-or-not is defined independent of whether STBI_NO_LINEAR is
// defined, for API simplicity; if STBI_NO_LINEAR is defined, it always
// reports false!

STBIDEF int stbi_is_hdr_from_memory(stbi_uc const *buffer, int len)
{
   #ifndef STBI_NO_HDR
   stbi__context s;
   stbi__start_mem(&s,buffer,len);
   return stbi__hdr_test(&s);
   #else
   STBI_NOTUSED(buffer);
   STBI_NOTUSED(len);
   return 0;
   #endif
}

#ifndef STBI_NO_STDIO
STBIDEF int      stbi_is_hdr          (char const *filename)
{
   FILE *f = stbi__fopen(filename, "rb");
   int result=0;
   if (f) {
	  result = stbi_is_hdr_from_file(f);
	  fclose(f);
   }
   return result;
}

STBIDEF int      stbi_is_hdr_from_file(FILE *f)
{
   #ifndef STBI_NO_HDR
   stbi__context s;
   stbi__start_file(&s,f);
   return stbi__hdr_test(&s);
   #else
   return 0;
   #endif
}
#endif // !STBI_NO_STDIO

STBIDEF int      stbi_is_hdr_from_callbacks(stbi_io_callbacks const *clbk, void *user)
{
   #ifndef STBI_NO_HDR
   stbi__context s;
   stbi__start_callbacks(&s, (stbi_io_callbacks *) clbk, user);
   return stbi__hdr_test(&s);
   #else
   return 0;
   #endif
}

static float stbi__h2l_gamma_i=1.0f/2.2f, stbi__h2l_scale_i=1.0f;
static float stbi__l2h_gamma=2.2f, stbi__l2h_scale=1.0f;

#ifndef STBI_NO_LINEAR
STBIDEF void   stbi_ldr_to_hdr_gamma(float gamma) { stbi__l2h_gamma = gamma; }
STBIDEF void   stbi_ldr_to_hdr_scale(float scale) { stbi__l2h_scale = scale; }
#endif

STBIDEF void   stbi_hdr_to_ldr_gamma(float gamma) { stbi__h2l_gamma_i = 1/gamma; }
STBIDEF void   stbi_hdr_to_ldr_scale(float scale) { stbi__h2l_scale_i = 1/scale; }

//////////////////////////////////////////////////////////////////////////////
//
// Common code used by all image loaders
//

enum
{
   STBI__SCAN_load=0,
   STBI__SCAN_type,
   STBI__SCAN_header
};

static void stbi__refill_buffer(stbi__context *s)
{
   int n = (s->io.read)(s->io_user_data,(char*)s->buffer_start,s->buflen);
   if (n == 0) {
	  // at end of file, treat same as if from memory, but need to handle case
	  // where s->img_buffer isn't pointing to safe memory, e.g. 0-byte file
	  s->read_from_callbacks = 0;
	  s->img_buffer = s->buffer_start;
	  s->img_buffer_end = s->buffer_start+1;
	  *s->img_buffer = 0;
   } else {
	  s->img_buffer = s->buffer_start;
	  s->img_buffer_end = s->buffer_start + n;
   }
}

stbi_inline static stbi_uc stbi__get8(stbi__context *s)
{
   if (s->img_buffer < s->img_buffer_end)
	  return *s->img_buffer++;
   if (s->read_from_callbacks) {
	  stbi__refill_buffer(s);
	  return *s->img_buffer++;
   }
   return 0;
}

stbi_inline static int stbi__at_eof(stbi__context *s)
{
   if (s->io.read) {
	  if (!(s->io.eof)(s->io_user_data)) return 0;
	  // if feof() is true, check if buffer = end
	  // special case: we've only got the special 0 character at the end
	  if (s->read_from_callbacks == 0) return 1;
   }

   return s->img_buffer >= s->img_buffer_end;
}

static void stbi__skip(stbi__context *s, int n)
{
   if (s->io.read) {
	  int blen = (int) (s->img_buffer_end - s->img_buffer);
	  if (blen < n) {
		 s->img_buffer = s->img_buffer_end;
		 (s->io.skip)(s->io_user_data, n - blen);
		 return;
	  }
   }
   s->img_buffer += n;
}

static int stbi__getn(stbi__context *s, stbi_uc *buffer, int n)
{
   if (s->io.read) {
	  int blen = (int) (s->img_buffer_end - s->img_buffer);
	  if (blen < n) {
		 int res, count;

		 memcpy(buffer, s->img_buffer, blen);

		 count = (s->io.read)(s->io_user_data, (char*) buffer + blen, n - blen);
		 res = (count == (n-blen));
		 s->img_buffer = s->img_buffer_end;
		 return res;
	  }
   }

   if (s->img_buffer+n <= s->img_buffer_end) {
	  memcpy(buffer, s->img_buffer, n);
	  s->img_buffer += n;
	  return 1;
   } else
	  return 0;
}

static int stbi__get16be(stbi__context *s)
{
   int z = stbi__get8(s);
   return (z << 8) + stbi__get8(s);
}

static stbi__uint32 stbi__get32be(stbi__context *s)
{
   stbi__uint32 z = stbi__get16be(s);
   return (z << 16) + stbi__get16be(s);
}

static int stbi__get16le(stbi__context *s)
{
   int z = stbi__get8(s);
   return z + (stbi__get8(s) << 8);
}

static stbi__uint32 stbi__get32le(stbi__context *s)
{
   stbi__uint32 z = stbi__get16le(s);
   return z + (stbi__get16le(s) << 16);
}

#define STBI__BYTECAST(x)  ((stbi_uc) ((x) & 255))  // truncate int to byte without warnings

//////////////////////////////////////////////////////////////////////////////
//
//  generic converter from built-in img_n to req_comp
//    individual types do this automatically as much as possible (e.g. jpeg
//    does all cases internally since it needs to colorspace convert anyway,
//    and it never has alpha, so very few cases ). png can automatically
//    interleave an alpha=255 channel, but falls back to this for other cases
//
//  assume data buffer is malloced, so malloc a new one and free that one
//  only failure mode is malloc failing

static stbi_uc stbi__compute_y(int r, int g, int b)
{
   return (stbi_uc) (((r*77) + (g*150) +  (29*b)) >> 8);
}

static unsigned char *stbi__convert_format(unsigned char *data, int img_n, int req_comp, unsigned int x, unsigned int y)
{
   int i,j;
   unsigned char *good;

   if (req_comp == img_n) return data;
   STBI_ASSERT(req_comp >= 1 && req_comp <= 4);

   good = (unsigned char *) stbi__malloc(req_comp * x * y);
   if (good == NULL) {
	  STBI_FREE(data);
	  return stbi__errpuc("outofmem", "Out of memory");
   }

   for (j=0; j < (int) y; ++j) {
	  unsigned char *src  = data + j * x * img_n   ;
	  unsigned char *dest = good + j * x * req_comp;

	  #define COMBO(a,b)  ((a)*8+(b))
	  #define CASE(a,b)   case COMBO(a,b): for(i=x-1; i >= 0; --i, src += a, dest += b)
	  // convert source image with img_n components to one with req_comp components;
	  // avoid switch per pixel, so use switch per scanline and massive macros
	  switch (COMBO(img_n, req_comp)) {
		 CASE(1,2) dest[0]=src[0], dest[1]=255; break;
		 CASE(1,3) dest[0]=dest[1]=dest[2]=src[0]; break;
		 CASE(1,4) dest[0]=dest[1]=dest[2]=src[0], dest[3]=255; break;
		 CASE(2,1) dest[0]=src[0]; break;
		 CASE(2,3) dest[0]=dest[1]=dest[2]=src[0]; break;
		 CASE(2,4) dest[0]=dest[1]=dest[2]=src[0], dest[3]=src[1]; break;
		 CASE(3,4) dest[0]=src[0],dest[1]=src[1],dest[2]=src[2],dest[3]=255; break;
		 CASE(3,1) dest[0]=stbi__compute_y(src[0],src[1],src[2]); break;
		 CASE(3,2) dest[0]=stbi__compute_y(src[0],src[1],src[2]), dest[1] = 255; break;
		 CASE(4,1) dest[0]=stbi__compute_y(src[0],src[1],src[2]); break;
		 CASE(4,2) dest[0]=stbi__compute_y(src[0],src[1],src[2]), dest[1] = src[3]; break;
		 CASE(4,3) dest[0]=src[0],dest[1]=src[1],dest[2]=src[2]; break;
		 default: STBI_ASSERT(0);
	  }
	  #undef CASE
   }

   STBI_FREE(data);
   return good;
}

#ifndef STBI_NO_LINEAR
static float   *stbi__ldr_to_hdr(stbi_uc *data, int x, int y, int comp)
{
   int i,k,n;
   float *output = (float *) stbi__malloc(x * y * comp * sizeof(float));
   if (output == NULL) { STBI_FREE(data); return stbi__errpf("outofmem", "Out of memory"); }
   // compute number of non-alpha components
   if (comp & 1) n = comp; else n = comp-1;
   for (i=0; i < x*y; ++i) {
	  for (k=0; k < n; ++k) {
		 output[i*comp + k] = (float) (pow(data[i*comp+k]/255.0f, stbi__l2h_gamma) * stbi__l2h_scale);
	  }
	  if (k < comp) output[i*comp + k] = data[i*comp+k]/255.0f;
   }
   STBI_FREE(data);
   return output;
}
#endif

#ifndef STBI_NO_HDR
#define stbi__float2int(x)   ((int) (x))
static stbi_uc *stbi__hdr_to_ldr(float   *data, int x, int y, int comp)
{
   int i,k,n;
   stbi_uc *output = (stbi_uc *) stbi__malloc(x * y * comp);
   if (output == NULL) { STBI_FREE(data); return stbi__errpuc("outofmem", "Out of memory"); }
   // compute number of non-alpha components
   if (comp & 1) n = comp; else n = comp-1;
   for (i=0; i < x*y; ++i) {
	  for (k=0; k < n; ++k) {
		 float z = (float) pow(data[i*comp+k]*stbi__h2l_scale_i, stbi__h2l_gamma_i) * 255 + 0.5f;
		 if (z < 0) z = 0;
		 if (z > 255) z = 255;
		 output[i*comp + k] = (stbi_uc) stbi__float2int(z);
	  }
	  if (k < comp) {
		 float z = data[i*comp+k] * 255 + 0.5f;
		 if (z < 0) z = 0;
		 if (z > 255) z = 255;
		 output[i*comp + k] = (stbi_uc) stbi__float2int(z);
	  }
   }
   STBI_FREE(data);
   return output;
}
#endif

//////////////////////////////////////////////////////////////////////////////
//
//  "baseline" JPEG/JFIF decoder
//
//    simple implementation
//      - doesn't support delayed output of y-dimension
//      - simple interface (only one output format: 8-bit interleaved RGB)
//      - doesn't try to recover corrupt jpegs
//      - doesn't allow partial loading, loading multiple at once
//      - still fast on x86 (copying globals into locals doesn't help x86)
//      - allocates lots of intermediate memory (full size of all components)
//        - non-interleaved case requires this anyway
//        - allows good upsampling (see next)
//    high-quality
//      - upsampled channels are bilinearly interpolated, even across blocks
//      - quality integer IDCT derived from IJG's 'slow'
//    performance
//      - fast huffman; reasonable integer IDCT
//      - some SIMD kernels for common paths on targets with SSE2/NEON
//      - uses a lot of intermediate memory, could cache poorly

#ifndef STBI_NO_JPEG

// huffman decoding acceleration
#define FAST_BITS   9  // larger handles more cases; smaller stomps less cache

typedef struct
{
   stbi_uc  fast[1 << FAST_BITS];
   // weirdly, repacking this into AoS is a 10% speed loss, instead of a win
   stbi__uint16 code[256];
   stbi_uc  values[256];
   stbi_uc  size[257];
   unsigned int maxcode[18];
   int    delta[17];   // old 'firstsymbol' - old 'firstcode'
} stbi__huffman;

typedef struct
{
   stbi__context *s;
   stbi__huffman huff_dc[4];
   stbi__huffman huff_ac[4];
   stbi_uc dequant[4][64];
   stbi__int16 fast_ac[4][1 << FAST_BITS];

// sizes for components, interleaved MCUs
   int img_h_max, img_v_max;
   int img_mcu_x, img_mcu_y;
   int img_mcu_w, img_mcu_h;

// definition of jpeg image component
   struct
   {
	  int id;
	  int h,v;
	  int tq;
	  int hd,ha;
	  int dc_pred;

	  int x,y,w2,h2;
	  stbi_uc *data;
	  void *raw_data, *raw_coeff;
	  stbi_uc *linebuf;
	  short   *coeff;   // progressive only
	  int      coeff_w, coeff_h; // number of 8x8 coefficient blocks
   } img_comp[4];

   stbi__uint32   code_buffer; // jpeg entropy-coded buffer
   int            code_bits;   // number of valid bits
   unsigned char  marker;      // marker seen while filling entropy buffer
   int            nomore;      // flag if we saw a marker so must stop

   int            progressive;
   int            spec_start;
   int            spec_end;
   int            succ_high;
   int            succ_low;
   int            eob_run;

   int scan_n, order[4];
   int restart_interval, todo;

// kernels
   void (*idct_block_kernel)(stbi_uc *out, int out_stride, short data[64]);
   void (*YCbCr_to_RGB_kernel)(stbi_uc *out, const stbi_uc *y, const stbi_uc *pcb, const stbi_uc *pcr, int count, int step);
   stbi_uc *(*resample_row_hv_2_kernel)(stbi_uc *out, stbi_uc *in_near, stbi_uc *in_far, int w, int hs);
} stbi__jpeg;

static int stbi__build_huffman(stbi__huffman *h, int *count)
{
   int i,j,k=0,code;
   // build size list for each symbol (from JPEG spec)
   for (i=0; i < 16; ++i)
	  for (j=0; j < count[i]; ++j)
		 h->size[k++] = (stbi_uc) (i+1);
   h->size[k] = 0;

   // compute actual symbols (from jpeg spec)
   code = 0;
   k = 0;
   for(j=1; j <= 16; ++j) {
	  // compute delta to add to code to compute symbol id
	  h->delta[j] = k - code;
	  if (h->size[k] == j) {
		 while (h->size[k] == j)
			h->code[k++] = (stbi__uint16) (code++);
		 if (code-1 >= (1 << j)) return stbi__err("bad code lengths","Corrupt JPEG");
	  }
	  // compute largest code + 1 for this size, preshifted as needed later
	  h->maxcode[j] = code << (16-j);
	  code <<= 1;
   }
   h->maxcode[j] = 0xffffffff;

   // build non-spec acceleration table; 255 is flag for not-accelerated
   memset(h->fast, 255, 1 << FAST_BITS);
   for (i=0; i < k; ++i) {
	  int s = h->size[i];
	  if (s <= FAST_BITS) {
		 int c = h->code[i] << (FAST_BITS-s);
		 int m = 1 << (FAST_BITS-s);
		 for (j=0; j < m; ++j) {
			h->fast[c+j] = (stbi_uc) i;
		 }
	  }
   }
   return 1;
}

// build a table that decodes both magnitude and value of small ACs in
// one go.
static void stbi__build_fast_ac(stbi__int16 *fast_ac, stbi__huffman *h)
{
   int i;
   for (i=0; i < (1 << FAST_BITS); ++i) {
	  stbi_uc fast = h->fast[i];
	  fast_ac[i] = 0;
	  if (fast < 255) {
		 int rs = h->values[fast];
		 int run = (rs >> 4) & 15;
		 int magbits = rs & 15;
		 int len = h->size[fast];

		 if (magbits && len + magbits <= FAST_BITS) {
			// magnitude code followed by receive_extend code
			int k = ((i << len) & ((1 << FAST_BITS) - 1)) >> (FAST_BITS - magbits);
			int m = 1 << (magbits - 1);
			if (k < m) k += (-1 << magbits) + 1;
			// if the result is small enough, we can fit it in fast_ac table
			if (k >= -128 && k <= 127)
			   fast_ac[i] = (stbi__int16) ((k << 8) + (run << 4) + (len + magbits));
		 }
	  }
   }
}

static void stbi__grow_buffer_unsafe(stbi__jpeg *j)
{
   do {
	  int b = j->nomore ? 0 : stbi__get8(j->s);
	  if (b == 0xff) {
		 int c = stbi__get8(j->s);
		 if (c != 0) {
			j->marker = (unsigned char) c;
			j->nomore = 1;
			return;
		 }
	  }
	  j->code_buffer |= b << (24 - j->code_bits);
	  j->code_bits += 8;
   } while (j->code_bits <= 24);
}

// (1 << n) - 1
static stbi__uint32 stbi__bmask[17]={0,1,3,7,15,31,63,127,255,511,1023,2047,4095,8191,16383,32767,65535};

// decode a jpeg huffman value from the bitstream
stbi_inline static int stbi__jpeg_huff_decode(stbi__jpeg *j, stbi__huffman *h)
{
   unsigned int temp;
   int c,k;

   if (j->code_bits < 16) stbi__grow_buffer_unsafe(j);

   // look at the top FAST_BITS and determine what symbol ID it is,
   // if the code is <= FAST_BITS
   c = (j->code_buffer >> (32 - FAST_BITS)) & ((1 << FAST_BITS)-1);
   k = h->fast[c];
   if (k < 255) {
	  int s = h->size[k];
	  if (s > j->code_bits)
		 return -1;
	  j->code_buffer <<= s;
	  j->code_bits -= s;
	  return h->values[k];
   }

   // naive test is to shift the code_buffer down so k bits are
   // valid, then test against maxcode. To speed this up, we've
   // preshifted maxcode left so that it has (16-k) 0s at the
   // end; in other words, regardless of the number of bits, it
   // wants to be compared against something shifted to have 16;
   // that way we don't need to shift inside the loop.
   temp = j->code_buffer >> 16;
   for (k=FAST_BITS+1 ; ; ++k)
	  if (temp < h->maxcode[k])
		 break;
   if (k == 17) {
	  // error! code not found
	  j->code_bits -= 16;
	  return -1;
   }

   if (k > j->code_bits)
	  return -1;

   // convert the huffman code to the symbol id
   c = ((j->code_buffer >> (32 - k)) & stbi__bmask[k]) + h->delta[k];
   STBI_ASSERT((((j->code_buffer) >> (32 - h->size[c])) & stbi__bmask[h->size[c]]) == h->code[c]);

   // convert the id to a symbol
   j->code_bits -= k;
   j->code_buffer <<= k;
   return h->values[c];
}

// bias[n] = (-1<<n) + 1
static int const stbi__jbias[16] = {0,-1,-3,-7,-15,-31,-63,-127,-255,-511,-1023,-2047,-4095,-8191,-16383,-32767};

// combined JPEG 'receive' and JPEG 'extend', since baseline
// always extends everything it receives.
stbi_inline static int stbi__extend_receive(stbi__jpeg *j, int n)
{
   unsigned int k;
   int sgn;
   if (j->code_bits < n) stbi__grow_buffer_unsafe(j);

   sgn = (stbi__int32)j->code_buffer >> 31; // sign bit is always in MSB
   k = stbi_lrot(j->code_buffer, n);
   j->code_buffer = k & ~stbi__bmask[n];
   k &= stbi__bmask[n];
   j->code_bits -= n;
   return k + (stbi__jbias[n] & ~sgn);
}

// get some unsigned bits
stbi_inline static int stbi__jpeg_get_bits(stbi__jpeg *j, int n)
{
   unsigned int k;
   if (j->code_bits < n) stbi__grow_buffer_unsafe(j);
   k = stbi_lrot(j->code_buffer, n);
   j->code_buffer = k & ~stbi__bmask[n];
   k &= stbi__bmask[n];
   j->code_bits -= n;
   return k;
}

stbi_inline static int stbi__jpeg_get_bit(stbi__jpeg *j)
{
   unsigned int k;
   if (j->code_bits < 1) stbi__grow_buffer_unsafe(j);
   k = j->code_buffer;
   j->code_buffer <<= 1;
   --j->code_bits;
   return k & 0x80000000;
}

// given a value that's at position X in the zigzag stream,
// where does it appear in the 8x8 matrix coded as row-major?
static stbi_uc stbi__jpeg_dezigzag[64+15] =
{
	0,  1,  8, 16,  9,  2,  3, 10,
   17, 24, 32, 25, 18, 11,  4,  5,
   12, 19, 26, 33, 40, 48, 41, 34,
   27, 20, 13,  6,  7, 14, 21, 28,
   35, 42, 49, 56, 57, 50, 43, 36,
   29, 22, 15, 23, 30, 37, 44, 51,
   58, 59, 52, 45, 38, 31, 39, 46,
   53, 60, 61, 54, 47, 55, 62, 63,
   // let corrupt input sample past end
   63, 63, 63, 63, 63, 63, 63, 63,
   63, 63, 63, 63, 63, 63, 63
};

// decode one 64-entry block--
static int stbi__jpeg_decode_block(stbi__jpeg *j, short data[64], stbi__huffman *hdc, stbi__huffman *hac, stbi__int16 *fac, int b, stbi_uc *dequant)
{
   int diff,dc,k;
   int t;

   if (j->code_bits < 16) stbi__grow_buffer_unsafe(j);
   t = stbi__jpeg_huff_decode(j, hdc);
   if (t < 0) return stbi__err("bad huffman code","Corrupt JPEG");

   // 0 all the ac values now so we can do it 32-bits at a time
   memset(data,0,64*sizeof(data[0]));

   diff = t ? stbi__extend_receive(j, t) : 0;
   dc = j->img_comp[b].dc_pred + diff;
   j->img_comp[b].dc_pred = dc;
   data[0] = (short) (dc * dequant[0]);

   // decode AC components, see JPEG spec
   k = 1;
   do {
	  unsigned int zig;
	  int c,r,s;
	  if (j->code_bits < 16) stbi__grow_buffer_unsafe(j);
	  c = (j->code_buffer >> (32 - FAST_BITS)) & ((1 << FAST_BITS)-1);
	  r = fac[c];
	  if (r) { // fast-AC path
		 k += (r >> 4) & 15; // run
		 s = r & 15; // combined length
		 j->code_buffer <<= s;
		 j->code_bits -= s;
		 // decode into unzigzag'd location
		 zig = stbi__jpeg_dezigzag[k++];
		 data[zig] = (short) ((r >> 8) * dequant[zig]);
	  } else {
		 int rs = stbi__jpeg_huff_decode(j, hac);
		 if (rs < 0) return stbi__err("bad huffman code","Corrupt JPEG");
		 s = rs & 15;
		 r = rs >> 4;
		 if (s == 0) {
			if (rs != 0xf0) break; // end block
			k += 16;
		 } else {
			k += r;
			// decode into unzigzag'd location
			zig = stbi__jpeg_dezigzag[k++];
			data[zig] = (short) (stbi__extend_receive(j,s) * dequant[zig]);
		 }
	  }
   } while (k < 64);
   return 1;
}

static int stbi__jpeg_decode_block_prog_dc(stbi__jpeg *j, short data[64], stbi__huffman *hdc, int b)
{
   int diff,dc;
   int t;
   if (j->spec_end != 0) return stbi__err("can't merge dc and ac", "Corrupt JPEG");

   if (j->code_bits < 16) stbi__grow_buffer_unsafe(j);

   if (j->succ_high == 0) {
	  // first scan for DC coefficient, must be first
	  memset(data,0,64*sizeof(data[0])); // 0 all the ac values now
	  t = stbi__jpeg_huff_decode(j, hdc);
	  diff = t ? stbi__extend_receive(j, t) : 0;

	  dc = j->img_comp[b].dc_pred + diff;
	  j->img_comp[b].dc_pred = dc;
	  data[0] = (short) (dc << j->succ_low);
   } else {
	  // refinement scan for DC coefficient
	  if (stbi__jpeg_get_bit(j))
		 data[0] += (short) (1 << j->succ_low);
   }
   return 1;
}

// @OPTIMIZE: store non-zigzagged during the decode passes,
// and only de-zigzag when dequantizing
static int stbi__jpeg_decode_block_prog_ac(stbi__jpeg *j, short data[64], stbi__huffman *hac, stbi__int16 *fac)
{
   int k;
   if (j->spec_start == 0) return stbi__err("can't merge dc and ac", "Corrupt JPEG");

   if (j->succ_high == 0) {
	  int shift = j->succ_low;

	  if (j->eob_run) {
		 --j->eob_run;
		 return 1;
	  }

	  k = j->spec_start;
	  do {
		 unsigned int zig;
		 int c,r,s;
		 if (j->code_bits < 16) stbi__grow_buffer_unsafe(j);
		 c = (j->code_buffer >> (32 - FAST_BITS)) & ((1 << FAST_BITS)-1);
		 r = fac[c];
		 if (r) { // fast-AC path
			k += (r >> 4) & 15; // run
			s = r & 15; // combined length
			j->code_buffer <<= s;
			j->code_bits -= s;
			zig = stbi__jpeg_dezigzag[k++];
			data[zig] = (short) ((r >> 8) << shift);
		 } else {
			int rs = stbi__jpeg_huff_decode(j, hac);
			if (rs < 0) return stbi__err("bad huffman code","Corrupt JPEG");
			s = rs & 15;
			r = rs >> 4;
			if (s == 0) {
			   if (r < 15) {
				  j->eob_run = (1 << r);
				  if (r)
					 j->eob_run += stbi__jpeg_get_bits(j, r);
				  --j->eob_run;
				  break;
			   }
			   k += 16;
			} else {
			   k += r;
			   zig = stbi__jpeg_dezigzag[k++];
			   data[zig] = (short) (stbi__extend_receive(j,s) << shift);
			}
		 }
	  } while (k <= j->spec_end);
   } else {
	  // refinement scan for these AC coefficients

	  short bit = (short) (1 << j->succ_low);

	  if (j->eob_run) {
		 --j->eob_run;
		 for (k = j->spec_start; k <= j->spec_end; ++k) {
			short *p = &data[stbi__jpeg_dezigzag[k]];
			if (*p != 0)
			   if (stbi__jpeg_get_bit(j))
				  if ((*p & bit)==0){
					 if (*p > 0)
						*p += bit;
					 else
						*p -= bit;
				  }
		 }
	  } else {
		 k = j->spec_start;
		 do {
			int r,s;
			int rs = stbi__jpeg_huff_decode(j, hac); // @OPTIMIZE see if we can use the fast path here, advance-by-r is so slow, eh
			if (rs < 0) return stbi__err("bad huffman code","Corrupt JPEG");
			s = rs & 15;
			r = rs >> 4;
			if (s == 0) {
			   if (r < 15) {
				  j->eob_run = (1 << r) - 1;
				  if (r)
					 j->eob_run += stbi__jpeg_get_bits(j, r);
				  r = 64; // force end of block
			   } else
				  r = 16; // r=15 is the code for 16 0s
			} else {
			   if (s != 1) return stbi__err("bad huffman code", "Corrupt JPEG");
			   // sign bit
			   if (stbi__jpeg_get_bit(j))
				  s = bit;
			   else
				  s = -bit;
			}

			// advance by r
			while (k <= j->spec_end) {
			   short *p = &data[stbi__jpeg_dezigzag[k]];
			   if (*p != 0) {
				  if (stbi__jpeg_get_bit(j))
					 if ((*p & bit)==0){
						if (*p > 0)
						   *p += bit;
						else
						   *p -= bit;
					 }
				  ++k;
			   } else {
				  if (r == 0) {
					 if (s)
						data[stbi__jpeg_dezigzag[k++]] = (short) s;
					 break;
				  }
				  --r;
				  ++k;
			   }
			}
		 } while (k <= j->spec_end);
	  }
   }
   return 1;
}

// take a -128..127 value and stbi__clamp it and convert to 0..255
stbi_inline static stbi_uc stbi__clamp(int x)
{
   // trick to use a single test to catch both cases
   if ((unsigned int) x > 255) {
	  if (x < 0) return 0;
	  if (x > 255) return 255;
   }
   return (stbi_uc) x;
}

#define stbi__f2f(x)  ((int) (((x) * 4096 + 0.5)))
#define stbi__fsh(x)  ((x) << 12)

// derived from jidctint -- DCT_ISLOW
#define STBI__IDCT_1D(s0,s1,s2,s3,s4,s5,s6,s7) \
   int t0,t1,t2,t3,p1,p2,p3,p4,p5,x0,x1,x2,x3; \
   p2 = s2;                                    \
   p3 = s6;                                    \
   p1 = (p2+p3) * stbi__f2f(0.5411961f);       \
   t2 = p1 + p3*stbi__f2f(-1.847759065f);      \
   t3 = p1 + p2*stbi__f2f( 0.765366865f);      \
   p2 = s0;                                    \
   p3 = s4;                                    \
   t0 = stbi__fsh(p2+p3);                      \
   t1 = stbi__fsh(p2-p3);                      \
   x0 = t0+t3;                                 \
   x3 = t0-t3;                                 \
   x1 = t1+t2;                                 \
   x2 = t1-t2;                                 \
   t0 = s7;                                    \
   t1 = s5;                                    \
   t2 = s3;                                    \
   t3 = s1;                                    \
   p3 = t0+t2;                                 \
   p4 = t1+t3;                                 \
   p1 = t0+t3;                                 \
   p2 = t1+t2;                                 \
   p5 = (p3+p4)*stbi__f2f( 1.175875602f);      \
   t0 = t0*stbi__f2f( 0.298631336f);           \
   t1 = t1*stbi__f2f( 2.053119869f);           \
   t2 = t2*stbi__f2f( 3.072711026f);           \
   t3 = t3*stbi__f2f( 1.501321110f);           \
   p1 = p5 + p1*stbi__f2f(-0.899976223f);      \
   p2 = p5 + p2*stbi__f2f(-2.562915447f);      \
   p3 = p3*stbi__f2f(-1.961570560f);           \
   p4 = p4*stbi__f2f(-0.390180644f);           \
   t3 += p1+p4;                                \
   t2 += p2+p3;                                \
   t1 += p2+p4;                                \
   t0 += p1+p3;

static void stbi__idct_block(stbi_uc *out, int out_stride, short data[64])
{
   int i,val[64],*v=val;
   stbi_uc *o;
   short *d = data;

   // columns
   for (i=0; i < 8; ++i,++d, ++v) {
	  // if all zeroes, shortcut -- this avoids dequantizing 0s and IDCTing
	  if (d[ 8]==0 && d[16]==0 && d[24]==0 && d[32]==0
		   && d[40]==0 && d[48]==0 && d[56]==0) {
		 //    no shortcut                 0     seconds
		 //    (1|2|3|4|5|6|7)==0          0     seconds
		 //    all separate               -0.047 seconds
		 //    1 && 2|3 && 4|5 && 6|7:    -0.047 seconds
		 int dcterm = d[0] << 2;
		 v[0] = v[8] = v[16] = v[24] = v[32] = v[40] = v[48] = v[56] = dcterm;
	  } else {
		 STBI__IDCT_1D(d[ 0],d[ 8],d[16],d[24],d[32],d[40],d[48],d[56])
		 // constants scaled things up by 1<<12; let's bring them back
		 // down, but keep 2 extra bits of precision
		 x0 += 512; x1 += 512; x2 += 512; x3 += 512;
		 v[ 0] = (x0+t3) >> 10;
		 v[56] = (x0-t3) >> 10;
		 v[ 8] = (x1+t2) >> 10;
		 v[48] = (x1-t2) >> 10;
		 v[16] = (x2+t1) >> 10;
		 v[40] = (x2-t1) >> 10;
		 v[24] = (x3+t0) >> 10;
		 v[32] = (x3-t0) >> 10;
	  }
   }

   for (i=0, v=val, o=out; i < 8; ++i,v+=8,o+=out_stride) {
	  // no fast case since the first 1D IDCT spread components out
	  STBI__IDCT_1D(v[0],v[1],v[2],v[3],v[4],v[5],v[6],v[7])
	  // constants scaled things up by 1<<12, plus we had 1<<2 from first
	  // loop, plus horizontal and vertical each scale by sqrt(8) so together
	  // we've got an extra 1<<3, so 1<<17 total we need to remove.
	  // so we want to round that, which means adding 0.5 * 1<<17,
	  // aka 65536. Also, we'll end up with -128 to 127 that we want
	  // to encode as 0..255 by adding 128, so we'll add that before the shift
	  x0 += 65536 + (128<<17);
	  x1 += 65536 + (128<<17);
	  x2 += 65536 + (128<<17);
	  x3 += 65536 + (128<<17);
	  // tried computing the shifts into temps, or'ing the temps to see
	  // if any were out of range, but that was slower
	  o[0] = stbi__clamp((x0+t3) >> 17);
	  o[7] = stbi__clamp((x0-t3) >> 17);
	  o[1] = stbi__clamp((x1+t2) >> 17);
	  o[6] = stbi__clamp((x1-t2) >> 17);
	  o[2] = stbi__clamp((x2+t1) >> 17);
	  o[5] = stbi__clamp((x2-t1) >> 17);
	  o[3] = stbi__clamp((x3+t0) >> 17);
	  o[4] = stbi__clamp((x3-t0) >> 17);
   }
}

#ifdef STBI_SSE2
// sse2 integer IDCT. not the fastest possible implementation but it
// produces bit-identical results to the generic C version so it's
// fully "transparent".
static void stbi__idct_simd(stbi_uc *out, int out_stride, short data[64])
{
   // This is constructed to match our regular (generic) integer IDCT exactly.
   __m128i row0, row1, row2, row3, row4, row5, row6, row7;
   __m128i tmp;

   // dot product constant: even elems=x, odd elems=y
   #define dct_const(x,y)  _mm_setr_epi16((x),(y),(x),(y),(x),(y),(x),(y))

   // out(0) = c0[even]*x + c0[odd]*y   (c0, x, y 16-bit, out 32-bit)
   // out(1) = c1[even]*x + c1[odd]*y
   #define dct_rot(out0,out1, x,y,c0,c1) \
	  __m128i c0##lo = _mm_unpacklo_epi16((x),(y)); \
	  __m128i c0##hi = _mm_unpackhi_epi16((x),(y)); \
	  __m128i out0##_l = _mm_madd_epi16(c0##lo, c0); \
	  __m128i out0##_h = _mm_madd_epi16(c0##hi, c0); \
	  __m128i out1##_l = _mm_madd_epi16(c0##lo, c1); \
	  __m128i out1##_h = _mm_madd_epi16(c0##hi, c1)

   // out = in << 12  (in 16-bit, out 32-bit)
   #define dct_widen(out, in) \
	  __m128i out##_l = _mm_srai_epi32(_mm_unpacklo_epi16(_mm_setzero_si128(), (in)), 4); \
	  __m128i out##_h = _mm_srai_epi32(_mm_unpackhi_epi16(_mm_setzero_si128(), (in)), 4)

   // wide add
   #define dct_wadd(out, a, b) \
	  __m128i out##_l = _mm_add_epi32(a##_l, b##_l); \
	  __m128i out##_h = _mm_add_epi32(a##_h, b##_h)

   // wide sub
   #define dct_wsub(out, a, b) \
	  __m128i out##_l = _mm_sub_epi32(a##_l, b##_l); \
	  __m128i out##_h = _mm_sub_epi32(a##_h, b##_h)

   // butterfly a/b, add bias, then shift by "s" and pack
   #define dct_bfly32o(out0, out1, a,b,bias,s) \
	  { \
		 __m128i abiased_l = _mm_add_epi32(a##_l, bias); \
		 __m128i abiased_h = _mm_add_epi32(a##_h, bias); \
		 dct_wadd(sum, abiased, b); \
		 dct_wsub(dif, abiased, b); \
		 out0 = _mm_packs_epi32(_mm_srai_epi32(sum_l, s), _mm_srai_epi32(sum_h, s)); \
		 out1 = _mm_packs_epi32(_mm_srai_epi32(dif_l, s), _mm_srai_epi32(dif_h, s)); \
	  }

   // 8-bit interleave step (for transposes)
   #define dct_interleave8(a, b) \
	  tmp = a; \
	  a = _mm_unpacklo_epi8(a, b); \
	  b = _mm_unpackhi_epi8(tmp, b)

   // 16-bit interleave step (for transposes)
   #define dct_interleave16(a, b) \
	  tmp = a; \
	  a = _mm_unpacklo_epi16(a, b); \
	  b = _mm_unpackhi_epi16(tmp, b)

   #define dct_pass(bias,shift) \
	  { \
		 /* even part */ \
		 dct_rot(t2e,t3e, row2,row6, rot0_0,rot0_1); \
		 __m128i sum04 = _mm_add_epi16(row0, row4); \
		 __m128i dif04 = _mm_sub_epi16(row0, row4); \
		 dct_widen(t0e, sum04); \
		 dct_widen(t1e, dif04); \
		 dct_wadd(x0, t0e, t3e); \
		 dct_wsub(x3, t0e, t3e); \
		 dct_wadd(x1, t1e, t2e); \
		 dct_wsub(x2, t1e, t2e); \
		 /* odd part */ \
		 dct_rot(y0o,y2o, row7,row3, rot2_0,rot2_1); \
		 dct_rot(y1o,y3o, row5,row1, rot3_0,rot3_1); \
		 __m128i sum17 = _mm_add_epi16(row1, row7); \
		 __m128i sum35 = _mm_add_epi16(row3, row5); \
		 dct_rot(y4o,y5o, sum17,sum35, rot1_0,rot1_1); \
		 dct_wadd(x4, y0o, y4o); \
		 dct_wadd(x5, y1o, y5o); \
		 dct_wadd(x6, y2o, y5o); \
		 dct_wadd(x7, y3o, y4o); \
		 dct_bfly32o(row0,row7, x0,x7,bias,shift); \
		 dct_bfly32o(row1,row6, x1,x6,bias,shift); \
		 dct_bfly32o(row2,row5, x2,x5,bias,shift); \
		 dct_bfly32o(row3,row4, x3,x4,bias,shift); \
	  }

   __m128i rot0_0 = dct_const(stbi__f2f(0.5411961f), stbi__f2f(0.5411961f) + stbi__f2f(-1.847759065f));
   __m128i rot0_1 = dct_const(stbi__f2f(0.5411961f) + stbi__f2f( 0.765366865f), stbi__f2f(0.5411961f));
   __m128i rot1_0 = dct_const(stbi__f2f(1.175875602f) + stbi__f2f(-0.899976223f), stbi__f2f(1.175875602f));
   __m128i rot1_1 = dct_const(stbi__f2f(1.175875602f), stbi__f2f(1.175875602f) + stbi__f2f(-2.562915447f));
   __m128i rot2_0 = dct_const(stbi__f2f(-1.961570560f) + stbi__f2f( 0.298631336f), stbi__f2f(-1.961570560f));
   __m128i rot2_1 = dct_const(stbi__f2f(-1.961570560f), stbi__f2f(-1.961570560f) + stbi__f2f( 3.072711026f));
   __m128i rot3_0 = dct_const(stbi__f2f(-0.390180644f) + stbi__f2f( 2.053119869f), stbi__f2f(-0.390180644f));
   __m128i rot3_1 = dct_const(stbi__f2f(-0.390180644f), stbi__f2f(-0.390180644f) + stbi__f2f( 1.501321110f));

   // rounding biases in column/row passes, see stbi__idct_block for explanation.
   __m128i bias_0 = _mm_set1_epi32(512);
   __m128i bias_1 = _mm_set1_epi32(65536 + (128<<17));

   // load
   row0 = _mm_load_si128((const __m128i *) (data + 0*8));
   row1 = _mm_load_si128((const __m128i *) (data + 1*8));
   row2 = _mm_load_si128((const __m128i *) (data + 2*8));
   row3 = _mm_load_si128((const __m128i *) (data + 3*8));
   row4 = _mm_load_si128((const __m128i *) (data + 4*8));
   row5 = _mm_load_si128((const __m128i *) (data + 5*8));
   row6 = _mm_load_si128((const __m128i *) (data + 6*8));
   row7 = _mm_load_si128((const __m128i *) (data + 7*8));

   // column pass
   dct_pass(bias_0, 10);

   {
	  // 16bit 8x8 transpose pass 1
	  dct_interleave16(row0, row4);
	  dct_interleave16(row1, row5);
	  dct_interleave16(row2, row6);
	  dct_interleave16(row3, row7);

	  // transpose pass 2
	  dct_interleave16(row0, row2);
	  dct_interleave16(row1, row3);
	  dct_interleave16(row4, row6);
	  dct_interleave16(row5, row7);

	  // transpose pass 3
	  dct_interleave16(row0, row1);
	  dct_interleave16(row2, row3);
	  dct_interleave16(row4, row5);
	  dct_interleave16(row6, row7);
   }

   // row pass
   dct_pass(bias_1, 17);

   {
	  // pack
	  __m128i p0 = _mm_packus_epi16(row0, row1); // a0a1a2a3...a7b0b1b2b3...b7
	  __m128i p1 = _mm_packus_epi16(row2, row3);
	  __m128i p2 = _mm_packus_epi16(row4, row5);
	  __m128i p3 = _mm_packus_epi16(row6, row7);

	  // 8bit 8x8 transpose pass 1
	  dct_interleave8(p0, p2); // a0e0a1e1...
	  dct_interleave8(p1, p3); // c0g0c1g1...

	  // transpose pass 2
	  dct_interleave8(p0, p1); // a0c0e0g0...
	  dct_interleave8(p2, p3); // b0d0f0h0...

	  // transpose pass 3
	  dct_interleave8(p0, p2); // a0b0c0d0...
	  dct_interleave8(p1, p3); // a4b4c4d4...

	  // store
	  _mm_storel_epi64((__m128i *) out, p0); out += out_stride;
	  _mm_storel_epi64((__m128i *) out, _mm_shuffle_epi32(p0, 0x4e)); out += out_stride;
	  _mm_storel_epi64((__m128i *) out, p2); out += out_stride;
	  _mm_storel_epi64((__m128i *) out, _mm_shuffle_epi32(p2, 0x4e)); out += out_stride;
	  _mm_storel_epi64((__m128i *) out, p1); out += out_stride;
	  _mm_storel_epi64((__m128i *) out, _mm_shuffle_epi32(p1, 0x4e)); out += out_stride;
	  _mm_storel_epi64((__m128i *) out, p3); out += out_stride;
	  _mm_storel_epi64((__m128i *) out, _mm_shuffle_epi32(p3, 0x4e));
   }

#undef dct_const
#undef dct_rot
#undef dct_widen
#undef dct_wadd
#undef dct_wsub
#undef dct_bfly32o
#undef dct_interleave8
#undef dct_interleave16
#undef dct_pass
}

#endif // STBI_SSE2

#ifdef STBI_NEON

// NEON integer IDCT. should produce bit-identical
// results to the generic C version.
static void stbi__idct_simd(stbi_uc *out, int out_stride, short data[64])
{
   int16x8_t row0, row1, row2, row3, row4, row5, row6, row7;

   int16x4_t rot0_0 = vdup_n_s16(stbi__f2f(0.5411961f));
   int16x4_t rot0_1 = vdup_n_s16(stbi__f2f(-1.847759065f));
   int16x4_t rot0_2 = vdup_n_s16(stbi__f2f( 0.765366865f));
   int16x4_t rot1_0 = vdup_n_s16(stbi__f2f( 1.175875602f));
   int16x4_t rot1_1 = vdup_n_s16(stbi__f2f(-0.899976223f));
   int16x4_t rot1_2 = vdup_n_s16(stbi__f2f(-2.562915447f));
   int16x4_t rot2_0 = vdup_n_s16(stbi__f2f(-1.961570560f));
   int16x4_t rot2_1 = vdup_n_s16(stbi__f2f(-0.390180644f));
   int16x4_t rot3_0 = vdup_n_s16(stbi__f2f( 0.298631336f));
   int16x4_t rot3_1 = vdup_n_s16(stbi__f2f( 2.053119869f));
   int16x4_t rot3_2 = vdup_n_s16(stbi__f2f( 3.072711026f));
   int16x4_t rot3_3 = vdup_n_s16(stbi__f2f( 1.501321110f));

#define dct_long_mul(out, inq, coeff) \
   int32x4_t out##_l = vmull_s16(vget_low_s16(inq), coeff); \
   int32x4_t out##_h = vmull_s16(vget_high_s16(inq), coeff)

#define dct_long_mac(out, acc, inq, coeff) \
   int32x4_t out##_l = vmlal_s16(acc##_l, vget_low_s16(inq), coeff); \
   int32x4_t out##_h = vmlal_s16(acc##_h, vget_high_s16(inq), coeff)

#define dct_widen(out, inq) \
   int32x4_t out##_l = vshll_n_s16(vget_low_s16(inq), 12); \
   int32x4_t out##_h = vshll_n_s16(vget_high_s16(inq), 12)

// wide add
#define dct_wadd(out, a, b) \
   int32x4_t out##_l = vaddq_s32(a##_l, b##_l); \
   int32x4_t out##_h = vaddq_s32(a##_h, b##_h)

// wide sub
#define dct_wsub(out, a, b) \
   int32x4_t out##_l = vsubq_s32(a##_l, b##_l); \
   int32x4_t out##_h = vsubq_s32(a##_h, b##_h)

// butterfly a/b, then shift using "shiftop" by "s" and pack
#define dct_bfly32o(out0,out1, a,b,shiftop,s) \
   { \
	  dct_wadd(sum, a, b); \
	  dct_wsub(dif, a, b); \
	  out0 = vcombine_s16(shiftop(sum_l, s), shiftop(sum_h, s)); \
	  out1 = vcombine_s16(shiftop(dif_l, s), shiftop(dif_h, s)); \
   }

#define dct_pass(shiftop, shift) \
   { \
	  /* even part */ \
	  int16x8_t sum26 = vaddq_s16(row2, row6); \
	  dct_long_mul(p1e, sum26, rot0_0); \
	  dct_long_mac(t2e, p1e, row6, rot0_1); \
	  dct_long_mac(t3e, p1e, row2, rot0_2); \
	  int16x8_t sum04 = vaddq_s16(row0, row4); \
	  int16x8_t dif04 = vsubq_s16(row0, row4); \
	  dct_widen(t0e, sum04); \
	  dct_widen(t1e, dif04); \
	  dct_wadd(x0, t0e, t3e); \
	  dct_wsub(x3, t0e, t3e); \
	  dct_wadd(x1, t1e, t2e); \
	  dct_wsub(x2, t1e, t2e); \
	  /* odd part */ \
	  int16x8_t sum15 = vaddq_s16(row1, row5); \
	  int16x8_t sum17 = vaddq_s16(row1, row7); \
	  int16x8_t sum35 = vaddq_s16(row3, row5); \
	  int16x8_t sum37 = vaddq_s16(row3, row7); \
	  int16x8_t sumodd = vaddq_s16(sum17, sum35); \
	  dct_long_mul(p5o, sumodd, rot1_0); \
	  dct_long_mac(p1o, p5o, sum17, rot1_1); \
	  dct_long_mac(p2o, p5o, sum35, rot1_2); \
	  dct_long_mul(p3o, sum37, rot2_0); \
	  dct_long_mul(p4o, sum15, rot2_1); \
	  dct_wadd(sump13o, p1o, p3o); \
	  dct_wadd(sump24o, p2o, p4o); \
	  dct_wadd(sump23o, p2o, p3o); \
	  dct_wadd(sump14o, p1o, p4o); \
	  dct_long_mac(x4, sump13o, row7, rot3_0); \
	  dct_long_mac(x5, sump24o, row5, rot3_1); \
	  dct_long_mac(x6, sump23o, row3, rot3_2); \
	  dct_long_mac(x7, sump14o, row1, rot3_3); \
	  dct_bfly32o(row0,row7, x0,x7,shiftop,shift); \
	  dct_bfly32o(row1,row6, x1,x6,shiftop,shift); \
	  dct_bfly32o(row2,row5, x2,x5,shiftop,shift); \
	  dct_bfly32o(row3,row4, x3,x4,shiftop,shift); \
   }

   // load
   row0 = vld1q_s16(data + 0*8);
   row1 = vld1q_s16(data + 1*8);
   row2 = vld1q_s16(data + 2*8);
   row3 = vld1q_s16(data + 3*8);
   row4 = vld1q_s16(data + 4*8);
   row5 = vld1q_s16(data + 5*8);
   row6 = vld1q_s16(data + 6*8);
   row7 = vld1q_s16(data + 7*8);

   // add DC bias
   row0 = vaddq_s16(row0, vsetq_lane_s16(1024, vdupq_n_s16(0), 0));

   // column pass
   dct_pass(vrshrn_n_s32, 10);

   // 16bit 8x8 transpose
   {
// these three map to a single VTRN.16, VTRN.32, and VSWP, respectively.
// whether compilers actually get this is another story, sadly.
#define dct_trn16(x, y) { int16x8x2_t t = vtrnq_s16(x, y); x = t.val[0]; y = t.val[1]; }
#define dct_trn32(x, y) { int32x4x2_t t = vtrnq_s32(vreinterpretq_s32_s16(x), vreinterpretq_s32_s16(y)); x = vreinterpretq_s16_s32(t.val[0]); y = vreinterpretq_s16_s32(t.val[1]); }
#define dct_trn64(x, y) { int16x8_t x0 = x; int16x8_t y0 = y; x = vcombine_s16(vget_low_s16(x0), vget_low_s16(y0)); y = vcombine_s16(vget_high_s16(x0), vget_high_s16(y0)); }

	  // pass 1
	  dct_trn16(row0, row1); // a0b0a2b2a4b4a6b6
	  dct_trn16(row2, row3);
	  dct_trn16(row4, row5);
	  dct_trn16(row6, row7);

	  // pass 2
	  dct_trn32(row0, row2); // a0b0c0d0a4b4c4d4
	  dct_trn32(row1, row3);
	  dct_trn32(row4, row6);
	  dct_trn32(row5, row7);

	  // pass 3
	  dct_trn64(row0, row4); // a0b0c0d0e0f0g0h0
	  dct_trn64(row1, row5);
	  dct_trn64(row2, row6);
	  dct_trn64(row3, row7);

#undef dct_trn16
#undef dct_trn32
#undef dct_trn64
   }

   // row pass
   // vrshrn_n_s32 only supports shifts up to 16, we need
   // 17. so do a non-rounding shift of 16 first then follow
   // up with a rounding shift by 1.
   dct_pass(vshrn_n_s32, 16);

   {
	  // pack and round
	  uint8x8_t p0 = vqrshrun_n_s16(row0, 1);
	  uint8x8_t p1 = vqrshrun_n_s16(row1, 1);
	  uint8x8_t p2 = vqrshrun_n_s16(row2, 1);
	  uint8x8_t p3 = vqrshrun_n_s16(row3, 1);
	  uint8x8_t p4 = vqrshrun_n_s16(row4, 1);
	  uint8x8_t p5 = vqrshrun_n_s16(row5, 1);
	  uint8x8_t p6 = vqrshrun_n_s16(row6, 1);
	  uint8x8_t p7 = vqrshrun_n_s16(row7, 1);

	  // again, these can translate into one instruction, but often don't.
#define dct_trn8_8(x, y) { uint8x8x2_t t = vtrn_u8(x, y); x = t.val[0]; y = t.val[1]; }
#define dct_trn8_16(x, y) { uint16x4x2_t t = vtrn_u16(vreinterpret_u16_u8(x), vreinterpret_u16_u8(y)); x = vreinterpret_u8_u16(t.val[0]); y = vreinterpret_u8_u16(t.val[1]); }
#define dct_trn8_32(x, y) { uint32x2x2_t t = vtrn_u32(vreinterpret_u32_u8(x), vreinterpret_u32_u8(y)); x = vreinterpret_u8_u32(t.val[0]); y = vreinterpret_u8_u32(t.val[1]); }

	  // sadly can't use interleaved stores here since we only write
	  // 8 bytes to each scan line!

	  // 8x8 8-bit transpose pass 1
	  dct_trn8_8(p0, p1);
	  dct_trn8_8(p2, p3);
	  dct_trn8_8(p4, p5);
	  dct_trn8_8(p6, p7);

	  // pass 2
	  dct_trn8_16(p0, p2);
	  dct_trn8_16(p1, p3);
	  dct_trn8_16(p4, p6);
	  dct_trn8_16(p5, p7);

	  // pass 3
	  dct_trn8_32(p0, p4);
	  dct_trn8_32(p1, p5);
	  dct_trn8_32(p2, p6);
	  dct_trn8_32(p3, p7);

	  // store
	  vst1_u8(out, p0); out += out_stride;
	  vst1_u8(out, p1); out += out_stride;
	  vst1_u8(out, p2); out += out_stride;
	  vst1_u8(out, p3); out += out_stride;
	  vst1_u8(out, p4); out += out_stride;
	  vst1_u8(out, p5); out += out_stride;
	  vst1_u8(out, p6); out += out_stride;
	  vst1_u8(out, p7);

#undef dct_trn8_8
#undef dct_trn8_16
#undef dct_trn8_32
   }

#undef dct_long_mul
#undef dct_long_mac
#undef dct_widen
#undef dct_wadd
#undef dct_wsub
#undef dct_bfly32o
#undef dct_pass
}

#endif // STBI_NEON

#define STBI__MARKER_none  0xff
// if there's a pending marker from the entropy stream, return that
// otherwise, fetch from the stream and get a marker. if there's no
// marker, return 0xff, which is never a valid marker value
static stbi_uc stbi__get_marker(stbi__jpeg *j)
{
   stbi_uc x;
   if (j->marker != STBI__MARKER_none) { x = j->marker; j->marker = STBI__MARKER_none; return x; }
   x = stbi__get8(j->s);
   if (x != 0xff) return STBI__MARKER_none;
   while (x == 0xff)
	  x = stbi__get8(j->s);
   return x;
}

// in each scan, we'll have scan_n components, and the order
// of the components is specified by order[]
#define STBI__RESTART(x)     ((x) >= 0xd0 && (x) <= 0xd7)

// after a restart interval, stbi__jpeg_reset the entropy decoder and
// the dc prediction
static void stbi__jpeg_reset(stbi__jpeg *j)
{
   j->code_bits = 0;
   j->code_buffer = 0;
   j->nomore = 0;
   j->img_comp[0].dc_pred = j->img_comp[1].dc_pred = j->img_comp[2].dc_pred = 0;
   j->marker = STBI__MARKER_none;
   j->todo = j->restart_interval ? j->restart_interval : 0x7fffffff;
   j->eob_run = 0;
   // no more than 1<<31 MCUs if no restart_interal? that's plenty safe,
   // since we don't even allow 1<<30 pixels
}

static int stbi__parse_entropy_coded_data(stbi__jpeg *z)
{
   stbi__jpeg_reset(z);
   if (!z->progressive) {
	  if (z->scan_n == 1) {
		 int i,j;
		 STBI_SIMD_ALIGN(short, data[64]);
		 int n = z->order[0];
		 // non-interleaved data, we just need to process one block at a time,
		 // in trivial scanline order
		 // number of blocks to do just depends on how many actual "pixels" this
		 // component has, independent of interleaved MCU blocking and such
		 int w = (z->img_comp[n].x+7) >> 3;
		 int h = (z->img_comp[n].y+7) >> 3;
		 for (j=0; j < h; ++j) {
			for (i=0; i < w; ++i) {
			   int ha = z->img_comp[n].ha;
			   if (!stbi__jpeg_decode_block(z, data, z->huff_dc+z->img_comp[n].hd, z->huff_ac+ha, z->fast_ac[ha], n, z->dequant[z->img_comp[n].tq])) return 0;
			   z->idct_block_kernel(z->img_comp[n].data+z->img_comp[n].w2*j*8+i*8, z->img_comp[n].w2, data);
			   // every data block is an MCU, so countdown the restart interval
			   if (--z->todo <= 0) {
				  if (z->code_bits < 24) stbi__grow_buffer_unsafe(z);
				  // if it's NOT a restart, then just bail, so we get corrupt data
				  // rather than no data
				  if (!STBI__RESTART(z->marker)) return 1;
				  stbi__jpeg_reset(z);
			   }
			}
		 }
		 return 1;
	  } else { // interleaved
		 int i,j,k,x,y;
		 STBI_SIMD_ALIGN(short, data[64]);
		 for (j=0; j < z->img_mcu_y; ++j) {
			for (i=0; i < z->img_mcu_x; ++i) {
			   // scan an interleaved mcu... process scan_n components in order
			   for (k=0; k < z->scan_n; ++k) {
				  int n = z->order[k];
				  // scan out an mcu's worth of this component; that's just determined
				  // by the basic H and V specified for the component
				  for (y=0; y < z->img_comp[n].v; ++y) {
					 for (x=0; x < z->img_comp[n].h; ++x) {
						int x2 = (i*z->img_comp[n].h + x)*8;
						int y2 = (j*z->img_comp[n].v + y)*8;
						int ha = z->img_comp[n].ha;
						if (!stbi__jpeg_decode_block(z, data, z->huff_dc+z->img_comp[n].hd, z->huff_ac+ha, z->fast_ac[ha], n, z->dequant[z->img_comp[n].tq])) return 0;
						z->idct_block_kernel(z->img_comp[n].data+z->img_comp[n].w2*y2+x2, z->img_comp[n].w2, data);
					 }
				  }
			   }
			   // after all interleaved components, that's an interleaved MCU,
			   // so now count down the restart interval
			   if (--z->todo <= 0) {
				  if (z->code_bits < 24) stbi__grow_buffer_unsafe(z);
				  if (!STBI__RESTART(z->marker)) return 1;
				  stbi__jpeg_reset(z);
			   }
			}
		 }
		 return 1;
	  }
   } else {
	  if (z->scan_n == 1) {
		 int i,j;
		 int n = z->order[0];
		 // non-interleaved data, we just need to process one block at a time,
		 // in trivial scanline order
		 // number of blocks to do just depends on how many actual "pixels" this
		 // component has, independent of interleaved MCU blocking and such
		 int w = (z->img_comp[n].x+7) >> 3;
		 int h = (z->img_comp[n].y+7) >> 3;
		 for (j=0; j < h; ++j) {
			for (i=0; i < w; ++i) {
			   short *data = z->img_comp[n].coeff + 64 * (i + j * z->img_comp[n].coeff_w);
			   if (z->spec_start == 0) {
				  if (!stbi__jpeg_decode_block_prog_dc(z, data, &z->huff_dc[z->img_comp[n].hd], n))
					 return 0;
			   } else {
				  int ha = z->img_comp[n].ha;
				  if (!stbi__jpeg_decode_block_prog_ac(z, data, &z->huff_ac[ha], z->fast_ac[ha]))
					 return 0;
			   }
			   // every data block is an MCU, so countdown the restart interval
			   if (--z->todo <= 0) {
				  if (z->code_bits < 24) stbi__grow_buffer_unsafe(z);
				  if (!STBI__RESTART(z->marker)) return 1;
				  stbi__jpeg_reset(z);
			   }
			}
		 }
		 return 1;
	  } else { // interleaved
		 int i,j,k,x,y;
		 for (j=0; j < z->img_mcu_y; ++j) {
			for (i=0; i < z->img_mcu_x; ++i) {
			   // scan an interleaved mcu... process scan_n components in order
			   for (k=0; k < z->scan_n; ++k) {
				  int n = z->order[k];
				  // scan out an mcu's worth of this component; that's just determined
				  // by the basic H and V specified for the component
				  for (y=0; y < z->img_comp[n].v; ++y) {
					 for (x=0; x < z->img_comp[n].h; ++x) {
						int x2 = (i*z->img_comp[n].h + x);
						int y2 = (j*z->img_comp[n].v + y);
						short *data = z->img_comp[n].coeff + 64 * (x2 + y2 * z->img_comp[n].coeff_w);
						if (!stbi__jpeg_decode_block_prog_dc(z, data, &z->huff_dc[z->img_comp[n].hd], n))
						   return 0;
					 }
				  }
			   }
			   // after all interleaved components, that's an interleaved MCU,
			   // so now count down the restart interval
			   if (--z->todo <= 0) {
				  if (z->code_bits < 24) stbi__grow_buffer_unsafe(z);
				  if (!STBI__RESTART(z->marker)) return 1;
				  stbi__jpeg_reset(z);
			   }
			}
		 }
		 return 1;
	  }
   }
}

static void stbi__jpeg_dequantize(short *data, stbi_uc *dequant)
{
   int i;
   for (i=0; i < 64; ++i)
	  data[i] *= dequant[i];
}

static void stbi__jpeg_finish(stbi__jpeg *z)
{
   if (z->progressive) {
	  // dequantize and idct the data
	  int i,j,n;
	  for (n=0; n < z->s->img_n; ++n) {
		 int w = (z->img_comp[n].x+7) >> 3;
		 int h = (z->img_comp[n].y+7) >> 3;
		 for (j=0; j < h; ++j) {
			for (i=0; i < w; ++i) {
			   short *data = z->img_comp[n].coeff + 64 * (i + j * z->img_comp[n].coeff_w);
			   stbi__jpeg_dequantize(data, z->dequant[z->img_comp[n].tq]);
			   z->idct_block_kernel(z->img_comp[n].data+z->img_comp[n].w2*j*8+i*8, z->img_comp[n].w2, data);
			}
		 }
	  }
   }
}

static int stbi__process_marker(stbi__jpeg *z, int m)
{
   int L;
   switch (m) {
	  case STBI__MARKER_none: // no marker found
		 return stbi__err("expected marker","Corrupt JPEG");

	  case 0xDD: // DRI - specify restart interval
		 if (stbi__get16be(z->s) != 4) return stbi__err("bad DRI len","Corrupt JPEG");
		 z->restart_interval = stbi__get16be(z->s);
		 return 1;

	  case 0xDB: // DQT - define quantization table
		 L = stbi__get16be(z->s)-2;
		 while (L > 0) {
			int q = stbi__get8(z->s);
			int p = q >> 4;
			int t = q & 15,i;
			if (p != 0) return stbi__err("bad DQT type","Corrupt JPEG");
			if (t > 3) return stbi__err("bad DQT table","Corrupt JPEG");
			for (i=0; i < 64; ++i)
			   z->dequant[t][stbi__jpeg_dezigzag[i]] = stbi__get8(z->s);
			L -= 65;
		 }
		 return L==0;

	  case 0xC4: // DHT - define huffman table
		 L = stbi__get16be(z->s)-2;
		 while (L > 0) {
			stbi_uc *v;
			int sizes[16],i,n=0;
			int q = stbi__get8(z->s);
			int tc = q >> 4;
			int th = q & 15;
			if (tc > 1 || th > 3) return stbi__err("bad DHT header","Corrupt JPEG");
			for (i=0; i < 16; ++i) {
			   sizes[i] = stbi__get8(z->s);
			   n += sizes[i];
			}
			L -= 17;
			if (tc == 0) {
			   if (!stbi__build_huffman(z->huff_dc+th, sizes)) return 0;
			   v = z->huff_dc[th].values;
			} else {
			   if (!stbi__build_huffman(z->huff_ac+th, sizes)) return 0;
			   v = z->huff_ac[th].values;
			}
			for (i=0; i < n; ++i)
			   v[i] = stbi__get8(z->s);
			if (tc != 0)
			   stbi__build_fast_ac(z->fast_ac[th], z->huff_ac + th);
			L -= n;
		 }
		 return L==0;
   }
   // check for comment block or APP blocks
   if ((m >= 0xE0 && m <= 0xEF) || m == 0xFE) {
	  stbi__skip(z->s, stbi__get16be(z->s)-2);
	  return 1;
   }
   return 0;
}

// after we see SOS
static int stbi__process_scan_header(stbi__jpeg *z)
{
   int i;
   int Ls = stbi__get16be(z->s);
   z->scan_n = stbi__get8(z->s);
   if (z->scan_n < 1 || z->scan_n > 4 || z->scan_n > (int) z->s->img_n) return stbi__err("bad SOS component count","Corrupt JPEG");
   if (Ls != 6+2*z->scan_n) return stbi__err("bad SOS len","Corrupt JPEG");
   for (i=0; i < z->scan_n; ++i) {
	  int id = stbi__get8(z->s), which;
	  int q = stbi__get8(z->s);
	  for (which = 0; which < z->s->img_n; ++which)
		 if (z->img_comp[which].id == id)
			break;
	  if (which == z->s->img_n) return 0; // no match
	  z->img_comp[which].hd = q >> 4;   if (z->img_comp[which].hd > 3) return stbi__err("bad DC huff","Corrupt JPEG");
	  z->img_comp[which].ha = q & 15;   if (z->img_comp[which].ha > 3) return stbi__err("bad AC huff","Corrupt JPEG");
	  z->order[i] = which;
   }

   {
	  int aa;
	  z->spec_start = stbi__get8(z->s);
	  z->spec_end   = stbi__get8(z->s); // should be 63, but might be 0
	  aa = stbi__get8(z->s);
	  z->succ_high = (aa >> 4);
	  z->succ_low  = (aa & 15);
	  if (z->progressive) {
		 if (z->spec_start > 63 || z->spec_end > 63  || z->spec_start > z->spec_end || z->succ_high > 13 || z->succ_low > 13)
			return stbi__err("bad SOS", "Corrupt JPEG");
	  } else {
		 if (z->spec_start != 0) return stbi__err("bad SOS","Corrupt JPEG");
		 if (z->succ_high != 0 || z->succ_low != 0) return stbi__err("bad SOS","Corrupt JPEG");
		 z->spec_end = 63;
	  }
   }

   return 1;
}

static int stbi__process_frame_header(stbi__jpeg *z, int scan)
{
   stbi__context *s = z->s;
   int Lf,p,i,q, h_max=1,v_max=1,c;
   Lf = stbi__get16be(s);         if (Lf < 11) return stbi__err("bad SOF len","Corrupt JPEG"); // JPEG
   p  = stbi__get8(s);            if (p != 8) return stbi__err("only 8-bit","JPEG format not supported: 8-bit only"); // JPEG baseline
   s->img_y = stbi__get16be(s);   if (s->img_y == 0) return stbi__err("no header height", "JPEG format not supported: delayed height"); // Legal, but we don't handle it--but neither does IJG
   s->img_x = stbi__get16be(s);   if (s->img_x == 0) return stbi__err("0 width","Corrupt JPEG"); // JPEG requires
   c = stbi__get8(s);
   if (c != 3 && c != 1) return stbi__err("bad component count","Corrupt JPEG");    // JFIF requires
   s->img_n = c;
   for (i=0; i < c; ++i) {
	  z->img_comp[i].data = NULL;
	  z->img_comp[i].linebuf = NULL;
   }

   if (Lf != 8+3*s->img_n) return stbi__err("bad SOF len","Corrupt JPEG");

   for (i=0; i < s->img_n; ++i) {
	  z->img_comp[i].id = stbi__get8(s);
	  if (z->img_comp[i].id != i+1)   // JFIF requires
		 if (z->img_comp[i].id != i)  // some version of jpegtran outputs non-JFIF-compliant files!
			return stbi__err("bad component ID","Corrupt JPEG");
	  q = stbi__get8(s);
	  z->img_comp[i].h = (q >> 4);  if (!z->img_comp[i].h || z->img_comp[i].h > 4) return stbi__err("bad H","Corrupt JPEG");
	  z->img_comp[i].v = q & 15;    if (!z->img_comp[i].v || z->img_comp[i].v > 4) return stbi__err("bad V","Corrupt JPEG");
	  z->img_comp[i].tq = stbi__get8(s);  if (z->img_comp[i].tq > 3) return stbi__err("bad TQ","Corrupt JPEG");
   }

   if (scan != STBI__SCAN_load) return 1;

   if ((1 << 30) / s->img_x / s->img_n < s->img_y) return stbi__err("too large", "Image too large to decode");

   for (i=0; i < s->img_n; ++i) {
	  if (z->img_comp[i].h > h_max) h_max = z->img_comp[i].h;
	  if (z->img_comp[i].v > v_max) v_max = z->img_comp[i].v;
   }

   // compute interleaved mcu info
   z->img_h_max = h_max;
   z->img_v_max = v_max;
   z->img_mcu_w = h_max * 8;
   z->img_mcu_h = v_max * 8;
   z->img_mcu_x = (s->img_x + z->img_mcu_w-1) / z->img_mcu_w;
   z->img_mcu_y = (s->img_y + z->img_mcu_h-1) / z->img_mcu_h;

   for (i=0; i < s->img_n; ++i) {
	  // number of effective pixels (e.g. for non-interleaved MCU)
	  z->img_comp[i].x = (s->img_x * z->img_comp[i].h + h_max-1) / h_max;
	  z->img_comp[i].y = (s->img_y * z->img_comp[i].v + v_max-1) / v_max;
	  // to simplify generation, we'll allocate enough memory to decode
	  // the bogus oversized data from using interleaved MCUs and their
	  // big blocks (e.g. a 16x16 iMCU on an image of width 33); we won't
	  // discard the extra data until colorspace conversion
	  z->img_comp[i].w2 = z->img_mcu_x * z->img_comp[i].h * 8;
	  z->img_comp[i].h2 = z->img_mcu_y * z->img_comp[i].v * 8;
	  z->img_comp[i].raw_data = stbi__malloc(z->img_comp[i].w2 * z->img_comp[i].h2+15);

	  if (z->img_comp[i].raw_data == NULL) {
		 for(--i; i >= 0; --i) {
			STBI_FREE(z->img_comp[i].raw_data);
			z->img_comp[i].data = NULL;
		 }
		 return stbi__err("outofmem", "Out of memory");
	  }
	  // align blocks for idct using mmx/sse
	  z->img_comp[i].data = (stbi_uc*) (((size_t) z->img_comp[i].raw_data + 15) & ~15);
	  z->img_comp[i].linebuf = NULL;
	  if (z->progressive) {
		 z->img_comp[i].coeff_w = (z->img_comp[i].w2 + 7) >> 3;
		 z->img_comp[i].coeff_h = (z->img_comp[i].h2 + 7) >> 3;
		 z->img_comp[i].raw_coeff = STBI_MALLOC(z->img_comp[i].coeff_w * z->img_comp[i].coeff_h * 64 * sizeof(short) + 15);
		 z->img_comp[i].coeff = (short*) (((size_t) z->img_comp[i].raw_coeff + 15) & ~15);
	  } else {
		 z->img_comp[i].coeff = 0;
		 z->img_comp[i].raw_coeff = 0;
	  }
   }

   return 1;
}

// use comparisons since in some cases we handle more than one case (e.g. SOF)
#define stbi__DNL(x)         ((x) == 0xdc)
#define stbi__SOI(x)         ((x) == 0xd8)
#define stbi__EOI(x)         ((x) == 0xd9)
#define stbi__SOF(x)         ((x) == 0xc0 || (x) == 0xc1 || (x) == 0xc2)
#define stbi__SOS(x)         ((x) == 0xda)

#define stbi__SOF_progressive(x)   ((x) == 0xc2)

static int stbi__decode_jpeg_header(stbi__jpeg *z, int scan)
{
   int m;
   z->marker = STBI__MARKER_none; // initialize cached marker to empty
   m = stbi__get_marker(z);
   if (!stbi__SOI(m)) return stbi__err("no SOI","Corrupt JPEG");
   if (scan == STBI__SCAN_type) return 1;
   m = stbi__get_marker(z);
   while (!stbi__SOF(m)) {
	  if (!stbi__process_marker(z,m)) return 0;
	  m = stbi__get_marker(z);
	  while (m == STBI__MARKER_none) {
		 // some files have extra padding after their blocks, so ok, we'll scan
		 if (stbi__at_eof(z->s)) return stbi__err("no SOF", "Corrupt JPEG");
		 m = stbi__get_marker(z);
	  }
   }
   z->progressive = stbi__SOF_progressive(m);
   if (!stbi__process_frame_header(z, scan)) return 0;
   return 1;
}

// decode image to YCbCr format
static int stbi__decode_jpeg_image(stbi__jpeg *j)
{
   int m;
   j->restart_interval = 0;
   if (!stbi__decode_jpeg_header(j, STBI__SCAN_load)) return 0;
   m = stbi__get_marker(j);
   while (!stbi__EOI(m)) {
	  if (stbi__SOS(m)) {
		 if (!stbi__process_scan_header(j)) return 0;
		 if (!stbi__parse_entropy_coded_data(j)) return 0;
		 if (j->marker == STBI__MARKER_none ) {
			// handle 0s at the end of image data from IP Kamera 9060
			while (!stbi__at_eof(j->s)) {
			   int x = stbi__get8(j->s);
			   if (x == 255) {
				  j->marker = stbi__get8(j->s);
				  break;
			   } else if (x != 0) {
				  return stbi__err("junk before marker", "Corrupt JPEG");
			   }
			}
			// if we reach eof without hitting a marker, stbi__get_marker() below will fail and we'll eventually return 0
		 }
	  } else {
		 if (!stbi__process_marker(j, m)) return 0;
	  }
	  m = stbi__get_marker(j);
   }
   if (j->progressive)
	  stbi__jpeg_finish(j);
   return 1;
}

// static jfif-centered resampling (across block boundaries)

typedef stbi_uc *(*resample_row_func)(stbi_uc *out, stbi_uc *in0, stbi_uc *in1,
									int w, int hs);

#define stbi__div4(x) ((stbi_uc) ((x) >> 2))

static stbi_uc *resample_row_1(stbi_uc *out, stbi_uc *in_near, stbi_uc *in_far, int w, int hs)
{
   STBI_NOTUSED(out);
   STBI_NOTUSED(in_far);
   STBI_NOTUSED(w);
   STBI_NOTUSED(hs);
   return in_near;
}

static stbi_uc* stbi__resample_row_v_2(stbi_uc *out, stbi_uc *in_near, stbi_uc *in_far, int w, int hs)
{
   // need to generate two samples vertically for every one in input
   int i;
   STBI_NOTUSED(hs);
   for (i=0; i < w; ++i)
	  out[i] = stbi__div4(3*in_near[i] + in_far[i] + 2);
   return out;
}

static stbi_uc*  stbi__resample_row_h_2(stbi_uc *out, stbi_uc *in_near, stbi_uc *in_far, int w, int hs)
{
   // need to generate two samples horizontally for every one in input
   int i;
   stbi_uc *input = in_near;

   if (w == 1) {
	  // if only one sample, can't do any interpolation
	  out[0] = out[1] = input[0];
	  return out;
   }

   out[0] = input[0];
   out[1] = stbi__div4(input[0]*3 + input[1] + 2);
   for (i=1; i < w-1; ++i) {
	  int n = 3*input[i]+2;
	  out[i*2+0] = stbi__div4(n+input[i-1]);
	  out[i*2+1] = stbi__div4(n+input[i+1]);
   }
   out[i*2+0] = stbi__div4(input[w-2]*3 + input[w-1] + 2);
   out[i*2+1] = input[w-1];

   STBI_NOTUSED(in_far);
   STBI_NOTUSED(hs);

   return out;
}

#define stbi__div16(x) ((stbi_uc) ((x) >> 4))

static stbi_uc *stbi__resample_row_hv_2(stbi_uc *out, stbi_uc *in_near, stbi_uc *in_far, int w, int hs)
{
   // need to generate 2x2 samples for every one in input
   int i,t0,t1;
   if (w == 1) {
	  out[0] = out[1] = stbi__div4(3*in_near[0] + in_far[0] + 2);
	  return out;
   }

   t1 = 3*in_near[0] + in_far[0];
   out[0] = stbi__div4(t1+2);
   for (i=1; i < w; ++i) {
	  t0 = t1;
	  t1 = 3*in_near[i]+in_far[i];
	  out[i*2-1] = stbi__div16(3*t0 + t1 + 8);
	  out[i*2  ] = stbi__div16(3*t1 + t0 + 8);
   }
   out[w*2-1] = stbi__div4(t1+2);

   STBI_NOTUSED(hs);

   return out;
}

#if defined(STBI_SSE2) || defined(STBI_NEON)
static stbi_uc *stbi__resample_row_hv_2_simd(stbi_uc *out, stbi_uc *in_near, stbi_uc *in_far, int w, int hs)
{
   // need to generate 2x2 samples for every one in input
   int i=0,t0,t1;

   if (w == 1) {
	  out[0] = out[1] = stbi__div4(3*in_near[0] + in_far[0] + 2);
	  return out;
   }

   t1 = 3*in_near[0] + in_far[0];
   // process groups of 8 pixels for as long as we can.
   // note we can't handle the last pixel in a row in this loop
   // because we need to handle the filter boundary conditions.
   for (; i < ((w-1) & ~7); i += 8) {
#if defined(STBI_SSE2)
	  // load and perform the vertical filtering pass
	  // this uses 3*x + y = 4*x + (y - x)
	  __m128i zero  = _mm_setzero_si128();
	  __m128i farb  = _mm_loadl_epi64((__m128i *) (in_far + i));
	  __m128i nearb = _mm_loadl_epi64((__m128i *) (in_near + i));
	  __m128i farw  = _mm_unpacklo_epi8(farb, zero);
	  __m128i nearw = _mm_unpacklo_epi8(nearb, zero);
	  __m128i diff  = _mm_sub_epi16(farw, nearw);
	  __m128i nears = _mm_slli_epi16(nearw, 2);
	  __m128i curr  = _mm_add_epi16(nears, diff); // current row

	  // horizontal filter works the same based on shifted vers of current
	  // row. "prev" is current row shifted right by 1 pixel; we need to
	  // insert the previous pixel value (from t1).
	  // "next" is current row shifted left by 1 pixel, with first pixel
	  // of next block of 8 pixels added in.
	  __m128i prv0 = _mm_slli_si128(curr, 2);
	  __m128i nxt0 = _mm_srli_si128(curr, 2);
	  __m128i prev = _mm_insert_epi16(prv0, t1, 0);
	  __m128i next = _mm_insert_epi16(nxt0, 3*in_near[i+8] + in_far[i+8], 7);

	  // horizontal filter, polyphase implementation since it's convenient:
	  // even pixels = 3*cur + prev = cur*4 + (prev - cur)
	  // odd  pixels = 3*cur + next = cur*4 + (next - cur)
	  // note the shared term.
	  __m128i bias  = _mm_set1_epi16(8);
	  __m128i curs = _mm_slli_epi16(curr, 2);
	  __m128i prvd = _mm_sub_epi16(prev, curr);
	  __m128i nxtd = _mm_sub_epi16(next, curr);
	  __m128i curb = _mm_add_epi16(curs, bias);
	  __m128i even = _mm_add_epi16(prvd, curb);
	  __m128i odd  = _mm_add_epi16(nxtd, curb);

	  // interleave even and odd pixels, then undo scaling.
	  __m128i int0 = _mm_unpacklo_epi16(even, odd);
	  __m128i int1 = _mm_unpackhi_epi16(even, odd);
	  __m128i de0  = _mm_srli_epi16(int0, 4);
	  __m128i de1  = _mm_srli_epi16(int1, 4);

	  // pack and write output
	  __m128i outv = _mm_packus_epi16(de0, de1);
	  _mm_storeu_si128((__m128i *) (out + i*2), outv);
#elif defined(STBI_NEON)
	  // load and perform the vertical filtering pass
	  // this uses 3*x + y = 4*x + (y - x)
	  uint8x8_t farb  = vld1_u8(in_far + i);
	  uint8x8_t nearb = vld1_u8(in_near + i);
	  int16x8_t diff  = vreinterpretq_s16_u16(vsubl_u8(farb, nearb));
	  int16x8_t nears = vreinterpretq_s16_u16(vshll_n_u8(nearb, 2));
	  int16x8_t curr  = vaddq_s16(nears, diff); // current row

	  // horizontal filter works the same based on shifted vers of current
	  // row. "prev" is current row shifted right by 1 pixel; we need to
	  // insert the previous pixel value (from t1).
	  // "next" is current row shifted left by 1 pixel, with first pixel
	  // of next block of 8 pixels added in.
	  int16x8_t prv0 = vextq_s16(curr, curr, 7);
	  int16x8_t nxt0 = vextq_s16(curr, curr, 1);
	  int16x8_t prev = vsetq_lane_s16(t1, prv0, 0);
	  int16x8_t next = vsetq_lane_s16(3*in_near[i+8] + in_far[i+8], nxt0, 7);

	  // horizontal filter, polyphase implementation since it's convenient:
	  // even pixels = 3*cur + prev = cur*4 + (prev - cur)
	  // odd  pixels = 3*cur + next = cur*4 + (next - cur)
	  // note the shared term.
	  int16x8_t curs = vshlq_n_s16(curr, 2);
	  int16x8_t prvd = vsubq_s16(prev, curr);
	  int16x8_t nxtd = vsubq_s16(next, curr);
	  int16x8_t even = vaddq_s16(curs, prvd);
	  int16x8_t odd  = vaddq_s16(curs, nxtd);

	  // undo scaling and round, then store with even/odd phases interleaved
	  uint8x8x2_t o;
	  o.val[0] = vqrshrun_n_s16(even, 4);
	  o.val[1] = vqrshrun_n_s16(odd,  4);
	  vst2_u8(out + i*2, o);
#endif

	  // "previous" value for next iter
	  t1 = 3*in_near[i+7] + in_far[i+7];
   }

   t0 = t1;
   t1 = 3*in_near[i] + in_far[i];
   out[i*2] = stbi__div16(3*t1 + t0 + 8);

   for (++i; i < w; ++i) {
	  t0 = t1;
	  t1 = 3*in_near[i]+in_far[i];
	  out[i*2-1] = stbi__div16(3*t0 + t1 + 8);
	  out[i*2  ] = stbi__div16(3*t1 + t0 + 8);
   }
   out[w*2-1] = stbi__div4(t1+2);

   STBI_NOTUSED(hs);

   return out;
}
#endif

static stbi_uc *stbi__resample_row_generic(stbi_uc *out, stbi_uc *in_near, stbi_uc *in_far, int w, int hs)
{
   // resample with nearest-neighbor
   int i,j;
   STBI_NOTUSED(in_far);
   for (i=0; i < w; ++i)
	  for (j=0; j < hs; ++j)
		 out[i*hs+j] = in_near[i];
   return out;
}

#ifdef STBI_JPEG_OLD
// this is the same YCbCr-to-RGB calculation that stb_image has used
// historically before the algorithm changes in 1.49
#define float2fixed(x)  ((int) ((x) * 65536 + 0.5))
static void stbi__YCbCr_to_RGB_row(stbi_uc *out, const stbi_uc *y, const stbi_uc *pcb, const stbi_uc *pcr, int count, int step)
{
   int i;
   for (i=0; i < count; ++i) {
	  int y_fixed = (y[i] << 16) + 32768; // rounding
	  int r,g,b;
	  int cr = pcr[i] - 128;
	  int cb = pcb[i] - 128;
	  r = y_fixed + cr*float2fixed(1.40200f);
	  g = y_fixed - cr*float2fixed(0.71414f) - cb*float2fixed(0.34414f);
	  b = y_fixed                            + cb*float2fixed(1.77200f);
	  r >>= 16;
	  g >>= 16;
	  b >>= 16;
	  if ((unsigned) r > 255) { if (r < 0) r = 0; else r = 255; }
	  if ((unsigned) g > 255) { if (g < 0) g = 0; else g = 255; }
	  if ((unsigned) b > 255) { if (b < 0) b = 0; else b = 255; }
	  out[0] = (stbi_uc)r;
	  out[1] = (stbi_uc)g;
	  out[2] = (stbi_uc)b;
	  out[3] = 255;
	  out += step;
   }
}
#else
// this is a reduced-precision calculation of YCbCr-to-RGB introduced
// to make sure the code produces the same results in both SIMD and scalar
#define float2fixed(x)  (((int) ((x) * 4096.0f + 0.5f)) << 8)
static void stbi__YCbCr_to_RGB_row(stbi_uc *out, const stbi_uc *y, const stbi_uc *pcb, const stbi_uc *pcr, int count, int step)
{
   int i;
   for (i=0; i < count; ++i) {
	  int y_fixed = (y[i] << 20) + (1<<19); // rounding
	  int r,g,b;
	  int cr = pcr[i] - 128;
	  int cb = pcb[i] - 128;
	  r = y_fixed +  cr* float2fixed(1.40200f);
	  g = y_fixed + (cr*-float2fixed(0.71414f)) + ((cb*-float2fixed(0.34414f)) & 0xffff0000);
	  b = y_fixed                               +   cb* float2fixed(1.77200f);
	  r >>= 20;
	  g >>= 20;
	  b >>= 20;
	  if ((unsigned) r > 255) { if (r < 0) r = 0; else r = 255; }
	  if ((unsigned) g > 255) { if (g < 0) g = 0; else g = 255; }
	  if ((unsigned) b > 255) { if (b < 0) b = 0; else b = 255; }
	  out[0] = (stbi_uc)r;
	  out[1] = (stbi_uc)g;
	  out[2] = (stbi_uc)b;
	  out[3] = 255;
	  out += step;
   }
}
#endif

#if defined(STBI_SSE2) || defined(STBI_NEON)
static void stbi__YCbCr_to_RGB_simd(stbi_uc *out, stbi_uc const *y, stbi_uc const *pcb, stbi_uc const *pcr, int count, int step)
{
   int i = 0;

#ifdef STBI_SSE2
   // step == 3 is pretty ugly on the final interleave, and i'm not convinced
   // it's useful in practice (you wouldn't use it for textures, for example).
   // so just accelerate step == 4 case.
   if (step == 4) {
	  // this is a fairly straightforward implementation and not super-optimized.
	  __m128i signflip  = _mm_set1_epi8(-0x80);
	  __m128i cr_const0 = _mm_set1_epi16(   (short) ( 1.40200f*4096.0f+0.5f));
	  __m128i cr_const1 = _mm_set1_epi16( - (short) ( 0.71414f*4096.0f+0.5f));
	  __m128i cb_const0 = _mm_set1_epi16( - (short) ( 0.34414f*4096.0f+0.5f));
	  __m128i cb_const1 = _mm_set1_epi16(   (short) ( 1.77200f*4096.0f+0.5f));
	  __m128i y_bias = _mm_set1_epi8((char) (unsigned char) 128);
	  __m128i xw = _mm_set1_epi16(255); // alpha channel

	  for (; i+7 < count; i += 8) {
		 // load
		 __m128i y_bytes = _mm_loadl_epi64((__m128i *) (y+i));
		 __m128i cr_bytes = _mm_loadl_epi64((__m128i *) (pcr+i));
		 __m128i cb_bytes = _mm_loadl_epi64((__m128i *) (pcb+i));
		 __m128i cr_biased = _mm_xor_si128(cr_bytes, signflip); // -128
		 __m128i cb_biased = _mm_xor_si128(cb_bytes, signflip); // -128

		 // unpack to short (and left-shift cr, cb by 8)
		 __m128i yw  = _mm_unpacklo_epi8(y_bias, y_bytes);
		 __m128i crw = _mm_unpacklo_epi8(_mm_setzero_si128(), cr_biased);
		 __m128i cbw = _mm_unpacklo_epi8(_mm_setzero_si128(), cb_biased);

		 // color transform
		 __m128i yws = _mm_srli_epi16(yw, 4);
		 __m128i cr0 = _mm_mulhi_epi16(cr_const0, crw);
		 __m128i cb0 = _mm_mulhi_epi16(cb_const0, cbw);
		 __m128i cb1 = _mm_mulhi_epi16(cbw, cb_const1);
		 __m128i cr1 = _mm_mulhi_epi16(crw, cr_const1);
		 __m128i rws = _mm_add_epi16(cr0, yws);
		 __m128i gwt = _mm_add_epi16(cb0, yws);
		 __m128i bws = _mm_add_epi16(yws, cb1);
		 __m128i gws = _mm_add_epi16(gwt, cr1);

		 // descale
		 __m128i rw = _mm_srai_epi16(rws, 4);
		 __m128i bw = _mm_srai_epi16(bws, 4);
		 __m128i gw = _mm_srai_epi16(gws, 4);

		 // back to byte, set up for transpose
		 __m128i brb = _mm_packus_epi16(rw, bw);
		 __m128i gxb = _mm_packus_epi16(gw, xw);

		 // transpose to interleave channels
		 __m128i t0 = _mm_unpacklo_epi8(brb, gxb);
		 __m128i t1 = _mm_unpackhi_epi8(brb, gxb);
		 __m128i o0 = _mm_unpacklo_epi16(t0, t1);
		 __m128i o1 = _mm_unpackhi_epi16(t0, t1);

		 // store
		 _mm_storeu_si128((__m128i *) (out + 0), o0);
		 _mm_storeu_si128((__m128i *) (out + 16), o1);
		 out += 32;
	  }
   }
#endif

#ifdef STBI_NEON
   // in this version, step=3 support would be easy to add. but is there demand?
   if (step == 4) {
	  // this is a fairly straightforward implementation and not super-optimized.
	  uint8x8_t signflip = vdup_n_u8(0x80);
	  int16x8_t cr_const0 = vdupq_n_s16(   (short) ( 1.40200f*4096.0f+0.5f));
	  int16x8_t cr_const1 = vdupq_n_s16( - (short) ( 0.71414f*4096.0f+0.5f));
	  int16x8_t cb_const0 = vdupq_n_s16( - (short) ( 0.34414f*4096.0f+0.5f));
	  int16x8_t cb_const1 = vdupq_n_s16(   (short) ( 1.77200f*4096.0f+0.5f));

	  for (; i+7 < count; i += 8) {
		 // load
		 uint8x8_t y_bytes  = vld1_u8(y + i);
		 uint8x8_t cr_bytes = vld1_u8(pcr + i);
		 uint8x8_t cb_bytes = vld1_u8(pcb + i);
		 int8x8_t cr_biased = vreinterpret_s8_u8(vsub_u8(cr_bytes, signflip));
		 int8x8_t cb_biased = vreinterpret_s8_u8(vsub_u8(cb_bytes, signflip));

		 // expand to s16
		 int16x8_t yws = vreinterpretq_s16_u16(vshll_n_u8(y_bytes, 4));
		 int16x8_t crw = vshll_n_s8(cr_biased, 7);
		 int16x8_t cbw = vshll_n_s8(cb_biased, 7);

		 // color transform
		 int16x8_t cr0 = vqdmulhq_s16(crw, cr_const0);
		 int16x8_t cb0 = vqdmulhq_s16(cbw, cb_const0);
		 int16x8_t cr1 = vqdmulhq_s16(crw, cr_const1);
		 int16x8_t cb1 = vqdmulhq_s16(cbw, cb_const1);
		 int16x8_t rws = vaddq_s16(yws, cr0);
		 int16x8_t gws = vaddq_s16(vaddq_s16(yws, cb0), cr1);
		 int16x8_t bws = vaddq_s16(yws, cb1);

		 // undo scaling, round, convert to byte
		 uint8x8x4_t o;
		 o.val[0] = vqrshrun_n_s16(rws, 4);
		 o.val[1] = vqrshrun_n_s16(gws, 4);
		 o.val[2] = vqrshrun_n_s16(bws, 4);
		 o.val[3] = vdup_n_u8(255);

		 // store, interleaving r/g/b/a
		 vst4_u8(out, o);
		 out += 8*4;
	  }
   }
#endif

   for (; i < count; ++i) {
	  int y_fixed = (y[i] << 20) + (1<<19); // rounding
	  int r,g,b;
	  int cr = pcr[i] - 128;
	  int cb = pcb[i] - 128;
	  r = y_fixed + cr* float2fixed(1.40200f);
	  g = y_fixed + cr*-float2fixed(0.71414f) + ((cb*-float2fixed(0.34414f)) & 0xffff0000);
	  b = y_fixed                             +   cb* float2fixed(1.77200f);
	  r >>= 20;
	  g >>= 20;
	  b >>= 20;
	  if ((unsigned) r > 255) { if (r < 0) r = 0; else r = 255; }
	  if ((unsigned) g > 255) { if (g < 0) g = 0; else g = 255; }
	  if ((unsigned) b > 255) { if (b < 0) b = 0; else b = 255; }
	  out[0] = (stbi_uc)r;
	  out[1] = (stbi_uc)g;
	  out[2] = (stbi_uc)b;
	  out[3] = 255;
	  out += step;
   }
}
#endif

// set up the kernels
static void stbi__setup_jpeg(stbi__jpeg *j)
{
   j->idct_block_kernel = stbi__idct_block;
   j->YCbCr_to_RGB_kernel = stbi__YCbCr_to_RGB_row;
   j->resample_row_hv_2_kernel = stbi__resample_row_hv_2;

#ifdef STBI_SSE2
   if (stbi__sse2_available()) {
	  j->idct_block_kernel = stbi__idct_simd;
	  #ifndef STBI_JPEG_OLD
	  j->YCbCr_to_RGB_kernel = stbi__YCbCr_to_RGB_simd;
	  #endif
	  j->resample_row_hv_2_kernel = stbi__resample_row_hv_2_simd;
   }
#endif

#ifdef STBI_NEON
   j->idct_block_kernel = stbi__idct_simd;
   #ifndef STBI_JPEG_OLD
   j->YCbCr_to_RGB_kernel = stbi__YCbCr_to_RGB_simd;
   #endif
   j->resample_row_hv_2_kernel = stbi__resample_row_hv_2_simd;
#endif
}

// clean up the temporary component buffers
static void stbi__cleanup_jpeg(stbi__jpeg *j)
{
   int i;
   for (i=0; i < j->s->img_n; ++i) {
	  if (j->img_comp[i].raw_data) {
		 STBI_FREE(j->img_comp[i].raw_data);
		 j->img_comp[i].raw_data = NULL;
		 j->img_comp[i].data = NULL;
	  }
	  if (j->img_comp[i].raw_coeff) {
		 STBI_FREE(j->img_comp[i].raw_coeff);
		 j->img_comp[i].raw_coeff = 0;
		 j->img_comp[i].coeff = 0;
	  }
	  if (j->img_comp[i].linebuf) {
		 STBI_FREE(j->img_comp[i].linebuf);
		 j->img_comp[i].linebuf = NULL;
	  }
   }
}

typedef struct
{
   resample_row_func resample;
   stbi_uc *line0,*line1;
   int hs,vs;   // expansion factor in each axis
   int w_lores; // horizontal pixels pre-expansion
   int ystep;   // how far through vertical expansion we are
   int ypos;    // which pre-expansion row we're on
} stbi__resample;

static stbi_uc *load_jpeg_image(stbi__jpeg *z, int *out_x, int *out_y, int *comp, int req_comp)
{
   int n, decode_n;
   z->s->img_n = 0; // make stbi__cleanup_jpeg safe

   // validate req_comp
   if (req_comp < 0 || req_comp > 4) return stbi__errpuc("bad req_comp", "Internal error");

   // load a jpeg image from whichever source, but leave in YCbCr format
   if (!stbi__decode_jpeg_image(z)) { stbi__cleanup_jpeg(z); return NULL; }

   // determine actual number of components to generate
   n = req_comp ? req_comp : z->s->img_n;

   if (z->s->img_n == 3 && n < 3)
	  decode_n = 1;
   else
	  decode_n = z->s->img_n;

   // resample and color-convert
   {
	  int k;
	  unsigned int i,j;
	  stbi_uc *output;
	  stbi_uc *coutput[4];

	  stbi__resample res_comp[4];

	  for (k=0; k < decode_n; ++k) {
		 stbi__resample *r = &res_comp[k];

		 // allocate line buffer big enough for upsampling off the edges
		 // with upsample factor of 4
		 z->img_comp[k].linebuf = (stbi_uc *) stbi__malloc(z->s->img_x + 3);
		 if (!z->img_comp[k].linebuf) { stbi__cleanup_jpeg(z); return stbi__errpuc("outofmem", "Out of memory"); }

		 r->hs      = z->img_h_max / z->img_comp[k].h;
		 r->vs      = z->img_v_max / z->img_comp[k].v;
		 r->ystep   = r->vs >> 1;
		 r->w_lores = (z->s->img_x + r->hs-1) / r->hs;
		 r->ypos    = 0;
		 r->line0   = r->line1 = z->img_comp[k].data;

		 if      (r->hs == 1 && r->vs == 1) r->resample = resample_row_1;
		 else if (r->hs == 1 && r->vs == 2) r->resample = stbi__resample_row_v_2;
		 else if (r->hs == 2 && r->vs == 1) r->resample = stbi__resample_row_h_2;
		 else if (r->hs == 2 && r->vs == 2) r->resample = z->resample_row_hv_2_kernel;
		 else                               r->resample = stbi__resample_row_generic;
	  }

	  // can't error after this so, this is safe
	  output = (stbi_uc *) stbi__malloc(n * z->s->img_x * z->s->img_y + 1);
	  if (!output) { stbi__cleanup_jpeg(z); return stbi__errpuc("outofmem", "Out of memory"); }

	  // now go ahead and resample
	  for (j=0; j < z->s->img_y; ++j) {
		 stbi_uc *out = output + n * z->s->img_x * j;
		 for (k=0; k < decode_n; ++k) {
			stbi__resample *r = &res_comp[k];
			int y_bot = r->ystep >= (r->vs >> 1);
			coutput[k] = r->resample(z->img_comp[k].linebuf,
									 y_bot ? r->line1 : r->line0,
									 y_bot ? r->line0 : r->line1,
									 r->w_lores, r->hs);
			if (++r->ystep >= r->vs) {
			   r->ystep = 0;
			   r->line0 = r->line1;
			   if (++r->ypos < z->img_comp[k].y)
				  r->line1 += z->img_comp[k].w2;
			}
		 }
		 if (n >= 3) {
			stbi_uc *y = coutput[0];
			if (z->s->img_n == 3) {
			   z->YCbCr_to_RGB_kernel(out, y, coutput[1], coutput[2], z->s->img_x, n);
			} else
			   for (i=0; i < z->s->img_x; ++i) {
				  out[0] = out[1] = out[2] = y[i];
				  out[3] = 255; // not used if n==3
				  out += n;
			   }
		 } else {
			stbi_uc *y = coutput[0];
			if (n == 1)
			   for (i=0; i < z->s->img_x; ++i) out[i] = y[i];
			else
			   for (i=0; i < z->s->img_x; ++i) *out++ = y[i], *out++ = 255;
		 }
	  }
	  stbi__cleanup_jpeg(z);
	  *out_x = z->s->img_x;
	  *out_y = z->s->img_y;
	  if (comp) *comp  = z->s->img_n; // report original components, not output
	  return output;
   }
}

static unsigned char *stbi__jpeg_load(stbi__context *s, int *x, int *y, int *comp, int req_comp)
{
   stbi__jpeg j;
   j.s = s;
   stbi__setup_jpeg(&j);
   return load_jpeg_image(&j, x,y,comp,req_comp);
}

static int stbi__jpeg_test(stbi__context *s)
{
   int r;
   stbi__jpeg j;
   j.s = s;
   stbi__setup_jpeg(&j);
   r = stbi__decode_jpeg_header(&j, STBI__SCAN_type);
   stbi__rewind(s);
   return r;
}

static int stbi__jpeg_info_raw(stbi__jpeg *j, int *x, int *y, int *comp)
{
   if (!stbi__decode_jpeg_header(j, STBI__SCAN_header)) {
	  stbi__rewind( j->s );
	  return 0;
   }
   if (x) *x = j->s->img_x;
   if (y) *y = j->s->img_y;
   if (comp) *comp = j->s->img_n;
   return 1;
}

static int stbi__jpeg_info(stbi__context *s, int *x, int *y, int *comp)
{
   stbi__jpeg j;
   j.s = s;
   return stbi__jpeg_info_raw(&j, x, y, comp);
}
#endif

// public domain zlib decode    v0.2  Sean Barrett 2006-11-18
//    simple implementation
//      - all input must be provided in an upfront buffer
//      - all output is written to a single output buffer (can malloc/realloc)
//    performance
//      - fast huffman

#ifndef STBI_NO_ZLIB

// fast-way is faster to check than jpeg huffman, but slow way is slower
#define STBI__ZFAST_BITS  9 // accelerate all cases in default tables
#define STBI__ZFAST_MASK  ((1 << STBI__ZFAST_BITS) - 1)

// zlib-style huffman encoding
// (jpegs packs from left, zlib from right, so can't share code)
typedef struct
{
   stbi__uint16 fast[1 << STBI__ZFAST_BITS];
   stbi__uint16 firstcode[16];
   int maxcode[17];
   stbi__uint16 firstsymbol[16];
   stbi_uc  size[288];
   stbi__uint16 value[288];
} stbi__zhuffman;

stbi_inline static int stbi__bitreverse16(int n)
{
  n = ((n & 0xAAAA) >>  1) | ((n & 0x5555) << 1);
  n = ((n & 0xCCCC) >>  2) | ((n & 0x3333) << 2);
  n = ((n & 0xF0F0) >>  4) | ((n & 0x0F0F) << 4);
  n = ((n & 0xFF00) >>  8) | ((n & 0x00FF) << 8);
  return n;
}

stbi_inline static int stbi__bit_reverse(int v, int bits)
{
   STBI_ASSERT(bits <= 16);
   // to bit reverse n bits, reverse 16 and shift
   // e.g. 11 bits, bit reverse and shift away 5
   return stbi__bitreverse16(v) >> (16-bits);
}

static int stbi__zbuild_huffman(stbi__zhuffman *z, stbi_uc *sizelist, int num)
{
   int i,k=0;
   int code, next_code[16], sizes[17];

   // DEFLATE spec for generating codes
   memset(sizes, 0, sizeof(sizes));
   memset(z->fast, 0, sizeof(z->fast));
   for (i=0; i < num; ++i)
	  ++sizes[sizelist[i]];
   sizes[0] = 0;
   for (i=1; i < 16; ++i)
	  STBI_ASSERT(sizes[i] <= (1 << i));
   code = 0;
   for (i=1; i < 16; ++i) {
	  next_code[i] = code;
	  z->firstcode[i] = (stbi__uint16) code;
	  z->firstsymbol[i] = (stbi__uint16) k;
	  code = (code + sizes[i]);
	  if (sizes[i])
		 if (code-1 >= (1 << i)) return stbi__err("bad codelengths","Corrupt JPEG");
	  z->maxcode[i] = code << (16-i); // preshift for inner loop
	  code <<= 1;
	  k += sizes[i];
   }
   z->maxcode[16] = 0x10000; // sentinel
   for (i=0; i < num; ++i) {
	  int s = sizelist[i];
	  if (s) {
		 int c = next_code[s] - z->firstcode[s] + z->firstsymbol[s];
		 stbi__uint16 fastv = (stbi__uint16) ((s << 9) | i);
		 z->size [c] = (stbi_uc     ) s;
		 z->value[c] = (stbi__uint16) i;
		 if (s <= STBI__ZFAST_BITS) {
			int k = stbi__bit_reverse(next_code[s],s);
			while (k < (1 << STBI__ZFAST_BITS)) {
			   z->fast[k] = fastv;
			   k += (1 << s);
			}
		 }
		 ++next_code[s];
	  }
   }
   return 1;
}

// zlib-from-memory implementation for PNG reading
//    because PNG allows splitting the zlib stream arbitrarily,
//    and it's annoying structurally to have PNG call ZLIB call PNG,
//    we require PNG read all the IDATs and combine them into a single
//    memory buffer

typedef struct
{
   stbi_uc *zbuffer, *zbuffer_end;
   int num_bits;
   stbi__uint32 code_buffer;

   char *zout;
   char *zout_start;
   char *zout_end;
   int   z_expandable;

   stbi__zhuffman z_length, z_distance;
} stbi__zbuf;

stbi_inline static stbi_uc stbi__zget8(stbi__zbuf *z)
{
   if (z->zbuffer >= z->zbuffer_end) return 0;
   return *z->zbuffer++;
}

static void stbi__fill_bits(stbi__zbuf *z)
{
   do {
	  STBI_ASSERT(z->code_buffer < (1U << z->num_bits));
	  z->code_buffer |= stbi__zget8(z) << z->num_bits;
	  z->num_bits += 8;
   } while (z->num_bits <= 24);
}

stbi_inline static unsigned int stbi__zreceive(stbi__zbuf *z, int n)
{
   unsigned int k;
   if (z->num_bits < n) stbi__fill_bits(z);
   k = z->code_buffer & ((1 << n) - 1);
   z->code_buffer >>= n;
   z->num_bits -= n;
   return k;
}

static int stbi__zhuffman_decode_slowpath(stbi__zbuf *a, stbi__zhuffman *z)
{
   int b,s,k;
   // not resolved by fast table, so compute it the slow way
   // use jpeg approach, which requires MSbits at top
   k = stbi__bit_reverse(a->code_buffer, 16);
   for (s=STBI__ZFAST_BITS+1; ; ++s)
	  if (k < z->maxcode[s])
		 break;
   if (s == 16) return -1; // invalid code!
   // code size is s, so:
   b = (k >> (16-s)) - z->firstcode[s] + z->firstsymbol[s];
   STBI_ASSERT(z->size[b] == s);
   a->code_buffer >>= s;
   a->num_bits -= s;
   return z->value[b];
}

stbi_inline static int stbi__zhuffman_decode(stbi__zbuf *a, stbi__zhuffman *z)
{
   int b,s;
   if (a->num_bits < 16) stbi__fill_bits(a);
   b = z->fast[a->code_buffer & STBI__ZFAST_MASK];
   if (b) {
	  s = b >> 9;
	  a->code_buffer >>= s;
	  a->num_bits -= s;
	  return b & 511;
   }
   return stbi__zhuffman_decode_slowpath(a, z);
}

static int stbi__zexpand(stbi__zbuf *z, char *zout, int n)  // need to make room for n bytes
{
   char *q;
   int cur, limit;
   z->zout = zout;
   if (!z->z_expandable) return stbi__err("output buffer limit","Corrupt PNG");
   cur   = (int) (z->zout     - z->zout_start);
   limit = (int) (z->zout_end - z->zout_start);
   while (cur + n > limit)
	  limit *= 2;
   q = (char *) STBI_REALLOC(z->zout_start, limit);
   if (q == NULL) return stbi__err("outofmem", "Out of memory");
   z->zout_start = q;
   z->zout       = q + cur;
   z->zout_end   = q + limit;
   return 1;
}

static int stbi__zlength_base[31] = {
   3,4,5,6,7,8,9,10,11,13,
   15,17,19,23,27,31,35,43,51,59,
   67,83,99,115,131,163,195,227,258,0,0 };

static int stbi__zlength_extra[31]=
{ 0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0,0,0 };

static int stbi__zdist_base[32] = { 1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,
257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577,0,0};

static int stbi__zdist_extra[32] =
{ 0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};

static int stbi__parse_huffman_block(stbi__zbuf *a)
{
   char *zout = a->zout;
   for(;;) {
	  int z = stbi__zhuffman_decode(a, &a->z_length);
	  if (z < 256) {
		 if (z < 0) return stbi__err("bad huffman code","Corrupt PNG"); // error in huffman codes
		 if (zout >= a->zout_end) {
			if (!stbi__zexpand(a, zout, 1)) return 0;
			zout = a->zout;
		 }
		 *zout++ = (char) z;
	  } else {
		 stbi_uc *p;
		 int len,dist;
		 if (z == 256) {
			a->zout = zout;
			return 1;
		 }
		 z -= 257;
		 len = stbi__zlength_base[z];
		 if (stbi__zlength_extra[z]) len += stbi__zreceive(a, stbi__zlength_extra[z]);
		 z = stbi__zhuffman_decode(a, &a->z_distance);
		 if (z < 0) return stbi__err("bad huffman code","Corrupt PNG");
		 dist = stbi__zdist_base[z];
		 if (stbi__zdist_extra[z]) dist += stbi__zreceive(a, stbi__zdist_extra[z]);
		 if (zout - a->zout_start < dist) return stbi__err("bad dist","Corrupt PNG");
		 if (zout + len > a->zout_end) {
			if (!stbi__zexpand(a, zout, len)) return 0;
			zout = a->zout;
		 }
		 p = (stbi_uc *) (zout - dist);
		 if (dist == 1) { // run of one byte; common in images.
			stbi_uc v = *p;
			do *zout++ = v; while (--len);
		 } else {
			do *zout++ = *p++; while (--len);
		 }
	  }
   }
}

static int stbi__compute_huffman_codes(stbi__zbuf *a)
{
   static stbi_uc length_dezigzag[19] = { 16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15 };
   stbi__zhuffman z_codelength;
   stbi_uc lencodes[286+32+137];//padding for maximum single op
   stbi_uc codelength_sizes[19];
   int i,n;

   int hlit  = stbi__zreceive(a,5) + 257;
   int hdist = stbi__zreceive(a,5) + 1;
   int hclen = stbi__zreceive(a,4) + 4;

   memset(codelength_sizes, 0, sizeof(codelength_sizes));
   for (i=0; i < hclen; ++i) {
	  int s = stbi__zreceive(a,3);
	  codelength_sizes[length_dezigzag[i]] = (stbi_uc) s;
   }
   if (!stbi__zbuild_huffman(&z_codelength, codelength_sizes, 19)) return 0;

   n = 0;
   while (n < hlit + hdist) {
	  int c = stbi__zhuffman_decode(a, &z_codelength);
	  STBI_ASSERT(c >= 0 && c < 19);
	  if (c < 16)
		 lencodes[n++] = (stbi_uc) c;
	  else if (c == 16) {
		 c = stbi__zreceive(a,2)+3;
		 memset(lencodes+n, lencodes[n-1], c);
		 n += c;
	  } else if (c == 17) {
		 c = stbi__zreceive(a,3)+3;
		 memset(lencodes+n, 0, c);
		 n += c;
	  } else {
		 STBI_ASSERT(c == 18);
		 c = stbi__zreceive(a,7)+11;
		 memset(lencodes+n, 0, c);
		 n += c;
	  }
   }
   if (n != hlit+hdist) return stbi__err("bad codelengths","Corrupt PNG");
   if (!stbi__zbuild_huffman(&a->z_length, lencodes, hlit)) return 0;
   if (!stbi__zbuild_huffman(&a->z_distance, lencodes+hlit, hdist)) return 0;
   return 1;
}

static int stbi__parse_uncomperssed_block(stbi__zbuf *a)
{
   stbi_uc header[4];
   int len,nlen,k;
   if (a->num_bits & 7)
	  stbi__zreceive(a, a->num_bits & 7); // discard
   // drain the bit-packed data into header
   k = 0;
   while (a->num_bits > 0) {
	  header[k++] = (stbi_uc) (a->code_buffer & 255); // suppress MSVC run-time check
	  a->code_buffer >>= 8;
	  a->num_bits -= 8;
   }
   STBI_ASSERT(a->num_bits == 0);
   // now fill header the normal way
   while (k < 4)
	  header[k++] = stbi__zget8(a);
   len  = header[1] * 256 + header[0];
   nlen = header[3] * 256 + header[2];
   if (nlen != (len ^ 0xffff)) return stbi__err("zlib corrupt","Corrupt PNG");
   if (a->zbuffer + len > a->zbuffer_end) return stbi__err("read past buffer","Corrupt PNG");
   if (a->zout + len > a->zout_end)
	  if (!stbi__zexpand(a, a->zout, len)) return 0;
   memcpy(a->zout, a->zbuffer, len);
   a->zbuffer += len;
   a->zout += len;
   return 1;
}

static int stbi__parse_zlib_header(stbi__zbuf *a)
{
   int cmf   = stbi__zget8(a);
   int cm    = cmf & 15;
   /* int cinfo = cmf >> 4; */
   int flg   = stbi__zget8(a);
   if ((cmf*256+flg) % 31 != 0) return stbi__err("bad zlib header","Corrupt PNG"); // zlib spec
   if (flg & 32) return stbi__err("no preset dict","Corrupt PNG"); // preset dictionary not allowed in png
   if (cm != 8) return stbi__err("bad compression","Corrupt PNG"); // DEFLATE required for png
   // window = 1 << (8 + cinfo)... but who cares, we fully buffer output
   return 1;
}

// @TODO: should statically initialize these for optimal thread safety
static stbi_uc stbi__zdefault_length[288], stbi__zdefault_distance[32];
static void stbi__init_zdefaults(void)
{
   int i;   // use <= to match clearly with spec
   for (i=0; i <= 143; ++i)     stbi__zdefault_length[i]   = 8;
   for (   ; i <= 255; ++i)     stbi__zdefault_length[i]   = 9;
   for (   ; i <= 279; ++i)     stbi__zdefault_length[i]   = 7;
   for (   ; i <= 287; ++i)     stbi__zdefault_length[i]   = 8;

   for (i=0; i <=  31; ++i)     stbi__zdefault_distance[i] = 5;
}

static int stbi__parse_zlib(stbi__zbuf *a, int parse_header)
{
   int final, type;
   if (parse_header)
	  if (!stbi__parse_zlib_header(a)) return 0;
   a->num_bits = 0;
   a->code_buffer = 0;
   do {
	  final = stbi__zreceive(a,1);
	  type = stbi__zreceive(a,2);
	  if (type == 0) {
		 if (!stbi__parse_uncomperssed_block(a)) return 0;
	  } else if (type == 3) {
		 return 0;
	  } else {
		 if (type == 1) {
			// use fixed code lengths
			if (!stbi__zdefault_distance[31]) stbi__init_zdefaults();
			if (!stbi__zbuild_huffman(&a->z_length  , stbi__zdefault_length  , 288)) return 0;
			if (!stbi__zbuild_huffman(&a->z_distance, stbi__zdefault_distance,  32)) return 0;
		 } else {
			if (!stbi__compute_huffman_codes(a)) return 0;
		 }
		 if (!stbi__parse_huffman_block(a)) return 0;
	  }
   } while (!final);
   return 1;
}

static int stbi__do_zlib(stbi__zbuf *a, char *obuf, int olen, int exp, int parse_header)
{
   a->zout_start = obuf;
   a->zout       = obuf;
   a->zout_end   = obuf + olen;
   a->z_expandable = exp;

   return stbi__parse_zlib(a, parse_header);
}

STBIDEF char *stbi_zlib_decode_malloc_guesssize(const char *buffer, int len, int initial_size, int *outlen)
{
   stbi__zbuf a;
   char *p = (char *) stbi__malloc(initial_size);
   if (p == NULL) return NULL;
   a.zbuffer = (stbi_uc *) buffer;
   a.zbuffer_end = (stbi_uc *) buffer + len;
   if (stbi__do_zlib(&a, p, initial_size, 1, 1)) {
	  if (outlen) *outlen = (int) (a.zout - a.zout_start);
	  return a.zout_start;
   } else {
	  STBI_FREE(a.zout_start);
	  return NULL;
   }
}

STBIDEF char *stbi_zlib_decode_malloc(char const *buffer, int len, int *outlen)
{
   return stbi_zlib_decode_malloc_guesssize(buffer, len, 16384, outlen);
}

STBIDEF char *stbi_zlib_decode_malloc_guesssize_headerflag(const char *buffer, int len, int initial_size, int *outlen, int parse_header)
{
   stbi__zbuf a;
   char *p = (char *) stbi__malloc(initial_size);
   if (p == NULL) return NULL;
   a.zbuffer = (stbi_uc *) buffer;
   a.zbuffer_end = (stbi_uc *) buffer + len;
   if (stbi__do_zlib(&a, p, initial_size, 1, parse_header)) {
	  if (outlen) *outlen = (int) (a.zout - a.zout_start);
	  return a.zout_start;
   } else {
	  STBI_FREE(a.zout_start);
	  return NULL;
   }
}

STBIDEF int stbi_zlib_decode_buffer(char *obuffer, int olen, char const *ibuffer, int ilen)
{
   stbi__zbuf a;
   a.zbuffer = (stbi_uc *) ibuffer;
   a.zbuffer_end = (stbi_uc *) ibuffer + ilen;
   if (stbi__do_zlib(&a, obuffer, olen, 0, 1))
	  return (int) (a.zout - a.zout_start);
   else
	  return -1;
}

STBIDEF char *stbi_zlib_decode_noheader_malloc(char const *buffer, int len, int *outlen)
{
   stbi__zbuf a;
   char *p = (char *) stbi__malloc(16384);
   if (p == NULL) return NULL;
   a.zbuffer = (stbi_uc *) buffer;
   a.zbuffer_end = (stbi_uc *) buffer+len;
   if (stbi__do_zlib(&a, p, 16384, 1, 0)) {
	  if (outlen) *outlen = (int) (a.zout - a.zout_start);
	  return a.zout_start;
   } else {
	  STBI_FREE(a.zout_start);
	  return NULL;
   }
}

STBIDEF int stbi_zlib_decode_noheader_buffer(char *obuffer, int olen, const char *ibuffer, int ilen)
{
   stbi__zbuf a;
   a.zbuffer = (stbi_uc *) ibuffer;
   a.zbuffer_end = (stbi_uc *) ibuffer + ilen;
   if (stbi__do_zlib(&a, obuffer, olen, 0, 0))
	  return (int) (a.zout - a.zout_start);
   else
	  return -1;
}
#endif

// public domain "baseline" PNG decoder   v0.10  Sean Barrett 2006-11-18
//    simple implementation
//      - only 8-bit samples
//      - no CRC checking
//      - allocates lots of intermediate memory
//        - avoids problem of streaming data between subsystems
//        - avoids explicit window management
//    performance
//      - uses stb_zlib, a PD zlib implementation with fast huffman decoding

#ifndef STBI_NO_PNG
typedef struct
{
   stbi__uint32 length;
   stbi__uint32 type;
} stbi__pngchunk;

static stbi__pngchunk stbi__get_chunk_header(stbi__context *s)
{
   stbi__pngchunk c;
   c.length = stbi__get32be(s);
   c.type   = stbi__get32be(s);
   return c;
}

static int stbi__check_png_header(stbi__context *s)
{
   static stbi_uc png_sig[8] = { 137,80,78,71,13,10,26,10 };
   int i;
   for (i=0; i < 8; ++i)
	  if (stbi__get8(s) != png_sig[i]) return stbi__err("bad png sig","Not a PNG");
   return 1;
}

typedef struct
{
   stbi__context *s;
   stbi_uc *idata, *expanded, *out;
} stbi__png;

enum {
   STBI__F_none=0,
   STBI__F_sub=1,
   STBI__F_up=2,
   STBI__F_avg=3,
   STBI__F_paeth=4,
   // synthetic filters used for first scanline to avoid needing a dummy row of 0s
   STBI__F_avg_first,
   STBI__F_paeth_first
};

static stbi_uc first_row_filter[5] =
{
   STBI__F_none,
   STBI__F_sub,
   STBI__F_none,
   STBI__F_avg_first,
   STBI__F_paeth_first
};

static int stbi__paeth(int a, int b, int c)
{
   int p = a + b - c;
   int pa = abs(p-a);
   int pb = abs(p-b);
   int pc = abs(p-c);
   if (pa <= pb && pa <= pc) return a;
   if (pb <= pc) return b;
   return c;
}

static stbi_uc stbi__depth_scale_table[9] = { 0, 0xff, 0x55, 0, 0x11, 0,0,0, 0x01 };

// create the png data from post-deflated data
static int stbi__create_png_image_raw(stbi__png *a, stbi_uc *raw, stbi__uint32 raw_len, int out_n, stbi__uint32 x, stbi__uint32 y, int depth, int color)
{
   stbi__context *s = a->s;
   stbi__uint32 i,j,stride = x*out_n;
   stbi__uint32 img_len, img_width_bytes;
   int k;
   int img_n = s->img_n; // copy it into a local for later

   STBI_ASSERT(out_n == s->img_n || out_n == s->img_n+1);
   a->out = (stbi_uc *) stbi__malloc(x * y * out_n); // extra bytes to write off the end into
   if (!a->out) return stbi__err("outofmem", "Out of memory");

   img_width_bytes = (((img_n * x * depth) + 7) >> 3);
   img_len = (img_width_bytes + 1) * y;
   if (s->img_x == x && s->img_y == y) {
	  if (raw_len != img_len) return stbi__err("not enough pixels","Corrupt PNG");
   } else { // interlaced:
	  if (raw_len < img_len) return stbi__err("not enough pixels","Corrupt PNG");
   }

   for (j=0; j < y; ++j) {
	  stbi_uc *cur = a->out + stride*j;
	  stbi_uc *prior = cur - stride;
	  int filter = *raw++;
	  int filter_bytes = img_n;
	  int width = x;
	  if (filter > 4)
		 return stbi__err("invalid filter","Corrupt PNG");

	  if (depth < 8) {
		 STBI_ASSERT(img_width_bytes <= x);
		 cur += x*out_n - img_width_bytes; // store output to the rightmost img_len bytes, so we can decode in place
		 filter_bytes = 1;
		 width = img_width_bytes;
	  }

	  // if first row, use special filter that doesn't sample previous row
	  if (j == 0) filter = first_row_filter[filter];

	  // handle first byte explicitly
	  for (k=0; k < filter_bytes; ++k) {
		 switch (filter) {
			case STBI__F_none       : cur[k] = raw[k]; break;
			case STBI__F_sub        : cur[k] = raw[k]; break;
			case STBI__F_up         : cur[k] = STBI__BYTECAST(raw[k] + prior[k]); break;
			case STBI__F_avg        : cur[k] = STBI__BYTECAST(raw[k] + (prior[k]>>1)); break;
			case STBI__F_paeth      : cur[k] = STBI__BYTECAST(raw[k] + stbi__paeth(0,prior[k],0)); break;
			case STBI__F_avg_first  : cur[k] = raw[k]; break;
			case STBI__F_paeth_first: cur[k] = raw[k]; break;
		 }
	  }

	  if (depth == 8) {
		 if (img_n != out_n)
			cur[img_n] = 255; // first pixel
		 raw += img_n;
		 cur += out_n;
		 prior += out_n;
	  } else {
		 raw += 1;
		 cur += 1;
		 prior += 1;
	  }

	  // this is a little gross, so that we don't switch per-pixel or per-component
	  if (depth < 8 || img_n == out_n) {
		 int nk = (width - 1)*img_n;
		 #define CASE(f) \
			 case f:     \
				for (k=0; k < nk; ++k)
		 switch (filter) {
			// "none" filter turns into a memcpy here; make that explicit.
			case STBI__F_none:         memcpy(cur, raw, nk); break;
			CASE(STBI__F_sub)          cur[k] = STBI__BYTECAST(raw[k] + cur[k-filter_bytes]); break;
			CASE(STBI__F_up)           cur[k] = STBI__BYTECAST(raw[k] + prior[k]); break;
			CASE(STBI__F_avg)          cur[k] = STBI__BYTECAST(raw[k] + ((prior[k] + cur[k-filter_bytes])>>1)); break;
			CASE(STBI__F_paeth)        cur[k] = STBI__BYTECAST(raw[k] + stbi__paeth(cur[k-filter_bytes],prior[k],prior[k-filter_bytes])); break;
			CASE(STBI__F_avg_first)    cur[k] = STBI__BYTECAST(raw[k] + (cur[k-filter_bytes] >> 1)); break;
			CASE(STBI__F_paeth_first)  cur[k] = STBI__BYTECAST(raw[k] + stbi__paeth(cur[k-filter_bytes],0,0)); break;
		 }
		 #undef CASE
		 raw += nk;
	  } else {
		 STBI_ASSERT(img_n+1 == out_n);
		 #define CASE(f) \
			 case f:     \
				for (i=x-1; i >= 1; --i, cur[img_n]=255,raw+=img_n,cur+=out_n,prior+=out_n) \
				   for (k=0; k < img_n; ++k)
		 switch (filter) {
			CASE(STBI__F_none)         cur[k] = raw[k]; break;
			CASE(STBI__F_sub)          cur[k] = STBI__BYTECAST(raw[k] + cur[k-out_n]); break;
			CASE(STBI__F_up)           cur[k] = STBI__BYTECAST(raw[k] + prior[k]); break;
			CASE(STBI__F_avg)          cur[k] = STBI__BYTECAST(raw[k] + ((prior[k] + cur[k-out_n])>>1)); break;
			CASE(STBI__F_paeth)        cur[k] = STBI__BYTECAST(raw[k] + stbi__paeth(cur[k-out_n],prior[k],prior[k-out_n])); break;
			CASE(STBI__F_avg_first)    cur[k] = STBI__BYTECAST(raw[k] + (cur[k-out_n] >> 1)); break;
			CASE(STBI__F_paeth_first)  cur[k] = STBI__BYTECAST(raw[k] + stbi__paeth(cur[k-out_n],0,0)); break;
		 }
		 #undef CASE
	  }
   }

   // we make a separate pass to expand bits to pixels; for performance,
   // this could run two scanlines behind the above code, so it won't
   // intefere with filtering but will still be in the cache.
   if (depth < 8) {
	  for (j=0; j < y; ++j) {
		 stbi_uc *cur = a->out + stride*j;
		 stbi_uc *in  = a->out + stride*j + x*out_n - img_width_bytes;
		 // unpack 1/2/4-bit into a 8-bit buffer. allows us to keep the common 8-bit path optimal at minimal cost for 1/2/4-bit
		 // png guarante byte alignment, if width is not multiple of 8/4/2 we'll decode dummy trailing data that will be skipped in the later loop
		 stbi_uc scale = (color == 0) ? stbi__depth_scale_table[depth] : 1; // scale grayscale values to 0..255 range

		 // note that the final byte might overshoot and write more data than desired.
		 // we can allocate enough data that this never writes out of memory, but it
		 // could also overwrite the next scanline. can it overwrite non-empty data
		 // on the next scanline? yes, consider 1-pixel-wide scanlines with 1-bit-per-pixel.
		 // so we need to explicitly clamp the final ones

		 if (depth == 4) {
			for (k=x*img_n; k >= 2; k-=2, ++in) {
			   *cur++ = scale * ((*in >> 4)       );
			   *cur++ = scale * ((*in     ) & 0x0f);
			}
			if (k > 0) *cur++ = scale * ((*in >> 4)       );
		 } else if (depth == 2) {
			for (k=x*img_n; k >= 4; k-=4, ++in) {
			   *cur++ = scale * ((*in >> 6)       );
			   *cur++ = scale * ((*in >> 4) & 0x03);
			   *cur++ = scale * ((*in >> 2) & 0x03);
			   *cur++ = scale * ((*in     ) & 0x03);
			}
			if (k > 0) *cur++ = scale * ((*in >> 6)       );
			if (k > 1) *cur++ = scale * ((*in >> 4) & 0x03);
			if (k > 2) *cur++ = scale * ((*in >> 2) & 0x03);
		 } else if (depth == 1) {
			for (k=x*img_n; k >= 8; k-=8, ++in) {
			   *cur++ = scale * ((*in >> 7)       );
			   *cur++ = scale * ((*in >> 6) & 0x01);
			   *cur++ = scale * ((*in >> 5) & 0x01);
			   *cur++ = scale * ((*in >> 4) & 0x01);
			   *cur++ = scale * ((*in >> 3) & 0x01);
			   *cur++ = scale * ((*in >> 2) & 0x01);
			   *cur++ = scale * ((*in >> 1) & 0x01);
			   *cur++ = scale * ((*in     ) & 0x01);
			}
			if (k > 0) *cur++ = scale * ((*in >> 7)       );
			if (k > 1) *cur++ = scale * ((*in >> 6) & 0x01);
			if (k > 2) *cur++ = scale * ((*in >> 5) & 0x01);
			if (k > 3) *cur++ = scale * ((*in >> 4) & 0x01);
			if (k > 4) *cur++ = scale * ((*in >> 3) & 0x01);
			if (k > 5) *cur++ = scale * ((*in >> 2) & 0x01);
			if (k > 6) *cur++ = scale * ((*in >> 1) & 0x01);
		 }
		 if (img_n != out_n) {
			// insert alpha = 255
			stbi_uc *cur = a->out + stride*j;
			int i;
			if (img_n == 1) {
			   for (i=x-1; i >= 0; --i) {
				  cur[i*2+1] = 255;
				  cur[i*2+0] = cur[i];
			   }
			} else {
			   STBI_ASSERT(img_n == 3);
			   for (i=x-1; i >= 0; --i) {
				  cur[i*4+3] = 255;
				  cur[i*4+2] = cur[i*3+2];
				  cur[i*4+1] = cur[i*3+1];
				  cur[i*4+0] = cur[i*3+0];
			   }
			}
		 }
	  }
   }

   return 1;
}

static int stbi__create_png_image(stbi__png *a, stbi_uc *image_data, stbi__uint32 image_data_len, int out_n, int depth, int color, int interlaced)
{
   stbi_uc *final;
   int p;
   if (!interlaced)
	  return stbi__create_png_image_raw(a, image_data, image_data_len, out_n, a->s->img_x, a->s->img_y, depth, color);

   // de-interlacing
   final = (stbi_uc *) stbi__malloc(a->s->img_x * a->s->img_y * out_n);
   for (p=0; p < 7; ++p) {
	  int xorig[] = { 0,4,0,2,0,1,0 };
	  int yorig[] = { 0,0,4,0,2,0,1 };
	  int xspc[]  = { 8,8,4,4,2,2,1 };
	  int yspc[]  = { 8,8,8,4,4,2,2 };
	  int i,j,x,y;
	  // pass1_x[4] = 0, pass1_x[5] = 1, pass1_x[12] = 1
	  x = (a->s->img_x - xorig[p] + xspc[p]-1) / xspc[p];
	  y = (a->s->img_y - yorig[p] + yspc[p]-1) / yspc[p];
	  if (x && y) {
		 stbi__uint32 img_len = ((((a->s->img_n * x * depth) + 7) >> 3) + 1) * y;
		 if (!stbi__create_png_image_raw(a, image_data, image_data_len, out_n, x, y, depth, color)) {
			STBI_FREE(final);
			return 0;
		 }
		 for (j=0; j < y; ++j) {
			for (i=0; i < x; ++i) {
			   int out_y = j*yspc[p]+yorig[p];
			   int out_x = i*xspc[p]+xorig[p];
			   memcpy(final + out_y*a->s->img_x*out_n + out_x*out_n,
					  a->out + (j*x+i)*out_n, out_n);
			}
		 }
		 STBI_FREE(a->out);
		 image_data += img_len;
		 image_data_len -= img_len;
	  }
   }
   a->out = final;

   return 1;
}

static int stbi__compute_transparency(stbi__png *z, stbi_uc tc[3], int out_n)
{
   stbi__context *s = z->s;
   stbi__uint32 i, pixel_count = s->img_x * s->img_y;
   stbi_uc *p = z->out;

   // compute color-based transparency, assuming we've
   // already got 255 as the alpha value in the output
   STBI_ASSERT(out_n == 2 || out_n == 4);

   if (out_n == 2) {
	  for (i=0; i < pixel_count; ++i) {
		 p[1] = (p[0] == tc[0] ? 0 : 255);
		 p += 2;
	  }
   } else {
	  for (i=0; i < pixel_count; ++i) {
		 if (p[0] == tc[0] && p[1] == tc[1] && p[2] == tc[2])
			p[3] = 0;
		 p += 4;
	  }
   }
   return 1;
}

static int stbi__expand_png_palette(stbi__png *a, stbi_uc *palette, int len, int pal_img_n)
{
   stbi__uint32 i, pixel_count = a->s->img_x * a->s->img_y;
   stbi_uc *p, *temp_out, *orig = a->out;

   p = (stbi_uc *) stbi__malloc(pixel_count * pal_img_n);
   if (p == NULL) return stbi__err("outofmem", "Out of memory");

   // between here and free(out) below, exitting would leak
   temp_out = p;

   if (pal_img_n == 3) {
	  for (i=0; i < pixel_count; ++i) {
		 int n = orig[i]*4;
		 p[0] = palette[n  ];
		 p[1] = palette[n+1];
		 p[2] = palette[n+2];
		 p += 3;
	  }
   } else {
	  for (i=0; i < pixel_count; ++i) {
		 int n = orig[i]*4;
		 p[0] = palette[n  ];
		 p[1] = palette[n+1];
		 p[2] = palette[n+2];
		 p[3] = palette[n+3];
		 p += 4;
	  }
   }
   STBI_FREE(a->out);
   a->out = temp_out;

   STBI_NOTUSED(len);

   return 1;
}

static int stbi__unpremultiply_on_load = 0;
static int stbi__de_iphone_flag = 0;

STBIDEF void stbi_set_unpremultiply_on_load(int flag_true_if_should_unpremultiply)
{
   stbi__unpremultiply_on_load = flag_true_if_should_unpremultiply;
}

STBIDEF void stbi_convert_iphone_png_to_rgb(int flag_true_if_should_convert)
{
   stbi__de_iphone_flag = flag_true_if_should_convert;
}

static void stbi__de_iphone(stbi__png *z)
{
   stbi__context *s = z->s;
   stbi__uint32 i, pixel_count = s->img_x * s->img_y;
   stbi_uc *p = z->out;

   if (s->img_out_n == 3) {  // convert bgr to rgb
	  for (i=0; i < pixel_count; ++i) {
		 stbi_uc t = p[0];
		 p[0] = p[2];
		 p[2] = t;
		 p += 3;
	  }
   } else {
	  STBI_ASSERT(s->img_out_n == 4);
	  if (stbi__unpremultiply_on_load) {
		 // convert bgr to rgb and unpremultiply
		 for (i=0; i < pixel_count; ++i) {
			stbi_uc a = p[3];
			stbi_uc t = p[0];
			if (a) {
			   p[0] = p[2] * 255 / a;
			   p[1] = p[1] * 255 / a;
			   p[2] =  t   * 255 / a;
			} else {
			   p[0] = p[2];
			   p[2] = t;
			}
			p += 4;
		 }
	  } else {
		 // convert bgr to rgb
		 for (i=0; i < pixel_count; ++i) {
			stbi_uc t = p[0];
			p[0] = p[2];
			p[2] = t;
			p += 4;
		 }
	  }
   }
}

#define STBI__PNG_TYPE(a,b,c,d)  (((a) << 24) + ((b) << 16) + ((c) << 8) + (d))

static int stbi__parse_png_file(stbi__png *z, int scan, int req_comp)
{
   stbi_uc palette[1024], pal_img_n=0;
   stbi_uc has_trans=0, tc[3];
   stbi__uint32 ioff=0, idata_limit=0, i, pal_len=0;
   int first=1,k,interlace=0, color=0, depth=0, is_iphone=0;
   stbi__context *s = z->s;

   z->expanded = NULL;
   z->idata = NULL;
   z->out = NULL;

   if (!stbi__check_png_header(s)) return 0;

   if (scan == STBI__SCAN_type) return 1;

   for (;;) {
	  stbi__pngchunk c = stbi__get_chunk_header(s);
	  switch (c.type) {
		 case STBI__PNG_TYPE('C','g','B','I'):
			is_iphone = 1;
			stbi__skip(s, c.length);
			break;
		 case STBI__PNG_TYPE('I','H','D','R'): {
			int comp,filter;
			if (!first) return stbi__err("multiple IHDR","Corrupt PNG");
			first = 0;
			if (c.length != 13) return stbi__err("bad IHDR len","Corrupt PNG");
			s->img_x = stbi__get32be(s); if (s->img_x > (1 << 24)) return stbi__err("too large","Very large image (corrupt?)");
			s->img_y = stbi__get32be(s); if (s->img_y > (1 << 24)) return stbi__err("too large","Very large image (corrupt?)");
			depth = stbi__get8(s);  if (depth != 1 && depth != 2 && depth != 4 && depth != 8)  return stbi__err("1/2/4/8-bit only","PNG not supported: 1/2/4/8-bit only");
			color = stbi__get8(s);  if (color > 6)         return stbi__err("bad ctype","Corrupt PNG");
			if (color == 3) pal_img_n = 3; else if (color & 1) return stbi__err("bad ctype","Corrupt PNG");
			comp  = stbi__get8(s);  if (comp) return stbi__err("bad comp method","Corrupt PNG");
			filter= stbi__get8(s);  if (filter) return stbi__err("bad filter method","Corrupt PNG");
			interlace = stbi__get8(s); if (interlace>1) return stbi__err("bad interlace method","Corrupt PNG");
			if (!s->img_x || !s->img_y) return stbi__err("0-pixel image","Corrupt PNG");
			if (!pal_img_n) {
			   s->img_n = (color & 2 ? 3 : 1) + (color & 4 ? 1 : 0);
			   if ((1 << 30) / s->img_x / s->img_n < s->img_y) return stbi__err("too large", "Image too large to decode");
			   if (scan == STBI__SCAN_header) return 1;
			} else {
			   // if paletted, then pal_n is our final components, and
			   // img_n is # components to decompress/filter.
			   s->img_n = 1;
			   if ((1 << 30) / s->img_x / 4 < s->img_y) return stbi__err("too large","Corrupt PNG");
			   // if SCAN_header, have to scan to see if we have a tRNS
			}
			break;
		 }

		 case STBI__PNG_TYPE('P','L','T','E'):  {
			if (first) return stbi__err("first not IHDR", "Corrupt PNG");
			if (c.length > 256*3) return stbi__err("invalid PLTE","Corrupt PNG");
			pal_len = c.length / 3;
			if (pal_len * 3 != c.length) return stbi__err("invalid PLTE","Corrupt PNG");
			for (i=0; i < pal_len; ++i) {
			   palette[i*4+0] = stbi__get8(s);
			   palette[i*4+1] = stbi__get8(s);
			   palette[i*4+2] = stbi__get8(s);
			   palette[i*4+3] = 255;
			}
			break;
		 }

		 case STBI__PNG_TYPE('t','R','N','S'): {
			if (first) return stbi__err("first not IHDR", "Corrupt PNG");
			if (z->idata) return stbi__err("tRNS after IDAT","Corrupt PNG");
			if (pal_img_n) {
			   if (scan == STBI__SCAN_header) { s->img_n = 4; return 1; }
			   if (pal_len == 0) return stbi__err("tRNS before PLTE","Corrupt PNG");
			   if (c.length > pal_len) return stbi__err("bad tRNS len","Corrupt PNG");
			   pal_img_n = 4;
			   for (i=0; i < c.length; ++i)
				  palette[i*4+3] = stbi__get8(s);
			} else {
			   if (!(s->img_n & 1)) return stbi__err("tRNS with alpha","Corrupt PNG");
			   if (c.length != (stbi__uint32) s->img_n*2) return stbi__err("bad tRNS len","Corrupt PNG");
			   has_trans = 1;
			   for (k=0; k < s->img_n; ++k)
				  tc[k] = (stbi_uc) (stbi__get16be(s) & 255) * stbi__depth_scale_table[depth]; // non 8-bit images will be larger
			}
			break;
		 }

		 case STBI__PNG_TYPE('I','D','A','T'): {
			if (first) return stbi__err("first not IHDR", "Corrupt PNG");
			if (pal_img_n && !pal_len) return stbi__err("no PLTE","Corrupt PNG");
			if (scan == STBI__SCAN_header) { s->img_n = pal_img_n; return 1; }
			if (ioff + c.length > idata_limit) {
			   stbi_uc *p;
			   if (idata_limit == 0) idata_limit = c.length > 4096 ? c.length : 4096;
			   while (ioff + c.length > idata_limit)
				  idata_limit *= 2;
			   p = (stbi_uc *) STBI_REALLOC(z->idata, idata_limit); if (p == NULL) return stbi__err("outofmem", "Out of memory");
			   z->idata = p;
			}
			if (!stbi__getn(s, z->idata+ioff,c.length)) return stbi__err("outofdata","Corrupt PNG");
			ioff += c.length;
			break;
		 }

		 case STBI__PNG_TYPE('I','E','N','D'): {
			stbi__uint32 raw_len, bpl;
			if (first) return stbi__err("first not IHDR", "Corrupt PNG");
			if (scan != STBI__SCAN_load) return 1;
			if (z->idata == NULL) return stbi__err("no IDAT","Corrupt PNG");
			// initial guess for decoded data size to avoid unnecessary reallocs
			bpl = (s->img_x * depth + 7) / 8; // bytes per line, per component
			raw_len = bpl * s->img_y * s->img_n /* pixels */ + s->img_y /* filter mode per row */;
			z->expanded = (stbi_uc *) stbi_zlib_decode_malloc_guesssize_headerflag((char *) z->idata, ioff, raw_len, (int *) &raw_len, !is_iphone);
			if (z->expanded == NULL) return 0; // zlib should set error
			STBI_FREE(z->idata); z->idata = NULL;
			if ((req_comp == s->img_n+1 && req_comp != 3 && !pal_img_n) || has_trans)
			   s->img_out_n = s->img_n+1;
			else
			   s->img_out_n = s->img_n;
			if (!stbi__create_png_image(z, z->expanded, raw_len, s->img_out_n, depth, color, interlace)) return 0;
			if (has_trans)
			   if (!stbi__compute_transparency(z, tc, s->img_out_n)) return 0;
			if (is_iphone && stbi__de_iphone_flag && s->img_out_n > 2)
			   stbi__de_iphone(z);
			if (pal_img_n) {
			   // pal_img_n == 3 or 4
			   s->img_n = pal_img_n; // record the actual colors we had
			   s->img_out_n = pal_img_n;
			   if (req_comp >= 3) s->img_out_n = req_comp;
			   if (!stbi__expand_png_palette(z, palette, pal_len, s->img_out_n))
				  return 0;
			}
			STBI_FREE(z->expanded); z->expanded = NULL;
			return 1;
		 }

		 default:
			// if critical, fail
			if (first) return stbi__err("first not IHDR", "Corrupt PNG");
			if ((c.type & (1 << 29)) == 0) {
			   #ifndef STBI_NO_FAILURE_STRINGS
			   // not threadsafe
			   static char invalid_chunk[] = "XXXX PNG chunk not known";
			   invalid_chunk[0] = STBI__BYTECAST(c.type >> 24);
			   invalid_chunk[1] = STBI__BYTECAST(c.type >> 16);
			   invalid_chunk[2] = STBI__BYTECAST(c.type >>  8);
			   invalid_chunk[3] = STBI__BYTECAST(c.type >>  0);
			   #endif
			   return stbi__err(invalid_chunk, "PNG not supported: unknown PNG chunk type");
			}
			stbi__skip(s, c.length);
			break;
	  }
	  // end of PNG chunk, read and skip CRC
	  stbi__get32be(s);
   }
}

static unsigned char *stbi__do_png(stbi__png *p, int *x, int *y, int *n, int req_comp)
{
   unsigned char *result=NULL;
   if (req_comp < 0 || req_comp > 4) return stbi__errpuc("bad req_comp", "Internal error");
   if (stbi__parse_png_file(p, STBI__SCAN_load, req_comp)) {
	  result = p->out;
	  p->out = NULL;
	  if (req_comp && req_comp != p->s->img_out_n) {
		 result = stbi__convert_format(result, p->s->img_out_n, req_comp, p->s->img_x, p->s->img_y);
		 p->s->img_out_n = req_comp;
		 if (result == NULL) return result;
	  }
	  *x = p->s->img_x;
	  *y = p->s->img_y;
	  if (n) *n = p->s->img_out_n;
   }
   STBI_FREE(p->out);      p->out      = NULL;
   STBI_FREE(p->expanded); p->expanded = NULL;
   STBI_FREE(p->idata);    p->idata    = NULL;

   return result;
}

static unsigned char *stbi__png_load(stbi__context *s, int *x, int *y, int *comp, int req_comp)
{
   stbi__png p;
   p.s = s;
   return stbi__do_png(&p, x,y,comp,req_comp);
}

static int stbi__png_test(stbi__context *s)
{
   int r;
   r = stbi__check_png_header(s);
   stbi__rewind(s);
   return r;
}

static int stbi__png_info_raw(stbi__png *p, int *x, int *y, int *comp)
{
   if (!stbi__parse_png_file(p, STBI__SCAN_header, 0)) {
	  stbi__rewind( p->s );
	  return 0;
   }
   if (x) *x = p->s->img_x;
   if (y) *y = p->s->img_y;
   if (comp) *comp = p->s->img_n;
   return 1;
}

static int stbi__png_info(stbi__context *s, int *x, int *y, int *comp)
{
   stbi__png p;
   p.s = s;
   return stbi__png_info_raw(&p, x, y, comp);
}
#endif

// Microsoft/Windows BMP image

#ifndef STBI_NO_BMP
static int stbi__bmp_test_raw(stbi__context *s)
{
   int r;
   int sz;
   if (stbi__get8(s) != 'B') return 0;
   if (stbi__get8(s) != 'M') return 0;
   stbi__get32le(s); // discard filesize
   stbi__get16le(s); // discard reserved
   stbi__get16le(s); // discard reserved
   stbi__get32le(s); // discard data offset
   sz = stbi__get32le(s);
   r = (sz == 12 || sz == 40 || sz == 56 || sz == 108 || sz == 124);
   return r;
}

static int stbi__bmp_test(stbi__context *s)
{
   int r = stbi__bmp_test_raw(s);
   stbi__rewind(s);
   return r;
}

// returns 0..31 for the highest set bit
static int stbi__high_bit(unsigned int z)
{
   int n=0;
   if (z == 0) return -1;
   if (z >= 0x10000) n += 16, z >>= 16;
   if (z >= 0x00100) n +=  8, z >>=  8;
   if (z >= 0x00010) n +=  4, z >>=  4;
   if (z >= 0x00004) n +=  2, z >>=  2;
   if (z >= 0x00002) n +=  1, z >>=  1;
   return n;
}

static int stbi__bitcount(unsigned int a)
{
   a = (a & 0x55555555) + ((a >>  1) & 0x55555555); // max 2
   a = (a & 0x33333333) + ((a >>  2) & 0x33333333); // max 4
   a = (a + (a >> 4)) & 0x0f0f0f0f; // max 8 per 4, now 8 bits
   a = (a + (a >> 8)); // max 16 per 8 bits
   a = (a + (a >> 16)); // max 32 per 8 bits
   return a & 0xff;
}

static int stbi__shiftsigned(int v, int shift, int bits)
{
   int result;
   int z=0;

   if (shift < 0) v <<= -shift;
   else v >>= shift;
   result = v;

   z = bits;
   while (z < 8) {
	  result += v >> z;
	  z += bits;
   }
   return result;
}

static stbi_uc *stbi__bmp_load(stbi__context *s, int *x, int *y, int *comp, int req_comp)
{
   stbi_uc *out;
   unsigned int mr=0,mg=0,mb=0,ma=0, fake_a=0;
   stbi_uc pal[256][4];
   int psize=0,i,j,compress=0,width;
   int bpp, flip_vertically, pad, target, offset, hsz;
   if (stbi__get8(s) != 'B' || stbi__get8(s) != 'M') return stbi__errpuc("not BMP", "Corrupt BMP");
   stbi__get32le(s); // discard filesize
   stbi__get16le(s); // discard reserved
   stbi__get16le(s); // discard reserved
   offset = stbi__get32le(s);
   hsz = stbi__get32le(s);
   if (hsz != 12 && hsz != 40 && hsz != 56 && hsz != 108 && hsz != 124) return stbi__errpuc("unknown BMP", "BMP type not supported: unknown");
   if (hsz == 12) {
	  s->img_x = stbi__get16le(s);
	  s->img_y = stbi__get16le(s);
   } else {
	  s->img_x = stbi__get32le(s);
	  s->img_y = stbi__get32le(s);
   }
   if (stbi__get16le(s) != 1) return stbi__errpuc("bad BMP", "bad BMP");
   bpp = stbi__get16le(s);
   if (bpp == 1) return stbi__errpuc("monochrome", "BMP type not supported: 1-bit");
   flip_vertically = ((int) s->img_y) > 0;
   s->img_y = abs((int) s->img_y);
   if (hsz == 12) {
	  if (bpp < 24)
		 psize = (offset - 14 - 24) / 3;
   } else {
	  compress = stbi__get32le(s);
	  if (compress == 1 || compress == 2) return stbi__errpuc("BMP RLE", "BMP type not supported: RLE");
	  stbi__get32le(s); // discard sizeof
	  stbi__get32le(s); // discard hres
	  stbi__get32le(s); // discard vres
	  stbi__get32le(s); // discard colorsused
	  stbi__get32le(s); // discard max important
	  if (hsz == 40 || hsz == 56) {
		 if (hsz == 56) {
			stbi__get32le(s);
			stbi__get32le(s);
			stbi__get32le(s);
			stbi__get32le(s);
		 }
		 if (bpp == 16 || bpp == 32) {
			mr = mg = mb = 0;
			if (compress == 0) {
			   if (bpp == 32) {
				  mr = 0xffu << 16;
				  mg = 0xffu <<  8;
				  mb = 0xffu <<  0;
				  ma = 0xffu << 24;
				  fake_a = 1; // @TODO: check for cases like alpha value is all 0 and switch it to 255
				  STBI_NOTUSED(fake_a);
			   } else {
				  mr = 31u << 10;
				  mg = 31u <<  5;
				  mb = 31u <<  0;
			   }
			} else if (compress == 3) {
			   mr = stbi__get32le(s);
			   mg = stbi__get32le(s);
			   mb = stbi__get32le(s);
			   // not documented, but generated by photoshop and handled by mspaint
			   if (mr == mg && mg == mb) {
				  // ?!?!?
				  return stbi__errpuc("bad BMP", "bad BMP");
			   }
			} else
			   return stbi__errpuc("bad BMP", "bad BMP");
		 }
	  } else {
		 STBI_ASSERT(hsz == 108 || hsz == 124);
		 mr = stbi__get32le(s);
		 mg = stbi__get32le(s);
		 mb = stbi__get32le(s);
		 ma = stbi__get32le(s);
		 stbi__get32le(s); // discard color space
		 for (i=0; i < 12; ++i)
			stbi__get32le(s); // discard color space parameters
		 if (hsz == 124) {
			stbi__get32le(s); // discard rendering intent
			stbi__get32le(s); // discard offset of profile data
			stbi__get32le(s); // discard size of profile data
			stbi__get32le(s); // discard reserved
		 }
	  }
	  if (bpp < 16)
		 psize = (offset - 14 - hsz) >> 2;
   }
   s->img_n = ma ? 4 : 3;
   if (req_comp && req_comp >= 3) // we can directly decode 3 or 4
	  target = req_comp;
   else
	  target = s->img_n; // if they want monochrome, we'll post-convert
   out = (stbi_uc *) stbi__malloc(target * s->img_x * s->img_y);
   if (!out) return stbi__errpuc("outofmem", "Out of memory");
   if (bpp < 16) {
	  int z=0;
	  if (psize == 0 || psize > 256) { STBI_FREE(out); return stbi__errpuc("invalid", "Corrupt BMP"); }
	  for (i=0; i < psize; ++i) {
		 pal[i][2] = stbi__get8(s);
		 pal[i][1] = stbi__get8(s);
		 pal[i][0] = stbi__get8(s);
		 if (hsz != 12) stbi__get8(s);
		 pal[i][3] = 255;
	  }
	  stbi__skip(s, offset - 14 - hsz - psize * (hsz == 12 ? 3 : 4));
	  if (bpp == 4) width = (s->img_x + 1) >> 1;
	  else if (bpp == 8) width = s->img_x;
	  else { STBI_FREE(out); return stbi__errpuc("bad bpp", "Corrupt BMP"); }
	  pad = (-width)&3;
	  for (j=0; j < (int) s->img_y; ++j) {
		 for (i=0; i < (int) s->img_x; i += 2) {
			int v=stbi__get8(s),v2=0;
			if (bpp == 4) {
			   v2 = v & 15;
			   v >>= 4;
			}
			out[z++] = pal[v][0];
			out[z++] = pal[v][1];
			out[z++] = pal[v][2];
			if (target == 4) out[z++] = 255;
			if (i+1 == (int) s->img_x) break;
			v = (bpp == 8) ? stbi__get8(s) : v2;
			out[z++] = pal[v][0];
			out[z++] = pal[v][1];
			out[z++] = pal[v][2];
			if (target == 4) out[z++] = 255;
		 }
		 stbi__skip(s, pad);
	  }
   } else {
	  int rshift=0,gshift=0,bshift=0,ashift=0,rcount=0,gcount=0,bcount=0,acount=0;
	  int z = 0;
	  int easy=0;
	  stbi__skip(s, offset - 14 - hsz);
	  if (bpp == 24) width = 3 * s->img_x;
	  else if (bpp == 16) width = 2*s->img_x;
	  else /* bpp = 32 and pad = 0 */ width=0;
	  pad = (-width) & 3;
	  if (bpp == 24) {
		 easy = 1;
	  } else if (bpp == 32) {
		 if (mb == 0xff && mg == 0xff00 && mr == 0x00ff0000 && ma == 0xff000000)
			easy = 2;
	  }
	  if (!easy) {
		 if (!mr || !mg || !mb) { STBI_FREE(out); return stbi__errpuc("bad masks", "Corrupt BMP"); }
		 // right shift amt to put high bit in position #7
		 rshift = stbi__high_bit(mr)-7; rcount = stbi__bitcount(mr);
		 gshift = stbi__high_bit(mg)-7; gcount = stbi__bitcount(mg);
		 bshift = stbi__high_bit(mb)-7; bcount = stbi__bitcount(mb);
		 ashift = stbi__high_bit(ma)-7; acount = stbi__bitcount(ma);
	  }
	  for (j=0; j < (int) s->img_y; ++j) {
		 if (easy) {
			for (i=0; i < (int) s->img_x; ++i) {
			   unsigned char a;
			   out[z+2] = stbi__get8(s);
			   out[z+1] = stbi__get8(s);
			   out[z+0] = stbi__get8(s);
			   z += 3;
			   a = (easy == 2 ? stbi__get8(s) : 255);
			   if (target == 4) out[z++] = a;
			}
		 } else {
			for (i=0; i < (int) s->img_x; ++i) {
			   stbi__uint32 v = (stbi__uint32) (bpp == 16 ? stbi__get16le(s) : stbi__get32le(s));
			   int a;
			   out[z++] = STBI__BYTECAST(stbi__shiftsigned(v & mr, rshift, rcount));
			   out[z++] = STBI__BYTECAST(stbi__shiftsigned(v & mg, gshift, gcount));
			   out[z++] = STBI__BYTECAST(stbi__shiftsigned(v & mb, bshift, bcount));
			   a = (ma ? stbi__shiftsigned(v & ma, ashift, acount) : 255);
			   if (target == 4) out[z++] = STBI__BYTECAST(a);
			}
		 }
		 stbi__skip(s, pad);
	  }
   }
   if (flip_vertically) {
	  stbi_uc t;
	  for (j=0; j < (int) s->img_y>>1; ++j) {
		 stbi_uc *p1 = out +      j     *s->img_x*target;
		 stbi_uc *p2 = out + (s->img_y-1-j)*s->img_x*target;
		 for (i=0; i < (int) s->img_x*target; ++i) {
			t = p1[i], p1[i] = p2[i], p2[i] = t;
		 }
	  }
   }

   if (req_comp && req_comp != target) {
	  out = stbi__convert_format(out, target, req_comp, s->img_x, s->img_y);
	  if (out == NULL) return out; // stbi__convert_format frees input on failure
   }

   *x = s->img_x;
   *y = s->img_y;
   if (comp) *comp = s->img_n;
   return out;
}
#endif

// Targa Truevision - TGA
// by Jonathan Dummer
#ifndef STBI_NO_TGA
static int stbi__tga_info(stbi__context *s, int *x, int *y, int *comp)
{
	int tga_w, tga_h, tga_comp;
	int sz;
	stbi__get8(s);                   // discard Offset
	sz = stbi__get8(s);              // color type
	if( sz > 1 ) {
		stbi__rewind(s);
		return 0;      // only RGB or indexed allowed
	}
	sz = stbi__get8(s);              // image type
	// only RGB or grey allowed, +/- RLE
	if ((sz != 1) && (sz != 2) && (sz != 3) && (sz != 9) && (sz != 10) && (sz != 11)) return 0;
	stbi__skip(s,9);
	tga_w = stbi__get16le(s);
	if( tga_w < 1 ) {
		stbi__rewind(s);
		return 0;   // test width
	}
	tga_h = stbi__get16le(s);
	if( tga_h < 1 ) {
		stbi__rewind(s);
		return 0;   // test height
	}
	sz = stbi__get8(s);               // bits per pixel
	// only RGB or RGBA or grey allowed
	if ((sz != 8) && (sz != 16) && (sz != 24) && (sz != 32)) {
		stbi__rewind(s);
		return 0;
	}
	tga_comp = sz;
	if (x) *x = tga_w;
	if (y) *y = tga_h;
	if (comp) *comp = tga_comp / 8;
	return 1;                   // seems to have passed everything
}

static int stbi__tga_test(stbi__context *s)
{
   int res;
   int sz;
   stbi__get8(s);      //   discard Offset
   sz = stbi__get8(s);   //   color type
   if ( sz > 1 ) return 0;   //   only RGB or indexed allowed
   sz = stbi__get8(s);   //   image type
   if ( (sz != 1) && (sz != 2) && (sz != 3) && (sz != 9) && (sz != 10) && (sz != 11) ) return 0;   //   only RGB or grey allowed, +/- RLE
   stbi__get16be(s);      //   discard palette start
   stbi__get16be(s);      //   discard palette length
   stbi__get8(s);         //   discard bits per palette color entry
   stbi__get16be(s);      //   discard x origin
   stbi__get16be(s);      //   discard y origin
   if ( stbi__get16be(s) < 1 ) return 0;      //   test width
   if ( stbi__get16be(s) < 1 ) return 0;      //   test height
   sz = stbi__get8(s);   //   bits per pixel
   if ( (sz != 8) && (sz != 16) && (sz != 24) && (sz != 32) )
	  res = 0;
   else
	  res = 1;
   stbi__rewind(s);
   return res;
}

static stbi_uc *stbi__tga_load(stbi__context *s, int *x, int *y, int *comp, int req_comp)
{
   //   read in the TGA header stuff
   int tga_offset = stbi__get8(s);
   int tga_indexed = stbi__get8(s);
   int tga_image_type = stbi__get8(s);
   int tga_is_RLE = 0;
   int tga_palette_start = stbi__get16le(s);
   int tga_palette_len = stbi__get16le(s);
   int tga_palette_bits = stbi__get8(s);
   int tga_x_origin = stbi__get16le(s);
   int tga_y_origin = stbi__get16le(s);
   int tga_width = stbi__get16le(s);
   int tga_height = stbi__get16le(s);
   int tga_bits_per_pixel = stbi__get8(s);
   int tga_comp = tga_bits_per_pixel / 8;
   int tga_inverted = stbi__get8(s);
   //   image data
   unsigned char *tga_data;
   unsigned char *tga_palette = NULL;
   int i, j;
   unsigned char raw_data[4];
   int RLE_count = 0;
   int RLE_repeating = 0;
   int read_next_pixel = 1;

   //   do a tiny bit of precessing
   if ( tga_image_type >= 8 )
   {
	  tga_image_type -= 8;
	  tga_is_RLE = 1;
   }
   /* int tga_alpha_bits = tga_inverted & 15; */
   tga_inverted = 1 - ((tga_inverted >> 5) & 1);

   //   error check
   if ( //(tga_indexed) ||
	  (tga_width < 1) || (tga_height < 1) ||
	  (tga_image_type < 1) || (tga_image_type > 3) ||
	  ((tga_bits_per_pixel != 8) && (tga_bits_per_pixel != 16) &&
	  (tga_bits_per_pixel != 24) && (tga_bits_per_pixel != 32))
	  )
   {
	  return NULL; // we don't report this as a bad TGA because we don't even know if it's TGA
   }

   //   If I'm paletted, then I'll use the number of bits from the palette
   if ( tga_indexed )
   {
	  tga_comp = tga_palette_bits / 8;
   }

   //   tga info
   *x = tga_width;
   *y = tga_height;
   if (comp) *comp = tga_comp;

   tga_data = (unsigned char*)stbi__malloc( tga_width * tga_height * tga_comp );
   if (!tga_data) return stbi__errpuc("outofmem", "Out of memory");

   // skip to the data's starting position (offset usually = 0)
   stbi__skip(s, tga_offset );

   if ( !tga_indexed && !tga_is_RLE) {
	  for (i=0; i < tga_height; ++i) {
		 int y = tga_inverted ? tga_height -i - 1 : i;
		 stbi_uc *tga_row = tga_data + y*tga_width*tga_comp;
		 stbi__getn(s, tga_row, tga_width * tga_comp);
	  }
   } else  {
	  //   do I need to load a palette?
	  if ( tga_indexed)
	  {
		 //   any data to skip? (offset usually = 0)
		 stbi__skip(s, tga_palette_start );
		 //   load the palette
		 tga_palette = (unsigned char*)stbi__malloc( tga_palette_len * tga_palette_bits / 8 );
		 if (!tga_palette) {
			STBI_FREE(tga_data);
			return stbi__errpuc("outofmem", "Out of memory");
		 }
		 if (!stbi__getn(s, tga_palette, tga_palette_len * tga_palette_bits / 8 )) {
			STBI_FREE(tga_data);
			STBI_FREE(tga_palette);
			return stbi__errpuc("bad palette", "Corrupt TGA");
		 }
	  }
	  //   load the data
	  for (i=0; i < tga_width * tga_height; ++i)
	  {
		 //   if I'm in RLE mode, do I need to get a RLE stbi__pngchunk?
		 if ( tga_is_RLE )
		 {
			if ( RLE_count == 0 )
			{
			   //   yep, get the next byte as a RLE command
			   int RLE_cmd = stbi__get8(s);
			   RLE_count = 1 + (RLE_cmd & 127);
			   RLE_repeating = RLE_cmd >> 7;
			   read_next_pixel = 1;
			} else if ( !RLE_repeating )
			{
			   read_next_pixel = 1;
			}
		 } else
		 {
			read_next_pixel = 1;
		 }
		 //   OK, if I need to read a pixel, do it now
		 if ( read_next_pixel )
		 {
			//   load however much data we did have
			if ( tga_indexed )
			{
			   //   read in 1 byte, then perform the lookup
			   int pal_idx = stbi__get8(s);
			   if ( pal_idx >= tga_palette_len )
			   {
				  //   invalid index
				  pal_idx = 0;
			   }
			   pal_idx *= tga_bits_per_pixel / 8;
			   for (j = 0; j*8 < tga_bits_per_pixel; ++j)
			   {
				  raw_data[j] = tga_palette[pal_idx+j];
			   }
			} else
			{
			   //   read in the data raw
			   for (j = 0; j*8 < tga_bits_per_pixel; ++j)
			   {
				  raw_data[j] = stbi__get8(s);
			   }
			}
			//   clear the reading flag for the next pixel
			read_next_pixel = 0;
		 } // end of reading a pixel

		 // copy data
		 for (j = 0; j < tga_comp; ++j)
		   tga_data[i*tga_comp+j] = raw_data[j];

		 //   in case we're in RLE mode, keep counting down
		 --RLE_count;
	  }
	  //   do I need to invert the image?
	  if ( tga_inverted )
	  {
		 for (j = 0; j*2 < tga_height; ++j)
		 {
			int index1 = j * tga_width * tga_comp;
			int index2 = (tga_height - 1 - j) * tga_width * tga_comp;
			for (i = tga_width * tga_comp; i > 0; --i)
			{
			   unsigned char temp = tga_data[index1];
			   tga_data[index1] = tga_data[index2];
			   tga_data[index2] = temp;
			   ++index1;
			   ++index2;
			}
		 }
	  }
	  //   clear my palette, if I had one
	  if ( tga_palette != NULL )
	  {
		 STBI_FREE( tga_palette );
	  }
   }

   // swap RGB
   if (tga_comp >= 3)
   {
	  unsigned char* tga_pixel = tga_data;
	  for (i=0; i < tga_width * tga_height; ++i)
	  {
		 unsigned char temp = tga_pixel[0];
		 tga_pixel[0] = tga_pixel[2];
		 tga_pixel[2] = temp;
		 tga_pixel += tga_comp;
	  }
   }

   // convert to target component count
   if (req_comp && req_comp != tga_comp)
	  tga_data = stbi__convert_format(tga_data, tga_comp, req_comp, tga_width, tga_height);

   //   the things I do to get rid of an error message, and yet keep
   //   Microsoft's C compilers happy... [8^(
   tga_palette_start = tga_palette_len = tga_palette_bits =
		 tga_x_origin = tga_y_origin = 0;
   //   OK, done
   return tga_data;
}
#endif

// *************************************************************************************************
// Photoshop PSD loader -- PD by Thatcher Ulrich, integration by Nicolas Schulz, tweaked by STB

#ifndef STBI_NO_PSD
static int stbi__psd_test(stbi__context *s)
{
   int r = (stbi__get32be(s) == 0x38425053);
   stbi__rewind(s);
   return r;
}

static stbi_uc *stbi__psd_load(stbi__context *s, int *x, int *y, int *comp, int req_comp)
{
   int   pixelCount;
   int channelCount, compression;
   int channel, i, count, len;
   int w,h;
   stbi_uc *out;

   // Check identifier
   if (stbi__get32be(s) != 0x38425053)   // "8BPS"
	  return stbi__errpuc("not PSD", "Corrupt PSD image");

   // Check file type version.
   if (stbi__get16be(s) != 1)
	  return stbi__errpuc("wrong version", "Unsupported version of PSD image");

   // Skip 6 reserved bytes.
   stbi__skip(s, 6 );

   // Read the number of channels (R, G, B, A, etc).
   channelCount = stbi__get16be(s);
   if (channelCount < 0 || channelCount > 16)
	  return stbi__errpuc("wrong channel count", "Unsupported number of channels in PSD image");

   // Read the rows and columns of the image.
   h = stbi__get32be(s);
   w = stbi__get32be(s);

   // Make sure the depth is 8 bits.
   if (stbi__get16be(s) != 8)
	  return stbi__errpuc("unsupported bit depth", "PSD bit depth is not 8 bit");

   // Make sure the color mode is RGB.
   // Valid options are:
   //   0: Bitmap
   //   1: Grayscale
   //   2: Indexed color
   //   3: RGB color
   //   4: CMYK color
   //   7: Multichannel
   //   8: Duotone
   //   9: Lab color
   if (stbi__get16be(s) != 3)
	  return stbi__errpuc("wrong color format", "PSD is not in RGB color format");

   // Skip the Mode Data.  (It's the palette for indexed color; other info for other modes.)
   stbi__skip(s,stbi__get32be(s) );

   // Skip the image resources.  (resolution, pen tool paths, etc)
   stbi__skip(s, stbi__get32be(s) );

   // Skip the reserved data.
   stbi__skip(s, stbi__get32be(s) );

   // Find out if the data is compressed.
   // Known values:
   //   0: no compression
   //   1: RLE compressed
   compression = stbi__get16be(s);
   if (compression > 1)
	  return stbi__errpuc("bad compression", "PSD has an unknown compression format");

   // Create the destination image.
   out = (stbi_uc *) stbi__malloc(channelCount * w*h);
   if (!out) return stbi__errpuc("outofmem", "Out of memory");
   pixelCount = w*h;

   // Initialize the data to zero.
   //memset( out, 0, pixelCount * 4 );

   // Finally, the image data.
   if (compression) {
	  // RLE as used by .PSD and .TIFF
	  // Loop until you get the number of unpacked bytes you are expecting:
	  //     Read the next source byte into n.
	  //     If n is between 0 and 127 inclusive, copy the next n+1 bytes literally.
	  //     Else if n is between -127 and -1 inclusive, copy the next byte -n+1 times.
	  //     Else if n is 128, noop.
	  // Endloop

	  // The RLE-compressed data is preceeded by a 2-byte data count for each row in the data,
	  // which we're going to just skip.
	  stbi__skip(s, h * channelCount * 2 );

	  // Read the RLE data by channel.
	  for (channel = 0; channel < channelCount; channel++) {
		 stbi_uc *p;

		 p = out+channel;
		 if (channel >= channelCount) {
			// Fill this channel with default data.
			for (i = 0; i < pixelCount; i++) *p = (channel == 3 ? 255 : 0), p += channelCount;
		 } else {
			// Read the RLE data.
			count = 0;
			while (count < pixelCount) {
			   len = stbi__get8(s);
			   if (len == 128) {
				  // No-op.
			   } else if (len < 128) {
				  // Copy next len+1 bytes literally.
				  len++;
				  count += len;
				  while (len) {
					 *p = stbi__get8(s);
					 p += channelCount;
					 len--;
				  }
			   } else if (len > 128) {
				  stbi_uc   val;
				  // Next -len+1 bytes in the dest are replicated from next source byte.
				  // (Interpret len as a negative 8-bit int.)
				  len ^= 0x0FF;
				  len += 2;
				  val = stbi__get8(s);
				  count += len;
				  while (len) {
					 *p = val;
					 p += channelCount;
					 len--;
				  }
			   }
			}
		 }
	  }

   } else {
	  // We're at the raw image data.  It's each channel in order (Red, Green, Blue, Alpha, ...)
	  // where each channel consists of an 8-bit value for each pixel in the image.

	  // Read the data by channel.
	  for (channel = 0; channel < channelCount; channel++) {
		 stbi_uc *p;

		 p = out + channel;
		 if (channel > channelCount) {
			// Fill this channel with default data.
			for (i = 0; i < pixelCount; i++) *p = channel == 3 ? 255 : 0, p += channelCount;
		 } else {
			// Read the data.
			for (i = 0; i < pixelCount; i++)
			   *p = stbi__get8(s), p += channelCount;
		 }
	  }
   }

   if (req_comp && req_comp != channelCount) {
	  out = stbi__convert_format(out, channelCount, req_comp, w, h);
	  if (out == NULL) return out; // stbi__convert_format frees input on failure
   }

   if (comp) *comp = channelCount;
   *y = h;
   *x = w;

   return out;
}
#endif

// *************************************************************************************************
// Softimage PIC loader
// by Tom Seddon
//
// See http://softimage.wiki.softimage.com/index.php/INFO:_PIC_file_format
// See http://ozviz.wasp.uwa.edu.au/~pbourke/dataformats/softimagepic/

#ifndef STBI_NO_PIC
static int stbi__pic_is4(stbi__context *s,const char *str)
{
   int i;
   for (i=0; i<4; ++i)
	  if (stbi__get8(s) != (stbi_uc)str[i])
		 return 0;

   return 1;
}

static int stbi__pic_test_core(stbi__context *s)
{
   int i;

   if (!stbi__pic_is4(s,"\x53\x80\xF6\x34"))
	  return 0;

   for(i=0;i<84;++i)
	  stbi__get8(s);

   if (!stbi__pic_is4(s,"PICT"))
	  return 0;

   return 1;
}

typedef struct
{
   stbi_uc size,type,channel;
} stbi__pic_packet;

static stbi_uc *stbi__readval(stbi__context *s, int channel, stbi_uc *dest)
{
   int mask=0x80, i;

   for (i=0; i<4; ++i, mask>>=1) {
	  if (channel & mask) {
		 if (stbi__at_eof(s)) return stbi__errpuc("bad file","PIC file too short");
		 dest[i]=stbi__get8(s);
	  }
   }

   return dest;
}

static void stbi__copyval(int channel,stbi_uc *dest,const stbi_uc *src)
{
   int mask=0x80,i;

   for (i=0;i<4; ++i, mask>>=1)
	  if (channel&mask)
		 dest[i]=src[i];
}

static stbi_uc *stbi__pic_load_core(stbi__context *s,int width,int height,int *comp, stbi_uc *result)
{
   int act_comp=0,num_packets=0,y,chained;
   stbi__pic_packet packets[10];

   // this will (should...) cater for even some bizarre stuff like having data
	// for the same channel in multiple packets.
   do {
	  stbi__pic_packet *packet;

	  if (num_packets==sizeof(packets)/sizeof(packets[0]))
		 return stbi__errpuc("bad format","too many packets");

	  packet = &packets[num_packets++];

	  chained = stbi__get8(s);
	  packet->size    = stbi__get8(s);
	  packet->type    = stbi__get8(s);
	  packet->channel = stbi__get8(s);

	  act_comp |= packet->channel;

	  if (stbi__at_eof(s))          return stbi__errpuc("bad file","file too short (reading packets)");
	  if (packet->size != 8)  return stbi__errpuc("bad format","packet isn't 8bpp");
   } while (chained);

   *comp = (act_comp & 0x10 ? 4 : 3); // has alpha channel?

   for(y=0; y<height; ++y) {
	  int packet_idx;

	  for(packet_idx=0; packet_idx < num_packets; ++packet_idx) {
		 stbi__pic_packet *packet = &packets[packet_idx];
		 stbi_uc *dest = result+y*width*4;

		 switch (packet->type) {
			default:
			   return stbi__errpuc("bad format","packet has bad compression type");

			case 0: {//uncompressed
			   int x;

			   for(x=0;x<width;++x, dest+=4)
				  if (!stbi__readval(s,packet->channel,dest))
					 return 0;
			   break;
			}

			case 1://Pure RLE
			   {
				  int left=width, i;

				  while (left>0) {
					 stbi_uc count,value[4];

					 count=stbi__get8(s);
					 if (stbi__at_eof(s))   return stbi__errpuc("bad file","file too short (pure read count)");

					 if (count > left)
						count = (stbi_uc) left;

					 if (!stbi__readval(s,packet->channel,value))  return 0;

					 for(i=0; i<count; ++i,dest+=4)
						stbi__copyval(packet->channel,dest,value);
					 left -= count;
				  }
			   }
			   break;

			case 2: {//Mixed RLE
			   int left=width;
			   while (left>0) {
				  int count = stbi__get8(s), i;
				  if (stbi__at_eof(s))  return stbi__errpuc("bad file","file too short (mixed read count)");

				  if (count >= 128) { // Repeated
					 stbi_uc value[4];
					 int i;

					 if (count==128)
						count = stbi__get16be(s);
					 else
						count -= 127;
					 if (count > left)
						return stbi__errpuc("bad file","scanline overrun");

					 if (!stbi__readval(s,packet->channel,value))
						return 0;

					 for(i=0;i<count;++i, dest += 4)
						stbi__copyval(packet->channel,dest,value);
				  } else { // Raw
					 ++count;
					 if (count>left) return stbi__errpuc("bad file","scanline overrun");

					 for(i=0;i<count;++i, dest+=4)
						if (!stbi__readval(s,packet->channel,dest))
						   return 0;
				  }
				  left-=count;
			   }
			   break;
			}
		 }
	  }
   }

   return result;
}

static stbi_uc *stbi__pic_load(stbi__context *s,int *px,int *py,int *comp,int req_comp)
{
   stbi_uc *result;
   int i, x,y;

   for (i=0; i<92; ++i)
	  stbi__get8(s);

   x = stbi__get16be(s);
   y = stbi__get16be(s);
   if (stbi__at_eof(s))  return stbi__errpuc("bad file","file too short (pic header)");
   if ((1 << 28) / x < y) return stbi__errpuc("too large", "Image too large to decode");

   stbi__get32be(s); //skip `ratio'
   stbi__get16be(s); //skip `fields'
   stbi__get16be(s); //skip `pad'

   // intermediate buffer is RGBA
   result = (stbi_uc *) stbi__malloc(x*y*4);
   memset(result, 0xff, x*y*4);

   if (!stbi__pic_load_core(s,x,y,comp, result)) {
	  STBI_FREE(result);
	  result=0;
   }
   *px = x;
   *py = y;
   if (req_comp == 0) req_comp = *comp;
   result=stbi__convert_format(result,4,req_comp,x,y);

   return result;
}

static int stbi__pic_test(stbi__context *s)
{
   int r = stbi__pic_test_core(s);
   stbi__rewind(s);
   return r;
}
#endif

// *************************************************************************************************
// GIF loader -- public domain by Jean-Marc Lienher -- simplified/shrunk by stb

#ifndef STBI_NO_GIF
typedef struct
{
   stbi__int16 prefix;
   stbi_uc first;
   stbi_uc suffix;
} stbi__gif_lzw;

typedef struct
{
   int w,h;
   stbi_uc *out;                 // output buffer (always 4 components)
   int flags, bgindex, ratio, transparent, eflags;
   stbi_uc  pal[256][4];
   stbi_uc lpal[256][4];
   stbi__gif_lzw codes[4096];
   stbi_uc *color_table;
   int parse, step;
   int lflags;
   int start_x, start_y;
   int max_x, max_y;
   int cur_x, cur_y;
   int line_size;
} stbi__gif;

static int stbi__gif_test_raw(stbi__context *s)
{
   int sz;
   if (stbi__get8(s) != 'G' || stbi__get8(s) != 'I' || stbi__get8(s) != 'F' || stbi__get8(s) != '8') return 0;
   sz = stbi__get8(s);
   if (sz != '9' && sz != '7') return 0;
   if (stbi__get8(s) != 'a') return 0;
   return 1;
}

static int stbi__gif_test(stbi__context *s)
{
   int r = stbi__gif_test_raw(s);
   stbi__rewind(s);
   return r;
}

static void stbi__gif_parse_colortable(stbi__context *s, stbi_uc pal[256][4], int num_entries, int transp)
{
   int i;
   for (i=0; i < num_entries; ++i) {
	  pal[i][2] = stbi__get8(s);
	  pal[i][1] = stbi__get8(s);
	  pal[i][0] = stbi__get8(s);
	  pal[i][3] = transp == i ? 0 : 255;
   }
}

static int stbi__gif_header(stbi__context *s, stbi__gif *g, int *comp, int is_info)
{
   stbi_uc version;
   if (stbi__get8(s) != 'G' || stbi__get8(s) != 'I' || stbi__get8(s) != 'F' || stbi__get8(s) != '8')
	  return stbi__err("not GIF", "Corrupt GIF");

   version = stbi__get8(s);
   if (version != '7' && version != '9')    return stbi__err("not GIF", "Corrupt GIF");
   if (stbi__get8(s) != 'a')                return stbi__err("not GIF", "Corrupt GIF");

   stbi__g_failure_reason = "";
   g->w = stbi__get16le(s);
   g->h = stbi__get16le(s);
   g->flags = stbi__get8(s);
   g->bgindex = stbi__get8(s);
   g->ratio = stbi__get8(s);
   g->transparent = -1;

   if (comp != 0) *comp = 4;  // can't actually tell whether it's 3 or 4 until we parse the comments

   if (is_info) return 1;

   if (g->flags & 0x80)
	  stbi__gif_parse_colortable(s,g->pal, 2 << (g->flags & 7), -1);

   return 1;
}

static int stbi__gif_info_raw(stbi__context *s, int *x, int *y, int *comp)
{
   stbi__gif g;
   if (!stbi__gif_header(s, &g, comp, 1)) {
	  stbi__rewind( s );
	  return 0;
   }
   if (x) *x = g.w;
   if (y) *y = g.h;
   return 1;
}

static void stbi__out_gif_code(stbi__gif *g, stbi__uint16 code)
{
   stbi_uc *p, *c;

   // recurse to decode the prefixes, since the linked-list is backwards,
   // and working backwards through an interleaved image would be nasty
   if (g->codes[code].prefix >= 0)
	  stbi__out_gif_code(g, g->codes[code].prefix);

   if (g->cur_y >= g->max_y) return;

   p = &g->out[g->cur_x + g->cur_y];
   c = &g->color_table[g->codes[code].suffix * 4];

   if (c[3] >= 128) {
	  p[0] = c[2];
	  p[1] = c[1];
	  p[2] = c[0];
	  p[3] = c[3];
   }
   g->cur_x += 4;

   if (g->cur_x >= g->max_x) {
	  g->cur_x = g->start_x;
	  g->cur_y += g->step;

	  while (g->cur_y >= g->max_y && g->parse > 0) {
		 g->step = (1 << g->parse) * g->line_size;
		 g->cur_y = g->start_y + (g->step >> 1);
		 --g->parse;
	  }
   }
}

static stbi_uc *stbi__process_gif_raster(stbi__context *s, stbi__gif *g)
{
   stbi_uc lzw_cs;
   stbi__int32 len, code;
   stbi__uint32 first;
   stbi__int32 codesize, codemask, avail, oldcode, bits, valid_bits, clear;
   stbi__gif_lzw *p;

   lzw_cs = stbi__get8(s);
   clear = 1 << lzw_cs;
   first = 1;
   codesize = lzw_cs + 1;
   codemask = (1 << codesize) - 1;
   bits = 0;
   valid_bits = 0;
   for (code = 0; code < clear; code++) {
	  g->codes[code].prefix = -1;
	  g->codes[code].first = (stbi_uc) code;
	  g->codes[code].suffix = (stbi_uc) code;
   }

   // support no starting clear code
   avail = clear+2;
   oldcode = -1;

   len = 0;
   for(;;) {
	  if (valid_bits < codesize) {
		 if (len == 0) {
			len = stbi__get8(s); // start new block
			if (len == 0)
			   return g->out;
		 }
		 --len;
		 bits |= (stbi__int32) stbi__get8(s) << valid_bits;
		 valid_bits += 8;
	  } else {
		 stbi__int32 code = bits & codemask;
		 bits >>= codesize;
		 valid_bits -= codesize;
		 // @OPTIMIZE: is there some way we can accelerate the non-clear path?
		 if (code == clear) {  // clear code
			codesize = lzw_cs + 1;
			codemask = (1 << codesize) - 1;
			avail = clear + 2;
			oldcode = -1;
			first = 0;
		 } else if (code == clear + 1) { // end of stream code
			stbi__skip(s, len);
			while ((len = stbi__get8(s)) > 0)
			   stbi__skip(s,len);
			return g->out;
		 } else if (code <= avail) {
			if (first) return stbi__errpuc("no clear code", "Corrupt GIF");

			if (oldcode >= 0) {
			   p = &g->codes[avail++];
			   if (avail > 4096)        return stbi__errpuc("too many codes", "Corrupt GIF");
			   p->prefix = (stbi__int16) oldcode;
			   p->first = g->codes[oldcode].first;
			   p->suffix = (code == avail) ? p->first : g->codes[code].first;
			} else if (code == avail)
			   return stbi__errpuc("illegal code in raster", "Corrupt GIF");

			stbi__out_gif_code(g, (stbi__uint16) code);

			if ((avail & codemask) == 0 && avail <= 0x0FFF) {
			   codesize++;
			   codemask = (1 << codesize) - 1;
			}

			oldcode = code;
		 } else {
			return stbi__errpuc("illegal code in raster", "Corrupt GIF");
		 }
	  }
   }
}

static void stbi__fill_gif_background(stbi__gif *g)
{
   int i;
   stbi_uc *c = g->pal[g->bgindex];
   // @OPTIMIZE: write a dword at a time
   for (i = 0; i < g->w * g->h * 4; i += 4) {
	  stbi_uc *p  = &g->out[i];
	  p[0] = c[2];
	  p[1] = c[1];
	  p[2] = c[0];
	  p[3] = c[3];
   }
}

// this function is designed to support animated gifs, although stb_image doesn't support it
static stbi_uc *stbi__gif_load_next(stbi__context *s, stbi__gif *g, int *comp, int req_comp)
{
   int i;
   stbi_uc *old_out = 0;

   if (g->out == 0) {
	  if (!stbi__gif_header(s, g, comp,0))     return 0; // stbi__g_failure_reason set by stbi__gif_header
	  g->out = (stbi_uc *) stbi__malloc(4 * g->w * g->h);
	  if (g->out == 0)                      return stbi__errpuc("outofmem", "Out of memory");
	  stbi__fill_gif_background(g);
   } else {
	  // animated-gif-only path
	  if (((g->eflags & 0x1C) >> 2) == 3) {
		 old_out = g->out;
		 g->out = (stbi_uc *) stbi__malloc(4 * g->w * g->h);
		 if (g->out == 0)                   return stbi__errpuc("outofmem", "Out of memory");
		 memcpy(g->out, old_out, g->w*g->h*4);
	  }
   }

   for (;;) {
	  switch (stbi__get8(s)) {
		 case 0x2C: /* Image Descriptor */
		 {
			stbi__int32 x, y, w, h;
			stbi_uc *o;

			x = stbi__get16le(s);
			y = stbi__get16le(s);
			w = stbi__get16le(s);
			h = stbi__get16le(s);
			if (((x + w) > (g->w)) || ((y + h) > (g->h)))
			   return stbi__errpuc("bad Image Descriptor", "Corrupt GIF");

			g->line_size = g->w * 4;
			g->start_x = x * 4;
			g->start_y = y * g->line_size;
			g->max_x   = g->start_x + w * 4;
			g->max_y   = g->start_y + h * g->line_size;
			g->cur_x   = g->start_x;
			g->cur_y   = g->start_y;

			g->lflags = stbi__get8(s);

			if (g->lflags & 0x40) {
			   g->step = 8 * g->line_size; // first interlaced spacing
			   g->parse = 3;
			} else {
			   g->step = g->line_size;
			   g->parse = 0;
			}

			if (g->lflags & 0x80) {
			   stbi__gif_parse_colortable(s,g->lpal, 2 << (g->lflags & 7), g->eflags & 0x01 ? g->transparent : -1);
			   g->color_table = (stbi_uc *) g->lpal;
			} else if (g->flags & 0x80) {
			   for (i=0; i < 256; ++i)  // @OPTIMIZE: stbi__jpeg_reset only the previous transparent
				  g->pal[i][3] = 255;
			   if (g->transparent >= 0 && (g->eflags & 0x01))
				  g->pal[g->transparent][3] = 0;
			   g->color_table = (stbi_uc *) g->pal;
			} else
			   return stbi__errpuc("missing color table", "Corrupt GIF");

			o = stbi__process_gif_raster(s, g);
			if (o == NULL) return NULL;

			if (req_comp && req_comp != 4)
			   o = stbi__convert_format(o, 4, req_comp, g->w, g->h);
			return o;
		 }

		 case 0x21: // Comment Extension.
		 {
			int len;
			if (stbi__get8(s) == 0xF9) { // Graphic Control Extension.
			   len = stbi__get8(s);
			   if (len == 4) {
				  g->eflags = stbi__get8(s);
				  stbi__get16le(s); // delay
				  g->transparent = stbi__get8(s);
			   } else {
				  stbi__skip(s, len);
				  break;
			   }
			}
			while ((len = stbi__get8(s)) != 0)
			   stbi__skip(s, len);
			break;
		 }

		 case 0x3B: // gif stream termination code
			return (stbi_uc *) s; // using '1' causes warning on some compilers

		 default:
			return stbi__errpuc("unknown code", "Corrupt GIF");
	  }
   }
}

static stbi_uc *stbi__gif_load(stbi__context *s, int *x, int *y, int *comp, int req_comp)
{
   stbi_uc *u = 0;
   stbi__gif g;
   memset(&g, 0, sizeof(g));

   u = stbi__gif_load_next(s, &g, comp, req_comp);
   if (u == (stbi_uc *) s) u = 0;  // end of animated gif marker
   if (u) {
	  *x = g.w;
	  *y = g.h;
   }

   return u;
}

static int stbi__gif_info(stbi__context *s, int *x, int *y, int *comp)
{
   return stbi__gif_info_raw(s,x,y,comp);
}
#endif

// *************************************************************************************************
// Radiance RGBE HDR loader
// originally by Nicolas Schulz
#ifndef STBI_NO_HDR
static int stbi__hdr_test_core(stbi__context *s)
{
   const char *signature = "#?RADIANCE\n";
   int i;
   for (i=0; signature[i]; ++i)
	  if (stbi__get8(s) != signature[i])
		 return 0;
   return 1;
}

static int stbi__hdr_test(stbi__context* s)
{
   int r = stbi__hdr_test_core(s);
   stbi__rewind(s);
   return r;
}

#define STBI__HDR_BUFLEN  1024
static char *stbi__hdr_gettoken(stbi__context *z, char *buffer)
{
   int len=0;
   char c = '\0';

   c = (char) stbi__get8(z);

   while (!stbi__at_eof(z) && c != '\n') {
	  buffer[len++] = c;
	  if (len == STBI__HDR_BUFLEN-1) {
		 // flush to end of line
		 while (!stbi__at_eof(z) && stbi__get8(z) != '\n')
			;
		 break;
	  }
	  c = (char) stbi__get8(z);
   }

   buffer[len] = 0;
   return buffer;
}

static void stbi__hdr_convert(float *output, stbi_uc *input, int req_comp)
{
   if ( input[3] != 0 ) {
	  float f1;
	  // Exponent
	  f1 = (float) ldexp(1.0f, input[3] - (int)(128 + 8));
	  if (req_comp <= 2)
		 output[0] = (input[0] + input[1] + input[2]) * f1 / 3;
	  else {
		 output[0] = input[0] * f1;
		 output[1] = input[1] * f1;
		 output[2] = input[2] * f1;
	  }
	  if (req_comp == 2) output[1] = 1;
	  if (req_comp == 4) output[3] = 1;
   } else {
	  switch (req_comp) {
		 case 4: output[3] = 1; /* fallthrough */
		 case 3: output[0] = output[1] = output[2] = 0;
				 break;
		 case 2: output[1] = 1; /* fallthrough */
		 case 1: output[0] = 0;
				 break;
	  }
   }
}

static float *stbi__hdr_load(stbi__context *s, int *x, int *y, int *comp, int req_comp)
{
   char buffer[STBI__HDR_BUFLEN];
   char *token;
   int valid = 0;
   int width, height;
   stbi_uc *scanline;
   float *hdr_data;
   int len;
   unsigned char count, value;
   int i, j, k, c1,c2, z;

   // Check identifier
   if (strcmp(stbi__hdr_gettoken(s,buffer), "#?RADIANCE") != 0)
	  return stbi__errpf("not HDR", "Corrupt HDR image");

   // Parse header
   for(;;) {
	  token = stbi__hdr_gettoken(s,buffer);
	  if (token[0] == 0) break;
	  if (strcmp(token, "FORMAT=32-bit_rle_rgbe") == 0) valid = 1;
   }

   if (!valid)    return stbi__errpf("unsupported format", "Unsupported HDR format");

   // Parse width and height
   // can't use sscanf() if we're not using stdio!
   token = stbi__hdr_gettoken(s,buffer);
   if (strncmp(token, "-Y ", 3))  return stbi__errpf("unsupported data layout", "Unsupported HDR format");
   token += 3;
   height = (int) strtol(token, &token, 10);
   while (*token == ' ') ++token;
   if (strncmp(token, "+X ", 3))  return stbi__errpf("unsupported data layout", "Unsupported HDR format");
   token += 3;
   width = (int) strtol(token, NULL, 10);

   *x = width;
   *y = height;

   if (comp) *comp = 3;
   if (req_comp == 0) req_comp = 3;

   // Read data
   hdr_data = (float *) stbi__malloc(height * width * req_comp * sizeof(float));

   // Load image data
   // image data is stored as some number of sca
   if ( width < 8 || width >= 32768) {
	  // Read flat data
	  for (j=0; j < height; ++j) {
		 for (i=0; i < width; ++i) {
			stbi_uc rgbe[4];
		   main_decode_loop:
			stbi__getn(s, rgbe, 4);
			stbi__hdr_convert(hdr_data + j * width * req_comp + i * req_comp, rgbe, req_comp);
		 }
	  }
   } else {
	  // Read RLE-encoded data
	  scanline = NULL;

	  for (j = 0; j < height; ++j) {
		 c1 = stbi__get8(s);
		 c2 = stbi__get8(s);
		 len = stbi__get8(s);
		 if (c1 != 2 || c2 != 2 || (len & 0x80)) {
			// not run-length encoded, so we have to actually use THIS data as a decoded
			// pixel (note this can't be a valid pixel--one of RGB must be >= 128)
			stbi_uc rgbe[4];
			rgbe[0] = (stbi_uc) c1;
			rgbe[1] = (stbi_uc) c2;
			rgbe[2] = (stbi_uc) len;
			rgbe[3] = (stbi_uc) stbi__get8(s);
			stbi__hdr_convert(hdr_data, rgbe, req_comp);
			i = 1;
			j = 0;
			STBI_FREE(scanline);
			goto main_decode_loop; // yes, this makes no sense
		 }
		 len <<= 8;
		 len |= stbi__get8(s);
		 if (len != width) { STBI_FREE(hdr_data); STBI_FREE(scanline); return stbi__errpf("invalid decoded scanline length", "corrupt HDR"); }
		 if (scanline == NULL) scanline = (stbi_uc *) stbi__malloc(width * 4);

		 for (k = 0; k < 4; ++k) {
			i = 0;
			while (i < width) {
			   count = stbi__get8(s);
			   if (count > 128) {
				  // Run
				  value = stbi__get8(s);
				  count -= 128;
				  for (z = 0; z < count; ++z)
					 scanline[i++ * 4 + k] = value;
			   } else {
				  // Dump
				  for (z = 0; z < count; ++z)
					 scanline[i++ * 4 + k] = stbi__get8(s);
			   }
			}
		 }
		 for (i=0; i < width; ++i)
			stbi__hdr_convert(hdr_data+(j*width + i)*req_comp, scanline + i*4, req_comp);
	  }
	  STBI_FREE(scanline);
   }

   return hdr_data;
}

static int stbi__hdr_info(stbi__context *s, int *x, int *y, int *comp)
{
   char buffer[STBI__HDR_BUFLEN];
   char *token;
   int valid = 0;

   if (strcmp(stbi__hdr_gettoken(s,buffer), "#?RADIANCE") != 0) {
	   stbi__rewind( s );
	   return 0;
   }

   for(;;) {
	  token = stbi__hdr_gettoken(s,buffer);
	  if (token[0] == 0) break;
	  if (strcmp(token, "FORMAT=32-bit_rle_rgbe") == 0) valid = 1;
   }

   if (!valid) {
	   stbi__rewind( s );
	   return 0;
   }
   token = stbi__hdr_gettoken(s,buffer);
   if (strncmp(token, "-Y ", 3)) {
	   stbi__rewind( s );
	   return 0;
   }
   token += 3;
   *y = (int) strtol(token, &token, 10);
   while (*token == ' ') ++token;
   if (strncmp(token, "+X ", 3)) {
	   stbi__rewind( s );
	   return 0;
   }
   token += 3;
   *x = (int) strtol(token, NULL, 10);
   *comp = 3;
   return 1;
}
#endif // STBI_NO_HDR

#ifndef STBI_NO_BMP
static int stbi__bmp_info(stbi__context *s, int *x, int *y, int *comp)
{
   int hsz;
   if (stbi__get8(s) != 'B' || stbi__get8(s) != 'M') {
	   stbi__rewind( s );
	   return 0;
   }
   stbi__skip(s,12);
   hsz = stbi__get32le(s);
   if (hsz != 12 && hsz != 40 && hsz != 56 && hsz != 108 && hsz != 124) {
	   stbi__rewind( s );
	   return 0;
   }
   if (hsz == 12) {
	  *x = stbi__get16le(s);
	  *y = stbi__get16le(s);
   } else {
	  *x = stbi__get32le(s);
	  *y = stbi__get32le(s);
   }
   if (stbi__get16le(s) != 1) {
	   stbi__rewind( s );
	   return 0;
   }
   *comp = stbi__get16le(s) / 8;
   return 1;
}
#endif

#ifndef STBI_NO_PSD
static int stbi__psd_info(stbi__context *s, int *x, int *y, int *comp)
{
   int channelCount;
   if (stbi__get32be(s) != 0x38425053) {
	   stbi__rewind( s );
	   return 0;
   }
   if (stbi__get16be(s) != 1) {
	   stbi__rewind( s );
	   return 0;
   }
   stbi__skip(s, 6);
   channelCount = stbi__get16be(s);
   if (channelCount < 0 || channelCount > 16) {
	   stbi__rewind( s );
	   return 0;
   }
   *y = stbi__get32be(s);
   *x = stbi__get32be(s);
   if (stbi__get16be(s) != 8) {
	   stbi__rewind( s );
	   return 0;
   }
   if (stbi__get16be(s) != 3) {
	   stbi__rewind( s );
	   return 0;
   }
   *comp = 4;
   return 1;
}
#endif

#ifndef STBI_NO_PIC
static int stbi__pic_info(stbi__context *s, int *x, int *y, int *comp)
{
   int act_comp=0,num_packets=0,chained;
   stbi__pic_packet packets[10];

   stbi__skip(s, 92);

   *x = stbi__get16be(s);
   *y = stbi__get16be(s);
   if (stbi__at_eof(s))  return 0;
   if ( (*x) != 0 && (1 << 28) / (*x) < (*y)) {
	   stbi__rewind( s );
	   return 0;
   }

   stbi__skip(s, 8);

   do {
	  stbi__pic_packet *packet;

	  if (num_packets==sizeof(packets)/sizeof(packets[0]))
		 return 0;

	  packet = &packets[num_packets++];
	  chained = stbi__get8(s);
	  packet->size    = stbi__get8(s);
	  packet->type    = stbi__get8(s);
	  packet->channel = stbi__get8(s);
	  act_comp |= packet->channel;

	  if (stbi__at_eof(s)) {
		  stbi__rewind( s );
		  return 0;
	  }
	  if (packet->size != 8) {
		  stbi__rewind( s );
		  return 0;
	  }
   } while (chained);

   *comp = (act_comp & 0x10 ? 4 : 3);

   return 1;
}
#endif

// *************************************************************************************************
// Portable Gray Map and Portable Pixel Map loader
// by Ken Miller
//
// PGM: http://netpbm.sourceforge.net/doc/pgm.html
// PPM: http://netpbm.sourceforge.net/doc/ppm.html
//
// Known limitations:
//    Does not support comments in the header section
//    Does not support ASCII image data (formats P2 and P3)
//    Does not support 16-bit-per-channel

#ifndef STBI_NO_PNM

static int      stbi__pnm_test(stbi__context *s)
{
   char p, t;
   p = (char) stbi__get8(s);
   t = (char) stbi__get8(s);
   if (p != 'P' || (t != '5' && t != '6')) {
	   stbi__rewind( s );
	   return 0;
   }
   return 1;
}

static stbi_uc *stbi__pnm_load(stbi__context *s, int *x, int *y, int *comp, int req_comp)
{
   stbi_uc *out;
   if (!stbi__pnm_info(s, (int *)&s->img_x, (int *)&s->img_y, (int *)&s->img_n))
	  return 0;
   *x = s->img_x;
   *y = s->img_y;
   *comp = s->img_n;

   out = (stbi_uc *) stbi__malloc(s->img_n * s->img_x * s->img_y);
   if (!out) return stbi__errpuc("outofmem", "Out of memory");
   stbi__getn(s, out, s->img_n * s->img_x * s->img_y);

   if (req_comp && req_comp != s->img_n) {
	  out = stbi__convert_format(out, s->img_n, req_comp, s->img_x, s->img_y);
	  if (out == NULL) return out; // stbi__convert_format frees input on failure
   }
   return out;
}

static int      stbi__pnm_isspace(char c)
{
   return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}

static void     stbi__pnm_skip_whitespace(stbi__context *s, char *c)
{
   while (!stbi__at_eof(s) && stbi__pnm_isspace(*c))
	  *c = (char) stbi__get8(s);
}

static int      stbi__pnm_isdigit(char c)
{
   return c >= '0' && c <= '9';
}

static int      stbi__pnm_getinteger(stbi__context *s, char *c)
{
   int value = 0;

   while (!stbi__at_eof(s) && stbi__pnm_isdigit(*c)) {
	  value = value*10 + (*c - '0');
	  *c = (char) stbi__get8(s);
   }

   return value;
}

static int      stbi__pnm_info(stbi__context *s, int *x, int *y, int *comp)
{
   int maxv;
   char c, p, t;

   stbi__rewind( s );

   // Get identifier
   p = (char) stbi__get8(s);
   t = (char) stbi__get8(s);
   if (p != 'P' || (t != '5' && t != '6')) {
	   stbi__rewind( s );
	   return 0;
   }

   *comp = (t == '6') ? 3 : 1;  // '5' is 1-component .pgm; '6' is 3-component .ppm

   c = (char) stbi__get8(s);
   stbi__pnm_skip_whitespace(s, &c);

   *x = stbi__pnm_getinteger(s, &c); // read width
   stbi__pnm_skip_whitespace(s, &c);

   *y = stbi__pnm_getinteger(s, &c); // read height
   stbi__pnm_skip_whitespace(s, &c);

   maxv = stbi__pnm_getinteger(s, &c);  // read max value

   if (maxv > 255)
	  return stbi__err("max value > 255", "PPM image not 8-bit");
   else
	  return 1;
}
#endif

static int stbi__info_main(stbi__context *s, int *x, int *y, int *comp)
{
   #ifndef STBI_NO_JPEG
   if (stbi__jpeg_info(s, x, y, comp)) return 1;
   #endif

   #ifndef STBI_NO_PNG
   if (stbi__png_info(s, x, y, comp))  return 1;
   #endif

   #ifndef STBI_NO_GIF
   if (stbi__gif_info(s, x, y, comp))  return 1;
   #endif

   #ifndef STBI_NO_BMP
   if (stbi__bmp_info(s, x, y, comp))  return 1;
   #endif

   #ifndef STBI_NO_PSD
   if (stbi__psd_info(s, x, y, comp))  return 1;
   #endif

   #ifndef STBI_NO_PIC
   if (stbi__pic_info(s, x, y, comp))  return 1;
   #endif

   #ifndef STBI_NO_PNM
   if (stbi__pnm_info(s, x, y, comp))  return 1;
   #endif

   #ifndef STBI_NO_HDR
   if (stbi__hdr_info(s, x, y, comp))  return 1;
   #endif

   // test tga last because it's a crappy test!
   #ifndef STBI_NO_TGA
   if (stbi__tga_info(s, x, y, comp))
	   return 1;
   #endif
   return stbi__err("unknown image type", "Image not of any known type, or corrupt");
}

#ifndef STBI_NO_STDIO
STBIDEF int stbi_info(char const *filename, int *x, int *y, int *comp)
{
	FILE *f = stbi__fopen(filename, "rb");
	int result;
	if (!f) return stbi__err("can't fopen", "Unable to open file");
	result = stbi_info_from_file(f, x, y, comp);
	fclose(f);
	return result;
}

STBIDEF int stbi_info_from_file(FILE *f, int *x, int *y, int *comp)
{
   int r;
   stbi__context s;
   long pos = ftell(f);
   stbi__start_file(&s, f);
   r = stbi__info_main(&s,x,y,comp);
   fseek(f,pos,SEEK_SET);
   return r;
}
#endif // !STBI_NO_STDIO

STBIDEF int stbi_info_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp)
{
   stbi__context s;
   stbi__start_mem(&s,buffer,len);
   return stbi__info_main(&s,x,y,comp);
}

STBIDEF int stbi_info_from_callbacks(stbi_io_callbacks const *c, void *user, int *x, int *y, int *comp)
{
   stbi__context s;
   stbi__start_callbacks(&s, (stbi_io_callbacks *) c, user);
   return stbi__info_main(&s,x,y,comp);
}

// add in my DDS loading support
#ifndef STBI_NO_DDS

//#line 1 "stbi_DDS_c.h"
///	DDS file support, does decoding, _not_ direct uploading
///	(use SOIL for that ;-)

///	A bunch of DirectDraw Surface structures and flags
typedef struct {
	unsigned int    dwMagic;
	unsigned int    dwSize;
	unsigned int    dwFlags;
	unsigned int    dwHeight;
	unsigned int    dwWidth;
	unsigned int    dwPitchOrLinearSize;
	unsigned int    dwDepth;
	unsigned int    dwMipMapCount;
	unsigned int    dwReserved1[ 11 ];

	//  DDPIXELFORMAT
	struct {
	  unsigned int    dwSize;
	  unsigned int    dwFlags;
	  unsigned int    dwFourCC;
	  unsigned int    dwRGBBitCount;
	  unsigned int    dwRBitMask;
	  unsigned int    dwGBitMask;
	  unsigned int    dwBBitMask;
	  unsigned int    dwAlphaBitMask;
	}               sPixelFormat;

	//  DDCAPS2
	struct {
	  unsigned int    dwCaps1;
	  unsigned int    dwCaps2;
	  unsigned int    dwDDSX;
	  unsigned int    dwReserved;
	}               sCaps;
	unsigned int    dwReserved2;
} DDS_header ;

//	the following constants were copied directly off the MSDN website

//	The dwFlags member of the original DDSURFACEDESC2 structure
//	can be set to one or more of the following values.
#define DDSD_CAPS	0x00000001
#define DDSD_HEIGHT	0x00000002
#define DDSD_WIDTH	0x00000004
#define DDSD_PITCH	0x00000008
#define DDSD_PIXELFORMAT	0x00001000
#define DDSD_MIPMAPCOUNT	0x00020000
#define DDSD_LINEARSIZE	0x00080000
#define DDSD_DEPTH	0x00800000

//	DirectDraw Pixel Format
#define DDPF_ALPHAPIXELS	0x00000001
#define DDPF_FOURCC	0x00000004
#define DDPF_RGB	0x00000040

//	The dwCaps1 member of the DDSCAPS2 structure can be
//	set to one or more of the following values.
#define DDSCAPS_COMPLEX	0x00000008
#define DDSCAPS_TEXTURE	0x00001000
#define DDSCAPS_MIPMAP	0x00400000

//	The dwCaps2 member of the DDSCAPS2 structure can be
//	set to one or more of the following values.
#define DDSCAPS2_CUBEMAP	0x00000200
#define DDSCAPS2_CUBEMAP_POSITIVEX	0x00000400
#define DDSCAPS2_CUBEMAP_NEGATIVEX	0x00000800
#define DDSCAPS2_CUBEMAP_POSITIVEY	0x00001000
#define DDSCAPS2_CUBEMAP_NEGATIVEY	0x00002000
#define DDSCAPS2_CUBEMAP_POSITIVEZ	0x00004000
#define DDSCAPS2_CUBEMAP_NEGATIVEZ	0x00008000
#define DDSCAPS2_VOLUME	0x00200000

static int stbi__dds_test(stbi__context *s)
{
	//	check the magic number
	if (stbi__get8(s) != 'D') {
		stbi__rewind(s);
		return 0;
	}

	if (stbi__get8(s) != 'D') {
		stbi__rewind(s);
		return 0;
	}

	if (stbi__get8(s) != 'S') {
		stbi__rewind(s);
		return 0;
	}

	if (stbi__get8(s) != ' ') {
		stbi__rewind(s);
		return 0;
	}

	//	check header size
	if (stbi__get32le(s) != 124) {
		stbi__rewind(s);
		return 0;
	}

	// Also rewind because the loader needs to read the header
	stbi__rewind(s);

	return 1;
}
#ifndef STBI_NO_STDIO

int      stbi__dds_test_filename        		(char const *filename)
{
   int r;
   FILE *f = fopen(filename, "rb");
   if (!f) return 0;
   r = stbi__dds_test_file(f);
   fclose(f);
   return r;
}

int      stbi__dds_test_file        (FILE *f)
{
   stbi__context s;
   int r,n = ftell(f);
   stbi__start_file(&s,f);
   r = stbi__dds_test(&s);
   fseek(f,n,SEEK_SET);
   return r;
}
#endif

int      stbi__dds_test_memory      (stbi_uc const *buffer, int len)
{
   stbi__context s;
   stbi__start_mem(&s,buffer, len);
   return stbi__dds_test(&s);
}

int      stbi__dds_test_callbacks      (stbi_io_callbacks const *clbk, void *user)
{
   stbi__context s;
   stbi__start_callbacks(&s, (stbi_io_callbacks *) clbk, user);
   return stbi__dds_test(&s);
}

//	helper functions
int stbi_convert_bit_range( int c, int from_bits, int to_bits )
{
	int b = (1 << (from_bits - 1)) + c * ((1 << to_bits) - 1);
	return (b + (b >> from_bits)) >> from_bits;
}
void stbi_rgb_888_from_565( unsigned int c, int *r, int *g, int *b )
{
	*r = stbi_convert_bit_range( (c >> 11) & 31, 5, 8 );
	*g = stbi_convert_bit_range( (c >> 05) & 63, 6, 8 );
	*b = stbi_convert_bit_range( (c >> 00) & 31, 5, 8 );
}
void stbi_decode_DXT1_block(
			unsigned char uncompressed[16*4],
			unsigned char compressed[8] )
{
	int next_bit = 4*8;
	int i, r, g, b;
	int c0, c1;
	unsigned char decode_colors[4*4];
	//	find the 2 primary colors
	c0 = compressed[0] + (compressed[1] << 8);
	c1 = compressed[2] + (compressed[3] << 8);
	stbi_rgb_888_from_565( c0, &r, &g, &b );
	decode_colors[0] = r;
	decode_colors[1] = g;
	decode_colors[2] = b;
	decode_colors[3] = 255;
	stbi_rgb_888_from_565( c1, &r, &g, &b );
	decode_colors[4] = r;
	decode_colors[5] = g;
	decode_colors[6] = b;
	decode_colors[7] = 255;
	if( c0 > c1 )
	{
		//	no alpha, 2 interpolated colors
		decode_colors[8] = (2*decode_colors[0] + decode_colors[4]) / 3;
		decode_colors[9] = (2*decode_colors[1] + decode_colors[5]) / 3;
		decode_colors[10] = (2*decode_colors[2] + decode_colors[6]) / 3;
		decode_colors[11] = 255;
		decode_colors[12] = (decode_colors[0] + 2*decode_colors[4]) / 3;
		decode_colors[13] = (decode_colors[1] + 2*decode_colors[5]) / 3;
		decode_colors[14] = (decode_colors[2] + 2*decode_colors[6]) / 3;
		decode_colors[15] = 255;
	} else
	{
		//	1 interpolated color, alpha
		decode_colors[8] = (decode_colors[0] + decode_colors[4]) / 2;
		decode_colors[9] = (decode_colors[1] + decode_colors[5]) / 2;
		decode_colors[10] = (decode_colors[2] + decode_colors[6]) / 2;
		decode_colors[11] = 255;
		decode_colors[12] = 0;
		decode_colors[13] = 0;
		decode_colors[14] = 0;
		decode_colors[15] = 0;
	}
	//	decode the block
	for( i = 0; i < 16*4; i += 4 )
	{
		int idx = ((compressed[next_bit>>3] >> (next_bit & 7)) & 3) * 4;
		next_bit += 2;
		uncompressed[i+0] = decode_colors[idx+0];
		uncompressed[i+1] = decode_colors[idx+1];
		uncompressed[i+2] = decode_colors[idx+2];
		uncompressed[i+3] = decode_colors[idx+3];
	}
	//	done
}
void stbi_decode_DXT23_alpha_block(
			unsigned char uncompressed[16*4],
			unsigned char compressed[8] )
{
	int i, next_bit = 0;
	//	each alpha value gets 4 bits
	for( i = 3; i < 16*4; i += 4 )
	{
		uncompressed[i] = stbi_convert_bit_range(
				(compressed[next_bit>>3] >> (next_bit&7)) & 15,
				4, 8 );
		next_bit += 4;
	}
}
void stbi_decode_DXT45_alpha_block(
			unsigned char uncompressed[16*4],
			unsigned char compressed[8] )
{
	int i, next_bit = 8*2;
	unsigned char decode_alpha[8];
	//	each alpha value gets 3 bits, and the 1st 2 bytes are the range
	decode_alpha[0] = compressed[0];
	decode_alpha[1] = compressed[1];
	if( decode_alpha[0] > decode_alpha[1] )
	{
		//	6 step intermediate
		decode_alpha[2] = (6*decode_alpha[0] + 1*decode_alpha[1]) / 7;
		decode_alpha[3] = (5*decode_alpha[0] + 2*decode_alpha[1]) / 7;
		decode_alpha[4] = (4*decode_alpha[0] + 3*decode_alpha[1]) / 7;
		decode_alpha[5] = (3*decode_alpha[0] + 4*decode_alpha[1]) / 7;
		decode_alpha[6] = (2*decode_alpha[0] + 5*decode_alpha[1]) / 7;
		decode_alpha[7] = (1*decode_alpha[0] + 6*decode_alpha[1]) / 7;
	} else
	{
		//	4 step intermediate, pluss full and none
		decode_alpha[2] = (4*decode_alpha[0] + 1*decode_alpha[1]) / 5;
		decode_alpha[3] = (3*decode_alpha[0] + 2*decode_alpha[1]) / 5;
		decode_alpha[4] = (2*decode_alpha[0] + 3*decode_alpha[1]) / 5;
		decode_alpha[5] = (1*decode_alpha[0] + 4*decode_alpha[1]) / 5;
		decode_alpha[6] = 0;
		decode_alpha[7] = 255;
	}
	for( i = 3; i < 16*4; i += 4 )
	{
		int idx = 0, bit;
		bit = (compressed[next_bit>>3] >> (next_bit&7)) & 1;
		idx += bit << 0;
		++next_bit;
		bit = (compressed[next_bit>>3] >> (next_bit&7)) & 1;
		idx += bit << 1;
		++next_bit;
		bit = (compressed[next_bit>>3] >> (next_bit&7)) & 1;
		idx += bit << 2;
		++next_bit;
		uncompressed[i] = decode_alpha[idx & 7];
	}
	//	done
}
void stbi_decode_DXT_color_block(
			unsigned char uncompressed[16*4],
			unsigned char compressed[8] )
{
	int next_bit = 4*8;
	int i, r, g, b;
	int c0, c1;
	unsigned char decode_colors[4*3];
	//	find the 2 primary colors
	c0 = compressed[0] + (compressed[1] << 8);
	c1 = compressed[2] + (compressed[3] << 8);
	stbi_rgb_888_from_565( c0, &r, &g, &b );
	decode_colors[0] = r;
	decode_colors[1] = g;
	decode_colors[2] = b;
	stbi_rgb_888_from_565( c1, &r, &g, &b );
	decode_colors[3] = r;
	decode_colors[4] = g;
	decode_colors[5] = b;
	//	Like DXT1, but no choicees:
	//	no alpha, 2 interpolated colors
	decode_colors[6] = (2*decode_colors[0] + decode_colors[3]) / 3;
	decode_colors[7] = (2*decode_colors[1] + decode_colors[4]) / 3;
	decode_colors[8] = (2*decode_colors[2] + decode_colors[5]) / 3;
	decode_colors[9] = (decode_colors[0] + 2*decode_colors[3]) / 3;
	decode_colors[10] = (decode_colors[1] + 2*decode_colors[4]) / 3;
	decode_colors[11] = (decode_colors[2] + 2*decode_colors[5]) / 3;
	//	decode the block
	for( i = 0; i < 16*4; i += 4 )
	{
		int idx = ((compressed[next_bit>>3] >> (next_bit & 7)) & 3) * 3;
		next_bit += 2;
		uncompressed[i+0] = decode_colors[idx+0];
		uncompressed[i+1] = decode_colors[idx+1];
		uncompressed[i+2] = decode_colors[idx+2];
	}
	//	done
}

static int stbi__dds_info( stbi__context *s, int *x, int *y, int *comp, int *iscompressed ) {
	int flags,is_compressed,has_alpha;
	DDS_header header={0};

	if( sizeof( DDS_header ) != 128 )
	{
		return 0;
	}

	stbi__getn( s, (stbi_uc*)(&header), 128 );

	if( header.dwMagic != (('D' << 0) | ('D' << 8) | ('S' << 16) | (' ' << 24)) ) {
	   stbi__rewind( s );
	   return 0;
	}
	if( header.dwSize != 124 ) {
	   stbi__rewind( s );
	   return 0;
	}
	flags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
	if( (header.dwFlags & flags) != flags ) {
	   stbi__rewind( s );
	   return 0;
	}
	if( header.sPixelFormat.dwSize != 32 ) {
	   stbi__rewind( s );
	   return 0;
	}
	flags = DDPF_FOURCC | DDPF_RGB;
	if( (header.sPixelFormat.dwFlags & flags) == 0 ) {
	   stbi__rewind( s );
	   return 0;
	}
	if( (header.sCaps.dwCaps1 & DDSCAPS_TEXTURE) == 0 ) {
	   stbi__rewind( s );
	   return 0;
	}

	is_compressed = (header.sPixelFormat.dwFlags & DDPF_FOURCC) / DDPF_FOURCC;
	has_alpha = (header.sPixelFormat.dwFlags & DDPF_ALPHAPIXELS) / DDPF_ALPHAPIXELS;

	*x = header.dwWidth;
	*y = header.dwHeight;

	if ( !is_compressed ) {
		*comp = 3;

		if ( has_alpha )
			*comp = 4;
	}
	else
		*comp = 4;

	if ( iscompressed )
		*iscompressed = is_compressed;

	return 1;
}

int stbi__dds_info_from_memory (stbi_uc const *buffer, int len, int *x, int *y, int *comp, int *iscompressed)
{
	stbi__context s;
	stbi__start_mem(&s,buffer, len);
	return stbi__dds_info( &s, x, y, comp, iscompressed );
}

int stbi__dds_info_from_callbacks (stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp, int *iscompressed)
{
	stbi__context s;
	stbi__start_callbacks(&s, (stbi_io_callbacks *) clbk, user);
	return stbi__dds_info( &s, x, y, comp, iscompressed );
}

#ifndef STBI_NO_STDIO
int stbi__dds_info_from_path(char const *filename,     int *x, int *y, int *comp, int *iscompressed)
{
   int res;
   FILE *f = fopen(filename, "rb");
   if (!f) return 0;
   res = stbi__dds_info_from_file( f, x, y, comp, iscompressed );
   fclose(f);
   return res;
}

int stbi__dds_info_from_file(FILE *f,                  int *x, int *y, int *comp, int *iscompressed)
{
   stbi__context s;
   int res;
   long n = ftell(f);
   stbi__start_file(&s, f);
   res = stbi__dds_info(&s, x, y, comp, iscompressed);
   fseek(f, n, SEEK_SET);
   return res;
}
#endif

static stbi_uc * stbi__dds_load(stbi__context *s, int *x, int *y, int *comp, int req_comp)
{
	//	all variables go up front
	stbi_uc *dds_data = NULL;
	stbi_uc block[16*4];
	stbi_uc compressed[8];
	int flags, DXT_family;
	int has_alpha, has_mipmap;
	int is_compressed, cubemap_faces;
	int block_pitch, num_blocks;
	DDS_header header={0};
	int i, sz, cf;
	//	load the header
	if( sizeof( DDS_header ) != 128 )
	{
		return NULL;
	}
	stbi__getn( s, (stbi_uc*)(&header), 128 );
	//	and do some checking
	if( header.dwMagic != (('D' << 0) | ('D' << 8) | ('S' << 16) | (' ' << 24)) ) return NULL;
	if( header.dwSize != 124 ) return NULL;
	flags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
	if( (header.dwFlags & flags) != flags ) return NULL;
	/*	According to the MSDN spec, the dwFlags should contain
		DDSD_LINEARSIZE if it's compressed, or DDSD_PITCH if
		uncompressed.  Some DDS writers do not conform to the
		spec, so I need to make my reader more tolerant	*/
	if( header.sPixelFormat.dwSize != 32 ) return NULL;
	flags = DDPF_FOURCC | DDPF_RGB;
	if( (header.sPixelFormat.dwFlags & flags) == 0 ) return NULL;
	if( (header.sCaps.dwCaps1 & DDSCAPS_TEXTURE) == 0 ) return NULL;
	//	get the image data
	s->img_x = header.dwWidth;
	s->img_y = header.dwHeight;
	s->img_n = 4;
	is_compressed = (header.sPixelFormat.dwFlags & DDPF_FOURCC) / DDPF_FOURCC;
	has_alpha = (header.sPixelFormat.dwFlags & DDPF_ALPHAPIXELS) / DDPF_ALPHAPIXELS;
	has_mipmap = (header.sCaps.dwCaps1 & DDSCAPS_MIPMAP) && (header.dwMipMapCount > 1);
	cubemap_faces = (header.sCaps.dwCaps2 & DDSCAPS2_CUBEMAP) / DDSCAPS2_CUBEMAP;
	/*	I need cubemaps to have square faces	*/
	cubemap_faces &= (s->img_x == s->img_y);
	cubemap_faces *= 5;
	cubemap_faces += 1;
	block_pitch = (s->img_x+3) >> 2;
	num_blocks = block_pitch * ((s->img_y+3) >> 2);
	/*	let the user know what's going on	*/
	*x = s->img_x;
	*y = s->img_y;
	*comp = s->img_n;
	/*	is this uncompressed?	*/
	if( is_compressed )
	{
		/*	compressed	*/
		//	note: header.sPixelFormat.dwFourCC is something like (('D'<<0)|('X'<<8)|('T'<<16)|('1'<<24))
		DXT_family = 1 + (header.sPixelFormat.dwFourCC >> 24) - '1';
		if( (DXT_family < 1) || (DXT_family > 5) ) return NULL;
		/*	check the expected size...oops, nevermind...
			those non-compliant writers leave
			dwPitchOrLinearSize == 0	*/
		//	passed all the tests, get the RAM for decoding
		sz = (s->img_x)*(s->img_y)*4*cubemap_faces;
		dds_data = (unsigned char*)malloc( sz );
		/*	do this once for each face	*/
		for( cf = 0; cf < cubemap_faces; ++ cf )
		{
			//	now read and decode all the blocks
			for( i = 0; i < num_blocks; ++i )
			{
				//	where are we?
				int bx, by, bw=4, bh=4;
				int ref_x = 4 * (i % block_pitch);
				int ref_y = 4 * (i / block_pitch);
				//	get the next block's worth of compressed data, and decompress it
				if( DXT_family == 1 )
				{
					//	DXT1
					stbi__getn( s, compressed, 8 );
					stbi_decode_DXT1_block( block, compressed );
				} else if( DXT_family < 4 )
				{
					//	DXT2/3
					stbi__getn( s, compressed, 8 );
					stbi_decode_DXT23_alpha_block ( block, compressed );
					stbi__getn( s, compressed, 8 );
					stbi_decode_DXT_color_block ( block, compressed );
				} else
				{
					//	DXT4/5
					stbi__getn( s, compressed, 8 );
					stbi_decode_DXT45_alpha_block ( block, compressed );
					stbi__getn( s, compressed, 8 );
					stbi_decode_DXT_color_block ( block, compressed );
				}
				//	is this a partial block?
				if( ref_x + 4 > (int)s->img_x )
				{
					bw = s->img_x - ref_x;
				}
				if( ref_y + 4 > (int)s->img_y )
				{
					bh = s->img_y - ref_y;
				}
				//	now drop our decompressed data into the buffer
				for( by = 0; by < bh; ++by )
				{
					int idx = 4*((ref_y+by+cf*s->img_x)*s->img_x + ref_x);
					for( bx = 0; bx < bw*4; ++bx )
					{

						dds_data[idx+bx] = block[by*16+bx];
					}
				}
			}
			/*	done reading and decoding the main image...
				stbi__skip MIPmaps if present	*/
			if( has_mipmap )
			{
				int block_size = 16;
				if( DXT_family == 1 )
				{
					block_size = 8;
				}
				for( i = 1; i < (int)header.dwMipMapCount; ++i )
				{
					int mx = s->img_x >> (i + 2);
					int my = s->img_y >> (i + 2);
					if( mx < 1 )
					{
						mx = 1;
					}
					if( my < 1 )
					{
						my = 1;
					}
					stbi__skip( s, mx*my*block_size );
				}
			}
		}/* per cubemap face */
	} else
	{
		/*	uncompressed	*/
		DXT_family = 0;
		s->img_n = 3;
		if( has_alpha )
		{
			s->img_n = 4;
		}
		*comp = s->img_n;
		sz = s->img_x*s->img_y*s->img_n*cubemap_faces;
		dds_data = (unsigned char*)malloc( sz );
		/*	do this once for each face	*/
		for( cf = 0; cf < cubemap_faces; ++ cf )
		{
			/*	read the main image for this face	*/
			stbi__getn( s, &dds_data[cf*s->img_x*s->img_y*s->img_n], s->img_x*s->img_y*s->img_n );
			/*	done reading and decoding the main image...
				stbi__skip MIPmaps if present	*/
			if( has_mipmap )
			{
				for( i = 1; i < (int)header.dwMipMapCount; ++i )
				{
					int mx = s->img_x >> i;
					int my = s->img_y >> i;
					if( mx < 1 )
					{
						mx = 1;
					}
					if( my < 1 )
					{
						my = 1;
					}
					stbi__skip( s, mx*my*s->img_n );
				}
			}
		}
		/*	data was BGR, I need it RGB	*/
		for( i = 0; i < sz; i += s->img_n )
		{
			unsigned char temp = dds_data[i];
			dds_data[i] = dds_data[i+2];
			dds_data[i+2] = temp;
		}
	}
	/*	finished decompressing into RGBA,
		adjust the y size if we have a cubemap
		note: sz is already up to date	*/
	s->img_y *= cubemap_faces;
	*y = s->img_y;
	//	did the user want something else, or
	//	see if all the alpha values are 255 (i.e. no transparency)
	has_alpha = 0;
	if( s->img_n == 4)
	{
		for( i = 3; (i < sz) && (has_alpha == 0); i += 4 )
		{
			has_alpha |= (dds_data[i] < 255);
		}
	}
	if( (req_comp <= 4) && (req_comp >= 1) )
	{
		//	user has some requirements, meet them
		if( req_comp != s->img_n )
		{
			dds_data = stbi__convert_format( dds_data, s->img_n, req_comp, s->img_x, s->img_y );
			*comp = req_comp;
		}
	} else
	{
		//	user had no requirements, only drop to RGB is no alpha
		if( (has_alpha == 0) && (s->img_n == 4) )
		{
			dds_data = stbi__convert_format( dds_data, 4, 3, s->img_x, s->img_y );
			*comp = 3;
		}
	}
	//	OK, done
	return dds_data;
}

#ifndef STBI_NO_STDIO
stbi_uc *stbi__dds_load_from_file   (FILE *f,                  int *x, int *y, int *comp, int req_comp)
{
	stbi__context s;
	stbi__start_file(&s,f);
	return stbi__dds_load(&s,x,y,comp,req_comp);
}

stbi_uc *stbi__dds_load_from_path             (const char *filename,           int *x, int *y, int *comp, int req_comp)
{
   stbi_uc *data;
   FILE *f = fopen(filename, "rb");
   if (!f) return NULL;
   data = stbi__dds_load_from_file(f,x,y,comp,req_comp);
   fclose(f);
   return data;
}
#endif

stbi_uc *stbi__dds_load_from_memory (stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp)
{
	stbi__context s;
   stbi__start_mem(&s,buffer, len);
   return stbi__dds_load(&s,x,y,comp,req_comp);
}

stbi_uc *stbi__dds_load_from_callbacks (stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp, int req_comp)
{
	stbi__context s;
   stbi__start_callbacks(&s, (stbi_io_callbacks *) clbk, user);
   return stbi__dds_load(&s,x,y,comp,req_comp);
}


#endif

// add in my pvr loading support
#ifndef STBI_NO_PVR

//#line 1 "stbi_pvr_c.h"

//#line 1 "pvr_helper.h"
#ifndef PVR_HELPER_H
#define PVR_HELPER_H

// Taken from PowerVR SDK

/*!***************************************************************************
 Describes the header of a PVR header-texture
 *****************************************************************************/
typedef struct
{
	unsigned int dwHeaderSize;			/*!< size of the structure */
	unsigned int dwHeight;				/*!< height of surface to be created */
	unsigned int dwWidth;				/*!< width of input surface */
	unsigned int dwMipMapCount;			/*!< number of mip-map levels requested */
	unsigned int dwpfFlags;				/*!< pixel format flags */
	unsigned int dwTextureDataSize;		/*!< Total size in bytes */
	unsigned int dwBitCount;			/*!< number of bits per pixel  */
	unsigned int dwRBitMask;			/*!< mask for red bit */
	unsigned int dwGBitMask;			/*!< mask for green bits */
	unsigned int dwBBitMask;			/*!< mask for blue bits */
	unsigned int dwAlphaBitMask;		/*!< mask for alpha channel */
	unsigned int dwPVR;					/*!< magic number identifying pvr file */
	unsigned int dwNumSurfs;			/*!< the number of surfaces present in the pvr */
} PVR_Texture_Header;

/*****************************************************************************
 * ENUMS
 *****************************************************************************/

enum PixelType
{
	MGLPT_ARGB_4444 = 0x00,
	MGLPT_ARGB_1555,
	MGLPT_RGB_565,
	MGLPT_RGB_555,
	MGLPT_RGB_888,
	MGLPT_ARGB_8888,
	MGLPT_ARGB_8332,
	MGLPT_I_8,
	MGLPT_AI_88,
	MGLPT_1_BPP,
	MGLPT_VY1UY0,
	MGLPT_Y1VY0U,
	MGLPT_PVRTC2,
	MGLPT_PVRTC4,
	MGLPT_PVRTC2_2,
	MGLPT_PVRTC2_4,

	OGL_RGBA_4444= 0x10,
	OGL_RGBA_5551,
	OGL_RGBA_8888,
	OGL_RGB_565,
	OGL_RGB_555,
	OGL_RGB_888,
	OGL_I_8,
	OGL_AI_88,
	OGL_PVRTC2,
	OGL_PVRTC4,

	// OGL_BGRA_8888 extension
	OGL_BGRA_8888,

	D3D_DXT1 = 0x20,
	D3D_DXT2,
	D3D_DXT3,
	D3D_DXT4,
	D3D_DXT5,

	D3D_RGB_332,
	D3D_AI_44,
	D3D_LVU_655,
	D3D_XLVU_8888,
	D3D_QWVU_8888,

	//10 bits per channel
	D3D_ABGR_2101010,
	D3D_ARGB_2101010,
	D3D_AWVU_2101010,

	//16 bits per channel
	D3D_GR_1616,
	D3D_VU_1616,
	D3D_ABGR_16161616,

	//HDR formats
	D3D_R16F,
	D3D_GR_1616F,
	D3D_ABGR_16161616F,

	//32 bits per channel
	D3D_R32F,
	D3D_GR_3232F,
	D3D_ABGR_32323232F,

	// Ericsson
	ETC_RGB_4BPP,
	ETC_RGBA_EXPLICIT,
	ETC_RGBA_INTERPOLATED,

	// DX10

	ePT_DX10_R32G32B32A32_FLOAT= 0x50,
	ePT_DX10_R32G32B32A32_UINT ,
	ePT_DX10_R32G32B32A32_SINT,

	ePT_DX10_R32G32B32_FLOAT,
	ePT_DX10_R32G32B32_UINT,
	ePT_DX10_R32G32B32_SINT,

	ePT_DX10_R16G16B16A16_FLOAT ,
	ePT_DX10_R16G16B16A16_UNORM,
	ePT_DX10_R16G16B16A16_UINT ,
	ePT_DX10_R16G16B16A16_SNORM ,
	ePT_DX10_R16G16B16A16_SINT ,

	ePT_DX10_R32G32_FLOAT ,
	ePT_DX10_R32G32_UINT ,
	ePT_DX10_R32G32_SINT ,

	ePT_DX10_R10G10B10A2_UNORM ,
	ePT_DX10_R10G10B10A2_UINT ,

	ePT_DX10_R11G11B10_FLOAT ,

	ePT_DX10_R8G8B8A8_UNORM ,
	ePT_DX10_R8G8B8A8_UNORM_SRGB ,
	ePT_DX10_R8G8B8A8_UINT ,
	ePT_DX10_R8G8B8A8_SNORM ,
	ePT_DX10_R8G8B8A8_SINT ,

	ePT_DX10_R16G16_FLOAT ,
	ePT_DX10_R16G16_UNORM ,
	ePT_DX10_R16G16_UINT ,
	ePT_DX10_R16G16_SNORM ,
	ePT_DX10_R16G16_SINT ,

	ePT_DX10_R32_FLOAT ,
	ePT_DX10_R32_UINT ,
	ePT_DX10_R32_SINT ,

	ePT_DX10_R8G8_UNORM ,
	ePT_DX10_R8G8_UINT ,
	ePT_DX10_R8G8_SNORM ,
	ePT_DX10_R8G8_SINT ,

	ePT_DX10_R16_FLOAT ,
	ePT_DX10_R16_UNORM ,
	ePT_DX10_R16_UINT ,
	ePT_DX10_R16_SNORM ,
	ePT_DX10_R16_SINT ,

	ePT_DX10_R8_UNORM,
	ePT_DX10_R8_UINT,
	ePT_DX10_R8_SNORM,
	ePT_DX10_R8_SINT,

	ePT_DX10_A8_UNORM,
	ePT_DX10_R1_UNORM,
	ePT_DX10_R9G9B9E5_SHAREDEXP,
	ePT_DX10_R8G8_B8G8_UNORM,
	ePT_DX10_G8R8_G8B8_UNORM,

	ePT_DX10_BC1_UNORM,
	ePT_DX10_BC1_UNORM_SRGB,

	ePT_DX10_BC2_UNORM,
	ePT_DX10_BC2_UNORM_SRGB,

	ePT_DX10_BC3_UNORM,
	ePT_DX10_BC3_UNORM_SRGB,

	ePT_DX10_BC4_UNORM,
	ePT_DX10_BC4_SNORM,

	ePT_DX10_BC5_UNORM,
	ePT_DX10_BC5_SNORM,

	//ePT_DX10_B5G6R5_UNORM,			// defined but obsolete - won't actually load in DX10
	//ePT_DX10_B5G5R5A1_UNORM,
	//ePT_DX10_B8G8R8A8_UNORM,
	//ePT_DX10_B8G8R8X8_UNORM,

	// OpenVG

	/* RGB{A,X} channel ordering */
	ePT_VG_sRGBX_8888  = 0x90,
	ePT_VG_sRGBA_8888,
	ePT_VG_sRGBA_8888_PRE,
	ePT_VG_sRGB_565,
	ePT_VG_sRGBA_5551,
	ePT_VG_sRGBA_4444,
	ePT_VG_sL_8,
	ePT_VG_lRGBX_8888,
	ePT_VG_lRGBA_8888,
	ePT_VG_lRGBA_8888_PRE,
	ePT_VG_lL_8,
	ePT_VG_A_8,
	ePT_VG_BW_1,

	/* {A,X}RGB channel ordering */
	ePT_VG_sXRGB_8888,
	ePT_VG_sARGB_8888,
	ePT_VG_sARGB_8888_PRE,
	ePT_VG_sARGB_1555,
	ePT_VG_sARGB_4444,
	ePT_VG_lXRGB_8888,
	ePT_VG_lARGB_8888,
	ePT_VG_lARGB_8888_PRE,

	/* BGR{A,X} channel ordering */
	ePT_VG_sBGRX_8888,
	ePT_VG_sBGRA_8888,
	ePT_VG_sBGRA_8888_PRE,
	ePT_VG_sBGR_565,
	ePT_VG_sBGRA_5551,
	ePT_VG_sBGRA_4444,
	ePT_VG_lBGRX_8888,
	ePT_VG_lBGRA_8888,
	ePT_VG_lBGRA_8888_PRE,

	/* {A,X}BGR channel ordering */
	ePT_VG_sXBGR_8888,
	ePT_VG_sABGR_8888 ,
	ePT_VG_sABGR_8888_PRE,
	ePT_VG_sABGR_1555,
	ePT_VG_sABGR_4444,
	ePT_VG_lXBGR_8888,
	ePT_VG_lABGR_8888,
	ePT_VG_lABGR_8888_PRE,

	// max cap for iterating
	END_OF_PIXEL_TYPES,

	MGLPT_NOTYPE = 0xff

};

/*****************************************************************************
 * constants
 *****************************************************************************/

#define PVRTEX_MIPMAP		(1<<8)		// has mip map levels
#define PVRTEX_TWIDDLE		(1<<9)		// is twiddled
#define PVRTEX_BUMPMAP		(1<<10)		// has normals encoded for a bump map
#define PVRTEX_TILING		(1<<11)		// is bordered for tiled pvr
#define PVRTEX_CUBEMAP		(1<<12)		// is a cubemap/skybox
#define PVRTEX_FALSEMIPCOL	(1<<13)		//
#define PVRTEX_VOLUME		(1<<14)
#define PVRTEX_PIXELTYPE	0xff			// pixel type is always in the last 16bits of the flags
#define PVRTEX_IDENTIFIER	0x21525650	// the pvr identifier is the characters 'P','V','R'

#define PVRTEX_V1_HEADER_SIZE 44			// old header size was 44 for identification purposes

#define PVRTC2_MIN_TEXWIDTH		16
#define PVRTC2_MIN_TEXHEIGHT	8
#define PVRTC4_MIN_TEXWIDTH		8
#define PVRTC4_MIN_TEXHEIGHT	8
#define ETC_MIN_TEXWIDTH		4
#define ETC_MIN_TEXHEIGHT		4
#define DXT_MIN_TEXWIDTH		4
#define DXT_MIN_TEXHEIGHT		4

#endif

static int stbi__pvr_test(stbi__context *s)
{
	//	check header size
	if (stbi__get32le(s) != sizeof(PVR_Texture_Header)) {
		stbi__rewind(s);
		return 0;
	}

	// stbi__skip until the magic number
	stbi__skip(s, 10*4);

	// check the magic number
	if ( stbi__get32le(s) != PVRTEX_IDENTIFIER ) {
		stbi__rewind(s);
		return 0;
	}

	// Also rewind because the loader needs to read the header
	stbi__rewind(s);

	return 1;
}

#ifndef STBI_NO_STDIO

int      stbi__pvr_test_filename        		(char const *filename)
{
   int r;
   FILE *f = fopen(filename, "rb");
   if (!f) return 0;
   r = stbi__pvr_test_file(f);
   fclose(f);
   return r;
}

int      stbi__pvr_test_file        (FILE *f)
{
   stbi__context s;
   int r,n = ftell(f);
   stbi__start_file(&s,f);
   r = stbi__pvr_test(&s);
   fseek(f,n,SEEK_SET);
   return r;
}
#endif

int      stbi__pvr_test_memory      (stbi_uc const *buffer, int len)
{
   stbi__context s;
   stbi__start_mem(&s,buffer, len);
   return stbi__pvr_test(&s);
}

int      stbi__pvr_test_callbacks      (stbi_io_callbacks const *clbk, void *user)
{
   stbi__context s;
   stbi__start_callbacks(&s, (stbi_io_callbacks *) clbk, user);
   return stbi__pvr_test(&s);
}

static int stbi__pvr_info(stbi__context *s, int *x, int *y, int *comp, int * iscompressed )
{
	PVR_Texture_Header header={0};

	stbi__getn( s, (stbi_uc*)(&header), sizeof(PVR_Texture_Header) );

	// Check the header size
	if ( header.dwHeaderSize != sizeof(PVR_Texture_Header) ) {
		stbi__rewind( s );
		return 0;
	}

	// Check the magic identifier
	if ( header.dwPVR != PVRTEX_IDENTIFIER ) {
		stbi__rewind(s);
		return 0;
	}

	*x = s->img_x = header.dwWidth;
	*y = s->img_y = header.dwHeight;
	*comp = s->img_n = ( header.dwBitCount + 7 ) / 8;

	if ( iscompressed )
		*iscompressed = 0;

	switch ( header.dwpfFlags & PVRTEX_PIXELTYPE )
	{
		case OGL_RGBA_4444:
			s->img_n = 2;
			break;
		case OGL_RGBA_5551:
			s->img_n = 2;
			break;
		case OGL_RGBA_8888:
			s->img_n = 4;
			break;
		case OGL_RGB_565:
			s->img_n = 2;
			break;
		case OGL_RGB_888:
			s->img_n = 3;
			break;
		case OGL_I_8:
			s->img_n = 1;
			break;
		case OGL_AI_88:
			s->img_n = 2;
			break;
		case OGL_PVRTC2:
			s->img_n = 4;
			if ( iscompressed )
				*iscompressed = 1;
			break;
		case OGL_PVRTC4:
			s->img_n = 4;
			if ( iscompressed )
				*iscompressed = 1;
			break;
		case OGL_RGB_555:
		default:
			stbi__rewind(s);
			return 0;
	}

	*comp = s->img_n;

	return 1;
}

int stbi__pvr_info_from_memory (stbi_uc const *buffer, int len, int *x, int *y, int *comp, int * iscompressed )
{
	stbi__context s;
	stbi__start_mem(&s,buffer, len);
	return stbi__pvr_info( &s, x, y, comp, iscompressed );
}

int stbi__pvr_info_from_callbacks (stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp, int * iscompressed)
{
	stbi__context s;
	stbi__start_callbacks(&s, (stbi_io_callbacks *) clbk, user);
	return stbi__pvr_info( &s, x, y, comp, iscompressed );
}

#ifndef STBI_NO_STDIO
int stbi__pvr_info_from_path(char const *filename,     int *x, int *y, int *comp, int * iscompressed)
{
   int res;
   FILE *f = fopen(filename, "rb");
   if (!f) return 0;
   res = stbi__pvr_info_from_file( f, x, y, comp, iscompressed );
   fclose(f);
   return res;
}

int stbi__pvr_info_from_file(FILE *f,                  int *x, int *y, int *comp, int * iscompressed)
{
   stbi__context s;
   int res;
   long n = ftell(f);
   stbi__start_file(&s, f);
   res = stbi__pvr_info(&s, x, y, comp, iscompressed);
   fseek(f, n, SEEK_SET);
   return res;
}
#endif

/******************************************************************************
 Taken from:
 @File         PVRTDecompress.cpp
 @Title        PVRTDecompress
 @Copyright    Copyright (C)  Imagination Technologies Limited.
 @Platform     ANSI compatible
 @Description  PVRTC Texture Decompression.
******************************************************************************/

typedef unsigned char      PVRTuint8;
typedef unsigned short     PVRTuint16;
typedef unsigned int       PVRTuint32;

/*****************************************************************************
 * defines and consts
 *****************************************************************************/
#define PT_INDEX (2)	// The Punch-through index

#define BLK_Y_SIZE 	(4) // always 4 for all 2D block types

#define BLK_X_MAX	(8)	// Max X dimension for blocks

#define BLK_X_2BPP	(8) // dimensions for the two formats
#define BLK_X_4BPP	(4)

#define WRAP_COORD(Val, Size) ((Val) & ((Size)-1))

#define POWER_OF_2(X)   util_number_is_power_2(X)

/*
	Define an expression to either wrap or clamp large or small vals to the
	legal coordinate range
*/
#define PVRT_MIN(a,b)            (((a) < (b)) ? (a) : (b))
#define PVRT_MAX(a,b)            (((a) > (b)) ? (a) : (b))
#define PVRT_CLAMP(x, l, h)      (PVRT_MIN((h), PVRT_MAX((x), (l))))

#define LIMIT_COORD(Val, Size, AssumeImageTiles) \
	  ((AssumeImageTiles)? WRAP_COORD((Val), (Size)): PVRT_CLAMP((Val), 0, (Size)-1))

/*****************************************************************************
 * Useful typedefs
 *****************************************************************************/
typedef PVRTuint32 U32;
typedef PVRTuint8 U8;

/***********************************************************
				DECOMPRESSION ROUTINES
************************************************************/

/*!***********************************************************************
 @Struct	AMTC_BLOCK_STRUCT
 @Brief
*************************************************************************/
typedef struct
{
	// Uses 64 bits pre block
	U32 PackedData[2];
}AMTC_BLOCK_STRUCT;

 /*!***********************************************************************
  @Function		util_number_is_power_2
  @Input		input A number
  @Returns		TRUE if the number is an integer power of two, else FALSE.
  @Description	Check that a number is an integer power of two, i.e.
				1, 2, 4, 8, ... etc.
				Returns FALSE for zero.
*************************************************************************/
int util_number_is_power_2( unsigned  input )
{
  unsigned minus1;

  if( !input ) return 0;

  minus1 = input - 1;
  return ( (input | minus1) == (input ^ minus1) ) ? 1 : 0;
}

/*!***********************************************************************
 @Function		Unpack5554Colour
 @Input			pBlock
 @Input			ABColours
 @Description	Given a block, extract the colour information and convert
				to 5554 formats
*************************************************************************/
static void Unpack5554Colour(const AMTC_BLOCK_STRUCT *pBlock,
							 int   ABColours[2][4])
{
	U32 RawBits[2];

	int i;

	// Extract A and B
	RawBits[0] = pBlock->PackedData[1] & (0xFFFE); // 15 bits (shifted up by one)
	RawBits[1] = pBlock->PackedData[1] >> 16;	   // 16 bits

	// step through both colours
	for(i = 0; i < 2; i++)
	{
		// If completely opaque
		if(RawBits[i] & (1<<15))
		{
			// Extract R and G (both 5 bit)
			ABColours[i][0] = (RawBits[i] >> 10) & 0x1F;
			ABColours[i][1] = (RawBits[i] >>  5) & 0x1F;

			/*
				The precision of Blue depends on  A or B. If A then we need to
				replicate the top bit to get 5 bits in total
			*/
			ABColours[i][2] = RawBits[i] & 0x1F;
			if(i==0)
			{
				ABColours[0][2] |= ABColours[0][2] >> 4;
			}

			// set 4bit alpha fully on...
			ABColours[i][3] = 0xF;
		}
		else // Else if colour has variable translucency
		{
			/*
				Extract R and G (both 4 bit).
				(Leave a space on the end for the replication of bits
			*/
			ABColours[i][0] = (RawBits[i] >>  (8-1)) & 0x1E;
			ABColours[i][1] = (RawBits[i] >>  (4-1)) & 0x1E;

			// replicate bits to truly expand to 5 bits
			ABColours[i][0] |= ABColours[i][0] >> 4;
			ABColours[i][1] |= ABColours[i][1] >> 4;

			// grab the 3(+padding) or 4 bits of blue and add an extra padding bit
			ABColours[i][2] = (RawBits[i] & 0xF) << 1;

			/*
				expand from 3 to 5 bits if this is from colour A, or 4 to 5 bits if from
				colour B
			*/
			if(i==0)
			{
				ABColours[0][2] |= ABColours[0][2] >> 3;
			}
			else
			{
				ABColours[0][2] |= ABColours[0][2] >> 4;
			}

			// Set the alpha bits to be 3 + a zero on the end
			ABColours[i][3] = (RawBits[i] >> 11) & 0xE;
		}
	}
}

/*!***********************************************************************
 @Function		UnpackModulations
 @Input			pBlock
 @Input			Do2bitMode
 @Input			ModulationVals
 @Input			ModulationModes
 @Input			StartX
 @Input			StartY
 @Description	Given the block and the texture type and it's relative
				position in the 2x2 group of blocks, extract the bit
				patterns for the fully defined pixels.
*************************************************************************/
static void	UnpackModulations(const AMTC_BLOCK_STRUCT *pBlock,
							  const int Do2bitMode,
							  int ModulationVals[8][16],
							  int ModulationModes[8][16],
							  int StartX,
							  int StartY)
{
	int BlockModMode;
	U32 ModulationBits;

	int x, y;

	BlockModMode= pBlock->PackedData[1] & 1;
	ModulationBits	= pBlock->PackedData[0];

	// if it's in an interpolated mode
	if(Do2bitMode && BlockModMode)
	{
		/*
			run through all the pixels in the block. Note we can now treat all the
			"stored" values as if they have 2bits (even when they didn't!)
		*/
		for(y = 0; y < BLK_Y_SIZE; y++)
		{
			for(x = 0; x < BLK_X_2BPP; x++)
			{
				ModulationModes[y+StartY][x+StartX] = BlockModMode;

				// if this is a stored value...
				if(((x^y)&1) == 0)
				{
					ModulationVals[y+StartY][x+StartX] = ModulationBits & 3;
					ModulationBits >>= 2;
				}
			}
		}
	}
	else if(Do2bitMode) // else if direct encoded 2bit mode - i.e. 1 mode bit per pixel
	{
		for(y = 0; y < BLK_Y_SIZE; y++)
		{
			for(x = 0; x < BLK_X_2BPP; x++)
			{
				ModulationModes[y+StartY][x+StartX] = BlockModMode;

				// double the bits so 0=> 00, and 1=>11
				if(ModulationBits & 1)
				{
					ModulationVals[y+StartY][x+StartX] = 0x3;
				}
				else
				{
					ModulationVals[y+StartY][x+StartX] = 0x0;
				}
				ModulationBits >>= 1;
			}
		}
	}
	else // else its the 4bpp mode so each value has 2 bits
	{
		for(y = 0; y < BLK_Y_SIZE; y++)
		{
			for(x = 0; x < BLK_X_4BPP; x++)
			{
				ModulationModes[y+StartY][x+StartX] = BlockModMode;

				ModulationVals[y+StartY][x+StartX] = ModulationBits & 3;
				ModulationBits >>= 2;
			}
		}
	}

	// make sure nothing is left over
	assert(ModulationBits==0);
}

/*!***********************************************************************
 @Function		InterpolateColours
 @Input			ColourP
 @Input			ColourQ
 @Input			ColourR
 @Input			ColourS
 @Input			Do2bitMode
 @Input			x
 @Input			y
 @Modified		Result
 @Description	This performs a HW bit accurate interpolation of either the
				A or B colours for a particular pixel.

				NOTE: It is assumed that the source colours are in ARGB 5554
				format - This means that some "preparation" of the values will
				be necessary.
*************************************************************************/
static void InterpolateColours(const int ColourP[4],
						  const int ColourQ[4],
						  const int ColourR[4],
						  const int ColourS[4],
						  const int Do2bitMode,
						  const int x,
						  const int y,
						  int Result[4])
{
	int u, v, uscale;
	int k;

	int tmp1, tmp2;

	int P[4], Q[4], R[4], S[4];

	// Copy the colours
	for(k = 0; k < 4; k++)
	{
		P[k] = ColourP[k];
		Q[k] = ColourQ[k];
		R[k] = ColourR[k];
		S[k] = ColourS[k];
	}

	// put the x and y values into the right range
	v = (y & 0x3) | ((~y & 0x2) << 1);

	if(Do2bitMode)
		u = (x & 0x7) | ((~x & 0x4) << 1);
	else
		u = (x & 0x3) | ((~x & 0x2) << 1);

	// get the u and v scale amounts
	v  = v - BLK_Y_SIZE/2;

	if(Do2bitMode)
	{
		u = u - BLK_X_2BPP/2;
		uscale = 8;
	}
	else
	{
		u = u - BLK_X_4BPP/2;
		uscale = 4;
	}

	for(k = 0; k < 4; k++)
	{
		tmp1 = P[k] * uscale + u * (Q[k] - P[k]);
		tmp2 = R[k] * uscale + u * (S[k] - R[k]);

		tmp1 = tmp1 * 4 + v * (tmp2 - tmp1);

		Result[k] = tmp1;
	}

	// Lop off the appropriate number of bits to get us to 8 bit precision
	if(Do2bitMode)
	{
		// do RGB
		for(k = 0; k < 3; k++)
		{
			Result[k] >>= 2;
		}

		Result[3] >>= 1;
	}
	else
	{
		// do RGB  (A is ok)
		for(k = 0; k < 3; k++)
		{
			Result[k] >>= 1;
		}
	}

	// sanity check
	for(k = 0; k < 4; k++)
	{
		assert(Result[k] < 256);
	}

	/*
		Convert from 5554 to 8888

		do RGB 5.3 => 8
	*/
	for(k = 0; k < 3; k++)
	{
		Result[k] += Result[k] >> 5;
	}

	Result[3] += Result[3] >> 4;

	// 2nd sanity check
	for(k = 0; k < 4; k++)
	{
		assert(Result[k] < 256);
	}

}

/*!***********************************************************************
 @Function		GetModulationValue
 @Input			x
 @Input			y
 @Input			Do2bitMode
 @Input			ModulationVals
 @Input			ModulationModes
 @Input			Mod
 @Input			DoPT
 @Description	Get the modulation value as a numerator of a fraction of 8ths
*************************************************************************/
static void GetModulationValue(int x,
							   int y,
							   const int Do2bitMode,
							   const int ModulationVals[8][16],
							   const int ModulationModes[8][16],
							   int *Mod,
							   int *DoPT)
{
	static const int RepVals0[4] = {0, 3, 5, 8};
	static const int RepVals1[4] = {0, 4, 4, 8};

	int ModVal;

	// Map X and Y into the local 2x2 block
	y = (y & 0x3) | ((~y & 0x2) << 1);

	if(Do2bitMode)
		x = (x & 0x7) | ((~x & 0x4) << 1);
	else
		x = (x & 0x3) | ((~x & 0x2) << 1);

	// assume no PT for now
	*DoPT = 0;

	// extract the modulation value. If a simple encoding
	if(ModulationModes[y][x]==0)
	{
		ModVal = RepVals0[ModulationVals[y][x]];
	}
	else if(Do2bitMode)
	{
		// if this is a stored value
		if(((x^y)&1)==0)
			ModVal = RepVals0[ModulationVals[y][x]];
		else if(ModulationModes[y][x] == 1) // else average from the neighbours if H&V interpolation..
		{
			ModVal = (RepVals0[ModulationVals[y-1][x]] +
					  RepVals0[ModulationVals[y+1][x]] +
					  RepVals0[ModulationVals[y][x-1]] +
					  RepVals0[ModulationVals[y][x+1]] + 2) / 4;
		}
		else if(ModulationModes[y][x] == 2) // else if H-Only
		{
			ModVal = (RepVals0[ModulationVals[y][x-1]] +
					  RepVals0[ModulationVals[y][x+1]] + 1) / 2;
		}
		else // else it's V-Only
		{
			ModVal = (RepVals0[ModulationVals[y-1][x]] +
					  RepVals0[ModulationVals[y+1][x]] + 1) / 2;
		}
	}
	else // else it's 4BPP and PT encoding
	{
		ModVal = RepVals1[ModulationVals[y][x]];

		*DoPT = ModulationVals[y][x] == PT_INDEX;
	}

	*Mod =ModVal;
}

/*!***********************************************************************
 @Function		TwiddleUV
 @Input			YSize	Y dimension of the texture in pixels
 @Input			XSize	X dimension of the texture in pixels
 @Input			YPos	Pixel Y position
 @Input			XPos	Pixel X position
 @Returns		The twiddled offset of the pixel
 @Description	Given the Block (or pixel) coordinates and the dimension of
				the texture in blocks (or pixels) this returns the twiddled
				offset of the block (or pixel) from the start of the map.

				NOTE the dimensions of the texture must be a power of 2
*************************************************************************/
static int DisableTwiddlingRoutine = 0;

static U32 TwiddleUV(U32 YSize, U32 XSize, U32 YPos, U32 XPos)
{
	U32 Twiddled;

	U32 MinDimension;
	U32 MaxValue;

	U32 SrcBitPos;
	U32 DstBitPos;

	int ShiftCount;

	assert(YPos < YSize);
	assert(XPos < XSize);

	assert(POWER_OF_2(YSize));
	assert(POWER_OF_2(XSize));

	if(YSize < XSize)
	{
		MinDimension = YSize;
		MaxValue	 = XPos;
	}
	else
	{
		MinDimension = XSize;
		MaxValue	 = YPos;
	}

	// Nasty hack to disable twiddling
	if(DisableTwiddlingRoutine)
		return (YPos* XSize + XPos);

	// Step through all the bits in the "minimum" dimension
	SrcBitPos = 1;
	DstBitPos = 1;
	Twiddled  = 0;
	ShiftCount = 0;

	while(SrcBitPos < MinDimension)
	{
		if(YPos & SrcBitPos)
		{
			Twiddled |= DstBitPos;
		}

		if(XPos & SrcBitPos)
		{
			Twiddled |= (DstBitPos << 1);
		}

		SrcBitPos <<= 1;
		DstBitPos <<= 2;
		ShiftCount += 1;

	}

	// prepend any unused bits
	MaxValue >>= ShiftCount;

	Twiddled |=  (MaxValue << (2*ShiftCount));

	return Twiddled;
}

/***********************************************************/
/*
// Decompress
//
// Takes the compressed input data and outputs the equivalent decompressed
// image.
*/
/***********************************************************/

static void Decompress(AMTC_BLOCK_STRUCT *pCompressedData,
					   const int Do2bitMode,
					   const int XDim,
					   const int YDim,
					   const int AssumeImageTiles,
					   unsigned char* pResultImage)
{
	int x, y;
	int i, j;

	int BlkX, BlkY;
	int BlkXp1, BlkYp1;
	int XBlockSize;
	int BlkXDim, BlkYDim;

	int StartX, StartY;

	int ModulationVals[8][16];
	int ModulationModes[8][16];

	int Mod, DoPT;

	unsigned int uPosition;

	/*
	// local neighbourhood of blocks
	*/
	AMTC_BLOCK_STRUCT *pBlocks[2][2];

	AMTC_BLOCK_STRUCT *pPrevious[2][2] = {{NULL, NULL}, {NULL, NULL}};

	/*
	// Low precision colours extracted from the blocks
	*/
	struct
	{
		int Reps[2][4];
	}Colours5554[2][2];

	/*
	// Interpolated A and B colours for the pixel
	*/
	int ASig[4], BSig[4];

	int Result[4];

	if(Do2bitMode)
	{
		XBlockSize = BLK_X_2BPP;
	}
	else
	{
		XBlockSize = BLK_X_4BPP;
	}

	/*
	// For MBX don't allow the sizes to get too small
	*/
	BlkXDim = PVRT_MAX(2, XDim / XBlockSize);
	BlkYDim = PVRT_MAX(2, YDim / BLK_Y_SIZE);

	/*
	// Step through the pixels of the image decompressing each one in turn
	//
	// Note that this is a hideously inefficient way to do this!
	*/
	for(y = 0; y < YDim; y++)
	{
		for(x = 0; x < XDim; x++)
		{
			/*
			// map this pixel to the top left neighbourhood of blocks
			*/
			BlkX = (x - XBlockSize/2);
			BlkY = (y - BLK_Y_SIZE/2);

			BlkX = LIMIT_COORD(BlkX, XDim, AssumeImageTiles);
			BlkY = LIMIT_COORD(BlkY, YDim, AssumeImageTiles);

			BlkX /= XBlockSize;
			BlkY /= BLK_Y_SIZE;

			//BlkX = LIMIT_COORD(BlkX, BlkXDim, AssumeImageTiles);
			//BlkY = LIMIT_COORD(BlkY, BlkYDim, AssumeImageTiles);

			/*
			// compute the positions of the other 3 blocks
			*/
			BlkXp1 = LIMIT_COORD(BlkX+1, BlkXDim, AssumeImageTiles);
			BlkYp1 = LIMIT_COORD(BlkY+1, BlkYDim, AssumeImageTiles);

			/*
			// Map to block memory locations
			*/
			pBlocks[0][0] = pCompressedData +TwiddleUV(BlkYDim, BlkXDim, BlkY, BlkX);
			pBlocks[0][1] = pCompressedData +TwiddleUV(BlkYDim, BlkXDim, BlkY, BlkXp1);
			pBlocks[1][0] = pCompressedData +TwiddleUV(BlkYDim, BlkXDim, BlkYp1, BlkX);
			pBlocks[1][1] = pCompressedData +TwiddleUV(BlkYDim, BlkXDim, BlkYp1, BlkXp1);

			/*
			// extract the colours and the modulation information IF the previous values
			// have changed.
			*/
			if(memcmp(pPrevious, pBlocks, 4*sizeof(void*)) != 0)
			{
				StartY = 0;
				for(i = 0; i < 2; i++)
				{
					StartX = 0;
					for(j = 0; j < 2; j++)
					{
						Unpack5554Colour(pBlocks[i][j], Colours5554[i][j].Reps);

						UnpackModulations(pBlocks[i][j],
							Do2bitMode,
							ModulationVals,
							ModulationModes,
							StartX, StartY);

						StartX += XBlockSize;
					}/*end for j*/

					StartY += BLK_Y_SIZE;
				}/*end for i*/

				/*
				// make a copy of the new pointers
				*/
				memcpy(pPrevious, pBlocks, 4*sizeof(void*));
			}/*end if the blocks have changed*/

			/*
			// decompress the pixel.  First compute the interpolated A and B signals
			*/
			InterpolateColours(Colours5554[0][0].Reps[0],
				Colours5554[0][1].Reps[0],
				Colours5554[1][0].Reps[0],
				Colours5554[1][1].Reps[0],
				Do2bitMode, x, y,
				ASig);

			InterpolateColours(Colours5554[0][0].Reps[1],
				Colours5554[0][1].Reps[1],
				Colours5554[1][0].Reps[1],
				Colours5554[1][1].Reps[1],
				Do2bitMode, x, y,
				BSig);

			GetModulationValue(x,y, Do2bitMode, (const int (*)[16])ModulationVals, (const int (*)[16])ModulationModes,
				&Mod, &DoPT);

			/*
			// compute the modulated colour
			*/
			for(i = 0; i < 4; i++)
			{
				Result[i] = ASig[i] * 8 + Mod * (BSig[i] - ASig[i]);
				Result[i] >>= 3;
			}
			if(DoPT)
			{
				Result[3] = 0;
			}

			/*
			// Store the result in the output image
			*/
			uPosition = (x+y*XDim)<<2;
			pResultImage[uPosition+0] = (unsigned char)Result[0];
			pResultImage[uPosition+1] = (unsigned char)Result[1];
			pResultImage[uPosition+2] = (unsigned char)Result[2];
			pResultImage[uPosition+3] = (unsigned char)Result[3];

		}/*end for x*/
	}/*end for y*/

}

static stbi_uc * stbi__pvr_load(stbi__context *s, int *x, int *y, int *comp, int req_comp)
{
	stbi_uc *pvr_data = NULL;
	stbi_uc *pvr_res_data = NULL;
	PVR_Texture_Header header={0};
	int iscompressed = 0;
	int bitmode = 0;
	unsigned int levelSize = 0;

	stbi__getn( s, (stbi_uc*)(&header), sizeof(PVR_Texture_Header) );

	// Check the header size
	if ( header.dwHeaderSize != sizeof(PVR_Texture_Header) ) {
		return NULL;
	}

	// Check the magic identifier
	if ( header.dwPVR != PVRTEX_IDENTIFIER ) {
		return NULL;
	}

	*x = s->img_x = header.dwWidth;
	*y = s->img_y = header.dwHeight;

	/* Get if the texture is compressed and the texture mode ( 2bpp or 4bpp ) */
	switch ( header.dwpfFlags & PVRTEX_PIXELTYPE )
	{
		case OGL_RGBA_4444:
			s->img_n = 2;
			break;
		case OGL_RGBA_5551:
			s->img_n = 2;
			break;
		case OGL_RGBA_8888:
			s->img_n = 4;
			break;
		case OGL_RGB_565:
			s->img_n = 2;
			break;
		case OGL_RGB_888:
			s->img_n = 3;
			break;
		case OGL_I_8:
			s->img_n = 1;
			break;
		case OGL_AI_88:
			s->img_n = 2;
			break;
		case OGL_PVRTC2:
			bitmode = 1;
			s->img_n = 4;
			iscompressed = 1;
			break;
		case OGL_PVRTC4:
			s->img_n = 4;
			iscompressed = 1;
			break;
		case OGL_RGB_555:
		default:
			return NULL;
	}

	*comp = s->img_n;

	// Load only the first mip map level
	levelSize = (s->img_x * s->img_y * header.dwBitCount + 7) / 8;

	// get the raw data
	pvr_data = (stbi_uc *)malloc( levelSize );
	stbi__getn( s, pvr_data, levelSize );

	// if compressed decompress as RGBA
	if ( iscompressed ) {
		pvr_res_data = (stbi_uc *)malloc( s->img_x * s->img_y * 4 );
		Decompress( (AMTC_BLOCK_STRUCT*)pvr_data, bitmode, s->img_x, s->img_y, 1, (unsigned char*)pvr_res_data );
		free( pvr_data );
	} else {
		// otherwise use the raw data
		pvr_res_data = pvr_data;
	}

	if( (req_comp <= 4) && (req_comp >= 1) ) {
		//	user has some requirements, meet them
		if( req_comp != s->img_n ) {
			pvr_res_data = stbi__convert_format( pvr_res_data, s->img_n, req_comp, s->img_x, s->img_y );
			*comp = req_comp;
		}
	}

	return pvr_res_data;
}

#ifndef STBI_NO_STDIO
stbi_uc *stbi__pvr_load_from_file   (FILE *f,                  int *x, int *y, int *comp, int req_comp)
{
	stbi__context s;
	stbi__start_file(&s,f);
	return stbi__pvr_load(&s,x,y,comp,req_comp);
}

stbi_uc *stbi__pvr_load_from_path             (char const*filename,           int *x, int *y, int *comp, int req_comp)
{
   stbi_uc *data;
   FILE *f = fopen(filename, "rb");
   if (!f) return NULL;
   data = stbi__pvr_load_from_file(f,x,y,comp,req_comp);
   fclose(f);
   return data;
}
#endif

stbi_uc *stbi__pvr_load_from_memory (stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp)
{
   stbi__context s;
   stbi__start_mem(&s,buffer, len);
   return stbi__pvr_load(&s,x,y,comp,req_comp);
}

stbi_uc *stbi__pvr_load_from_callbacks (stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp, int req_comp)
{
	stbi__context s;
   stbi__start_callbacks(&s, (stbi_io_callbacks *) clbk, user);
   return stbi__pvr_load(&s,x,y,comp,req_comp);
}


#endif

// add in my pkm ( ETC1 ) loading support
#ifndef STBI_NO_PKM

//#line 1 "stbi_pkm_c.h"

//#line 1 "pkm_helper.h"
#ifndef PKM_HELPER_H
#define PKM_HELPER_H

typedef struct {
	char aName[6];
	unsigned short iBlank;
	unsigned char iPaddedWidthMSB;
	unsigned char iPaddedWidthLSB;
	unsigned char iPaddedHeightMSB;
	unsigned char iPaddedHeightLSB;
	unsigned char iWidthMSB;
	unsigned char iWidthLSB;
	unsigned char iHeightMSB;
	unsigned char iHeightLSB;
} PKMHeader;

#define PKM_HEADER_SIZE 16

#endif


//#line 1 "etc1_utils.h"
// Copyright 2009 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef __etc1_h__
#define __etc1_h__

#define ETC1_ENCODED_BLOCK_SIZE 8
#define ETC1_DECODED_BLOCK_SIZE 48

#ifndef ETC1_RGB8_OES
#define ETC1_RGB8_OES 0x8D64
#endif

typedef unsigned char etc1_byte;
typedef int etc1_bool;
typedef unsigned int etc1_uint32;

#ifdef __cplusplus
extern "C" {
#endif

// Encode a block of pixels.
//
// pIn is a pointer to a ETC_DECODED_BLOCK_SIZE array of bytes that represent a
// 4 x 4 square of 3-byte pixels in form R, G, B. Byte (3 * (x + 4 * y) is the R
// value of pixel (x, y).
//
// validPixelMask is a 16-bit mask where bit (1 << (x + y * 4)) indicates whether
// the corresponding (x,y) pixel is valid. Invalid pixel color values are ignored when compressing.
//
// pOut is an ETC1 compressed version of the data.

void etc1_encode_block(const etc1_byte* pIn, etc1_uint32 validPixelMask, etc1_byte* pOut);

// Decode a block of pixels.
//
// pIn is an ETC1 compressed version of the data.
//
// pOut is a pointer to a ETC_DECODED_BLOCK_SIZE array of bytes that represent a
// 4 x 4 square of 3-byte pixels in form R, G, B. Byte (3 * (x + 4 * y) is the R
// value of pixel (x, y).

void etc1_decode_block(const etc1_byte* pIn, etc1_byte* pOut);

// Return the size of the encoded image data (does not include size of PKM header).

etc1_uint32 etc1_get_encoded_data_size(etc1_uint32 width, etc1_uint32 height);

// Encode an entire image.
// pIn - pointer to the image data. Formatted such that
//       pixel (x,y) is at pIn + pixelSize * x + stride * y;
// pOut - pointer to encoded data. Must be large enough to store entire encoded image.
// pixelSize can be 2 or 3. 2 is an GL_UNSIGNED_SHORT_5_6_5 image, 3 is a GL_BYTE RGB image.
// returns non-zero if there is an error.

int etc1_encode_image(const etc1_byte* pIn, etc1_uint32 width, etc1_uint32 height,
		etc1_uint32 pixelSize, etc1_uint32 stride, etc1_byte* pOut);

// Decode an entire image.
// pIn - pointer to encoded data.
// pOut - pointer to the image data. Will be written such that
//        pixel (x,y) is at pIn + pixelSize * x + stride * y. Must be
//        large enough to store entire image.
// pixelSize can be 2 or 3. 2 is an GL_UNSIGNED_SHORT_5_6_5 image, 3 is a GL_BYTE RGB image.
// returns non-zero if there is an error.

int etc1_decode_image(const etc1_byte* pIn, etc1_byte* pOut,
		etc1_uint32 width, etc1_uint32 height,
		etc1_uint32 pixelSize, etc1_uint32 stride);

// Size of a PKM header, in bytes.

#define ETC_PKM_HEADER_SIZE 16

// Format a PKM header

void etc1_pkm_format_header(etc1_byte* pHeader, etc1_uint32 width, etc1_uint32 height);

// Check if a PKM header is correctly formatted.

etc1_bool etc1_pkm_is_valid(const etc1_byte* pHeader);

// Read the image width from a PKM header

etc1_uint32 etc1_pkm_get_width(const etc1_byte* pHeader);

// Read the image height from a PKM header

etc1_uint32 etc1_pkm_get_height(const etc1_byte* pHeader);

#ifdef __cplusplus
}
#endif

#endif
static int stbi__pkm_test(stbi__context *s)
{
	//	check the magic number
	if (stbi__get8(s) != 'P') {
		stbi__rewind(s);
		return 0;
	}

	if (stbi__get8(s) != 'K') {
		stbi__rewind(s);
		return 0;
	}

	if (stbi__get8(s) != 'M') {
		stbi__rewind(s);
		return 0;
	}

	if (stbi__get8(s) != ' ') {
		stbi__rewind(s);
		return 0;
	}

	if (stbi__get8(s) != '1') {
		stbi__rewind(s);
		return 0;
	}

	if (stbi__get8(s) != '0') {
		stbi__rewind(s);
		return 0;
	}

	stbi__rewind(s);
	return 1;
}

#ifndef STBI_NO_STDIO

int      stbi__pkm_test_filename        		(char const *filename)
{
   int r;
   FILE *f = fopen(filename, "rb");
   if (!f) return 0;
   r = stbi__pkm_test_file(f);
   fclose(f);
   return r;
}

int      stbi__pkm_test_file        (FILE *f)
{
   stbi__context s;
   int r,n = ftell(f);
   stbi__start_file(&s,f);
   r = stbi__pkm_test(&s);
   fseek(f,n,SEEK_SET);
   return r;
}
#endif

int      stbi__pkm_test_memory      (stbi_uc const *buffer, int len)
{
   stbi__context s;
   stbi__start_mem(&s,buffer, len);
   return stbi__pkm_test(&s);
}

int      stbi__pkm_test_callbacks      (stbi_io_callbacks const *clbk, void *user)
{
   stbi__context s;
   stbi__start_callbacks(&s, (stbi_io_callbacks *) clbk, user);
   return stbi__pkm_test(&s);
}

static int stbi__pkm_info(stbi__context *s, int *x, int *y, int *comp )
{
	PKMHeader header;
	unsigned int width, height;

	stbi__getn( s, (stbi_uc*)(&header), sizeof(PKMHeader) );

	if ( 0 != strcmp( header.aName, "PKM 10" ) ) {
		stbi__rewind(s);
		return 0;
	}

	width = (header.iWidthMSB << 8) | header.iWidthLSB;
	height = (header.iHeightMSB << 8) | header.iHeightLSB;

	*x = s->img_x = width;
	*y = s->img_y = height;
	*comp = s->img_n = 3;

	stbi__rewind(s);

	return 1;
}

int stbi__pkm_info_from_memory (stbi_uc const *buffer, int len, int *x, int *y, int *comp )
{
	stbi__context s;
	stbi__start_mem(&s,buffer, len);
	return stbi__pkm_info( &s, x, y, comp );
}

int stbi__pkm_info_from_callbacks (stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp)
{
	stbi__context s;
	stbi__start_callbacks(&s, (stbi_io_callbacks *) clbk, user);
	return stbi__pkm_info( &s, x, y, comp );
}

#ifndef STBI_NO_STDIO
int stbi__pkm_info_from_path(char const *filename,     int *x, int *y, int *comp)
{
   int res;
   FILE *f = fopen(filename, "rb");
   if (!f) return 0;
   res = stbi__pkm_info_from_file( f, x, y, comp );
   fclose(f);
   return res;
}

int stbi__pkm_info_from_file(FILE *f,                  int *x, int *y, int *comp)
{
   stbi__context s;
   int res;
   long n = ftell(f);
   stbi__start_file(&s, f);
   res = stbi__pkm_info(&s, x, y, comp);
   fseek(f, n, SEEK_SET);
   return res;
}
#endif

static stbi_uc * stbi__pkm_load(stbi__context *s, int *x, int *y, int *comp, int req_comp)
{
	stbi_uc *pkm_data = NULL;
	stbi_uc *pkm_res_data = NULL;
	PKMHeader header;
	unsigned int width;
	unsigned int height;
	unsigned int align = 0;
	unsigned int bpr;
	unsigned int size;
	unsigned int compressedSize;

	int res;

	stbi__getn( s, (stbi_uc*)(&header), sizeof(PKMHeader) );

	if ( 0 != strcmp( header.aName, "PKM 10" ) ) {
		return NULL;
	}

	width = (header.iWidthMSB << 8) | header.iWidthLSB;
	height = (header.iHeightMSB << 8) | header.iHeightLSB;

	*x = s->img_x = width;
	*y = s->img_y = height;
	*comp = s->img_n = 3;

	compressedSize = etc1_get_encoded_data_size(width, height);

	pkm_data = (stbi_uc *)malloc(compressedSize);
	stbi__getn( s, pkm_data, compressedSize );

	bpr = ((width * 3) + align) & ~align;
	size = bpr * height;
	pkm_res_data = (stbi_uc *)malloc(size);

	res = etc1_decode_image((const etc1_byte*)pkm_data, (etc1_byte*)pkm_res_data, width, height, 3, bpr);

	free( pkm_data );

	if ( 0 == res ) {
		if( (req_comp <= 4) && (req_comp >= 1) ) {
			//	user has some requirements, meet them
			if( req_comp != s->img_n ) {
				pkm_res_data = stbi__convert_format( pkm_res_data, s->img_n, req_comp, s->img_x, s->img_y );
				*comp = req_comp;
			}
		}

		return (stbi_uc *)pkm_res_data;
	} else {
		free( pkm_res_data );
	}

	return NULL;
}

#ifndef STBI_NO_STDIO
stbi_uc *stbi__pkm_load_from_file   (FILE *f,                  int *x, int *y, int *comp, int req_comp)
{
	stbi__context s;
	stbi__start_file(&s,f);
	return stbi__pkm_load(&s,x,y,comp,req_comp);
}

stbi_uc *stbi__pkm_load_from_path             (char const*filename,           int *x, int *y, int *comp, int req_comp)
{
   stbi_uc *data;
   FILE *f = fopen(filename, "rb");
   if (!f) return NULL;
   data = stbi__pkm_load_from_file(f,x,y,comp,req_comp);
   fclose(f);
   return data;
}
#endif

stbi_uc *stbi__pkm_load_from_memory (stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp)
{
   stbi__context s;
   stbi__start_mem(&s,buffer, len);
   return stbi__pkm_load(&s,x,y,comp,req_comp);
}

stbi_uc *stbi__pkm_load_from_callbacks (stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp, int req_comp)
{
	stbi__context s;
   stbi__start_callbacks(&s, (stbi_io_callbacks *) clbk, user);
   return stbi__pkm_load(&s,x,y,comp,req_comp);
}


#endif

#ifndef STBI_NO_EXT

//#line 1 "stbi_ext_c.h"
static int stbi_test_main(stbi__context *s)
{
   #ifndef STBI_NO_JPEG
   if (stbi__jpeg_test(s)) return STBI_jpeg;
   #endif
   #ifndef STBI_NO_PNG
   if (stbi__png_test(s))  return STBI_png;
   #endif
   #ifndef STBI_NO_BMP
   if (stbi__bmp_test(s))  return STBI_bmp;
   #endif
   #ifndef STBI_NO_GIF
   if (stbi__gif_test(s))  return STBI_gif;
   #endif
   #ifndef STBI_NO_PSD
   if (stbi__psd_test(s))  return STBI_psd;
   #endif
   #ifndef STBI_NO_PIC
   if (stbi__pic_test(s))  return STBI_pic;
   #endif
   #ifndef STBI_NO_PNM
   if (stbi__pnm_test(s))  return STBI_pnm;
   #endif
   #ifndef STBI_NO_DDS
   if (stbi__dds_test(s))  return STBI_dds;
   #endif
   #ifndef STBI_NO_PVR
   if (stbi__pvr_test(s))  return STBI_pvr;
   #endif
   #ifndef STBI_NO_PKM
   if (stbi__pkm_test(s))  return STBI_pkm;
   #endif
   #ifndef STBI_NO_HDR
   if (stbi__hdr_test(s))  return STBI_hdr;
   #endif
   #ifndef STBI_NO_TGA
   if (stbi__tga_test(s))  return STBI_tga;
   #endif
   return STBI_unknown;
}

#ifndef STBI_NO_STDIO
int stbi_test_from_file(FILE *f)
{
   stbi__context s;
   stbi__start_file(&s,f);
   return stbi_test_main(&s);
}

int stbi_test(char const *filename)
{
   FILE *f = fopen(filename, "rb");
   int result;
   if (!f) return STBI_unknown;
   result = stbi_test_from_file(f);
   fclose(f);
   return result;
}
#endif //!STBI_NO_STDIO

int stbi_test_from_memory(stbi_uc const *buffer, int len)
{
   stbi__context s;
   stbi__start_mem(&s,buffer,len);
   return stbi_test_main(&s);
}

int stbi_test_from_callbacks(stbi_io_callbacks const *clbk, void *user)
{
   stbi__context s;
   stbi__start_callbacks(&s, (stbi_io_callbacks *) clbk, user);
   return stbi_test_main(&s);
}


#endif


//#line 1 "stb_image_write.c"
//#ifdef STB_IMAGE_WRITE_IMPLEMENTATION

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

//#line 1 "stb_image_write.h"
#ifndef INCLUDE_STB_IMAGE_WRITE_H
#define INCLUDE_STB_IMAGE_WRITE_H

#ifdef __cplusplus
extern "C" {
#endif

int stbi_write_png(char const *filename, int w, int h, int comp, const void *data, int stride_in_bytes);
int stbi_write_bmp(char const *filename, int w, int h, int comp, const void *data);
int stbi_write_tga(char const *filename, int w, int h, int comp, const void *data);

#ifdef __cplusplus
}
#endif

#endif//INCLUDE_STB_IMAGE_WRITE_H


typedef unsigned int stbiw_uint32;
typedef int stb_image_write_test[sizeof(stbiw_uint32)==4 ? 1 : -1];

static void writefv(FILE *f, const char *fmt, va_list v)
{
   while (*fmt) {
	  switch (*fmt++) {
		 case ' ': break;
		 case '1': { unsigned char x = (unsigned char) va_arg(v, int); fputc(x,f); break; }
		 case '2': { int x = va_arg(v,int); unsigned char b[2];
					 b[0] = (unsigned char) x; b[1] = (unsigned char) (x>>8);
					 fwrite(b,2,1,f); break; }
		 case '4': { stbiw_uint32 x = va_arg(v,int); unsigned char b[4];
					 b[0]=(unsigned char)x; b[1]=(unsigned char)(x>>8);
					 b[2]=(unsigned char)(x>>16); b[3]=(unsigned char)(x>>24);
					 fwrite(b,4,1,f); break; }
		 default:
			assert(0);
			return;
	  }
   }
}

static void write3(FILE *f, unsigned char a, unsigned char b, unsigned char c)
{
   unsigned char arr[3];
   arr[0] = a, arr[1] = b, arr[2] = c;
   fwrite(arr, 3, 1, f);
}

static void write_pixels(FILE *f, int rgb_dir, int vdir, int x, int y, int comp, void *data, int write_alpha, int scanline_pad)
{
   unsigned char bg[3] = { 255, 0, 255}, px[3];
   stbiw_uint32 zero = 0;
   int i,j,k, j_end;

   if (y <= 0)
	  return;

   if (vdir < 0)
	  j_end = -1, j = y-1;
   else
	  j_end =  y, j = 0;

   for (; j != j_end; j += vdir) {
	  for (i=0; i < x; ++i) {
		 unsigned char *d = (unsigned char *) data + (j*x+i)*comp;
		 if (write_alpha < 0)
			fwrite(&d[comp-1], 1, 1, f);
		 switch (comp) {
			case 1:
			case 2: write3(f, d[0],d[0],d[0]);
					break;
			case 4:
			   if (!write_alpha) {
				  // composite against pink background
				  for (k=0; k < 3; ++k)
					 px[k] = bg[k] + ((d[k] - bg[k]) * d[3])/255;
				  write3(f, px[1-rgb_dir],px[1],px[1+rgb_dir]);
				  break;
			   }
			   /* FALLTHROUGH */
			case 3:
			   write3(f, d[1-rgb_dir],d[1],d[1+rgb_dir]);
			   break;
		 }
		 if (write_alpha > 0)
			fwrite(&d[comp-1], 1, 1, f);
	  }
	  fwrite(&zero,scanline_pad,1,f);
   }
}

static int outfile(char const *filename, int rgb_dir, int vdir, int x, int y, int comp, void *data, int alpha, int pad, const char *fmt, ...)
{
   FILE *f;
   if (y < 0 || x < 0) return 0;
   f = fopen(filename, "wb");
   if (f) {
	  va_list v;
	  va_start(v, fmt);
	  writefv(f, fmt, v);
	  va_end(v);
	  write_pixels(f,rgb_dir,vdir,x,y,comp,data,alpha,pad);
	  fclose(f);
   }
   return f != NULL;
}

int stbi_write_bmp(char const *filename, int x, int y, int comp, const void *data)
{
   int pad = (-x*3) & 3;
   return outfile(filename,-1,-1,x,y,comp,(void *) data,0,pad,
		   "11 4 22 4" "4 44 22 444444",
		   'B', 'M', 14+40+(x*3+pad)*y, 0,0, 14+40,  // file header
			40, x,y, 1,24, 0,0,0,0,0,0);             // bitmap header
}

int stbi_write_tga(char const *filename, int x, int y, int comp, const void *data)
{
   int has_alpha = !(comp & 1);
   return outfile(filename, -1,-1, x, y, comp, (void *) data, has_alpha, 0,
				  "111 221 2222 11", 0,0,2, 0,0,0, 0,0,x,y, 24+8*has_alpha, 8*has_alpha);
}

// stretchy buffer; stbi__sbpush() == vector<>::push_back() -- stbi__sbcount() == vector<>::size()
#define stbi__sbraw(a) ((int *) (a) - 2)
#define stbi__sbm(a)   stbi__sbraw(a)[0]
#define stbi__sbn(a)   stbi__sbraw(a)[1]

#define stbi__sbneedgrow(a,n)  ((a)==0 || stbi__sbn(a)+n >= stbi__sbm(a))
#define stbi__sbmaybegrow(a,n) (stbi__sbneedgrow(a,(n)) ? stbi__sbgrow(a,n) : 0)
#define stbi__sbgrow(a,n)  stbi__sbgrowf((void **) &(a), (n), sizeof(*(a)))

#define stbi__sbpush(a, v)      (stbi__sbmaybegrow(a,1), (a)[stbi__sbn(a)++] = (v))
#define stbi__sbcount(a)        ((a) ? stbi__sbn(a) : 0)
#define stbi__sbfree(a)         ((a) ? free(stbi__sbraw(a)),0 : 0)

static void *stbi__sbgrowf(void **arr, int increment, int itemsize)
{
   int m = *arr ? 2*stbi__sbm(*arr)+increment : increment+1;
   void *p = realloc(*arr ? stbi__sbraw(*arr) : 0, itemsize * m + sizeof(int)*2);
   assert(p);
   if (p) {
	  if (!*arr) ((int *) p)[1] = 0;
	  *arr = (void *) ((int *) p + 2);
	  stbi__sbm(*arr) = m;
   }
   return *arr;
}

static unsigned char *stbi__zlib_flushf(unsigned char *data, unsigned int *bitbuffer, int *bitcount)
{
   while (*bitcount >= 8) {
	  stbi__sbpush(data, (unsigned char) *bitbuffer);
	  *bitbuffer >>= 8;
	  *bitcount -= 8;
   }
   return data;
}

static int stbi__zlib_bitrev(int code, int codebits)
{
   int res=0;
   while (codebits--) {
	  res = (res << 1) | (code & 1);
	  code >>= 1;
   }
   return res;
}

static unsigned int stbi__zlib_countm(unsigned char *a, unsigned char *b, int limit)
{
   int i;
   for (i=0; i < limit && i < 258; ++i)
	  if (a[i] != b[i]) break;
   return i;
}

static unsigned int stbi__zhash(unsigned char *data)
{
   stbiw_uint32 hash = data[0] + (data[1] << 8) + (data[2] << 16);
   hash ^= hash << 3;
   hash += hash >> 5;
   hash ^= hash << 4;
   hash += hash >> 17;
   hash ^= hash << 25;
   hash += hash >> 6;
   return hash;
}

#define stbi__zlib_flush() (out = stbi__zlib_flushf(out, &bitbuf, &bitcount))
#define stbi__zlib_add(code,codebits) \
	  (bitbuf |= (code) << bitcount, bitcount += (codebits), stbi__zlib_flush())
#define stbi__zlib_huffa(b,c)  stbi__zlib_add(stbi__zlib_bitrev(b,c),c)
// default huffman tables
#define stbi__zlib_huff1(n)  stbi__zlib_huffa(0x30 + (n), 8)
#define stbi__zlib_huff2(n)  stbi__zlib_huffa(0x190 + (n)-144, 9)
#define stbi__zlib_huff3(n)  stbi__zlib_huffa(0 + (n)-256,7)
#define stbi__zlib_huff4(n)  stbi__zlib_huffa(0xc0 + (n)-280,8)
#define stbi__zlib_huff(n)  ((n) <= 143 ? stbi__zlib_huff1(n) : (n) <= 255 ? stbi__zlib_huff2(n) : (n) <= 279 ? stbi__zlib_huff3(n) : stbi__zlib_huff4(n))
#define stbi__zlib_huffb(n) ((n) <= 143 ? stbi__zlib_huff1(n) : stbi__zlib_huff2(n))

#define stbi__ZHASH   16384

unsigned char * stbi_zlib_compress(unsigned char *data, int data_len, int *out_len, int quality)
{
   static unsigned short lengthc[] = { 3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,51,59,67,83,99,115,131,163,195,227,258, 259 };
   static unsigned char  lengtheb[]= { 0,0,0,0,0,0,0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4,  4,  5,  5,  5,  5,  0 };
   static unsigned short distc[]   = { 1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577, 32768 };
   static unsigned char  disteb[]  = { 0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13 };
   unsigned int bitbuf=0;
   int i,j, bitcount=0;
   unsigned char *out = NULL;
   unsigned char **hash_table[stbi__ZHASH]; // 64KB on the stack!
   if (quality < 5) quality = 5;

   stbi__sbpush(out, 0x78);   // DEFLATE 32K window
   stbi__sbpush(out, 0x5e);   // FLEVEL = 1
   stbi__zlib_add(1,1);  // BFINAL = 1
   stbi__zlib_add(1,2);  // BTYPE = 1 -- fixed huffman

   for (i=0; i < stbi__ZHASH; ++i)
	  hash_table[i] = NULL;

   i=0;
   while (i < data_len-3) {
	  // hash next 3 bytes of data to be compressed
	  int h = stbi__zhash(data+i)&(stbi__ZHASH-1), best=3;
	  unsigned char *bestloc = 0;
	  unsigned char **hlist = hash_table[h];
	  int n = stbi__sbcount(hlist);
	  for (j=0; j < n; ++j) {
		 if (hlist[j]-data > i-32768) { // if entry lies within window
			int d = stbi__zlib_countm(hlist[j], data+i, data_len-i);
			if (d >= best) best=d,bestloc=hlist[j];
		 }
	  }
	  // when hash table entry is too long, delete half the entries
	  if (hash_table[h] && stbi__sbn(hash_table[h]) == 2*quality) {
		 memcpy(hash_table[h], hash_table[h]+quality, sizeof(hash_table[h][0])*quality);
		 stbi__sbn(hash_table[h]) = quality;
	  }
	  stbi__sbpush(hash_table[h],data+i);

	  if (bestloc) {
		 // "lazy matching" - check match at *next* byte, and if it's better, do cur byte as literal
		 h = stbi__zhash(data+i+1)&(stbi__ZHASH-1);
		 hlist = hash_table[h];
		 n = stbi__sbcount(hlist);
		 for (j=0; j < n; ++j) {
			if (hlist[j]-data > i-32767) {
			   int e = stbi__zlib_countm(hlist[j], data+i+1, data_len-i-1);
			   if (e > best) { // if next match is better, bail on current match
				  bestloc = NULL;
				  break;
			   }
			}
		 }
	  }

	  if (bestloc) {
		 int d = (int)( data+i - bestloc ); // distance back
		 assert(d <= 32767 && best <= 258);
		 for (j=0; best > lengthc[j+1]-1; ++j);
		 stbi__zlib_huff(j+257);
		 if (lengtheb[j]) stbi__zlib_add(best - lengthc[j], lengtheb[j]);
		 for (j=0; d > distc[j+1]-1; ++j);
		 stbi__zlib_add(stbi__zlib_bitrev(j,5),5);
		 if (disteb[j]) stbi__zlib_add(d - distc[j], disteb[j]);
		 i += best;
	  } else {
		 stbi__zlib_huffb(data[i]);
		 ++i;
	  }
   }
   // write out final bytes
   for (;i < data_len; ++i)
	  stbi__zlib_huffb(data[i]);
   stbi__zlib_huff(256); // end of block
   // pad with 0 bits to byte boundary
   while (bitcount)
	  stbi__zlib_add(0,1);

   for (i=0; i < stbi__ZHASH; ++i)
	  (void) stbi__sbfree(hash_table[i]);

   {
	  // compute adler32 on input
	  unsigned int i=0, s1=1, s2=0, blocklen = data_len % 5552;
	  int j=0;
	  while (j < data_len) {
		 for (i=0; i < blocklen; ++i) s1 += data[j+i], s2 += s1;
		 s1 %= 65521, s2 %= 65521;
		 j += blocklen;
		 blocklen = 5552;
	  }
	  stbi__sbpush(out, (unsigned char) (s2 >> 8));
	  stbi__sbpush(out, (unsigned char) s2);
	  stbi__sbpush(out, (unsigned char) (s1 >> 8));
	  stbi__sbpush(out, (unsigned char) s1);
   }
   *out_len = stbi__sbn(out);
   // make returned pointer freeable
   memmove(stbi__sbraw(out), out, *out_len);
   return (unsigned char *) stbi__sbraw(out);
}

unsigned int stbi__crc32(unsigned char *buffer, int len)
{
   static unsigned int crc_table[256];
   unsigned int crc = ~0u;
   int i,j;
   if (crc_table[1] == 0)
	  for(i=0; i < 256; i++)
		 for (crc_table[i]=i, j=0; j < 8; ++j)
			crc_table[i] = (crc_table[i] >> 1) ^ (crc_table[i] & 1 ? 0xedb88320 : 0);
   for (i=0; i < len; ++i)
	  crc = (crc >> 8) ^ crc_table[buffer[i] ^ (crc & 0xff)];
   return ~crc;
}

#define stbi__wpng4(o,a,b,c,d) ((o)[0]=(unsigned char)(a),(o)[1]=(unsigned char)(b),(o)[2]=(unsigned char)(c),(o)[3]=(unsigned char)(d),(o)+=4)
#define stbi__wp32(data,v) stbi__wpng4(data, (v)>>24,(v)>>16,(v)>>8,(v));
#define stbi__wptag(data,s) stbi__wpng4(data, s[0],s[1],s[2],s[3])

static void stbi__wpcrc(unsigned char **data, int len)
{
   unsigned int crc = stbi__crc32(*data - len - 4, len+4);
   stbi__wp32(*data, crc);
}

static unsigned char stbiw__paeth(int a, int b, int c)
{
   int p = a + b - c, pa = abs(p-a), pb = abs(p-b), pc = abs(p-c);
   if (pa <= pb && pa <= pc) return (unsigned char) a;
   if (pb <= pc) return (unsigned char) b;
   return (unsigned char) c;
}

unsigned char *stbi_write_png_to_mem(unsigned char *pixels, int stride_bytes, int x, int y, int n, int *out_len)
{
   int ctype[5] = { -1, 0, 4, 2, 6 };
   unsigned char sig[8] = { 137,80,78,71,13,10,26,10 };
   unsigned char *out,*o, *filt, *zlib;
   signed char *line_buffer;
   int i,j,k,p,zlen;

   if (stride_bytes == 0)
	  stride_bytes = x * n;

   filt = (unsigned char *) malloc((x*n+1) * y); if (!filt) return 0;
   line_buffer = (signed char *) malloc(x * n); if (!line_buffer) { free(filt); return 0; }
   for (j=0; j < y; ++j) {
	  static int mapping[] = { 0,1,2,3,4 };
	  static int firstmap[] = { 0,1,0,5,6 };
	  int *mymap = j ? mapping : firstmap;
	  int best = 0, bestval = 0x7fffffff;
	  for (p=0; p < 2; ++p) {
		 for (k= p?best:0; k < 5; ++k) {
			int type = mymap[k],est=0;
			unsigned char *z = pixels + stride_bytes*j;
			for (i=0; i < n; ++i)
			   switch (type) {
				  case 0: line_buffer[i] = z[i]; break;
				  case 1: line_buffer[i] = z[i]; break;
				  case 2: line_buffer[i] = z[i] - z[i-stride_bytes]; break;
				  case 3: line_buffer[i] = z[i] - (z[i-stride_bytes]>>1); break;
				  case 4: line_buffer[i] = (signed char) (z[i] - stbiw__paeth(0,z[i-stride_bytes],0)); break;
				  case 5: line_buffer[i] = z[i]; break;
				  case 6: line_buffer[i] = z[i]; break;
			   }
			for (i=n; i < x*n; ++i) {
			   switch (type) {
				  case 0: line_buffer[i] = z[i]; break;
				  case 1: line_buffer[i] = z[i] - z[i-n]; break;
				  case 2: line_buffer[i] = z[i] - z[i-stride_bytes]; break;
				  case 3: line_buffer[i] = z[i] - ((z[i-n] + z[i-stride_bytes])>>1); break;
				  case 4: line_buffer[i] = z[i] - stbiw__paeth(z[i-n], z[i-stride_bytes], z[i-stride_bytes-n]); break;
				  case 5: line_buffer[i] = z[i] - (z[i-n]>>1); break;
				  case 6: line_buffer[i] = z[i] - stbiw__paeth(z[i-n], 0,0); break;
			   }
			}
			if (p) break;
			for (i=0; i < x*n; ++i)
			   est += abs((signed char) line_buffer[i]);
			if (est < bestval) { bestval = est; best = k; }
		 }
	  }
	  // when we get here, best contains the filter type, and line_buffer contains the data
	  filt[j*(x*n+1)] = (unsigned char) best;
	  memcpy(filt+j*(x*n+1)+1, line_buffer, x*n);
   }
   free(line_buffer);
   zlib = stbi_zlib_compress(filt, y*( x*n+1), &zlen, 8); // increase 8 to get smaller but use more memory
   free(filt);
   if (!zlib) return 0;

   // each tag requires 12 bytes of overhead
   out = (unsigned char *) malloc(8 + 12+13 + 12+zlen + 12);
   if (!out) return 0;
   *out_len = 8 + 12+13 + 12+zlen + 12;

   o=out;
   memcpy(o,sig,8); o+= 8;
   stbi__wp32(o, 13); // header length
   stbi__wptag(o, "IHDR");
   stbi__wp32(o, x);
   stbi__wp32(o, y);
   *o++ = 8;
   *o++ = (unsigned char) ctype[n];
   *o++ = 0;
   *o++ = 0;
   *o++ = 0;
   stbi__wpcrc(&o,13);

   stbi__wp32(o, zlen);
   stbi__wptag(o, "IDAT");
   memcpy(o, zlib, zlen); o += zlen; free(zlib);
   stbi__wpcrc(&o, zlen);

   stbi__wp32(o,0);
   stbi__wptag(o, "IEND");
   stbi__wpcrc(&o,0);

   assert(o == out + *out_len);

   return out;
}

int stbi_write_png(char const *filename, int x, int y, int comp, const void *data, int stride_bytes)
{
   FILE *f;
   int len;
   unsigned char *png = stbi_write_png_to_mem((unsigned char *) data, stride_bytes, x, y, comp, &len);
   if (!png) return 0;
   f = fopen(filename, "wb");
   if (!f) { free(png); return 0; }
   fwrite(png, 1, len, f);
   fclose(f);
   free(png);
   return 1;
}
//#endif // STB_IMAGE_WRITE_IMPLEMENTATION

/* Revision history

	  0.92 (2010-08-01)
			 casts to unsigned char to fix warnings
	  0.91 (2010-07-17)
			 first public release
	  0.90   first internal release
*/

#define DDS_header DDS_header2

//#line 1 "image_DXT.h"
#ifndef HEADER_IMAGE_DXT
#define HEADER_IMAGE_DXT

/**
	Converts an image from an array of unsigned chars (RGB or RGBA) to
	DXT1 or DXT5, then saves the converted image to disk.
	\return 0 if failed, otherwise returns 1
**/
int
save_image_as_DDS
(
	const char *filename,
	int width, int height, int channels,
	const unsigned char *const data
);

/**
	take an image and convert it to DXT1 (no alpha)
**/
unsigned char*
convert_image_to_DXT1
(
	const unsigned char *const uncompressed,
	int width, int height, int channels,
	int *out_size
);

/**
	take an image and convert it to DXT5 (with alpha)
**/
unsigned char*
convert_image_to_DXT5
(
	const unsigned char *const uncompressed,
	int width, int height, int channels,
	int *out_size
);

/**	A bunch of DirectDraw Surface structures and flags **/
typedef struct
{
	unsigned int    dwMagic;
	unsigned int    dwSize;
	unsigned int    dwFlags;
	unsigned int    dwHeight;
	unsigned int    dwWidth;
	unsigned int    dwPitchOrLinearSize;
	unsigned int    dwDepth;
	unsigned int    dwMipMapCount;
	unsigned int    dwReserved1[ 11 ];

	/*  DDPIXELFORMAT	*/
	struct
	{
		unsigned int    dwSize;
		unsigned int    dwFlags;
		unsigned int    dwFourCC;
		unsigned int    dwRGBBitCount;
		unsigned int    dwRBitMask;
		unsigned int    dwGBitMask;
		unsigned int    dwBBitMask;
		unsigned int    dwAlphaBitMask;
	}
	sPixelFormat;

	/*  DDCAPS2	*/
	struct
	{
		unsigned int    dwCaps1;
		unsigned int    dwCaps2;
		unsigned int    dwDDSX;
		unsigned int    dwReserved;
	}
	sCaps;
	unsigned int    dwReserved2;
}
DDS_header ;

/*	the following constants were copied directly off the MSDN website	*/

/*	The dwFlags member of the original DDSURFACEDESC2 structure
	can be set to one or more of the following values.	*/
#define DDSD_CAPS	0x00000001
#define DDSD_HEIGHT	0x00000002
#define DDSD_WIDTH	0x00000004
#define DDSD_PITCH	0x00000008
#define DDSD_PIXELFORMAT	0x00001000
#define DDSD_MIPMAPCOUNT	0x00020000
#define DDSD_LINEARSIZE	0x00080000
#define DDSD_DEPTH	0x00800000

/*	DirectDraw Pixel Format	*/
#define DDPF_ALPHAPIXELS	0x00000001
#define DDPF_FOURCC	0x00000004
#define DDPF_RGB	0x00000040

/*	The dwCaps1 member of the DDSCAPS2 structure can be
	set to one or more of the following values.	*/
#define DDSCAPS_COMPLEX	0x00000008
#define DDSCAPS_TEXTURE	0x00001000
#define DDSCAPS_MIPMAP	0x00400000

/*	The dwCaps2 member of the DDSCAPS2 structure can be
	set to one or more of the following values.		*/
#define DDSCAPS2_CUBEMAP	0x00000200
#define DDSCAPS2_CUBEMAP_POSITIVEX	0x00000400
#define DDSCAPS2_CUBEMAP_NEGATIVEX	0x00000800
#define DDSCAPS2_CUBEMAP_POSITIVEY	0x00001000
#define DDSCAPS2_CUBEMAP_NEGATIVEY	0x00002000
#define DDSCAPS2_CUBEMAP_POSITIVEZ	0x00004000
#define DDSCAPS2_CUBEMAP_NEGATIVEZ	0x00008000
#define DDSCAPS2_VOLUME	0x00200000

#endif /* HEADER_IMAGE_DXT	*/



//#line 1 "image_DXT.c"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*	set this =1 if you want to use the covarince matrix method...
	which is better than my method of using standard deviations
	overall, except on the infintesimal chance that the power
	method fails for finding the largest eigenvector	*/
#define USE_COV_MAT	1

/********* Function Prototypes *********/
/*
	Takes a 4x4 block of pixels and compresses it into 8 bytes
	in DXT1 format (color only, no alpha).  Speed is valued
	over prettyness, at least for now.
*/
void compress_DDS_color_block(
				int channels,
				const unsigned char *const uncompressed,
				unsigned char compressed[8] );
/*
	Takes a 4x4 block of pixels and compresses the alpha
	component it into 8 bytes for use in DXT5 DDS files.
	Speed is valued over prettyness, at least for now.
*/
void compress_DDS_alpha_block(
				const unsigned char *const uncompressed,
				unsigned char compressed[8] );

/********* Actual Exposed Functions *********/
int
	save_image_as_DDS
	(
		const char *filename,
		int width, int height, int channels,
		const unsigned char *const data
	)
{
	/*	variables	*/
	FILE *fout;
	unsigned char *DDS_data;
	DDS_header header;
	int DDS_size;
	/*	error check	*/
	if( (NULL == filename) ||
		(width < 1) || (height < 1) ||
		(channels < 1) || (channels > 4) ||
		(data == NULL ) )
	{
		return 0;
	}
	/*	Convert the image	*/
	if( (channels & 1) == 1 )
	{
		/*	no alpha, just use DXT1	*/
		DDS_data = convert_image_to_DXT1( data, width, height, channels, &DDS_size );
	} else
	{
		/*	has alpha, so use DXT5	*/
		DDS_data = convert_image_to_DXT5( data, width, height, channels, &DDS_size );
	}
	/*	save it	*/
	memset( &header, 0, sizeof( DDS_header ) );
	header.dwMagic = ('D' << 0) | ('D' << 8) | ('S' << 16) | (' ' << 24);
	header.dwSize = 124;
	header.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_LINEARSIZE;
	header.dwWidth = width;
	header.dwHeight = height;
	header.dwPitchOrLinearSize = DDS_size;
	header.sPixelFormat.dwSize = 32;
	header.sPixelFormat.dwFlags = DDPF_FOURCC;
	if( (channels & 1) == 1 )
	{
		header.sPixelFormat.dwFourCC = ('D' << 0) | ('X' << 8) | ('T' << 16) | ('1' << 24);
	} else
	{
		header.sPixelFormat.dwFourCC = ('D' << 0) | ('X' << 8) | ('T' << 16) | ('5' << 24);
	}
	header.sCaps.dwCaps1 = DDSCAPS_TEXTURE;
	/*	write it out	*/
	fout = fopen( filename, "wb");
	fwrite( &header, sizeof( DDS_header ), 1, fout );
	fwrite( DDS_data, 1, DDS_size, fout );
	fclose( fout );
	/*	done	*/
	free( DDS_data );
	return 1;
}

unsigned char* convert_image_to_DXT1(
		const unsigned char *const uncompressed,
		int width, int height, int channels,
		int *out_size )
{
	unsigned char *compressed;
	int i, j, x, y;
	unsigned char ublock[16*3];
	unsigned char cblock[8];
	int index = 0, chan_step = 1;
	int block_count = 0;
	/*	error check	*/
	*out_size = 0;
	if( (width < 1) || (height < 1) ||
		(NULL == uncompressed) ||
		(channels < 1) || (channels > 4) )
	{
		return NULL;
	}
	/*	for channels == 1 or 2, I do not step forward for R,G,B values	*/
	if( channels < 3 )
	{
		chan_step = 0;
	}
	/*	get the RAM for the compressed image
		(8 bytes per 4x4 pixel block)	*/
	*out_size = ((width+3) >> 2) * ((height+3) >> 2) * 8;
	compressed = (unsigned char*)malloc( *out_size );
	/*	go through each block	*/
	for( j = 0; j < height; j += 4 )
	{
		for( i = 0; i < width; i += 4 )
		{
			/*	copy this block into a new one	*/
			int idx = 0;
			int mx = 4, my = 4;
			if( j+4 >= height )
			{
				my = height - j;
			}
			if( i+4 >= width )
			{
				mx = width - i;
			}
			for( y = 0; y < my; ++y )
			{
				for( x = 0; x < mx; ++x )
				{
					ublock[idx++] = uncompressed[(j+y)*width*channels+(i+x)*channels];
					ublock[idx++] = uncompressed[(j+y)*width*channels+(i+x)*channels+chan_step];
					ublock[idx++] = uncompressed[(j+y)*width*channels+(i+x)*channels+chan_step+chan_step];
				}
				for( x = mx; x < 4; ++x )
				{
					ublock[idx++] = ublock[0];
					ublock[idx++] = ublock[1];
					ublock[idx++] = ublock[2];
				}
			}
			for( y = my; y < 4; ++y )
			{
				for( x = 0; x < 4; ++x )
				{
					ublock[idx++] = ublock[0];
					ublock[idx++] = ublock[1];
					ublock[idx++] = ublock[2];
				}
			}
			/*	compress the block	*/
			++block_count;
			compress_DDS_color_block( 3, ublock, cblock );
			/*	copy the data from the block into the main block	*/
			for( x = 0; x < 8; ++x )
			{
				compressed[index++] = cblock[x];
			}
		}
	}
	return compressed;
}

unsigned char* convert_image_to_DXT5(
		const unsigned char *const uncompressed,
		int width, int height, int channels,
		int *out_size )
{
	unsigned char *compressed;
	int i, j, x, y;
	unsigned char ublock[16*4];
	unsigned char cblock[8];
	int index = 0, chan_step = 1;
	int block_count = 0, has_alpha;
	/*	error check	*/
	*out_size = 0;
	if( (width < 1) || (height < 1) ||
		(NULL == uncompressed) ||
		(channels < 1) || ( channels > 4) )
	{
		return NULL;
	}
	/*	for channels == 1 or 2, I do not step forward for R,G,B vales	*/
	if( channels < 3 )
	{
		chan_step = 0;
	}
	/*	# channels = 1 or 3 have no alpha, 2 & 4 do have alpha	*/
	has_alpha = 1 - (channels & 1);
	/*	get the RAM for the compressed image
		(16 bytes per 4x4 pixel block)	*/
	*out_size = ((width+3) >> 2) * ((height+3) >> 2) * 16;
	compressed = (unsigned char*)malloc( *out_size );
	/*	go through each block	*/
	for( j = 0; j < height; j += 4 )
	{
		for( i = 0; i < width; i += 4 )
		{
			/*	local variables, and my block counter	*/
			int idx = 0;
			int mx = 4, my = 4;
			if( j+4 >= height )
			{
				my = height - j;
			}
			if( i+4 >= width )
			{
				mx = width - i;
			}
			for( y = 0; y < my; ++y )
			{
				for( x = 0; x < mx; ++x )
				{
					ublock[idx++] = uncompressed[(j+y)*width*channels+(i+x)*channels];
					ublock[idx++] = uncompressed[(j+y)*width*channels+(i+x)*channels+chan_step];
					ublock[idx++] = uncompressed[(j+y)*width*channels+(i+x)*channels+chan_step+chan_step];
					ublock[idx++] =
						has_alpha * uncompressed[(j+y)*width*channels+(i+x)*channels+channels-1]
						+ (1-has_alpha)*255;
				}
				for( x = mx; x < 4; ++x )
				{
					ublock[idx++] = ublock[0];
					ublock[idx++] = ublock[1];
					ublock[idx++] = ublock[2];
					ublock[idx++] = ublock[3];
				}
			}
			for( y = my; y < 4; ++y )
			{
				for( x = 0; x < 4; ++x )
				{
					ublock[idx++] = ublock[0];
					ublock[idx++] = ublock[1];
					ublock[idx++] = ublock[2];
					ublock[idx++] = ublock[3];
				}
			}
			/*	now compress the alpha block	*/
			compress_DDS_alpha_block( ublock, cblock );
			/*	copy the data from the compressed alpha block into the main buffer	*/
			for( x = 0; x < 8; ++x )
			{
				compressed[index++] = cblock[x];
			}
			/*	then compress the color block	*/
			++block_count;
			compress_DDS_color_block( 4, ublock, cblock );
			/*	copy the data from the compressed color block into the main buffer	*/
			for( x = 0; x < 8; ++x )
			{
				compressed[index++] = cblock[x];
			}
		}
	}
	return compressed;
}

/********* Helper Functions *********/
int convert_bit_range( int c, int from_bits, int to_bits )
{
	int b = (1 << (from_bits - 1)) + c * ((1 << to_bits) - 1);
	return (b + (b >> from_bits)) >> from_bits;
}

int rgb_to_565( int r, int g, int b )
{
	return
		(convert_bit_range( r, 8, 5 ) << 11) |
		(convert_bit_range( g, 8, 6 ) << 05) |
		(convert_bit_range( b, 8, 5 ) << 00);
}

void rgb_888_from_565( unsigned int c, int *r, int *g, int *b )
{
	*r = convert_bit_range( (c >> 11) & 31, 5, 8 );
	*g = convert_bit_range( (c >> 05) & 63, 6, 8 );
	*b = convert_bit_range( (c >> 00) & 31, 5, 8 );
}

void compute_color_line_STDEV(
		const unsigned char *const uncompressed,
		int channels,
		float point[3], float direction[3] )
{
	const float inv_16 = 1.0f / 16.0f;
	int i;
	float sum_r = 0.0f, sum_g = 0.0f, sum_b = 0.0f;
	float sum_rr = 0.0f, sum_gg = 0.0f, sum_bb = 0.0f;
	float sum_rg = 0.0f, sum_rb = 0.0f, sum_gb = 0.0f;
	/*	calculate all data needed for the covariance matrix
		( to compare with _rygdxt code)	*/
	for( i = 0; i < 16*channels; i += channels )
	{
		sum_r += uncompressed[i+0];
		sum_rr += uncompressed[i+0] * uncompressed[i+0];
		sum_g += uncompressed[i+1];
		sum_gg += uncompressed[i+1] * uncompressed[i+1];
		sum_b += uncompressed[i+2];
		sum_bb += uncompressed[i+2] * uncompressed[i+2];
		sum_rg += uncompressed[i+0] * uncompressed[i+1];
		sum_rb += uncompressed[i+0] * uncompressed[i+2];
		sum_gb += uncompressed[i+1] * uncompressed[i+2];
	}
	/*	convert the sums to averages	*/
	sum_r *= inv_16;
	sum_g *= inv_16;
	sum_b *= inv_16;
	/*	and convert the squares to the squares of the value - avg_value	*/
	sum_rr -= 16.0f * sum_r * sum_r;
	sum_gg -= 16.0f * sum_g * sum_g;
	sum_bb -= 16.0f * sum_b * sum_b;
	sum_rg -= 16.0f * sum_r * sum_g;
	sum_rb -= 16.0f * sum_r * sum_b;
	sum_gb -= 16.0f * sum_g * sum_b;
	/*	the point on the color line is the average	*/
	point[0] = sum_r;
	point[1] = sum_g;
	point[2] = sum_b;
	#if USE_COV_MAT
	/*
		The following idea was from ryg.
		(https://mollyrocket.com/forums/viewtopic.php?t=392)
		The method worked great (less RMSE than mine) most of
		the time, but had some issues handling some simple
		boundary cases, like full green next to full red,
		which would generate a covariance matrix like this:

		| 1  -1  0 |
		| -1  1  0 |
		| 0   0  0 |

		For a given starting vector, the power method can
		generate all zeros!  So no starting with {1,1,1}
		as I was doing!  This kind of error is still a
		slight posibillity, but will be very rare.
	*/
	/*	use the covariance matrix directly
		(1st iteration, don't use all 1.0 values!)	*/
	sum_r = 1.0f;
	sum_g = 2.718281828f;
	sum_b = 3.141592654f;
	direction[0] = sum_r*sum_rr + sum_g*sum_rg + sum_b*sum_rb;
	direction[1] = sum_r*sum_rg + sum_g*sum_gg + sum_b*sum_gb;
	direction[2] = sum_r*sum_rb + sum_g*sum_gb + sum_b*sum_bb;
	/*	2nd iteration, use results from the 1st guy	*/
	sum_r = direction[0];
	sum_g = direction[1];
	sum_b = direction[2];
	direction[0] = sum_r*sum_rr + sum_g*sum_rg + sum_b*sum_rb;
	direction[1] = sum_r*sum_rg + sum_g*sum_gg + sum_b*sum_gb;
	direction[2] = sum_r*sum_rb + sum_g*sum_gb + sum_b*sum_bb;
	/*	3rd iteration, use results from the 2nd guy	*/
	sum_r = direction[0];
	sum_g = direction[1];
	sum_b = direction[2];
	direction[0] = sum_r*sum_rr + sum_g*sum_rg + sum_b*sum_rb;
	direction[1] = sum_r*sum_rg + sum_g*sum_gg + sum_b*sum_gb;
	direction[2] = sum_r*sum_rb + sum_g*sum_gb + sum_b*sum_bb;
	#else
	/*	use my standard deviation method
		(very robust, a tiny bit slower and less accurate)	*/
	direction[0] = sqrt( sum_rr );
	direction[1] = sqrt( sum_gg );
	direction[2] = sqrt( sum_bb );
	/*	which has a greater component	*/
	if( sum_gg > sum_rr )
	{
		/*	green has greater component, so base the other signs off of green	*/
		if( sum_rg < 0.0f )
		{
			direction[0] = -direction[0];
		}
		if( sum_gb < 0.0f )
		{
			direction[2] = -direction[2];
		}
	} else
	{
		/*	red has a greater component	*/
		if( sum_rg < 0.0f )
		{
			direction[1] = -direction[1];
		}
		if( sum_rb < 0.0f )
		{
			direction[2] = -direction[2];
		}
	}
	#endif
}

void LSE_master_colors_max_min(
		int *cmax, int *cmin,
		int channels,
		const unsigned char *const uncompressed )
{
	int i, j;
	/*	the master colors	*/
	int c0[3], c1[3];
	/*	used for fitting the line	*/
	float sum_x[] = { 0.0f, 0.0f, 0.0f };
	float sum_x2[] = { 0.0f, 0.0f, 0.0f };
	float dot_max = 1.0f, dot_min = -1.0f;
	float vec_len2 = 0.0f;
	float dot;
	/*	error check	*/
	if( (channels < 3) || (channels > 4) )
	{
		return;
	}
	compute_color_line_STDEV( uncompressed, channels, sum_x, sum_x2 );
	vec_len2 = 1.0f / ( 0.00001f +
			sum_x2[0]*sum_x2[0] + sum_x2[1]*sum_x2[1] + sum_x2[2]*sum_x2[2] );
	/*	finding the max and min vector values	*/
	dot_max =
			(
				sum_x2[0] * uncompressed[0] +
				sum_x2[1] * uncompressed[1] +
				sum_x2[2] * uncompressed[2]
			);
	dot_min = dot_max;
	for( i = 1; i < 16; ++i )
	{
		dot =
			(
				sum_x2[0] * uncompressed[i*channels+0] +
				sum_x2[1] * uncompressed[i*channels+1] +
				sum_x2[2] * uncompressed[i*channels+2]
			);
		if( dot < dot_min )
		{
			dot_min = dot;
		} else if( dot > dot_max )
		{
			dot_max = dot;
		}
	}
	/*	and the offset (from the average location)	*/
	dot = sum_x2[0]*sum_x[0] + sum_x2[1]*sum_x[1] + sum_x2[2]*sum_x[2];
	dot_min -= dot;
	dot_max -= dot;
	/*	post multiply by the scaling factor	*/
	dot_min *= vec_len2;
	dot_max *= vec_len2;
	/*	OK, build the master colors	*/
	for( i = 0; i < 3; ++i )
	{
		/*	color 0	*/
		c0[i] = (int)(0.5f + sum_x[i] + dot_max * sum_x2[i]);
		if( c0[i] < 0 )
		{
			c0[i] = 0;
		} else if( c0[i] > 255 )
		{
			c0[i] = 255;
		}
		/*	color 1	*/
		c1[i] = (int)(0.5f + sum_x[i] + dot_min * sum_x2[i]);
		if( c1[i] < 0 )
		{
			c1[i] = 0;
		} else if( c1[i] > 255 )
		{
			c1[i] = 255;
		}
	}
	/*	down_sample (with rounding?)	*/
	i = rgb_to_565( c0[0], c0[1], c0[2] );
	j = rgb_to_565( c1[0], c1[1], c1[2] );
	if( i > j )
	{
		*cmax = i;
		*cmin = j;
	} else
	{
		*cmax = j;
		*cmin = i;
	}
}

void
	compress_DDS_color_block
	(
		int channels,
		const unsigned char *const uncompressed,
		unsigned char compressed[8]
	)
{
	/*	variables	*/
	int i;
	int next_bit;
	int enc_c0, enc_c1;
	int c0[4], c1[4];
	float color_line[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	float vec_len2 = 0.0f, dot_offset = 0.0f;
	/*	stupid order	*/
	int swizzle4[] = { 0, 2, 3, 1 };
	/*	get the master colors	*/
	LSE_master_colors_max_min( &enc_c0, &enc_c1, channels, uncompressed );
	/*	store the 565 color 0 and color 1	*/
	compressed[0] = (enc_c0 >> 0) & 255;
	compressed[1] = (enc_c0 >> 8) & 255;
	compressed[2] = (enc_c1 >> 0) & 255;
	compressed[3] = (enc_c1 >> 8) & 255;
	/*	zero out the compressed data	*/
	compressed[4] = 0;
	compressed[5] = 0;
	compressed[6] = 0;
	compressed[7] = 0;
	/*	reconstitute the master color vectors	*/
	rgb_888_from_565( enc_c0, &c0[0], &c0[1], &c0[2] );
	rgb_888_from_565( enc_c1, &c1[0], &c1[1], &c1[2] );
	/*	the new vector	*/
	vec_len2 = 0.0f;
	for( i = 0; i < 3; ++i )
	{
		color_line[i] = (float)(c1[i] - c0[i]);
		vec_len2 += color_line[i] * color_line[i];
	}
	if( vec_len2 > 0.0f )
	{
		vec_len2 = 1.0f / vec_len2;
	}
	/*	pre-proform the scaling	*/
	color_line[0] *= vec_len2;
	color_line[1] *= vec_len2;
	color_line[2] *= vec_len2;
	/*	compute the offset (constant) portion of the dot product	*/
	dot_offset = color_line[0]*c0[0] + color_line[1]*c0[1] + color_line[2]*c0[2];
	/*	store the rest of the bits	*/
	next_bit = 8*4;
	for( i = 0; i < 16; ++i )
	{
		/*	find the dot product of this color, to place it on the line
			(should be [-1,1])	*/
		int next_value = 0;
		float dot_product =
			color_line[0] * uncompressed[i*channels+0] +
			color_line[1] * uncompressed[i*channels+1] +
			color_line[2] * uncompressed[i*channels+2] -
			dot_offset;
		/*	map to [0,3]	*/
		next_value = (int)( dot_product * 3.0f + 0.5f );
		if( next_value > 3 )
		{
			next_value = 3;
		} else if( next_value < 0 )
		{
			next_value = 0;
		}
		/*	OK, store this value	*/
		compressed[next_bit >> 3] |= swizzle4[ next_value ] << (next_bit & 7);
		next_bit += 2;
	}
	/*	done compressing to DXT1	*/
}

void
	compress_DDS_alpha_block
	(
		const unsigned char *const uncompressed,
		unsigned char compressed[8]
	)
{
	/*	variables	*/
	int i;
	int next_bit;
	int a0, a1;
	float scale_me;
	/*	stupid order	*/
	int swizzle8[] = { 1, 7, 6, 5, 4, 3, 2, 0 };
	/*	get the alpha limits (a0 > a1)	*/
	a0 = a1 = uncompressed[3];
	for( i = 4+3; i < 16*4; i += 4 )
	{
		if( uncompressed[i] > a0 )
		{
			a0 = uncompressed[i];
		} else if( uncompressed[i] < a1 )
		{
			a1 = uncompressed[i];
		}
	}
	/*	store those limits, and zero the rest of the compressed dataset	*/
	compressed[0] = a0;
	compressed[1] = a1;
	/*	zero out the compressed data	*/
	compressed[2] = 0;
	compressed[3] = 0;
	compressed[4] = 0;
	compressed[5] = 0;
	compressed[6] = 0;
	compressed[7] = 0;
	/*	store the all of the alpha values	*/
	next_bit = 8*2;
	scale_me = 7.9999f / (a0 - a1);
	for( i = 3; i < 16*4; i += 4 )
	{
		/*	convert this alpha value to a 3 bit number	*/
		int svalue;
		int value = (int)((uncompressed[i] - a1) * scale_me);
		svalue = swizzle8[ value&7 ];
		/*	OK, store this value, start with the 1st byte	*/
		compressed[next_bit >> 3] |= svalue << (next_bit & 7);
		if( (next_bit & 7) > 5 )
		{
			/*	spans 2 bytes, fill in the start of the 2nd byte	*/
			compressed[1 + (next_bit >> 3)] |= svalue >> (8 - (next_bit & 7) );
		}
		next_bit += 3;
	}
	/*	done compressing to DXT1	*/
}

#define clamp clamp2

//#line 1 "etc1_utils.c"
// Copyright 2009 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <string.h>

/* From http://www.khronos.org/registry/gles/extensions/OES/OES_compressed_ETC1_RGB8_texture.txt

 The number of bits that represent a 4x4 texel block is 64 bits if
 <internalformat> is given by ETC1_RGB8_OES.

 The data for a block is a number of bytes,

 {q0, q1, q2, q3, q4, q5, q6, q7}

 where byte q0 is located at the lowest memory address and q7 at
 the highest. The 64 bits specifying the block is then represented
 by the following 64 bit integer:

 int64bit = 256*(256*(256*(256*(256*(256*(256*q0+q1)+q2)+q3)+q4)+q5)+q6)+q7;

 ETC1_RGB8_OES:

 a) bit layout in bits 63 through 32 if diffbit = 0

 63 62 61 60 59 58 57 56 55 54 53 52 51 50 49 48
 -----------------------------------------------
 | base col1 | base col2 | base col1 | base col2 |
 | R1 (4bits)| R2 (4bits)| G1 (4bits)| G2 (4bits)|
 -----------------------------------------------

 47 46 45 44 43 42 41 40 39 38 37 36 35 34  33  32
 ---------------------------------------------------
 | base col1 | base col2 | table  | table  |diff|flip|
 | B1 (4bits)| B2 (4bits)| cw 1   | cw 2   |bit |bit |
 ---------------------------------------------------

 b) bit layout in bits 63 through 32 if diffbit = 1

 63 62 61 60 59 58 57 56 55 54 53 52 51 50 49 48
 -----------------------------------------------
 | base col1    | dcol 2 | base col1    | dcol 2 |
 | R1' (5 bits) | dR2    | G1' (5 bits) | dG2    |
 -----------------------------------------------

 47 46 45 44 43 42 41 40 39 38 37 36 35 34  33  32
 ---------------------------------------------------
 | base col 1   | dcol 2 | table  | table  |diff|flip|
 | B1' (5 bits) | dB2    | cw 1   | cw 2   |bit |bit |
 ---------------------------------------------------

 c) bit layout in bits 31 through 0 (in both cases)

 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16
 -----------------------------------------------
 |       most significant pixel index bits       |
 | p| o| n| m| l| k| j| i| h| g| f| e| d| c| b| a|
 -----------------------------------------------

 15 14 13 12 11 10  9  8  7  6  5  4  3   2   1  0
 --------------------------------------------------
 |         least significant pixel index bits       |
 | p| o| n| m| l| k| j| i| h| g| f| e| d| c | b | a |
 --------------------------------------------------

 Add table 3.17.2: Intensity modifier sets for ETC1 compressed textures:

 table codeword                modifier table
 ------------------        ----------------------
 0                     -8  -2  2   8
 1                    -17  -5  5  17
 2                    -29  -9  9  29
 3                    -42 -13 13  42
 4                    -60 -18 18  60
 5                    -80 -24 24  80
 6                   -106 -33 33 106
 7                   -183 -47 47 183

 Add table 3.17.3 Mapping from pixel index values to modifier values for
 ETC1 compressed textures:

 pixel index value
 ---------------
 msb     lsb           resulting modifier value
 -----   -----          -------------------------
 1       1            -b (large negative value)
 1       0            -a (small negative value)
 0       0             a (small positive value)
 0       1             b (large positive value)

 */

static const int kModifierTable[] = {
/* 0 */2, 8, -2, -8,
/* 1 */5, 17, -5, -17,
/* 2 */9, 29, -9, -29,
/* 3 */13, 42, -13, -42,
/* 4 */18, 60, -18, -60,
/* 5 */24, 80, -24, -80,
/* 6 */33, 106, -33, -106,
/* 7 */47, 183, -47, -183 };

static const int kLookup[8] = { 0, 1, 2, 3, -4, -3, -2, -1 };

static inline etc1_byte clamp(int x) {
	return (etc1_byte) (x >= 0 ? (x < 255 ? x : 255) : 0);
}

static
inline int convert4To8(int b) {
	int c = b & 0xf;
	return (c << 4) | c;
}

static
inline int convert5To8(int b) {
	int c = b & 0x1f;
	return (c << 3) | (c >> 2);
}

static
inline int convert6To8(int b) {
	int c = b & 0x3f;
	return (c << 2) | (c >> 4);
}

static
inline int divideBy255(int d) {
	return (d + 128 + (d >> 8)) >> 8;
}

static
inline int convert8To4(int b) {
	//int c = b & 0xff;
	return divideBy255(b * 15);
}

static
inline int convert8To5(int b) {
	//int c = b & 0xff;
	return divideBy255(b * 31);
}

static
inline int convertDiff(int base, int diff) {
	return convert5To8((0x1f & base) + kLookup[0x7 & diff]);
}

static
void decode_subblock(etc1_byte* pOut, int r, int g, int b, const int* table,
		etc1_uint32 low, etc1_bool second, etc1_bool flipped) {
	int baseX = 0;
	int baseY = 0;
	int i;

	if (second) {
		if (flipped) {
			baseY = 2;
		} else {
			baseX = 2;
		}
	}
	for (i = 0; i < 8; i++) {
		int x, y;
		if (flipped) {
			x = baseX + (i >> 1);
			y = baseY + (i & 1);
		} else {
			x = baseX + (i >> 2);
			y = baseY + (i & 3);
		}
		int k = y + (x * 4);
		int offset = ((low >> k) & 1) | ((low >> (k + 15)) & 2);
		int delta = table[offset];
		etc1_byte* q = pOut + 3 * (x + 4 * y);
		*q++ = clamp(r + delta);
		*q++ = clamp(g + delta);
		*q++ = clamp(b + delta);
	}
}

// Input is an ETC1 compressed version of the data.
// Output is a 4 x 4 square of 3-byte pixels in form R, G, B

void etc1_decode_block(const etc1_byte* pIn, etc1_byte* pOut) {
	etc1_uint32 high = (pIn[0] << 24) | (pIn[1] << 16) | (pIn[2] << 8) | pIn[3];
	etc1_uint32 low = (pIn[4] << 24) | (pIn[5] << 16) | (pIn[6] << 8) | pIn[7];
	int r1, r2, g1, g2, b1, b2;
	if (high & 2) {
		// differential
		int rBase = high >> 27;
		int gBase = high >> 19;
		int bBase = high >> 11;
		r1 = convert5To8(rBase);
		r2 = convertDiff(rBase, high >> 24);
		g1 = convert5To8(gBase);
		g2 = convertDiff(gBase, high >> 16);
		b1 = convert5To8(bBase);
		b2 = convertDiff(bBase, high >> 8);
	} else {
		// not differential
		r1 = convert4To8(high >> 28);
		r2 = convert4To8(high >> 24);
		g1 = convert4To8(high >> 20);
		g2 = convert4To8(high >> 16);
		b1 = convert4To8(high >> 12);
		b2 = convert4To8(high >> 8);
	}
	int tableIndexA = 7 & (high >> 5);
	int tableIndexB = 7 & (high >> 2);
	const int* tableA = kModifierTable + tableIndexA * 4;
	const int* tableB = kModifierTable + tableIndexB * 4;
	etc1_bool flipped = (high & 1) != 0;
	decode_subblock(pOut, r1, g1, b1, tableA, low, 0, flipped);
	decode_subblock(pOut, r2, g2, b2, tableB, low, 1, flipped);
}

typedef struct {
	etc1_uint32 high;
	etc1_uint32 low;
	etc1_uint32 score; // Lower is more accurate
} etc_compressed;

static
inline void take_best(etc_compressed* a, const etc_compressed* b) {
	if (a->score > b->score) {
		*a = *b;
	}
}

static
void etc_average_colors_subblock(const etc1_byte* pIn, etc1_uint32 inMask,
		etc1_byte* pColors, etc1_bool flipped, etc1_bool second) {
	int r = 0;
	int g = 0;
	int b = 0;
	int y, x;

	if (flipped) {
		int by = 0;
		if (second) {
			by = 2;
		}
		for ( y = 0; y < 2; y++) {
			int yy = by + y;
			for ( x = 0; x < 4; x++) {
				int i = x + 4 * yy;
				if (inMask & (1 << i)) {
					const etc1_byte* p = pIn + i * 3;
					r += *(p++);
					g += *(p++);
					b += *(p++);
				}
			}
		}
	} else {
		int bx = 0;
		if (second) {
			bx = 2;
		}
		for ( y = 0; y < 4; y++) {
			for ( x = 0; x < 2; x++) {
				int xx = bx + x;
				int i = xx + 4 * y;
				if (inMask & (1 << i)) {
					const etc1_byte* p = pIn + i * 3;
					r += *(p++);
					g += *(p++);
					b += *(p++);
				}
			}
		}
	}
	pColors[0] = (etc1_byte)((r + 4) >> 3);
	pColors[1] = (etc1_byte)((g + 4) >> 3);
	pColors[2] = (etc1_byte)((b + 4) >> 3);
}

static
inline int square(int x) {
	return x * x;
}

static etc1_uint32 chooseModifier(const etc1_byte* pBaseColors,
		const etc1_byte* pIn, etc1_uint32 *pLow, int bitIndex,
		const int* pModifierTable) {
	etc1_uint32 bestScore = ~0;
	int bestIndex = 0;
	int pixelR = pIn[0];
	int pixelG = pIn[1];
	int pixelB = pIn[2];
	int r = pBaseColors[0];
	int g = pBaseColors[1];
	int b = pBaseColors[2];
	int i;
	for ( i = 0; i < 4; i++) {
		int modifier = pModifierTable[i];
		int decodedG = clamp(g + modifier);
		etc1_uint32 score = (etc1_uint32) (6 * square(decodedG - pixelG));
		if (score >= bestScore) {
			continue;
		}
		int decodedR = clamp(r + modifier);
		score += (etc1_uint32) (3 * square(decodedR - pixelR));
		if (score >= bestScore) {
			continue;
		}
		int decodedB = clamp(b + modifier);
		score += (etc1_uint32) square(decodedB - pixelB);
		if (score < bestScore) {
			bestScore = score;
			bestIndex = i;
		}
	}
	etc1_uint32 lowMask = (((bestIndex >> 1) << 16) | (bestIndex & 1))
			<< bitIndex;
	*pLow |= lowMask;
	return bestScore;
}

static
void etc_encode_subblock_helper(const etc1_byte* pIn, etc1_uint32 inMask,
		etc_compressed* pCompressed, etc1_bool flipped, etc1_bool second,
		const etc1_byte* pBaseColors, const int* pModifierTable) {
	int score = pCompressed->score;
	int y, x;
	if (flipped) {
		int by = 0;
		if (second) {
			by = 2;
		}
		for ( y = 0; y < 2; y++) {
			int yy = by + y;
			for ( x = 0; x < 4; x++) {
				int i = x + 4 * yy;
				if (inMask & (1 << i)) {
					score += chooseModifier(pBaseColors, pIn + i * 3,
							&pCompressed->low, yy + x * 4, pModifierTable);
				}
			}
		}
	} else {
		int bx = 0;
		if (second) {
			bx = 2;
		}
		for ( y = 0; y < 4; y++) {
			for ( x = 0; x < 2; x++) {
				int xx = bx + x;
				int i = xx + 4 * y;
				if (inMask & (1 << i)) {
					score += chooseModifier(pBaseColors, pIn + i * 3,
							&pCompressed->low, y + xx * 4, pModifierTable);
				}
			}
		}
	}
	pCompressed->score = score;
}

static etc1_bool inRange4bitSigned(int color) {
	return color >= -4 && color <= 3;
}

static void etc_encodeBaseColors(etc1_byte* pBaseColors,
		const etc1_byte* pColors, etc_compressed* pCompressed) {
	int r1, g1, b1, r2 = 0, g2 = 0, b2 = 0; // 8 bit base colors for sub-blocks
	etc1_bool differential;
	{
		int r51 = convert8To5(pColors[0]);
		int g51 = convert8To5(pColors[1]);
		int b51 = convert8To5(pColors[2]);
		int r52 = convert8To5(pColors[3]);
		int g52 = convert8To5(pColors[4]);
		int b52 = convert8To5(pColors[5]);

		r1 = convert5To8(r51);
		g1 = convert5To8(g51);
		b1 = convert5To8(b51);

		int dr = r52 - r51;
		int dg = g52 - g51;
		int db = b52 - b51;

		differential = inRange4bitSigned(dr) && inRange4bitSigned(dg)
				&& inRange4bitSigned(db);
		if (differential) {
			r2 = convert5To8(r51 + dr);
			g2 = convert5To8(g51 + dg);
			b2 = convert5To8(b51 + db);
			pCompressed->high |= (r51 << 27) | ((7 & dr) << 24) | (g51 << 19)
					| ((7 & dg) << 16) | (b51 << 11) | ((7 & db) << 8) | 2;
		}
	}

	if (!differential) {
		int r41 = convert8To4(pColors[0]);
		int g41 = convert8To4(pColors[1]);
		int b41 = convert8To4(pColors[2]);
		int r42 = convert8To4(pColors[3]);
		int g42 = convert8To4(pColors[4]);
		int b42 = convert8To4(pColors[5]);
		r1 = convert4To8(r41);
		g1 = convert4To8(g41);
		b1 = convert4To8(b41);
		r2 = convert4To8(r42);
		g2 = convert4To8(g42);
		b2 = convert4To8(b42);
		pCompressed->high |= (r41 << 28) | (r42 << 24) | (g41 << 20) | (g42
				<< 16) | (b41 << 12) | (b42 << 8);
	}
	pBaseColors[0] = r1;
	pBaseColors[1] = g1;
	pBaseColors[2] = b1;
	pBaseColors[3] = r2;
	pBaseColors[4] = g2;
	pBaseColors[5] = b2;
}

static
void etc_encode_block_helper(const etc1_byte* pIn, etc1_uint32 inMask,
		const etc1_byte* pColors, etc_compressed* pCompressed, etc1_bool flipped) {
	int i;

	pCompressed->score = ~0;
	pCompressed->high = (flipped ? 1 : 0);
	pCompressed->low = 0;

	etc1_byte pBaseColors[6];

	etc_encodeBaseColors(pBaseColors, pColors, pCompressed);

	int originalHigh = pCompressed->high;

	const int* pModifierTable = kModifierTable;
	for ( i = 0; i < 8; i++, pModifierTable += 4) {
		etc_compressed temp;
		temp.score = 0;
		temp.high = originalHigh | (i << 5);
		temp.low = 0;
		etc_encode_subblock_helper(pIn, inMask, &temp, flipped, 0,
				pBaseColors, pModifierTable);
		take_best(pCompressed, &temp);
	}
	pModifierTable = kModifierTable;
	etc_compressed firstHalf = *pCompressed;
	for ( i = 0; i < 8; i++, pModifierTable += 4) {
		etc_compressed temp;
		temp.score = firstHalf.score;
		temp.high = firstHalf.high | (i << 2);
		temp.low = firstHalf.low;
		etc_encode_subblock_helper(pIn, inMask, &temp, flipped, 1,
				pBaseColors + 3, pModifierTable);
		if (i == 0) {
			*pCompressed = temp;
		} else {
			take_best(pCompressed, &temp);
		}
	}
}

static void writeBigEndian(etc1_byte* pOut, etc1_uint32 d) {
	pOut[0] = (etc1_byte)(d >> 24);
	pOut[1] = (etc1_byte)(d >> 16);
	pOut[2] = (etc1_byte)(d >> 8);
	pOut[3] = (etc1_byte) d;
}

// Input is a 4 x 4 square of 3-byte pixels in form R, G, B
// inmask is a 16-bit mask where bit (1 << (x + y * 4)) tells whether the corresponding (x,y)
// pixel is valid or not. Invalid pixel color values are ignored when compressing.
// Output is an ETC1 compressed version of the data.

void etc1_encode_block(const etc1_byte* pIn, etc1_uint32 inMask,
		etc1_byte* pOut) {
	etc1_byte colors[6];
	etc1_byte flippedColors[6];
	etc_average_colors_subblock(pIn, inMask, colors, 0, 0);
	etc_average_colors_subblock(pIn, inMask, colors + 3, 0, 1);
	etc_average_colors_subblock(pIn, inMask, flippedColors, 1, 0);
	etc_average_colors_subblock(pIn, inMask, flippedColors + 3, 1, 1);

	etc_compressed a, b;
	etc_encode_block_helper(pIn, inMask, colors, &a, 0);
	etc_encode_block_helper(pIn, inMask, flippedColors, &b, 1);
	take_best(&a, &b);
	writeBigEndian(pOut, a.high);
	writeBigEndian(pOut + 4, a.low);
}

// Return the size of the encoded image data (does not include size of PKM header).

etc1_uint32 etc1_get_encoded_data_size(etc1_uint32 width, etc1_uint32 height) {
	return (((width + 3) & ~3) * ((height + 3) & ~3)) >> 1;
}

// Encode an entire image.
// pIn - pointer to the image data. Formatted such that the Red component of
//       pixel (x,y) is at pIn + pixelSize * x + stride * y + redOffset;
// pOut - pointer to encoded data. Must be large enough to store entire encoded image.

int etc1_encode_image(const etc1_byte* pIn, etc1_uint32 width, etc1_uint32 height,
		etc1_uint32 pixelSize, etc1_uint32 stride, etc1_byte* pOut) {
	if (pixelSize < 2 || pixelSize > 3) {
		return -1;
	}
	static const unsigned short kYMask[] = { 0x0, 0xf, 0xff, 0xfff, 0xffff };
	static const unsigned short kXMask[] = { 0x0, 0x1111, 0x3333, 0x7777,
			0xffff };
	etc1_byte block[ETC1_DECODED_BLOCK_SIZE];
	etc1_byte encoded[ETC1_ENCODED_BLOCK_SIZE];
	etc1_uint32 y, x, cy, cx;

	etc1_uint32 encodedWidth = (width + 3) & ~3;
	etc1_uint32 encodedHeight = (height + 3) & ~3;

	for ( y = 0; y < encodedHeight; y += 4) {
		etc1_uint32 yEnd = height - y;
		if (yEnd > 4) {
			yEnd = 4;
		}
		int ymask = kYMask[yEnd];
		for ( x = 0; x < encodedWidth; x += 4) {
			etc1_uint32 xEnd = width - x;
			if (xEnd > 4) {
				xEnd = 4;
			}
			int mask = ymask & kXMask[xEnd];
			for ( cy = 0; cy < yEnd; cy++) {
				etc1_byte* q = block + (cy * 4) * 3;
				const etc1_byte* p = pIn + pixelSize * x + stride * (y + cy);
				if (pixelSize == 3) {
					memcpy(q, p, xEnd * 3);
				} else {
					for ( cx = 0; cx < xEnd; cx++) {
						int pixel = (p[1] << 8) | p[0];
						*q++ = convert5To8(pixel >> 11);
						*q++ = convert6To8(pixel >> 5);
						*q++ = convert5To8(pixel);
						p += pixelSize;
					}
				}
			}
			etc1_encode_block(block, mask, encoded);
			memcpy(pOut, encoded, sizeof(encoded));
			pOut += sizeof(encoded);
		}
	}
	return 0;
}

// Decode an entire image.
// pIn - pointer to encoded data.
// pOut - pointer to the image data. Will be written such that the Red component of
//       pixel (x,y) is at pIn + pixelSize * x + stride * y + redOffset. Must be
//        large enough to store entire image.

int etc1_decode_image(const etc1_byte* pIn, etc1_byte* pOut,
		etc1_uint32 width, etc1_uint32 height,
		etc1_uint32 pixelSize, etc1_uint32 stride) {
	if (pixelSize < 2 || pixelSize > 3) {
		return -1;
	}
	etc1_byte block[ETC1_DECODED_BLOCK_SIZE];

	etc1_uint32 encodedWidth = (width + 3) & ~3;
	etc1_uint32 encodedHeight = (height + 3) & ~3;

	etc1_uint32 y, x, cy, cx;

	for ( y = 0; y < encodedHeight; y += 4) {
		etc1_uint32 yEnd = height - y;
		if (yEnd > 4) {
			yEnd = 4;
		}
		for ( x = 0; x < encodedWidth; x += 4) {
			etc1_uint32 xEnd = width - x;
			if (xEnd > 4) {
				xEnd = 4;
			}
			etc1_decode_block(pIn, block);
			pIn += ETC1_ENCODED_BLOCK_SIZE;
			for ( cy = 0; cy < yEnd; cy++) {
				const etc1_byte* q = block + (cy * 4) * 3;
				etc1_byte* p = pOut + pixelSize * x + stride * (y + cy);
				if (pixelSize == 3) {
					memcpy(p, q, xEnd * 3);
				} else {
					for ( cx = 0; cx < xEnd; cx++) {
						etc1_byte r = *q++;
						etc1_byte g = *q++;
						etc1_byte b = *q++;
						etc1_uint32 pixel = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
						*p++ = (etc1_byte) pixel;
						*p++ = (etc1_byte) (pixel >> 8);
					}
				}
			}
		}
	}
	return 0;
}

static const char kMagic[] = { 'P', 'K', 'M', ' ', '1', '0' };

static const etc1_uint32 ETC1_PKM_FORMAT_OFFSET = 6;
static const etc1_uint32 ETC1_PKM_ENCODED_WIDTH_OFFSET = 8;
static const etc1_uint32 ETC1_PKM_ENCODED_HEIGHT_OFFSET = 10;
static const etc1_uint32 ETC1_PKM_WIDTH_OFFSET = 12;
static const etc1_uint32 ETC1_PKM_HEIGHT_OFFSET = 14;

static const etc1_uint32 ETC1_RGB_NO_MIPMAPS = 0;

static void writeBEUint16(etc1_byte* pOut, etc1_uint32 data) {
	pOut[0] = (etc1_byte) (data >> 8);
	pOut[1] = (etc1_byte) data;
}

static etc1_uint32 readBEUint16(const etc1_byte* pIn) {
	return (pIn[0] << 8) | pIn[1];
}

// Format a PKM header

void etc1_pkm_format_header(etc1_byte* pHeader, etc1_uint32 width, etc1_uint32 height) {
	memcpy(pHeader, kMagic, sizeof(kMagic));
	etc1_uint32 encodedWidth = (width + 3) & ~3;
	etc1_uint32 encodedHeight = (height + 3) & ~3;
	writeBEUint16(pHeader + ETC1_PKM_FORMAT_OFFSET, ETC1_RGB_NO_MIPMAPS);
	writeBEUint16(pHeader + ETC1_PKM_ENCODED_WIDTH_OFFSET, encodedWidth);
	writeBEUint16(pHeader + ETC1_PKM_ENCODED_HEIGHT_OFFSET, encodedHeight);
	writeBEUint16(pHeader + ETC1_PKM_WIDTH_OFFSET, width);
	writeBEUint16(pHeader + ETC1_PKM_HEIGHT_OFFSET, height);
}

// Check if a PKM header is correctly formatted.

etc1_bool etc1_pkm_is_valid(const etc1_byte* pHeader) {
	if (memcmp(pHeader, kMagic, sizeof(kMagic))) {
		return 0;
	}
	etc1_uint32 format = readBEUint16(pHeader + ETC1_PKM_FORMAT_OFFSET);
	etc1_uint32 encodedWidth = readBEUint16(pHeader + ETC1_PKM_ENCODED_WIDTH_OFFSET);
	etc1_uint32 encodedHeight = readBEUint16(pHeader + ETC1_PKM_ENCODED_HEIGHT_OFFSET);
	etc1_uint32 width = readBEUint16(pHeader + ETC1_PKM_WIDTH_OFFSET);
	etc1_uint32 height = readBEUint16(pHeader + ETC1_PKM_HEIGHT_OFFSET);
	return format == ETC1_RGB_NO_MIPMAPS &&
			encodedWidth >= width && encodedWidth - width < 4 &&
			encodedHeight >= height && encodedHeight - height < 4;
}

// Read the image width from a PKM header

etc1_uint32 etc1_pkm_get_width(const etc1_byte* pHeader) {
	return readBEUint16(pHeader + ETC1_PKM_WIDTH_OFFSET);
}

// Read the image height from a PKM header

etc1_uint32 etc1_pkm_get_height(const etc1_byte* pHeader){
	return readBEUint16(pHeader + ETC1_PKM_HEIGHT_OFFSET);
}



//#line 1 "image_helper.c"

//#line 1 "image_helper.h"
#ifndef HEADER_IMAGE_HELPER
#define HEADER_IMAGE_HELPER

#ifdef __cplusplus
extern "C" {
#endif

/**
	This function upscales an image.
	Not to be used to create MIPmaps,
	but to make it square,
	or to make it a power-of-two sized.
**/
int
	up_scale_image
	(
		const unsigned char* const orig,
		int width, int height, int channels,
		unsigned char* resampled,
		int resampled_width, int resampled_height
	);

/**
	This function downscales an image.
	Used for creating MIPmaps,
	the incoming image should be a
	power-of-two sized.
**/
int
	mipmap_image
	(
		const unsigned char* const orig,
		int width, int height, int channels,
		unsigned char* resampled,
		int block_size_x, int block_size_y
	);

/**
	This function takes the RGB components of the image
	and scales each channel from [0,255] to [16,235].
	This makes the colors "Safe" for display on NTSC
	displays.  Note that this is _NOT_ a good idea for
	loading images like normal- or height-maps!
**/
int
	scale_image_RGB_to_NTSC_safe
	(
		unsigned char* orig,
		int width, int height, int channels
	);

/**
	This function takes the RGB components of the image
	and converts them into YCoCg.  3 components will be
	re-ordered to CoYCg (for optimum DXT1 compression),
	while 4 components will be ordered CoCgAY (for DXT5
	compression).
**/
int
	convert_RGB_to_YCoCg
	(
		unsigned char* orig,
		int width, int height, int channels
	);

/**
	This function takes the YCoCg components of the image
	and converts them into RGB.  See above.
**/
int
	convert_YCoCg_to_RGB
	(
		unsigned char* orig,
		int width, int height, int channels
	);

/**
	Converts an HDR image from an array
	of unsigned chars (RGBE) to RGBdivA
	\return 0 if failed, otherwise returns 1
**/
int
	RGBE_to_RGBdivA
	(
		unsigned char *image,
		int width, int height,
		int rescale_to_max
	);

/**
	Converts an HDR image from an array
	of unsigned chars (RGBE) to RGBdivA2
	\return 0 if failed, otherwise returns 1
**/
int
	RGBE_to_RGBdivA2
	(
		unsigned char *image,
		int width, int height,
		int rescale_to_max
	);

#ifdef __cplusplus
}
#endif

#endif /* HEADER_IMAGE_HELPER	*/

#include <stdlib.h>
#include <math.h>

/*	Upscaling the image uses simple bilinear interpolation	*/
int
	up_scale_image
	(
		const unsigned char* const orig,
		int width, int height, int channels,
		unsigned char* resampled,
		int resampled_width, int resampled_height
	)
{
	float dx, dy;
	int x, y, c;

	/* error(s) check	*/
	if ( 	(width < 1) || (height < 1) ||
			(resampled_width < 2) || (resampled_height < 2) ||
			(channels < 1) ||
			(NULL == orig) || (NULL == resampled) )
	{
		/*	signify badness	*/
		return 0;
	}
	/*
		for each given pixel in the new map, find the exact location
		from the original map which would contribute to this guy
	*/
	dx = (width - 1.0f) / (resampled_width - 1.0f);
	dy = (height - 1.0f) / (resampled_height - 1.0f);
	for ( y = 0; y < resampled_height; ++y )
	{
		/* find the base y index and fractional offset from that	*/
		float sampley = y * dy;
		int inty = (int)sampley;
		/*	if( inty < 0 ) { inty = 0; } else	*/
		if( inty > height - 2 ) { inty = height - 2; }
		sampley -= inty;
		for ( x = 0; x < resampled_width; ++x )
		{
			float samplex = x * dx;
			int intx = (int)samplex;
			int base_index;
			/* find the base x index and fractional offset from that	*/
			/*	if( intx < 0 ) { intx = 0; } else	*/
			if( intx > width - 2 ) { intx = width - 2; }
			samplex -= intx;
			/*	base index into the original image	*/
			base_index = (inty * width + intx) * channels;
			for ( c = 0; c < channels; ++c )
			{
				/*	do the sampling	*/
				float value = 0.5f;
				value += orig[base_index]
							*(1.0f-samplex)*(1.0f-sampley);
				value += orig[base_index+channels]
							*(samplex)*(1.0f-sampley);
				value += orig[base_index+width*channels]
							*(1.0f-samplex)*(sampley);
				value += orig[base_index+width*channels+channels]
							*(samplex)*(sampley);
				/*	move to the next channel	*/
				++base_index;
				/*	save the new value	*/
				resampled[y*resampled_width*channels+x*channels+c] =
						(unsigned char)(value);
			}
		}
	}
	/*	done	*/
	return 1;
}

int
	mipmap_image
	(
		const unsigned char* const orig,
		int width, int height, int channels,
		unsigned char* resampled,
		int block_size_x, int block_size_y
	)
{
	int mip_width, mip_height;
	int i, j, c;

	/*	error check	*/
	if( (width < 1) || (height < 1) ||
		(channels < 1) || (orig == NULL) ||
		(resampled == NULL) ||
		(block_size_x < 1) || (block_size_y < 1) )
	{
		/*	nothing to do	*/
		return 0;
	}
	mip_width = width / block_size_x;
	mip_height = height / block_size_y;
	if( mip_width < 1 )
	{
		mip_width = 1;
	}
	if( mip_height < 1 )
	{
		mip_height = 1;
	}
	for( j = 0; j < mip_height; ++j )
	{
		for( i = 0; i < mip_width; ++i )
		{
			for( c = 0; c < channels; ++c )
			{
				const int index = (j*block_size_y)*width*channels + (i*block_size_x)*channels + c;
				int sum_value;
				int u,v;
				int u_block = block_size_x;
				int v_block = block_size_y;
				int block_area;
				/*	do a bit of checking so we don't over-run the boundaries
					(necessary for non-square textures!)	*/
				if( block_size_x * (i+1) > width )
				{
					u_block = width - i*block_size_y;
				}
				if( block_size_y * (j+1) > height )
				{
					v_block = height - j*block_size_y;
				}
				block_area = u_block*v_block;
				/*	for this pixel, see what the average
					of all the values in the block are.
					note: start the sum at the rounding value, not at 0	*/
				sum_value = block_area >> 1;
				for( v = 0; v < v_block; ++v )
				for( u = 0; u < u_block; ++u )
				{
					sum_value += orig[index + v*width*channels + u*channels];
				}
				resampled[j*mip_width*channels + i*channels + c] = sum_value / block_area;
			}
		}
	}
	return 1;
}

int
	scale_image_RGB_to_NTSC_safe
	(
		unsigned char* orig,
		int width, int height, int channels
	)
{
	const float scale_lo = 16.0f - 0.499f;
	const float scale_hi = 235.0f + 0.499f;
	int i, j;
	int nc = channels;
	unsigned char scale_LUT[256];
	/*	error check	*/
	if( (width < 1) || (height < 1) ||
		(channels < 1) || (orig == NULL) )
	{
		/*	nothing to do	*/
		return 0;
	}
	/*	set up the scaling Look Up Table	*/
	for( i = 0; i < 256; ++i )
	{
		scale_LUT[i] = (unsigned char)((scale_hi - scale_lo) * i / 255.0f + scale_lo);
	}
	/*	for channels = 2 or 4, ignore the alpha component	*/
	nc -= 1 - (channels & 1);
	/*	OK, go through the image and scale any non-alpha components	*/
	for( i = 0; i < width*height*channels; i += channels )
	{
		for( j = 0; j < nc; ++j )
		{
			orig[i+j] = scale_LUT[orig[i+j]];
		}
	}
	return 1;
}

unsigned char clamp_byte( int x ) { return ( (x) < 0 ? (0) : ( (x) > 255 ? 255 : (x) ) ); }

/*
	This function takes the RGB components of the image
	and converts them into YCoCg.  3 components will be
	re-ordered to CoYCg (for optimum DXT1 compression),
	while 4 components will be ordered CoCgAY (for DXT5
	compression).
*/
int
	convert_RGB_to_YCoCg
	(
		unsigned char* orig,
		int width, int height, int channels
	)
{
	int i;
	/*	error check	*/
	if( (width < 1) || (height < 1) ||
		(channels < 3) || (channels > 4) ||
		(orig == NULL) )
	{
		/*	nothing to do	*/
		return -1;
	}
	/*	do the conversion	*/
	if( channels == 3 )
	{
		for( i = 0; i < width*height*3; i += 3 )
		{
			int r = orig[i+0];
			int g = (orig[i+1] + 1) >> 1;
			int b = orig[i+2];
			int tmp = (2 + r + b) >> 2;
			/*	Co	*/
			orig[i+0] = clamp_byte( 128 + ((r - b + 1) >> 1) );
			/*	Y	*/
			orig[i+1] = clamp_byte( g + tmp );
			/*	Cg	*/
			orig[i+2] = clamp_byte( 128 + g - tmp );
		}
	} else
	{
		for( i = 0; i < width*height*4; i += 4 )
		{
			int r = orig[i+0];
			int g = (orig[i+1] + 1) >> 1;
			int b = orig[i+2];
			unsigned char a = orig[i+3];
			int tmp = (2 + r + b) >> 2;
			/*	Co	*/
			orig[i+0] = clamp_byte( 128 + ((r - b + 1) >> 1) );
			/*	Cg	*/
			orig[i+1] = clamp_byte( 128 + g - tmp );
			/*	Alpha	*/
			orig[i+2] = a;
			/*	Y	*/
			orig[i+3] = clamp_byte( g + tmp );
		}
	}
	/*	done	*/
	return 0;
}

/*
	This function takes the YCoCg components of the image
	and converts them into RGB.  See above.
*/
int
	convert_YCoCg_to_RGB
	(
		unsigned char* orig,
		int width, int height, int channels
	)
{
	int i;
	/*	error check	*/
	if( (width < 1) || (height < 1) ||
		(channels < 3) || (channels > 4) ||
		(orig == NULL) )
	{
		/*	nothing to do	*/
		return -1;
	}
	/*	do the conversion	*/
	if( channels == 3 )
	{
		for( i = 0; i < width*height*3; i += 3 )
		{
			int co = orig[i+0] - 128;
			int y  = orig[i+1];
			int cg = orig[i+2] - 128;
			/*	R	*/
			orig[i+0] = clamp_byte( y + co - cg );
			/*	G	*/
			orig[i+1] = clamp_byte( y + cg );
			/*	B	*/
			orig[i+2] = clamp_byte( y - co - cg );
		}
	} else
	{
		for( i = 0; i < width*height*4; i += 4 )
		{
			int co = orig[i+0] - 128;
			int cg = orig[i+1] - 128;
			unsigned char a  = orig[i+2];
			int y  = orig[i+3];
			/*	R	*/
			orig[i+0] = clamp_byte( y + co - cg );
			/*	G	*/
			orig[i+1] = clamp_byte( y + cg );
			/*	B	*/
			orig[i+2] = clamp_byte( y - co - cg );
			/*	A	*/
			orig[i+3] = a;
		}
	}
	/*	done	*/
	return 0;
}

float
find_max_RGBE
(
	unsigned char *image,
	int width, int height
)
{
	float max_val = 0.0f;
	unsigned char *img = image;
	int i, j;
	for( i = width * height; i > 0; --i )
	{
		/* float scale = powf( 2.0f, img[3] - 128.0f ) / 255.0f; */
		float scale = (float)ldexp( 1.0f / 255.0f, (int)(img[3]) - 128 );
		for( j = 0; j < 3; ++j )
		{
			if( img[j] * scale > max_val )
			{
				max_val = img[j] * scale;
			}
		}
		/* next pixel */
		img += 4;
	}
	return max_val;
}

int
RGBE_to_RGBdivA
(
	unsigned char *image,
	int width, int height,
	int rescale_to_max
)
{
	/* local variables */
	int i, iv;
	unsigned char *img = image;
	float scale = 1.0f;
	/* error check */
	if( (!image) || (width < 1) || (height < 1) )
	{
		return 0;
	}
	/* convert (note: no negative numbers, but 0.0 is possible) */
	if( rescale_to_max )
	{
		scale = 255.0f / find_max_RGBE( image, width, height );
	}
	for( i = width * height; i > 0; --i )
	{
		/* decode this pixel, and find the max */
		float r,g,b,e, m;
		/* e = scale * powf( 2.0f, img[3] - 128.0f ) / 255.0f; */
		e = scale * (float)ldexp( 1.0f / 255.0f, (int)(img[3]) - 128 );
		r = e * img[0];
		g = e * img[1];
		b = e * img[2];
		m = (r > g) ? r : g;
		m = (b > m) ? b : m;
		/* and encode it into RGBdivA */
		iv = (m != 0.0f) ? (int)(255.0f / m) : 1;
		iv = (iv < 1) ? 1 : iv;
		img[3] = (iv > 255) ? 255 : iv;
		iv = (int)(img[3] * r + 0.5f);
		img[0] = (iv > 255) ? 255 : iv;
		iv = (int)(img[3] * g + 0.5f);
		img[1] = (iv > 255) ? 255 : iv;
		iv = (int)(img[3] * b + 0.5f);
		img[2] = (iv > 255) ? 255 : iv;
		/* and on to the next pixel */
		img += 4;
	}
	return 1;
}

int
RGBE_to_RGBdivA2
(
	unsigned char *image,
	int width, int height,
	int rescale_to_max
)
{
	/* local variables */
	int i, iv;
	unsigned char *img = image;
	float scale = 1.0f;
	/* error check */
	if( (!image) || (width < 1) || (height < 1) )
	{
		return 0;
	}
	/* convert (note: no negative numbers, but 0.0 is possible) */
	if( rescale_to_max )
	{
		scale = 255.0f * 255.0f / find_max_RGBE( image, width, height );
	}
	for( i = width * height; i > 0; --i )
	{
		/* decode this pixel, and find the max */
		float r,g,b,e, m;
		/* e = scale * powf( 2.0f, img[3] - 128.0f ) / 255.0f; */
		e = scale * (float)ldexp( 1.0f / 255.0f, (int)(img[3]) - 128 );
		r = e * img[0];
		g = e * img[1];
		b = e * img[2];
		m = (r > g) ? r : g;
		m = (b > m) ? b : m;
		/* and encode it into RGBdivA */
		iv = (m != 0.0f) ? (int)sqrtf( 255.0f * 255.0f / m ) : 1;
		iv = (iv < 1) ? 1 : iv;
		img[3] = (iv > 255) ? 255 : iv;
		iv = (int)(img[3] * img[3] * r / 255.0f + 0.5f);
		img[0] = (iv > 255) ? 255 : iv;
		iv = (int)(img[3] * img[3] * g / 255.0f + 0.5f);
		img[1] = (iv > 255) ? 255 : iv;
		iv = (int)(img[3] * img[3] * b / 255.0f + 0.5f);
		img[2] = (iv > 255) ? 255 : iv;
		/* and on to the next pixel */
		img += 4;
	}
	return 1;
}

#undef  clamp


//#line 1 "jpge.h"
// jpge.h - C++ class for JPEG compression.
// Public domain, Rich Geldreich <richgel99@gmail.com>
// Alex Evans: Added RGBA support, linear memory allocator.
#ifndef JPEG_ENCODER_H
#define JPEG_ENCODER_H

namespace jpge
{
  typedef unsigned char  uint8;
  typedef signed short   int16;
  typedef signed int     int32;
  typedef unsigned short uint16;
  typedef unsigned int   uint32;
  typedef unsigned int   uint;

  // JPEG chroma subsampling factors. Y_ONLY (grayscale images) and H2V2 (color images) are the most common.
  enum subsampling_t { Y_ONLY = 0, H1V1 = 1, H2V1 = 2, H2V2 = 3 };

  // JPEG compression parameters structure.
  struct params
  {
	inline params() : m_quality(85), m_subsampling(H2V2), m_no_chroma_discrim_flag(false), m_two_pass_flag(false) { }

	inline bool check() const
	{
	  if ((m_quality < 1) || (m_quality > 100)) return false;
	  if ((uint)m_subsampling > (uint)H2V2) return false;
	  return true;
	}

	// Quality: 1-100, higher is better. Typical values are around 50-95.
	int m_quality;

	// m_subsampling:
	// 0 = Y (grayscale) only
	// 1 = YCbCr, no subsampling (H1V1, YCbCr 1x1x1, 3 blocks per MCU)
	// 2 = YCbCr, H2V1 subsampling (YCbCr 2x1x1, 4 blocks per MCU)
	// 3 = YCbCr, H2V2 subsampling (YCbCr 4x1x1, 6 blocks per MCU-- very common)
	subsampling_t m_subsampling;

	// Disables CbCr discrimination - only intended for testing.
	// If true, the Y quantization table is also used for the CbCr channels.
	bool m_no_chroma_discrim_flag;

	bool m_two_pass_flag;
  };

  // Writes JPEG image to a file.
  // num_channels must be 1 (Y) or 3 (RGB), image pitch must be width*num_channels.
  bool compress_image_to_jpeg_file(const char *pFilename, int width, int height, int num_channels, const uint8 *pImage_data, const params &comp_params = params());

  // Writes JPEG image to memory buffer.
  // On entry, buf_size is the size of the output buffer pointed at by pBuf, which should be at least ~1024 bytes.
  // If return value is true, buf_size will be set to the size of the compressed data.
  bool compress_image_to_jpeg_file_in_memory(void *pBuf, int &buf_size, int width, int height, int num_channels, const uint8 *pImage_data, const params &comp_params = params());

  // Output stream abstract class - used by the jpeg_encoder class to write to the output stream.
  // put_buf() is generally called with len==JPGE_OUT_BUF_SIZE bytes, but for headers it'll be called with smaller amounts.
  class output_stream
  {
  public:
	virtual ~output_stream() { };
	virtual bool put_buf(const void* Pbuf, int len) = 0;
	template<class T> inline bool put_obj(const T& obj) { return put_buf(&obj, sizeof(T)); }
  };

  // Lower level jpeg_encoder class - useful if more control is needed than the above helper functions.
  class jpeg_encoder
  {
  public:
	jpeg_encoder();
	~jpeg_encoder();

	// Initializes the compressor.
	// pStream: The stream object to use for writing compressed data.
	// params - Compression parameters structure, defined above.
	// width, height  - Image dimensions.
	// channels - May be 1, or 3. 1 indicates grayscale, 3 indicates RGB source data.
	// Returns false on out of memory or if a stream write fails.
	bool init(output_stream *pStream, int width, int height, int src_channels, const params &comp_params = params());

	const params &get_params() const { return m_params; }

	// Deinitializes the compressor, freeing any allocated memory. May be called at any time.
	void deinit();

	uint get_total_passes() const { return m_params.m_two_pass_flag ? 2 : 1; }
	inline uint get_cur_pass() { return m_pass_num; }

	// Call this method with each source scanline.
	// width * src_channels bytes per scanline is expected (RGB or Y format).
	// You must call with NULL after all scanlines are processed to finish compression.
	// Returns false on out of memory or if a stream write fails.
	bool process_scanline(const void* pScanline);

  private:
	jpeg_encoder(const jpeg_encoder &);
	jpeg_encoder &operator =(const jpeg_encoder &);

	typedef int32 sample_array_t;

	output_stream *m_pStream;
	params m_params;
	uint8 m_num_components;
	uint8 m_comp_h_samp[3], m_comp_v_samp[3];
	int m_image_x, m_image_y, m_image_bpp, m_image_bpl;
	int m_image_x_mcu, m_image_y_mcu;
	int m_image_bpl_xlt, m_image_bpl_mcu;
	int m_mcus_per_row;
	int m_mcu_x, m_mcu_y;
	uint8 *m_mcu_lines[16];
	uint8 m_mcu_y_ofs;
	sample_array_t m_sample_array[64];
	int16 m_coefficient_array[64];
	int32 m_quantization_tables[2][64];
	uint m_huff_codes[4][256];
	uint8 m_huff_code_sizes[4][256];
	uint8 m_huff_bits[4][17];
	uint8 m_huff_val[4][256];
	uint32 m_huff_count[4][256];
	int m_last_dc_val[3];
	enum { JPGE_OUT_BUF_SIZE = 2048 };
	uint8 m_out_buf[JPGE_OUT_BUF_SIZE];
	uint8 *m_pOut_buf;
	uint m_out_buf_left;
	uint32 m_bit_buffer;
	uint m_bits_in;
	uint8 m_pass_num;
	bool m_all_stream_writes_succeeded;

	void optimize_huffman_table(int table_num, int table_len);
	void emit_byte(uint8 i);
	void emit_word(uint i);
	void emit_marker(int marker);
	void emit_jfif_app0();
	void emit_dqt();
	void emit_sof();
	void emit_dht(uint8 *bits, uint8 *val, int index, bool ac_flag);
	void emit_dhts();
	void emit_sos();
	void emit_markers();
	void compute_huffman_table(uint *codes, uint8 *code_sizes, uint8 *bits, uint8 *val);
	void compute_quant_table(int32 *dst, int16 *src);
	void adjust_quant_table(int32 *dst, int32 *src);
	void first_pass_init();
	bool second_pass_init();
	bool jpg_open(int p_x_res, int p_y_res, int src_channels);
	void load_block_8_8_grey(int x);
	void load_block_8_8(int x, int y, int c);
	void load_block_16_8(int x, int c);
	void load_block_16_8_8(int x, int c);
	void load_quantized_coefficients(int component_num);
	void flush_output_buffer();
	void put_bits(uint bits, uint len);
	void code_coefficients_pass_one(int component_num);
	void code_coefficients_pass_two(int component_num);
	void code_block(int component_num);
	void process_mcu_row();
	bool terminate_pass_one();
	bool terminate_pass_two();
	bool process_end_of_image();
	void load_mcu(const void* src);
	void clear();
	void init();
  };

} // namespace jpge

#endif // JPEG_ENCODER


//#line 1 "jpge.cpp"
// jpge.cpp - C++ class for JPEG compression.
// Public domain, Rich Geldreich <richgel99@gmail.com>
// v1.01, Dec. 18, 2010 - Initial release
// v1.02, Apr. 6, 2011 - Removed 2x2 ordered dither in H2V1 chroma subsampling method load_block_16_8_8(). (The rounding factor was 2, when it should have been 1. Either way, it wasn't helping.)
// v1.03, Apr. 16, 2011 - Added support for optimized Huffman code tables, optimized dynamic memory allocation down to only 1 alloc.
//                        Also from Alex Evans: Added RGBA support, linear memory allocator (no longer needed in v1.03).
// v1.04, May. 19, 2012: Forgot to set m_pFile ptr to NULL in cfile_stream::close(). Thanks to Owen Kaluza for reporting this bug.
//                       Code tweaks to fix VS2008 static code analysis warnings (all looked harmless).
//                       Code review revealed method load_block_16_8_8() (used for the non-default H2V1 sampling mode to downsample chroma) somehow didn't get the rounding factor fix from v1.02.

#include <stdlib.h>
#include <string.h>

#if defined(__MACH__)
#include <stdlib.h>
#else
#include <malloc.h>
#endif

#define JPGE_MAX(a,b) (((a)>(b))?(a):(b))
#define JPGE_MIN(a,b) (((a)<(b))?(a):(b))

namespace jpge {

static inline void *jpge_malloc(size_t nSize) { return malloc(nSize); }
static inline void jpge_free(void *p) { free(p); }

// Various JPEG enums and tables.
enum { M_SOF0 = 0xC0, M_DHT = 0xC4, M_SOI = 0xD8, M_EOI = 0xD9, M_SOS = 0xDA, M_DQT = 0xDB, M_APP0 = 0xE0 };
enum { DC_LUM_CODES = 12, AC_LUM_CODES = 256, DC_CHROMA_CODES = 12, AC_CHROMA_CODES = 256, MAX_HUFF_SYMBOLS = 257, MAX_HUFF_CODESIZE = 32 };

static uint8 s_zag[64] = { 0,1,8,16,9,2,3,10,17,24,32,25,18,11,4,5,12,19,26,33,40,48,41,34,27,20,13,6,7,14,21,28,35,42,49,56,57,50,43,36,29,22,15,23,30,37,44,51,58,59,52,45,38,31,39,46,53,60,61,54,47,55,62,63 };
static int16 s_std_lum_quant[64] = { 16,11,12,14,12,10,16,14,13,14,18,17,16,19,24,40,26,24,22,22,24,49,35,37,29,40,58,51,61,60,57,51,56,55,64,72,92,78,64,68,87,69,55,56,80,109,81,87,95,98,103,104,103,62,77,113,121,112,100,120,92,101,103,99 };
static int16 s_std_croma_quant[64] = { 17,18,18,24,21,24,47,26,26,47,99,66,56,66,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99 };
static uint8 s_dc_lum_bits[17] = { 0,0,1,5,1,1,1,1,1,1,0,0,0,0,0,0,0 };
static uint8 s_dc_lum_val[DC_LUM_CODES] = { 0,1,2,3,4,5,6,7,8,9,10,11 };
static uint8 s_ac_lum_bits[17] = { 0,0,2,1,3,3,2,4,3,5,5,4,4,0,0,1,0x7d };
static uint8 s_ac_lum_val[AC_LUM_CODES]  =
{
  0x01,0x02,0x03,0x00,0x04,0x11,0x05,0x12,0x21,0x31,0x41,0x06,0x13,0x51,0x61,0x07,0x22,0x71,0x14,0x32,0x81,0x91,0xa1,0x08,0x23,0x42,0xb1,0xc1,0x15,0x52,0xd1,0xf0,
  0x24,0x33,0x62,0x72,0x82,0x09,0x0a,0x16,0x17,0x18,0x19,0x1a,0x25,0x26,0x27,0x28,0x29,0x2a,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x43,0x44,0x45,0x46,0x47,0x48,0x49,
  0x4a,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x83,0x84,0x85,0x86,0x87,0x88,0x89,
  0x8a,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xc2,0xc3,0xc4,0xc5,
  0xc6,0xc7,0xc8,0xc9,0xca,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,
  0xf9,0xfa
};
static uint8 s_dc_chroma_bits[17] = { 0,0,3,1,1,1,1,1,1,1,1,1,0,0,0,0,0 };
static uint8 s_dc_chroma_val[DC_CHROMA_CODES]  = { 0,1,2,3,4,5,6,7,8,9,10,11 };
static uint8 s_ac_chroma_bits[17] = { 0,0,2,1,2,4,4,3,4,7,5,4,4,0,1,2,0x77 };
static uint8 s_ac_chroma_val[AC_CHROMA_CODES] =
{
  0x00,0x01,0x02,0x03,0x11,0x04,0x05,0x21,0x31,0x06,0x12,0x41,0x51,0x07,0x61,0x71,0x13,0x22,0x32,0x81,0x08,0x14,0x42,0x91,0xa1,0xb1,0xc1,0x09,0x23,0x33,0x52,0xf0,
  0x15,0x62,0x72,0xd1,0x0a,0x16,0x24,0x34,0xe1,0x25,0xf1,0x17,0x18,0x19,0x1a,0x26,0x27,0x28,0x29,0x2a,0x35,0x36,0x37,0x38,0x39,0x3a,0x43,0x44,0x45,0x46,0x47,0x48,
  0x49,0x4a,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x82,0x83,0x84,0x85,0x86,0x87,
  0x88,0x89,0x8a,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xc2,0xc3,
  0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,
  0xf9,0xfa
};

// Low-level helper functions.
template <class T> inline void clear_obj(T &obj) { memset(&obj, 0, sizeof(obj)); }

const int YR = 19595, YG = 38470, YB = 7471, CB_R = -11059, CB_G = -21709, CB_B = 32768, CR_R = 32768, CR_G = -27439, CR_B = -5329;
static inline uint8 clamp(int i) { if (static_cast<uint>(i) > 255U) { if (i < 0) i = 0; else if (i > 255) i = 255; } return static_cast<uint8>(i); }

static void RGB_to_YCC(uint8* pDst, const uint8 *pSrc, int num_pixels)
{
  for ( ; num_pixels; pDst += 3, pSrc += 3, num_pixels--)
  {
	const int r = pSrc[0], g = pSrc[1], b = pSrc[2];
	pDst[0] = static_cast<uint8>((r * YR + g * YG + b * YB + 32768) >> 16);
	pDst[1] = clamp(128 + ((r * CB_R + g * CB_G + b * CB_B + 32768) >> 16));
	pDst[2] = clamp(128 + ((r * CR_R + g * CR_G + b * CR_B + 32768) >> 16));
  }
}

static void RGB_to_Y(uint8* pDst, const uint8 *pSrc, int num_pixels)
{
  for ( ; num_pixels; pDst++, pSrc += 3, num_pixels--)
	pDst[0] = static_cast<uint8>((pSrc[0] * YR + pSrc[1] * YG + pSrc[2] * YB + 32768) >> 16);
}

static void RGBA_to_YCC(uint8* pDst, const uint8 *pSrc, int num_pixels)
{
  for ( ; num_pixels; pDst += 3, pSrc += 4, num_pixels--)
  {
	const int r = pSrc[0], g = pSrc[1], b = pSrc[2];
	pDst[0] = static_cast<uint8>((r * YR + g * YG + b * YB + 32768) >> 16);
	pDst[1] = clamp(128 + ((r * CB_R + g * CB_G + b * CB_B + 32768) >> 16));
	pDst[2] = clamp(128 + ((r * CR_R + g * CR_G + b * CR_B + 32768) >> 16));
  }
}

static void RGBA_to_Y(uint8* pDst, const uint8 *pSrc, int num_pixels)
{
  for ( ; num_pixels; pDst++, pSrc += 4, num_pixels--)
	pDst[0] = static_cast<uint8>((pSrc[0] * YR + pSrc[1] * YG + pSrc[2] * YB + 32768) >> 16);
}

static void Y_to_YCC(uint8* pDst, const uint8* pSrc, int num_pixels)
{
  for( ; num_pixels; pDst += 3, pSrc++, num_pixels--) { pDst[0] = pSrc[0]; pDst[1] = 128; pDst[2] = 128; }
}

// Forward DCT - DCT derived from jfdctint.
enum { CONST_BITS = 13, ROW_BITS = 2 };
#define DCT_DESCALE(x, n) (((x) + (((int32)1) << ((n) - 1))) >> (n))
#define DCT_MUL(var, c) (static_cast<int16>(var) * static_cast<int32>(c))
#define DCT1D(s0, s1, s2, s3, s4, s5, s6, s7) \
  int32 t0 = s0 + s7, t7 = s0 - s7, t1 = s1 + s6, t6 = s1 - s6, t2 = s2 + s5, t5 = s2 - s5, t3 = s3 + s4, t4 = s3 - s4; \
  int32 t10 = t0 + t3, t13 = t0 - t3, t11 = t1 + t2, t12 = t1 - t2; \
  int32 u1 = DCT_MUL(t12 + t13, 4433); \
  s2 = u1 + DCT_MUL(t13, 6270); \
  s6 = u1 + DCT_MUL(t12, -15137); \
  u1 = t4 + t7; \
  int32 u2 = t5 + t6, u3 = t4 + t6, u4 = t5 + t7; \
  int32 z5 = DCT_MUL(u3 + u4, 9633); \
  t4 = DCT_MUL(t4, 2446); t5 = DCT_MUL(t5, 16819); \
  t6 = DCT_MUL(t6, 25172); t7 = DCT_MUL(t7, 12299); \
  u1 = DCT_MUL(u1, -7373); u2 = DCT_MUL(u2, -20995); \
  u3 = DCT_MUL(u3, -16069); u4 = DCT_MUL(u4, -3196); \
  u3 += z5; u4 += z5; \
  s0 = t10 + t11; s1 = t7 + u1 + u4; s3 = t6 + u2 + u3; s4 = t10 - t11; s5 = t5 + u2 + u4; s7 = t4 + u1 + u3;

static void DCT2D(int32 *p)
{
  int32 c, *q = p;
  for (c = 7; c >= 0; c--, q += 8)
  {
	int32 s0 = q[0], s1 = q[1], s2 = q[2], s3 = q[3], s4 = q[4], s5 = q[5], s6 = q[6], s7 = q[7];
	DCT1D(s0, s1, s2, s3, s4, s5, s6, s7);
	q[0] = s0 << ROW_BITS; q[1] = DCT_DESCALE(s1, CONST_BITS-ROW_BITS); q[2] = DCT_DESCALE(s2, CONST_BITS-ROW_BITS); q[3] = DCT_DESCALE(s3, CONST_BITS-ROW_BITS);
	q[4] = s4 << ROW_BITS; q[5] = DCT_DESCALE(s5, CONST_BITS-ROW_BITS); q[6] = DCT_DESCALE(s6, CONST_BITS-ROW_BITS); q[7] = DCT_DESCALE(s7, CONST_BITS-ROW_BITS);
  }
  for (q = p, c = 7; c >= 0; c--, q++)
  {
	int32 s0 = q[0*8], s1 = q[1*8], s2 = q[2*8], s3 = q[3*8], s4 = q[4*8], s5 = q[5*8], s6 = q[6*8], s7 = q[7*8];
	DCT1D(s0, s1, s2, s3, s4, s5, s6, s7);
	q[0*8] = DCT_DESCALE(s0, ROW_BITS+3); q[1*8] = DCT_DESCALE(s1, CONST_BITS+ROW_BITS+3); q[2*8] = DCT_DESCALE(s2, CONST_BITS+ROW_BITS+3); q[3*8] = DCT_DESCALE(s3, CONST_BITS+ROW_BITS+3);
	q[4*8] = DCT_DESCALE(s4, ROW_BITS+3); q[5*8] = DCT_DESCALE(s5, CONST_BITS+ROW_BITS+3); q[6*8] = DCT_DESCALE(s6, CONST_BITS+ROW_BITS+3); q[7*8] = DCT_DESCALE(s7, CONST_BITS+ROW_BITS+3);
  }
}

struct sym_freq { uint m_key, m_sym_index; };

// Radix sorts sym_freq[] array by 32-bit key m_key. Returns ptr to sorted values.
static inline sym_freq* radix_sort_syms(uint num_syms, sym_freq* pSyms0, sym_freq* pSyms1)
{
  const uint cMaxPasses = 4;
  uint32 hist[256 * cMaxPasses]; clear_obj(hist);
  for (uint i = 0; i < num_syms; i++) { uint freq = pSyms0[i].m_key; hist[freq & 0xFF]++; hist[256 + ((freq >> 8) & 0xFF)]++; hist[256*2 + ((freq >> 16) & 0xFF)]++; hist[256*3 + ((freq >> 24) & 0xFF)]++; }
  sym_freq* pCur_syms = pSyms0, *pNew_syms = pSyms1;
  uint total_passes = cMaxPasses; while ((total_passes > 1) && (num_syms == hist[(total_passes - 1) * 256])) total_passes--;
  for (uint pass_shift = 0, pass = 0; pass < total_passes; pass++, pass_shift += 8)
  {
	const uint32* pHist = &hist[pass << 8];
	uint offsets[256], cur_ofs = 0;
	for (uint i = 0; i < 256; i++) { offsets[i] = cur_ofs; cur_ofs += pHist[i]; }
	for (uint i = 0; i < num_syms; i++)
	  pNew_syms[offsets[(pCur_syms[i].m_key >> pass_shift) & 0xFF]++] = pCur_syms[i];
	sym_freq* t = pCur_syms; pCur_syms = pNew_syms; pNew_syms = t;
  }
  return pCur_syms;
}

// calculate_minimum_redundancy() originally written by: Alistair Moffat, alistair@cs.mu.oz.au, Jyrki Katajainen, jyrki@diku.dk, November 1996.
static void calculate_minimum_redundancy(sym_freq *A, int n)
{
  int root, leaf, next, avbl, used, dpth;
  if (n==0) return; else if (n==1) { A[0].m_key = 1; return; }
  A[0].m_key += A[1].m_key; root = 0; leaf = 2;
  for (next=1; next < n-1; next++)
  {
	if (leaf>=n || A[root].m_key<A[leaf].m_key) { A[next].m_key = A[root].m_key; A[root++].m_key = next; } else A[next].m_key = A[leaf++].m_key;
	if (leaf>=n || (root<next && A[root].m_key<A[leaf].m_key)) { A[next].m_key += A[root].m_key; A[root++].m_key = next; } else A[next].m_key += A[leaf++].m_key;
  }
  A[n-2].m_key = 0;
  for (next=n-3; next>=0; next--) A[next].m_key = A[A[next].m_key].m_key+1;
  avbl = 1; used = dpth = 0; root = n-2; next = n-1;
  while (avbl>0)
  {
	while (root>=0 && (int)A[root].m_key==dpth) { used++; root--; }
	while (avbl>used) { A[next--].m_key = dpth; avbl--; }
	avbl = 2*used; dpth++; used = 0;
  }
}

// Limits canonical Huffman code table's max code size to max_code_size.
static void huffman_enforce_max_code_size(int *pNum_codes, int code_list_len, int max_code_size)
{
  if (code_list_len <= 1) return;

  for (int i = max_code_size + 1; i <= MAX_HUFF_CODESIZE; i++) pNum_codes[max_code_size] += pNum_codes[i];

  uint32 total = 0;
  for (int i = max_code_size; i > 0; i--)
	total += (((uint32)pNum_codes[i]) << (max_code_size - i));

  while (total != (1UL << max_code_size))
  {
	pNum_codes[max_code_size]--;
	for (int i = max_code_size - 1; i > 0; i--)
	{
	  if (pNum_codes[i]) { pNum_codes[i]--; pNum_codes[i + 1] += 2; break; }
	}
	total--;
  }
}

// Generates an optimized offman table.
void jpeg_encoder::optimize_huffman_table(int table_num, int table_len)
{
  sym_freq syms0[MAX_HUFF_SYMBOLS], syms1[MAX_HUFF_SYMBOLS];
  syms0[0].m_key = 1; syms0[0].m_sym_index = 0;  // dummy symbol, assures that no valid code contains all 1's
  int num_used_syms = 1;
  const uint32 *pSym_count = &m_huff_count[table_num][0];
  for (int i = 0; i < table_len; i++)
	if (pSym_count[i]) { syms0[num_used_syms].m_key = pSym_count[i]; syms0[num_used_syms++].m_sym_index = i + 1; }
  sym_freq* pSyms = radix_sort_syms(num_used_syms, syms0, syms1);
  calculate_minimum_redundancy(pSyms, num_used_syms);

  // Count the # of symbols of each code size.
  int num_codes[1 + MAX_HUFF_CODESIZE]; clear_obj(num_codes);
  for (int i = 0; i < num_used_syms; i++)
	num_codes[pSyms[i].m_key]++;

  const uint JPGE_CODE_SIZE_LIMIT = 16; // the maximum possible size of a JPEG Huffman code (valid range is [9,16] - 9 vs. 8 because of the dummy symbol)
  huffman_enforce_max_code_size(num_codes, num_used_syms, JPGE_CODE_SIZE_LIMIT);

  // Compute m_huff_bits array, which contains the # of symbols per code size.
  clear_obj(m_huff_bits[table_num]);
  for (int i = 1; i <= (int)JPGE_CODE_SIZE_LIMIT; i++)
	m_huff_bits[table_num][i] = static_cast<uint8>(num_codes[i]);

  // Remove the dummy symbol added above, which must be in largest bucket.
  for (int i = JPGE_CODE_SIZE_LIMIT; i >= 1; i--)
  {
	if (m_huff_bits[table_num][i]) { m_huff_bits[table_num][i]--; break; }
  }

  // Compute the m_huff_val array, which contains the symbol indices sorted by code size (smallest to largest).
  for (int i = num_used_syms - 1; i >= 1; i--)
	m_huff_val[table_num][num_used_syms - 1 - i] = static_cast<uint8>(pSyms[i].m_sym_index - 1);
}

// JPEG marker generation.
void jpeg_encoder::emit_byte(uint8 i)
{
  m_all_stream_writes_succeeded = m_all_stream_writes_succeeded && m_pStream->put_obj(i);
}

void jpeg_encoder::emit_word(uint i)
{
  emit_byte(uint8(i >> 8)); emit_byte(uint8(i & 0xFF));
}

void jpeg_encoder::emit_marker(int marker)
{
  emit_byte(uint8(0xFF)); emit_byte(uint8(marker));
}

// Emit JFIF marker
void jpeg_encoder::emit_jfif_app0()
{
  emit_marker(M_APP0);
  emit_word(2 + 4 + 1 + 2 + 1 + 2 + 2 + 1 + 1);
  emit_byte(0x4A); emit_byte(0x46); emit_byte(0x49); emit_byte(0x46); /* Identifier: ASCII "JFIF" */
  emit_byte(0);
  emit_byte(1);      /* Major version */
  emit_byte(1);      /* Minor version */
  emit_byte(0);      /* Density unit */
  emit_word(1);
  emit_word(1);
  emit_byte(0);      /* No thumbnail image */
  emit_byte(0);
}

// Emit quantization tables
void jpeg_encoder::emit_dqt()
{
  for (int i = 0; i < ((m_num_components == 3) ? 2 : 1); i++)
  {
	emit_marker(M_DQT);
	emit_word(64 + 1 + 2);
	emit_byte(static_cast<uint8>(i));
	for (int j = 0; j < 64; j++)
	  emit_byte(static_cast<uint8>(m_quantization_tables[i][j]));
  }
}

// Emit start of frame marker
void jpeg_encoder::emit_sof()
{
  emit_marker(M_SOF0);                           /* baseline */
  emit_word(3 * m_num_components + 2 + 5 + 1);
  emit_byte(8);                                  /* precision */
  emit_word(m_image_y);
  emit_word(m_image_x);
  emit_byte(m_num_components);
  for (int i = 0; i < m_num_components; i++)
  {
	emit_byte(static_cast<uint8>(i + 1));                                   /* component ID     */
	emit_byte((m_comp_h_samp[i] << 4) + m_comp_v_samp[i]);  /* h and v sampling */
	emit_byte(i > 0);                                   /* quant. table num */
  }
}

// Emit Huffman table.
void jpeg_encoder::emit_dht(uint8 *bits, uint8 *val, int index, bool ac_flag)
{
  emit_marker(M_DHT);

  int length = 0;
  for (int i = 1; i <= 16; i++)
	length += bits[i];

  emit_word(length + 2 + 1 + 16);
  emit_byte(static_cast<uint8>(index + (ac_flag << 4)));

  for (int i = 1; i <= 16; i++)
	emit_byte(bits[i]);

  for (int i = 0; i < length; i++)
	emit_byte(val[i]);
}

// Emit all Huffman tables.
void jpeg_encoder::emit_dhts()
{
  emit_dht(m_huff_bits[0+0], m_huff_val[0+0], 0, false);
  emit_dht(m_huff_bits[2+0], m_huff_val[2+0], 0, true);
  if (m_num_components == 3)
  {
	emit_dht(m_huff_bits[0+1], m_huff_val[0+1], 1, false);
	emit_dht(m_huff_bits[2+1], m_huff_val[2+1], 1, true);
  }
}

// emit start of scan
void jpeg_encoder::emit_sos()
{
  emit_marker(M_SOS);
  emit_word(2 * m_num_components + 2 + 1 + 3);
  emit_byte(m_num_components);
  for (int i = 0; i < m_num_components; i++)
  {
	emit_byte(static_cast<uint8>(i + 1));
	if (i == 0)
	  emit_byte((0 << 4) + 0);
	else
	  emit_byte((1 << 4) + 1);
  }
  emit_byte(0);     /* spectral selection */
  emit_byte(63);
  emit_byte(0);
}

// Emit all markers at beginning of image file.
void jpeg_encoder::emit_markers()
{
  emit_marker(M_SOI);
  emit_jfif_app0();
  emit_dqt();
  emit_sof();
  emit_dhts();
  emit_sos();
}

// Compute the actual canonical Huffman codes/code sizes given the JPEG huff bits and val arrays.
void jpeg_encoder::compute_huffman_table(uint *codes, uint8 *code_sizes, uint8 *bits, uint8 *val)
{
  int i, l, last_p, si;
  uint8 huff_size[257];
  uint huff_code[257];
  uint code;

  int p = 0;
  for (l = 1; l <= 16; l++)
	for (i = 1; i <= bits[l]; i++)
	  huff_size[p++] = (char)l;

  huff_size[p] = 0; last_p = p; // write sentinel

  code = 0; si = huff_size[0]; p = 0;

  while (huff_size[p])
  {
	while (huff_size[p] == si)
	  huff_code[p++] = code++;
	code <<= 1;
	si++;
  }

  memset(codes, 0, sizeof(codes[0])*256);
  memset(code_sizes, 0, sizeof(code_sizes[0])*256);
  for (p = 0; p < last_p; p++)
  {
	codes[val[p]]      = huff_code[p];
	code_sizes[val[p]] = huff_size[p];
  }
}

// Quantization table generation.
void jpeg_encoder::compute_quant_table(int32 *pDst, int16 *pSrc)
{
  int32 q;
  if (m_params.m_quality < 50)
	q = 5000 / m_params.m_quality;
  else
	q = 200 - m_params.m_quality * 2;
  for (int i = 0; i < 64; i++)
  {
	int32 j = *pSrc++; j = (j * q + 50L) / 100L;
	*pDst++ = JPGE_MIN(JPGE_MAX(j, 1), 255);
  }
}

// Higher-level methods.
void jpeg_encoder::first_pass_init()
{
  m_bit_buffer = 0; m_bits_in = 0;
  memset(m_last_dc_val, 0, 3 * sizeof(m_last_dc_val[0]));
  m_mcu_y_ofs = 0;
  m_pass_num = 1;
}

bool jpeg_encoder::second_pass_init()
{
  compute_huffman_table(&m_huff_codes[0+0][0], &m_huff_code_sizes[0+0][0], m_huff_bits[0+0], m_huff_val[0+0]);
  compute_huffman_table(&m_huff_codes[2+0][0], &m_huff_code_sizes[2+0][0], m_huff_bits[2+0], m_huff_val[2+0]);
  if (m_num_components > 1)
  {
	compute_huffman_table(&m_huff_codes[0+1][0], &m_huff_code_sizes[0+1][0], m_huff_bits[0+1], m_huff_val[0+1]);
	compute_huffman_table(&m_huff_codes[2+1][0], &m_huff_code_sizes[2+1][0], m_huff_bits[2+1], m_huff_val[2+1]);
  }
  first_pass_init();
  emit_markers();
  m_pass_num = 2;
  return true;
}

bool jpeg_encoder::jpg_open(int p_x_res, int p_y_res, int src_channels)
{
  m_num_components = 3;
  switch (m_params.m_subsampling)
  {
	case Y_ONLY:
	{
	  m_num_components = 1;
	  m_comp_h_samp[0] = 1; m_comp_v_samp[0] = 1;
	  m_mcu_x          = 8; m_mcu_y          = 8;
	  break;
	}
	case H1V1:
	{
	  m_comp_h_samp[0] = 1; m_comp_v_samp[0] = 1;
	  m_comp_h_samp[1] = 1; m_comp_v_samp[1] = 1;
	  m_comp_h_samp[2] = 1; m_comp_v_samp[2] = 1;
	  m_mcu_x          = 8; m_mcu_y          = 8;
	  break;
	}
	case H2V1:
	{
	  m_comp_h_samp[0] = 2; m_comp_v_samp[0] = 1;
	  m_comp_h_samp[1] = 1; m_comp_v_samp[1] = 1;
	  m_comp_h_samp[2] = 1; m_comp_v_samp[2] = 1;
	  m_mcu_x          = 16; m_mcu_y         = 8;
	  break;
	}
	case H2V2:
	{
	  m_comp_h_samp[0] = 2; m_comp_v_samp[0] = 2;
	  m_comp_h_samp[1] = 1; m_comp_v_samp[1] = 1;
	  m_comp_h_samp[2] = 1; m_comp_v_samp[2] = 1;
	  m_mcu_x          = 16; m_mcu_y         = 16;
	}
  }

  m_image_x        = p_x_res; m_image_y = p_y_res;
  m_image_bpp      = src_channels;
  m_image_bpl      = m_image_x * src_channels;
  m_image_x_mcu    = (m_image_x + m_mcu_x - 1) & (~(m_mcu_x - 1));
  m_image_y_mcu    = (m_image_y + m_mcu_y - 1) & (~(m_mcu_y - 1));
  m_image_bpl_xlt  = m_image_x * m_num_components;
  m_image_bpl_mcu  = m_image_x_mcu * m_num_components;
  m_mcus_per_row   = m_image_x_mcu / m_mcu_x;

  if ((m_mcu_lines[0] = static_cast<uint8*>(jpge_malloc(m_image_bpl_mcu * m_mcu_y))) == NULL) return false;
  for (int i = 1; i < m_mcu_y; i++)
	m_mcu_lines[i] = m_mcu_lines[i-1] + m_image_bpl_mcu;

  compute_quant_table(m_quantization_tables[0], s_std_lum_quant);
  compute_quant_table(m_quantization_tables[1], m_params.m_no_chroma_discrim_flag ? s_std_lum_quant : s_std_croma_quant);

  m_out_buf_left = JPGE_OUT_BUF_SIZE;
  m_pOut_buf = m_out_buf;

  if (m_params.m_two_pass_flag)
  {
	clear_obj(m_huff_count);
	first_pass_init();
  }
  else
  {
	memcpy(m_huff_bits[0+0], s_dc_lum_bits, 17);    memcpy(m_huff_val [0+0], s_dc_lum_val, DC_LUM_CODES);
	memcpy(m_huff_bits[2+0], s_ac_lum_bits, 17);    memcpy(m_huff_val [2+0], s_ac_lum_val, AC_LUM_CODES);
	memcpy(m_huff_bits[0+1], s_dc_chroma_bits, 17); memcpy(m_huff_val [0+1], s_dc_chroma_val, DC_CHROMA_CODES);
	memcpy(m_huff_bits[2+1], s_ac_chroma_bits, 17); memcpy(m_huff_val [2+1], s_ac_chroma_val, AC_CHROMA_CODES);
	if (!second_pass_init()) return false;   // in effect, skip over the first pass
  }
  return m_all_stream_writes_succeeded;
}

void jpeg_encoder::load_block_8_8_grey(int x)
{
  uint8 *pSrc;
  sample_array_t *pDst = m_sample_array;
  x <<= 3;
  for (int i = 0; i < 8; i++, pDst += 8)
  {
	pSrc = m_mcu_lines[i] + x;
	pDst[0] = pSrc[0] - 128; pDst[1] = pSrc[1] - 128; pDst[2] = pSrc[2] - 128; pDst[3] = pSrc[3] - 128;
	pDst[4] = pSrc[4] - 128; pDst[5] = pSrc[5] - 128; pDst[6] = pSrc[6] - 128; pDst[7] = pSrc[7] - 128;
  }
}

void jpeg_encoder::load_block_8_8(int x, int y, int c)
{
  uint8 *pSrc;
  sample_array_t *pDst = m_sample_array;
  x = (x * (8 * 3)) + c;
  y <<= 3;
  for (int i = 0; i < 8; i++, pDst += 8)
  {
	pSrc = m_mcu_lines[y + i] + x;
	pDst[0] = pSrc[0 * 3] - 128; pDst[1] = pSrc[1 * 3] - 128; pDst[2] = pSrc[2 * 3] - 128; pDst[3] = pSrc[3 * 3] - 128;
	pDst[4] = pSrc[4 * 3] - 128; pDst[5] = pSrc[5 * 3] - 128; pDst[6] = pSrc[6 * 3] - 128; pDst[7] = pSrc[7 * 3] - 128;
  }
}

void jpeg_encoder::load_block_16_8(int x, int c)
{
  uint8 *pSrc1, *pSrc2;
  sample_array_t *pDst = m_sample_array;
  x = (x * (16 * 3)) + c;
  int a = 0, b = 2;
  for (int i = 0; i < 16; i += 2, pDst += 8)
  {
	pSrc1 = m_mcu_lines[i + 0] + x;
	pSrc2 = m_mcu_lines[i + 1] + x;
	pDst[0] = ((pSrc1[ 0 * 3] + pSrc1[ 1 * 3] + pSrc2[ 0 * 3] + pSrc2[ 1 * 3] + a) >> 2) - 128; pDst[1] = ((pSrc1[ 2 * 3] + pSrc1[ 3 * 3] + pSrc2[ 2 * 3] + pSrc2[ 3 * 3] + b) >> 2) - 128;
	pDst[2] = ((pSrc1[ 4 * 3] + pSrc1[ 5 * 3] + pSrc2[ 4 * 3] + pSrc2[ 5 * 3] + a) >> 2) - 128; pDst[3] = ((pSrc1[ 6 * 3] + pSrc1[ 7 * 3] + pSrc2[ 6 * 3] + pSrc2[ 7 * 3] + b) >> 2) - 128;
	pDst[4] = ((pSrc1[ 8 * 3] + pSrc1[ 9 * 3] + pSrc2[ 8 * 3] + pSrc2[ 9 * 3] + a) >> 2) - 128; pDst[5] = ((pSrc1[10 * 3] + pSrc1[11 * 3] + pSrc2[10 * 3] + pSrc2[11 * 3] + b) >> 2) - 128;
	pDst[6] = ((pSrc1[12 * 3] + pSrc1[13 * 3] + pSrc2[12 * 3] + pSrc2[13 * 3] + a) >> 2) - 128; pDst[7] = ((pSrc1[14 * 3] + pSrc1[15 * 3] + pSrc2[14 * 3] + pSrc2[15 * 3] + b) >> 2) - 128;
	int temp = a; a = b; b = temp;
  }
}

void jpeg_encoder::load_block_16_8_8(int x, int c)
{
  uint8 *pSrc1;
  sample_array_t *pDst = m_sample_array;
  x = (x * (16 * 3)) + c;
  for (int i = 0; i < 8; i++, pDst += 8)
  {
	pSrc1 = m_mcu_lines[i + 0] + x;
	pDst[0] = ((pSrc1[ 0 * 3] + pSrc1[ 1 * 3]) >> 1) - 128; pDst[1] = ((pSrc1[ 2 * 3] + pSrc1[ 3 * 3]) >> 1) - 128;
	pDst[2] = ((pSrc1[ 4 * 3] + pSrc1[ 5 * 3]) >> 1) - 128; pDst[3] = ((pSrc1[ 6 * 3] + pSrc1[ 7 * 3]) >> 1) - 128;
	pDst[4] = ((pSrc1[ 8 * 3] + pSrc1[ 9 * 3]) >> 1) - 128; pDst[5] = ((pSrc1[10 * 3] + pSrc1[11 * 3]) >> 1) - 128;
	pDst[6] = ((pSrc1[12 * 3] + pSrc1[13 * 3]) >> 1) - 128; pDst[7] = ((pSrc1[14 * 3] + pSrc1[15 * 3]) >> 1) - 128;
  }
}

void jpeg_encoder::load_quantized_coefficients(int component_num)
{
  int32 *q = m_quantization_tables[component_num > 0];
  int16 *pDst = m_coefficient_array;
  for (int i = 0; i < 64; i++)
  {
	sample_array_t j = m_sample_array[s_zag[i]];
	if (j < 0)
	{
	  if ((j = -j + (*q >> 1)) < *q)
		*pDst++ = 0;
	  else
		*pDst++ = static_cast<int16>(-(j / *q));
	}
	else
	{
	  if ((j = j + (*q >> 1)) < *q)
		*pDst++ = 0;
	  else
		*pDst++ = static_cast<int16>((j / *q));
	}
	q++;
  }
}

void jpeg_encoder::flush_output_buffer()
{
  if (m_out_buf_left != JPGE_OUT_BUF_SIZE)
	m_all_stream_writes_succeeded = m_all_stream_writes_succeeded && m_pStream->put_buf(m_out_buf, JPGE_OUT_BUF_SIZE - m_out_buf_left);
  m_pOut_buf = m_out_buf;
  m_out_buf_left = JPGE_OUT_BUF_SIZE;
}

void jpeg_encoder::put_bits(uint bits, uint len)
{
  m_bit_buffer |= ((uint32)bits << (24 - (m_bits_in += len)));
  while (m_bits_in >= 8)
  {
	uint8 c;
	#define JPGE_PUT_BYTE(c) { *m_pOut_buf++ = (c); if (--m_out_buf_left == 0) flush_output_buffer(); }
	JPGE_PUT_BYTE(c = (uint8)((m_bit_buffer >> 16) & 0xFF));
	if (c == 0xFF) JPGE_PUT_BYTE(0);
	m_bit_buffer <<= 8;
	m_bits_in -= 8;
  }
}

void jpeg_encoder::code_coefficients_pass_one(int component_num)
{
  if (component_num >= 3) return; // just to shut up static analysis
  int i, run_len, nbits, temp1;
  int16 *src = m_coefficient_array;
  uint32 *dc_count = component_num ? m_huff_count[0 + 1] : m_huff_count[0 + 0], *ac_count = component_num ? m_huff_count[2 + 1] : m_huff_count[2 + 0];

  temp1 = src[0] - m_last_dc_val[component_num];
  m_last_dc_val[component_num] = src[0];
  if (temp1 < 0) temp1 = -temp1;

  nbits = 0;
  while (temp1)
  {
	nbits++; temp1 >>= 1;
  }

  dc_count[nbits]++;
  for (run_len = 0, i = 1; i < 64; i++)
  {
	if ((temp1 = m_coefficient_array[i]) == 0)
	  run_len++;
	else
	{
	  while (run_len >= 16)
	  {
		ac_count[0xF0]++;
		run_len -= 16;
	  }
	  if (temp1 < 0) temp1 = -temp1;
	  nbits = 1;
	  while (temp1 >>= 1) nbits++;
	  ac_count[(run_len << 4) + nbits]++;
	  run_len = 0;
	}
  }
  if (run_len) ac_count[0]++;
}

void jpeg_encoder::code_coefficients_pass_two(int component_num)
{
  int i, j, run_len, nbits, temp1, temp2;
  int16 *pSrc = m_coefficient_array;
  uint *codes[2];
  uint8 *code_sizes[2];

  if (component_num == 0)
  {
	codes[0] = m_huff_codes[0 + 0]; codes[1] = m_huff_codes[2 + 0];
	code_sizes[0] = m_huff_code_sizes[0 + 0]; code_sizes[1] = m_huff_code_sizes[2 + 0];
  }
  else
  {
	codes[0] = m_huff_codes[0 + 1]; codes[1] = m_huff_codes[2 + 1];
	code_sizes[0] = m_huff_code_sizes[0 + 1]; code_sizes[1] = m_huff_code_sizes[2 + 1];
  }

  temp1 = temp2 = pSrc[0] - m_last_dc_val[component_num];
  m_last_dc_val[component_num] = pSrc[0];

  if (temp1 < 0)
  {
	temp1 = -temp1; temp2--;
  }

  nbits = 0;
  while (temp1)
  {
	nbits++; temp1 >>= 1;
  }

  put_bits(codes[0][nbits], code_sizes[0][nbits]);
  if (nbits) put_bits(temp2 & ((1 << nbits) - 1), nbits);

  for (run_len = 0, i = 1; i < 64; i++)
  {
	if ((temp1 = m_coefficient_array[i]) == 0)
	  run_len++;
	else
	{
	  while (run_len >= 16)
	  {
		put_bits(codes[1][0xF0], code_sizes[1][0xF0]);
		run_len -= 16;
	  }
	  if ((temp2 = temp1) < 0)
	  {
		temp1 = -temp1;
		temp2--;
	  }
	  nbits = 1;
	  while (temp1 >>= 1)
		nbits++;
	  j = (run_len << 4) + nbits;
	  put_bits(codes[1][j], code_sizes[1][j]);
	  put_bits(temp2 & ((1 << nbits) - 1), nbits);
	  run_len = 0;
	}
  }
  if (run_len)
	put_bits(codes[1][0], code_sizes[1][0]);
}

void jpeg_encoder::code_block(int component_num)
{
  DCT2D(m_sample_array);
  load_quantized_coefficients(component_num);
  if (m_pass_num == 1)
	code_coefficients_pass_one(component_num);
  else
	code_coefficients_pass_two(component_num);
}

void jpeg_encoder::process_mcu_row()
{
  if (m_num_components == 1)
  {
	for (int i = 0; i < m_mcus_per_row; i++)
	{
	  load_block_8_8_grey(i); code_block(0);
	}
  }
  else if ((m_comp_h_samp[0] == 1) && (m_comp_v_samp[0] == 1))
  {
	for (int i = 0; i < m_mcus_per_row; i++)
	{
	  load_block_8_8(i, 0, 0); code_block(0); load_block_8_8(i, 0, 1); code_block(1); load_block_8_8(i, 0, 2); code_block(2);
	}
  }
  else if ((m_comp_h_samp[0] == 2) && (m_comp_v_samp[0] == 1))
  {
	for (int i = 0; i < m_mcus_per_row; i++)
	{
	  load_block_8_8(i * 2 + 0, 0, 0); code_block(0); load_block_8_8(i * 2 + 1, 0, 0); code_block(0);
	  load_block_16_8_8(i, 1); code_block(1); load_block_16_8_8(i, 2); code_block(2);
	}
  }
  else if ((m_comp_h_samp[0] == 2) && (m_comp_v_samp[0] == 2))
  {
	for (int i = 0; i < m_mcus_per_row; i++)
	{
	  load_block_8_8(i * 2 + 0, 0, 0); code_block(0); load_block_8_8(i * 2 + 1, 0, 0); code_block(0);
	  load_block_8_8(i * 2 + 0, 1, 0); code_block(0); load_block_8_8(i * 2 + 1, 1, 0); code_block(0);
	  load_block_16_8(i, 1); code_block(1); load_block_16_8(i, 2); code_block(2);
	}
  }
}

bool jpeg_encoder::terminate_pass_one()
{
  optimize_huffman_table(0+0, DC_LUM_CODES); optimize_huffman_table(2+0, AC_LUM_CODES);
  if (m_num_components > 1)
  {
	optimize_huffman_table(0+1, DC_CHROMA_CODES); optimize_huffman_table(2+1, AC_CHROMA_CODES);
  }
  return second_pass_init();
}

bool jpeg_encoder::terminate_pass_two()
{
  put_bits(0x7F, 7);
  flush_output_buffer();
  emit_marker(M_EOI);
  m_pass_num++; // purposely bump up m_pass_num, for debugging
  return true;
}

bool jpeg_encoder::process_end_of_image()
{
  if (m_mcu_y_ofs)
  {
	if (m_mcu_y_ofs < 16) // check here just to shut up static analysis
	{
	  for (int i = m_mcu_y_ofs; i < m_mcu_y; i++)
		memcpy(m_mcu_lines[i], m_mcu_lines[m_mcu_y_ofs - 1], m_image_bpl_mcu);
	}

	process_mcu_row();
  }

  if (m_pass_num == 1)
	return terminate_pass_one();
  else
	return terminate_pass_two();
}

void jpeg_encoder::load_mcu(const void *pSrc)
{
  const uint8* Psrc = reinterpret_cast<const uint8*>(pSrc);

  uint8* pDst = m_mcu_lines[m_mcu_y_ofs]; // OK to write up to m_image_bpl_xlt bytes to pDst

  if (m_num_components == 1)
  {
	if (m_image_bpp == 4)
	  RGBA_to_Y(pDst, Psrc, m_image_x);
	else if (m_image_bpp == 3)
	  RGB_to_Y(pDst, Psrc, m_image_x);
	else
	  memcpy(pDst, Psrc, m_image_x);
  }
  else
  {
	if (m_image_bpp == 4)
	  RGBA_to_YCC(pDst, Psrc, m_image_x);
	else if (m_image_bpp == 3)
	  RGB_to_YCC(pDst, Psrc, m_image_x);
	else
	  Y_to_YCC(pDst, Psrc, m_image_x);
  }

  // Possibly duplicate pixels at end of scanline if not a multiple of 8 or 16
  if (m_num_components == 1)
	memset(m_mcu_lines[m_mcu_y_ofs] + m_image_bpl_xlt, pDst[m_image_bpl_xlt - 1], m_image_x_mcu - m_image_x);
  else
  {
	const uint8 y = pDst[m_image_bpl_xlt - 3 + 0], cb = pDst[m_image_bpl_xlt - 3 + 1], cr = pDst[m_image_bpl_xlt - 3 + 2];
	uint8 *q = m_mcu_lines[m_mcu_y_ofs] + m_image_bpl_xlt;
	for (int i = m_image_x; i < m_image_x_mcu; i++)
	{
	  *q++ = y; *q++ = cb; *q++ = cr;
	}
  }

  if (++m_mcu_y_ofs == m_mcu_y)
  {
	process_mcu_row();
	m_mcu_y_ofs = 0;
  }
}

void jpeg_encoder::clear()
{
  m_mcu_lines[0] = NULL;
  m_pass_num = 0;
  m_all_stream_writes_succeeded = true;
}

jpeg_encoder::jpeg_encoder()
{
  clear();
}

jpeg_encoder::~jpeg_encoder()
{
  deinit();
}

bool jpeg_encoder::init(output_stream *pStream, int width, int height, int src_channels, const params &comp_params)
{
  deinit();
  if (((!pStream) || (width < 1) || (height < 1)) || ((src_channels != 1) && (src_channels != 3) && (src_channels != 4)) || (!comp_params.check())) return false;
  m_pStream = pStream;
  m_params = comp_params;
  return jpg_open(width, height, src_channels);
}

void jpeg_encoder::deinit()
{
  jpge_free(m_mcu_lines[0]);
  clear();
}

bool jpeg_encoder::process_scanline(const void* pScanline)
{
  if ((m_pass_num < 1) || (m_pass_num > 2)) return false;
  if (m_all_stream_writes_succeeded)
  {
	if (!pScanline)
	{
	  if (!process_end_of_image()) return false;
	}
	else
	{
	  load_mcu(pScanline);
	}
  }
  return m_all_stream_writes_succeeded;
}

// Higher level wrappers/examples (optional).
#include <stdio.h>

class cfile_stream : public output_stream
{
   cfile_stream(const cfile_stream &);
   cfile_stream &operator= (const cfile_stream &);

   FILE* m_pFile;
   bool m_bStatus;

public:
   cfile_stream() : m_pFile(NULL), m_bStatus(false) { }

   virtual ~cfile_stream()
   {
	  close();
   }

   bool open(const char *pFilename)
   {
	  close();
	  m_pFile = fopen(pFilename, "wb");
	  m_bStatus = (m_pFile != NULL);
	  return m_bStatus;
   }

   bool close()
   {
	  if (m_pFile)
	  {
		 if (fclose(m_pFile) == EOF)
		 {
			m_bStatus = false;
		 }
		 m_pFile = NULL;
	  }
	  return m_bStatus;
   }

   virtual bool put_buf(const void* pBuf, int len)
   {
	  m_bStatus = m_bStatus && (fwrite(pBuf, len, 1, m_pFile) == 1);
	  return m_bStatus;
   }

   uint get_size() const
   {
	  return m_pFile ? ftell(m_pFile) : 0;
   }
};

// Writes JPEG image to file.
bool compress_image_to_jpeg_file(const char *pFilename, int width, int height, int num_channels, const uint8 *pImage_data, const params &comp_params)
{
  cfile_stream dst_stream;
  if (!dst_stream.open(pFilename))
	return false;

  jpge::jpeg_encoder dst_image;
  if (!dst_image.init(&dst_stream, width, height, num_channels, comp_params))
	return false;

  for (uint pass_index = 0; pass_index < dst_image.get_total_passes(); pass_index++)
  {
	for (int i = 0; i < height; i++)
	{
	   const uint8* pBuf = pImage_data + i * width * num_channels;
	   if (!dst_image.process_scanline(pBuf))
		  return false;
	}
	if (!dst_image.process_scanline(NULL))
	   return false;
  }

  dst_image.deinit();

  return dst_stream.close();
}

class memory_stream : public output_stream
{
   memory_stream(const memory_stream &);
   memory_stream &operator= (const memory_stream &);

   uint8 *m_pBuf;
   uint m_buf_size, m_buf_ofs;

public:
   memory_stream(void *pBuf, uint buf_size) : m_pBuf(static_cast<uint8*>(pBuf)), m_buf_size(buf_size), m_buf_ofs(0) { }

   virtual ~memory_stream() { }

   virtual bool put_buf(const void* pBuf, int len)
   {
	  uint buf_remaining = m_buf_size - m_buf_ofs;
	  if ((uint)len > buf_remaining)
		 return false;
	  memcpy(m_pBuf + m_buf_ofs, pBuf, len);
	  m_buf_ofs += len;
	  return true;
   }

   uint get_size() const
   {
	  return m_buf_ofs;
   }
};

bool compress_image_to_jpeg_file_in_memory(void *pDstBuf, int &buf_size, int width, int height, int num_channels, const uint8 *pImage_data, const params &comp_params)
{
   if ((!pDstBuf) || (!buf_size))
	  return false;

   memory_stream dst_stream(pDstBuf, buf_size);

   buf_size = 0;

   jpge::jpeg_encoder dst_image;
   if (!dst_image.init(&dst_stream, width, height, num_channels, comp_params))
	  return false;

   for (uint pass_index = 0; pass_index < dst_image.get_total_passes(); pass_index++)
   {
	 for (int i = 0; i < height; i++)
	 {
		const uint8* pScanline = pImage_data + i * width * num_channels;
		if (!dst_image.process_scanline(pScanline))
		   return false;
	 }
	 if (!dst_image.process_scanline(NULL))
		return false;
   }

   dst_image.deinit();

   buf_size = dst_stream.get_size();
   return true;
}

} // namespace jpge


//#line 1 "lodepng.h"
#ifndef LODEPNG_H
#define LODEPNG_H

#include <string.h> /*for size_t*/

#ifdef __cplusplus
#include <vector>
#include <string>
#endif /*__cplusplus*/

#define LODEPNG_VERSION_STRING "20141130"

/*
The following #defines are used to create code sections. They can be disabled
to disable code sections, which can give faster compile time and smaller binary.
The "NO_COMPILE" defines are designed to be used to pass as defines to the
compiler command to disable them without modifying this header, e.g.
-DLODEPNG_NO_COMPILE_ZLIB for gcc.
*/
/*deflate & zlib. If disabled, you must specify alternative zlib functions in
the custom_zlib field of the compress and decompress settings*/
#ifndef LODEPNG_NO_COMPILE_ZLIB
#define LODEPNG_COMPILE_ZLIB
#endif
/*png encoder and png decoder*/
#ifndef LODEPNG_NO_COMPILE_PNG
#define LODEPNG_COMPILE_PNG
#endif
/*deflate&zlib decoder and png decoder*/
#ifndef LODEPNG_NO_COMPILE_DECODER
#define LODEPNG_COMPILE_DECODER
#endif
/*deflate&zlib encoder and png encoder*/
#ifndef LODEPNG_NO_COMPILE_ENCODER
#define LODEPNG_COMPILE_ENCODER
#endif
/*the optional built in harddisk file loading and saving functions*/
#ifndef LODEPNG_NO_COMPILE_DISK
#define LODEPNG_COMPILE_DISK
#endif
/*support for chunks other than IHDR, IDAT, PLTE, tRNS, IEND: ancillary and unknown chunks*/
#ifndef LODEPNG_NO_COMPILE_ANCILLARY_CHUNKS
#define LODEPNG_COMPILE_ANCILLARY_CHUNKS
#endif
/*ability to convert error numerical codes to English text string*/
#ifndef LODEPNG_NO_COMPILE_ERROR_TEXT
#define LODEPNG_COMPILE_ERROR_TEXT
#endif
/*Compile the default allocators (C's free, malloc and realloc). If you disable this,
you can define the functions lodepng_free, lodepng_malloc and lodepng_realloc in your
source files with custom allocators.*/
#ifndef LODEPNG_NO_COMPILE_ALLOCATORS
#define LODEPNG_COMPILE_ALLOCATORS
#endif
/*compile the C++ version (you can disable the C++ wrapper here even when compiling for C++)*/
#ifdef __cplusplus
#ifndef LODEPNG_NO_COMPILE_CPP
#define LODEPNG_COMPILE_CPP
#endif
#endif

#ifdef LODEPNG_COMPILE_PNG
/*The PNG color types (also used for raw).*/
typedef enum LodePNGColorType
{
  LCT_GREY = 0, /*greyscale: 1,2,4,8,16 bit*/
  LCT_RGB = 2, /*RGB: 8,16 bit*/
  LCT_PALETTE = 3, /*palette: 1,2,4,8 bit*/
  LCT_GREY_ALPHA = 4, /*greyscale with alpha: 8,16 bit*/
  LCT_RGBA = 6 /*RGB with alpha: 8,16 bit*/
} LodePNGColorType;

#ifdef LODEPNG_COMPILE_DECODER
/*
Converts PNG data in memory to raw pixel data.
out: Output parameter. Pointer to buffer that will contain the raw pixel data.
	 After decoding, its size is w * h * (bytes per pixel) bytes larger than
	 initially. Bytes per pixel depends on colortype and bitdepth.
	 Must be freed after usage with free(*out).
	 Note: for 16-bit per channel colors, uses big endian format like PNG does.
w: Output parameter. Pointer to width of pixel data.
h: Output parameter. Pointer to height of pixel data.
in: Memory buffer with the PNG file.
insize: size of the in buffer.
colortype: the desired color type for the raw output image. See explanation on PNG color types.
bitdepth: the desired bit depth for the raw output image. See explanation on PNG color types.
Return value: LodePNG error code (0 means no error).
*/
unsigned lodepng_decode_memory(unsigned char** out, unsigned* w, unsigned* h,
							   const unsigned char* in, size_t insize,
							   LodePNGColorType colortype, unsigned bitdepth);

/*Same as lodepng_decode_memory, but always decodes to 32-bit RGBA raw image*/
unsigned lodepng_decode32(unsigned char** out, unsigned* w, unsigned* h,
						  const unsigned char* in, size_t insize);

/*Same as lodepng_decode_memory, but always decodes to 24-bit RGB raw image*/
unsigned lodepng_decode24(unsigned char** out, unsigned* w, unsigned* h,
						  const unsigned char* in, size_t insize);

#ifdef LODEPNG_COMPILE_DISK
/*
Load PNG from disk, from file with given name.
Same as the other decode functions, but instead takes a filename as input.
*/
unsigned lodepng_decode_file(unsigned char** out, unsigned* w, unsigned* h,
							 const char* filename,
							 LodePNGColorType colortype, unsigned bitdepth);

/*Same as lodepng_decode_file, but always decodes to 32-bit RGBA raw image.*/
unsigned lodepng_decode32_file(unsigned char** out, unsigned* w, unsigned* h,
							   const char* filename);

/*Same as lodepng_decode_file, but always decodes to 24-bit RGB raw image.*/
unsigned lodepng_decode24_file(unsigned char** out, unsigned* w, unsigned* h,
							   const char* filename);
#endif /*LODEPNG_COMPILE_DISK*/
#endif /*LODEPNG_COMPILE_DECODER*/

#ifdef LODEPNG_COMPILE_ENCODER
/*
Converts raw pixel data into a PNG image in memory. The colortype and bitdepth
  of the output PNG image cannot be chosen, they are automatically determined
  by the colortype, bitdepth and content of the input pixel data.
  Note: for 16-bit per channel colors, needs big endian format like PNG does.
out: Output parameter. Pointer to buffer that will contain the PNG image data.
	 Must be freed after usage with free(*out).
outsize: Output parameter. Pointer to the size in bytes of the out buffer.
image: The raw pixel data to encode. The size of this buffer should be
	   w * h * (bytes per pixel), bytes per pixel depends on colortype and bitdepth.
w: width of the raw pixel data in pixels.
h: height of the raw pixel data in pixels.
colortype: the color type of the raw input image. See explanation on PNG color types.
bitdepth: the bit depth of the raw input image. See explanation on PNG color types.
Return value: LodePNG error code (0 means no error).
*/
unsigned lodepng_encode_memory(unsigned char** out, size_t* outsize,
							   const unsigned char* image, unsigned w, unsigned h,
							   LodePNGColorType colortype, unsigned bitdepth);

/* @r-lyeh { */
unsigned lodepng_encode_memory_std(unsigned char** out, size_t* outsize,
							   const unsigned char* image, unsigned w, unsigned h,
							   unsigned colortype, unsigned bitdepth);
/* } */

/*Same as lodepng_encode_memory, but always encodes from 32-bit RGBA raw image.*/
unsigned lodepng_encode32(unsigned char** out, size_t* outsize,
						  const unsigned char* image, unsigned w, unsigned h);

/*Same as lodepng_encode_memory, but always encodes from 24-bit RGB raw image.*/
unsigned lodepng_encode24(unsigned char** out, size_t* outsize,
						  const unsigned char* image, unsigned w, unsigned h);

#ifdef LODEPNG_COMPILE_DISK
/*
Converts raw pixel data into a PNG file on disk.
Same as the other encode functions, but instead takes a filename as output.
NOTE: This overwrites existing files without warning!
*/
unsigned lodepng_encode_file(const char* filename,
							 const unsigned char* image, unsigned w, unsigned h,
							 LodePNGColorType colortype, unsigned bitdepth);

/*Same as lodepng_encode_file, but always encodes from 32-bit RGBA raw image.*/
unsigned lodepng_encode32_file(const char* filename,
							   const unsigned char* image, unsigned w, unsigned h);

/*Same as lodepng_encode_file, but always encodes from 24-bit RGB raw image.*/
unsigned lodepng_encode24_file(const char* filename,
							   const unsigned char* image, unsigned w, unsigned h);
#endif /*LODEPNG_COMPILE_DISK*/
#endif /*LODEPNG_COMPILE_ENCODER*/

#ifdef LODEPNG_COMPILE_CPP
namespace lodepng
{
#ifdef LODEPNG_COMPILE_DECODER
/*Same as lodepng_decode_memory, but decodes to an std::vector. The colortype
is the format to output the pixels to. Default is RGBA 8-bit per channel.*/
unsigned decode(std::vector<unsigned char>& out, unsigned& w, unsigned& h,
				const unsigned char* in, size_t insize,
				LodePNGColorType colortype = LCT_RGBA, unsigned bitdepth = 8);
unsigned decode(std::vector<unsigned char>& out, unsigned& w, unsigned& h,
				const std::vector<unsigned char>& in,
				LodePNGColorType colortype = LCT_RGBA, unsigned bitdepth = 8);
#ifdef LODEPNG_COMPILE_DISK
/*
Converts PNG file from disk to raw pixel data in memory.
Same as the other decode functions, but instead takes a filename as input.
*/
unsigned decode(std::vector<unsigned char>& out, unsigned& w, unsigned& h,
				const std::string& filename,
				LodePNGColorType colortype = LCT_RGBA, unsigned bitdepth = 8);
#endif //LODEPNG_COMPILE_DISK
#endif //LODEPNG_COMPILE_DECODER

#ifdef LODEPNG_COMPILE_ENCODER
/*Same as lodepng_encode_memory, but encodes to an std::vector. colortype
is that of the raw input data. The output PNG color type will be auto chosen.*/
unsigned encode(std::vector<unsigned char>& out,
				const unsigned char* in, unsigned w, unsigned h,
				LodePNGColorType colortype = LCT_RGBA, unsigned bitdepth = 8);
unsigned encode(std::vector<unsigned char>& out,
				const std::vector<unsigned char>& in, unsigned w, unsigned h,
				LodePNGColorType colortype = LCT_RGBA, unsigned bitdepth = 8);
#ifdef LODEPNG_COMPILE_DISK
/*
Converts 32-bit RGBA raw pixel data into a PNG file on disk.
Same as the other encode functions, but instead takes a filename as output.
NOTE: This overwrites existing files without warning!
*/
unsigned encode(const std::string& filename,
				const unsigned char* in, unsigned w, unsigned h,
				LodePNGColorType colortype = LCT_RGBA, unsigned bitdepth = 8);
unsigned encode(const std::string& filename,
				const std::vector<unsigned char>& in, unsigned w, unsigned h,
				LodePNGColorType colortype = LCT_RGBA, unsigned bitdepth = 8);
#endif //LODEPNG_COMPILE_DISK
#endif //LODEPNG_COMPILE_ENCODER
} //namespace lodepng
#endif /*LODEPNG_COMPILE_CPP*/
#endif /*LODEPNG_COMPILE_PNG*/

#ifdef LODEPNG_COMPILE_ERROR_TEXT
/*Returns an English description of the numerical error code.*/
const char* lodepng_error_text(unsigned code);
#endif /*LODEPNG_COMPILE_ERROR_TEXT*/

#ifdef LODEPNG_COMPILE_DECODER
/*Settings for zlib decompression*/
typedef struct LodePNGDecompressSettings LodePNGDecompressSettings;
struct LodePNGDecompressSettings
{
  unsigned ignore_adler32; /*if 1, continue and don't give an error message if the Adler32 checksum is corrupted*/

  /*use custom zlib decoder instead of built in one (default: null)*/
  unsigned (*custom_zlib)(unsigned char**, size_t*,
						  const unsigned char*, size_t,
						  const LodePNGDecompressSettings*);
  /*use custom deflate decoder instead of built in one (default: null)
  if custom_zlib is used, custom_deflate is ignored since only the built in
  zlib function will call custom_deflate*/
  unsigned (*custom_inflate)(unsigned char**, size_t*,
							 const unsigned char*, size_t,
							 const LodePNGDecompressSettings*);

  const void* custom_context; /*optional custom settings for custom functions*/
};

extern const LodePNGDecompressSettings lodepng_default_decompress_settings;
void lodepng_decompress_settings_init(LodePNGDecompressSettings* settings);
#endif /*LODEPNG_COMPILE_DECODER*/

#ifdef LODEPNG_COMPILE_ENCODER
/*
Settings for zlib compression. Tweaking these settings tweaks the balance
between speed and compression ratio.
*/
typedef struct LodePNGCompressSettings LodePNGCompressSettings;
struct LodePNGCompressSettings /*deflate = compress*/
{
  /*LZ77 related settings*/
  unsigned btype; /*the block type for LZ (0, 1, 2 or 3, see zlib standard). Should be 2 for proper compression.*/
  unsigned use_lz77; /*whether or not to use LZ77. Should be 1 for proper compression.*/
  unsigned windowsize; /*must be a power of two <= 32768. higher compresses more but is slower. Default value: 2048.*/
  unsigned minmatch; /*mininum lz77 length. 3 is normally best, 6 can be better for some PNGs. Default: 0*/
  unsigned nicematch; /*stop searching if >= this length found. Set to 258 for best compression. Default: 128*/
  unsigned lazymatching; /*use lazy matching: better compression but a bit slower. Default: true*/

  /*use custom zlib encoder instead of built in one (default: null)*/
  unsigned (*custom_zlib)(unsigned char**, size_t*,
						  const unsigned char*, size_t,
						  const LodePNGCompressSettings*);
  /*use custom deflate encoder instead of built in one (default: null)
  if custom_zlib is used, custom_deflate is ignored since only the built in
  zlib function will call custom_deflate*/
  unsigned (*custom_deflate)(unsigned char**, size_t*,
							 const unsigned char*, size_t,
							 const LodePNGCompressSettings*);

  const void* custom_context; /*optional custom settings for custom functions*/
};

extern const LodePNGCompressSettings lodepng_default_compress_settings;
void lodepng_compress_settings_init(LodePNGCompressSettings* settings);
#endif /*LODEPNG_COMPILE_ENCODER*/

#ifdef LODEPNG_COMPILE_PNG
/*
Color mode of an image. Contains all information required to decode the pixel
bits to RGBA colors. This information is the same as used in the PNG file
format, and is used both for PNG and raw image data in LodePNG.
*/
typedef struct LodePNGColorMode
{
  /*header (IHDR)*/
  LodePNGColorType colortype; /*color type, see PNG standard or documentation further in this header file*/
  unsigned bitdepth;  /*bits per sample, see PNG standard or documentation further in this header file*/

  /*
  palette (PLTE and tRNS)

  Dynamically allocated with the colors of the palette, including alpha.
  When encoding a PNG, to store your colors in the palette of the LodePNGColorMode, first use
  lodepng_palette_clear, then for each color use lodepng_palette_add.
  If you encode an image without alpha with palette, don't forget to put value 255 in each A byte of the palette.

  When decoding, by default you can ignore this palette, since LodePNG already
  fills the palette colors in the pixels of the raw RGBA output.

  The palette is only supported for color type 3.
  */
  unsigned char* palette; /*palette in RGBARGBA... order. When allocated, must be either 0, or have size 1024*/
  size_t palettesize; /*palette size in number of colors (amount of bytes is 4 * palettesize)*/

  /*
  transparent color key (tRNS)

  This color uses the same bit depth as the bitdepth value in this struct, which can be 1-bit to 16-bit.
  For greyscale PNGs, r, g and b will all 3 be set to the same.

  When decoding, by default you can ignore this information, since LodePNG sets
  pixels with this key to transparent already in the raw RGBA output.

  The color key is only supported for color types 0 and 2.
  */
  unsigned key_defined; /*is a transparent color key given? 0 = false, 1 = true*/
  unsigned key_r;       /*red/greyscale component of color key*/
  unsigned key_g;       /*green component of color key*/
  unsigned key_b;       /*blue component of color key*/
} LodePNGColorMode;

/*init, cleanup and copy functions to use with this struct*/
void lodepng_color_mode_init(LodePNGColorMode* info);
void lodepng_color_mode_cleanup(LodePNGColorMode* info);
/*return value is error code (0 means no error)*/
unsigned lodepng_color_mode_copy(LodePNGColorMode* dest, const LodePNGColorMode* source);

void lodepng_palette_clear(LodePNGColorMode* info);
/*add 1 color to the palette*/
unsigned lodepng_palette_add(LodePNGColorMode* info,
							 unsigned char r, unsigned char g, unsigned char b, unsigned char a);

/*get the total amount of bits per pixel, based on colortype and bitdepth in the struct*/
unsigned lodepng_get_bpp(const LodePNGColorMode* info);
/*get the amount of color channels used, based on colortype in the struct.
If a palette is used, it counts as 1 channel.*/
unsigned lodepng_get_channels(const LodePNGColorMode* info);
/*is it a greyscale type? (only colortype 0 or 4)*/
unsigned lodepng_is_greyscale_type(const LodePNGColorMode* info);
/*has it got an alpha channel? (only colortype 2 or 6)*/
unsigned lodepng_is_alpha_type(const LodePNGColorMode* info);
/*has it got a palette? (only colortype 3)*/
unsigned lodepng_is_palette_type(const LodePNGColorMode* info);
/*only returns true if there is a palette and there is a value in the palette with alpha < 255.
Loops through the palette to check this.*/
unsigned lodepng_has_palette_alpha(const LodePNGColorMode* info);
/*
Check if the given color info indicates the possibility of having non-opaque pixels in the PNG image.
Returns true if the image can have translucent or invisible pixels (it still be opaque if it doesn't use such pixels).
Returns false if the image can only have opaque pixels.
In detail, it returns true only if it's a color type with alpha, or has a palette with non-opaque values,
or if "key_defined" is true.
*/
unsigned lodepng_can_have_alpha(const LodePNGColorMode* info);
/*Returns the byte size of a raw image buffer with given width, height and color mode*/
size_t lodepng_get_raw_size(unsigned w, unsigned h, const LodePNGColorMode* color);

#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
/*The information of a Time chunk in PNG.*/
typedef struct LodePNGTime
{
  unsigned year;    /*2 bytes used (0-65535)*/
  unsigned month;   /*1-12*/
  unsigned day;     /*1-31*/
  unsigned hour;    /*0-23*/
  unsigned minute;  /*0-59*/
  unsigned second;  /*0-60 (to allow for leap seconds)*/
} LodePNGTime;
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/

/*Information about the PNG image, except pixels, width and height.*/
typedef struct LodePNGInfo
{
  /*header (IHDR), palette (PLTE) and transparency (tRNS) chunks*/
  unsigned compression_method;/*compression method of the original file. Always 0.*/
  unsigned filter_method;     /*filter method of the original file*/
  unsigned interlace_method;  /*interlace method of the original file*/
  LodePNGColorMode color;     /*color type and bits, palette and transparency of the PNG file*/

#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
  /*
  suggested background color chunk (bKGD)
  This color uses the same color mode as the PNG (except alpha channel), which can be 1-bit to 16-bit.

  For greyscale PNGs, r, g and b will all 3 be set to the same. When encoding
  the encoder writes the red one. For palette PNGs: When decoding, the RGB value
  will be stored, not a palette index. But when encoding, specify the index of
  the palette in background_r, the other two are then ignored.

  The decoder does not use this background color to edit the color of pixels.
  */
  unsigned background_defined; /*is a suggested background color given?*/
  unsigned background_r;       /*red component of suggested background color*/
  unsigned background_g;       /*green component of suggested background color*/
  unsigned background_b;       /*blue component of suggested background color*/

  /*
  non-international text chunks (tEXt and zTXt)

  The char** arrays each contain num strings. The actual messages are in
  text_strings, while text_keys are keywords that give a short description what
  the actual text represents, e.g. Title, Author, Description, or anything else.

  A keyword is minimum 1 character and maximum 79 characters long. It's
  discouraged to use a single line length longer than 79 characters for texts.

  Don't allocate these text buffers yourself. Use the init/cleanup functions
  correctly and use lodepng_add_text and lodepng_clear_text.
  */
  size_t text_num; /*the amount of texts in these char** buffers (there may be more texts in itext)*/
  char** text_keys; /*the keyword of a text chunk (e.g. "Comment")*/
  char** text_strings; /*the actual text*/

  /*
  international text chunks (iTXt)
  Similar to the non-international text chunks, but with additional strings
  "langtags" and "transkeys".
  */
  size_t itext_num; /*the amount of international texts in this PNG*/
  char** itext_keys; /*the English keyword of the text chunk (e.g. "Comment")*/
  char** itext_langtags; /*language tag for this text's language, ISO/IEC 646 string, e.g. ISO 639 language tag*/
  char** itext_transkeys; /*keyword translated to the international language - UTF-8 string*/
  char** itext_strings; /*the actual international text - UTF-8 string*/

  /*time chunk (tIME)*/
  unsigned time_defined; /*set to 1 to make the encoder generate a tIME chunk*/
  LodePNGTime time;

  /*phys chunk (pHYs)*/
  unsigned phys_defined; /*if 0, there is no pHYs chunk and the values below are undefined, if 1 else there is one*/
  unsigned phys_x; /*pixels per unit in x direction*/
  unsigned phys_y; /*pixels per unit in y direction*/
  unsigned phys_unit; /*may be 0 (unknown unit) or 1 (metre)*/

  /*
  unknown chunks
  There are 3 buffers, one for each position in the PNG where unknown chunks can appear
  each buffer contains all unknown chunks for that position consecutively
  The 3 buffers are the unknown chunks between certain critical chunks:
  0: IHDR-PLTE, 1: PLTE-IDAT, 2: IDAT-IEND
  Do not allocate or traverse this data yourself. Use the chunk traversing functions declared
  later, such as lodepng_chunk_next and lodepng_chunk_append, to read/write this struct.
  */
  unsigned char* unknown_chunks_data[3];
  size_t unknown_chunks_size[3]; /*size in bytes of the unknown chunks, given for protection*/
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
} LodePNGInfo;

/*init, cleanup and copy functions to use with this struct*/
void lodepng_info_init(LodePNGInfo* info);
void lodepng_info_cleanup(LodePNGInfo* info);
/*return value is error code (0 means no error)*/
unsigned lodepng_info_copy(LodePNGInfo* dest, const LodePNGInfo* source);

#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
void lodepng_clear_text(LodePNGInfo* info); /*use this to clear the texts again after you filled them in*/
unsigned lodepng_add_text(LodePNGInfo* info, const char* key, const char* str); /*push back both texts at once*/

void lodepng_clear_itext(LodePNGInfo* info); /*use this to clear the itexts again after you filled them in*/
unsigned lodepng_add_itext(LodePNGInfo* info, const char* key, const char* langtag,
						   const char* transkey, const char* str); /*push back the 4 texts of 1 chunk at once*/
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/

/*
Converts raw buffer from one color type to another color type, based on
LodePNGColorMode structs to describe the input and output color type.
See the reference manual at the end of this header file to see which color conversions are supported.
return value = LodePNG error code (0 if all went ok, an error if the conversion isn't supported)
The out buffer must have size (w * h * bpp + 7) / 8, where bpp is the bits per pixel
of the output color type (lodepng_get_bpp).
For < 8 bpp images, there should not be padding bits at the end of scanlines.
For 16-bit per channel colors, uses big endian format like PNG does.
Return value is LodePNG error code
*/
unsigned lodepng_convert(unsigned char* out, const unsigned char* in,
						 LodePNGColorMode* mode_out, const LodePNGColorMode* mode_in,
						 unsigned w, unsigned h);

#ifdef LODEPNG_COMPILE_DECODER
/*
Settings for the decoder. This contains settings for the PNG and the Zlib
decoder, but not the Info settings from the Info structs.
*/
typedef struct LodePNGDecoderSettings
{
  LodePNGDecompressSettings zlibsettings; /*in here is the setting to ignore Adler32 checksums*/

  unsigned ignore_crc; /*ignore CRC checksums*/

  unsigned color_convert; /*whether to convert the PNG to the color type you want. Default: yes*/

#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
  unsigned read_text_chunks; /*if false but remember_unknown_chunks is true, they're stored in the unknown chunks*/
  /*store all bytes from unknown chunks in the LodePNGInfo (off by default, useful for a png editor)*/
  unsigned remember_unknown_chunks;
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
} LodePNGDecoderSettings;

void lodepng_decoder_settings_init(LodePNGDecoderSettings* settings);
#endif /*LODEPNG_COMPILE_DECODER*/

#ifdef LODEPNG_COMPILE_ENCODER
/*automatically use color type with less bits per pixel if losslessly possible. Default: AUTO*/
typedef enum LodePNGFilterStrategy
{
  /*every filter at zero*/
  LFS_ZERO,
  /*Use filter that gives minumum sum, as described in the official PNG filter heuristic.*/
  LFS_MINSUM,
  /*Use the filter type that gives smallest Shannon entropy for this scanline. Depending
  on the image, this is better or worse than minsum.*/
  LFS_ENTROPY,
  /*
  Brute-force-search PNG filters by compressing each filter for each scanline.
  Experimental, very slow, and only rarely gives better compression than MINSUM.
  */
  LFS_BRUTE_FORCE,
  /*use predefined_filters buffer: you specify the filter type for each scanline*/
  LFS_PREDEFINED
} LodePNGFilterStrategy;

/*Gives characteristics about the colors of the image, which helps decide which color model to use for encoding.
Used internally by default if "auto_convert" is enabled. Public because it's useful for custom algorithms.*/
typedef struct LodePNGColorProfile
{
  unsigned colored; /*not greyscale*/
  unsigned key; /*if true, image is not opaque. Only if true and alpha is false, color key is possible.*/
  unsigned short key_r; /*these values are always in 16-bit bitdepth in the profile*/
  unsigned short key_g;
  unsigned short key_b;
  unsigned alpha; /*alpha channel or alpha palette required*/
  unsigned numcolors; /*amount of colors, up to 257. Not valid if bits == 16.*/
  unsigned char palette[1024]; /*Remembers up to the first 256 RGBA colors, in no particular order*/
  unsigned bits; /*bits per channel (not for palette). 1,2 or 4 for greyscale only. 16 if 16-bit per channel required.*/
} LodePNGColorProfile;

void lodepng_color_profile_init(LodePNGColorProfile* profile);

/*Get a LodePNGColorProfile of the image.*/
unsigned lodepng_get_color_profile(LodePNGColorProfile* profile,
								   const unsigned char* image, unsigned w, unsigned h,
								   const LodePNGColorMode* mode_in);
/*The function LodePNG uses internally to decide the PNG color with auto_convert.
Chooses an optimal color model, e.g. grey if only grey pixels, palette if < 256 colors, ...*/
unsigned lodepng_auto_choose_color(LodePNGColorMode* mode_out,
								   const unsigned char* image, unsigned w, unsigned h,
								   const LodePNGColorMode* mode_in);

/*Settings for the encoder.*/
typedef struct LodePNGEncoderSettings
{
  LodePNGCompressSettings zlibsettings; /*settings for the zlib encoder, such as window size, ...*/

  unsigned auto_convert; /*automatically choose output PNG color type. Default: true*/

  /*If true, follows the official PNG heuristic: if the PNG uses a palette or lower than
  8 bit depth, set all filters to zero. Otherwise use the filter_strategy. Note that to
  completely follow the official PNG heuristic, filter_palette_zero must be true and
  filter_strategy must be LFS_MINSUM*/
  unsigned filter_palette_zero;
  /*Which filter strategy to use when not using zeroes due to filter_palette_zero.
  Set filter_palette_zero to 0 to ensure always using your chosen strategy. Default: LFS_MINSUM*/
  LodePNGFilterStrategy filter_strategy;
  /*used if filter_strategy is LFS_PREDEFINED. In that case, this must point to a buffer with
  the same length as the amount of scanlines in the image, and each value must <= 5. You
  have to cleanup this buffer, LodePNG will never free it. Don't forget that filter_palette_zero
  must be set to 0 to ensure this is also used on palette or low bitdepth images.*/
  const unsigned char* predefined_filters;

  /*force creating a PLTE chunk if colortype is 2 or 6 (= a suggested palette).
  If colortype is 3, PLTE is _always_ created.*/
  unsigned force_palette;
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
  /*add LodePNG identifier and version as a text chunk, for debugging*/
  unsigned add_id;
  /*encode text chunks as zTXt chunks instead of tEXt chunks, and use compression in iTXt chunks*/
  unsigned text_compression;
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
} LodePNGEncoderSettings;

void lodepng_encoder_settings_init(LodePNGEncoderSettings* settings);
#endif /*LODEPNG_COMPILE_ENCODER*/

#if defined(LODEPNG_COMPILE_DECODER) || defined(LODEPNG_COMPILE_ENCODER)
/*The settings, state and information for extended encoding and decoding.*/
typedef struct LodePNGState
{
#ifdef LODEPNG_COMPILE_DECODER
  LodePNGDecoderSettings decoder; /*the decoding settings*/
#endif /*LODEPNG_COMPILE_DECODER*/
#ifdef LODEPNG_COMPILE_ENCODER
  LodePNGEncoderSettings encoder; /*the encoding settings*/
#endif /*LODEPNG_COMPILE_ENCODER*/
  LodePNGColorMode info_raw; /*specifies the format in which you would like to get the raw pixel buffer*/
  LodePNGInfo info_png; /*info of the PNG image obtained after decoding*/
  unsigned error;
#ifdef LODEPNG_COMPILE_CPP
  //For the lodepng::State subclass.
  virtual ~LodePNGState(){}
#endif
} LodePNGState;

/*init, cleanup and copy functions to use with this struct*/
void lodepng_state_init(LodePNGState* state);
void lodepng_state_cleanup(LodePNGState* state);
void lodepng_state_copy(LodePNGState* dest, const LodePNGState* source);
#endif /* defined(LODEPNG_COMPILE_DECODER) || defined(LODEPNG_COMPILE_ENCODER) */

#ifdef LODEPNG_COMPILE_DECODER
/*
Same as lodepng_decode_memory, but uses a LodePNGState to allow custom settings and
getting much more information about the PNG image and color mode.
*/
unsigned lodepng_decode(unsigned char** out, unsigned* w, unsigned* h,
						LodePNGState* state,
						const unsigned char* in, size_t insize);

/*
Read the PNG header, but not the actual data. This returns only the information
that is in the header chunk of the PNG, such as width, height and color type. The
information is placed in the info_png field of the LodePNGState.
*/
unsigned lodepng_inspect(unsigned* w, unsigned* h,
						 LodePNGState* state,
						 const unsigned char* in, size_t insize);
#endif /*LODEPNG_COMPILE_DECODER*/

#ifdef LODEPNG_COMPILE_ENCODER
/*This function allocates the out buffer with standard malloc and stores the size in *outsize.*/
unsigned lodepng_encode(unsigned char** out, size_t* outsize,
						const unsigned char* image, unsigned w, unsigned h,
						LodePNGState* state);
#endif /*LODEPNG_COMPILE_ENCODER*/

/*
The lodepng_chunk functions are normally not needed, except to traverse the
unknown chunks stored in the LodePNGInfo struct, or add new ones to it.
It also allows traversing the chunks of an encoded PNG file yourself.

PNG standard chunk naming conventions:
First byte: uppercase = critical, lowercase = ancillary
Second byte: uppercase = public, lowercase = private
Third byte: must be uppercase
Fourth byte: uppercase = unsafe to copy, lowercase = safe to copy
*/

/*
Gets the length of the data of the chunk. Total chunk length has 12 bytes more.
There must be at least 4 bytes to read from. If the result value is too large,
it may be corrupt data.
*/
unsigned lodepng_chunk_length(const unsigned char* chunk);

/*puts the 4-byte type in null terminated string*/
void lodepng_chunk_type(char type[5], const unsigned char* chunk);

/*check if the type is the given type*/
unsigned char lodepng_chunk_type_equals(const unsigned char* chunk, const char* type);

/*0: it's one of the critical chunk types, 1: it's an ancillary chunk (see PNG standard)*/
unsigned char lodepng_chunk_ancillary(const unsigned char* chunk);

/*0: public, 1: private (see PNG standard)*/
unsigned char lodepng_chunk_private(const unsigned char* chunk);

/*0: the chunk is unsafe to copy, 1: the chunk is safe to copy (see PNG standard)*/
unsigned char lodepng_chunk_safetocopy(const unsigned char* chunk);

/*get pointer to the data of the chunk, where the input points to the header of the chunk*/
unsigned char* lodepng_chunk_data(unsigned char* chunk);
const unsigned char* lodepng_chunk_data_const(const unsigned char* chunk);

/*returns 0 if the crc is correct, 1 if it's incorrect (0 for OK as usual!)*/
unsigned lodepng_chunk_check_crc(const unsigned char* chunk);

/*generates the correct CRC from the data and puts it in the last 4 bytes of the chunk*/
void lodepng_chunk_generate_crc(unsigned char* chunk);

/*iterate to next chunks. don't use on IEND chunk, as there is no next chunk then*/
unsigned char* lodepng_chunk_next(unsigned char* chunk);
const unsigned char* lodepng_chunk_next_const(const unsigned char* chunk);

/*
Appends chunk to the data in out. The given chunk should already have its chunk header.
The out variable and outlength are updated to reflect the new reallocated buffer.
Returns error code (0 if it went ok)
*/
unsigned lodepng_chunk_append(unsigned char** out, size_t* outlength, const unsigned char* chunk);

/*
Appends new chunk to out. The chunk to append is given by giving its length, type
and data separately. The type is a 4-letter string.
The out variable and outlength are updated to reflect the new reallocated buffer.
Returne error code (0 if it went ok)
*/
unsigned lodepng_chunk_create(unsigned char** out, size_t* outlength, unsigned length,
							  const char* type, const unsigned char* data);

/*Calculate CRC32 of buffer*/
unsigned lodepng_crc32(const unsigned char* buf, size_t len);
#endif /*LODEPNG_COMPILE_PNG*/

#ifdef LODEPNG_COMPILE_ZLIB
/*
This zlib part can be used independently to zlib compress and decompress a
buffer. It cannot be used to create gzip files however, and it only supports the
part of zlib that is required for PNG, it does not support dictionaries.
*/

#ifdef LODEPNG_COMPILE_DECODER
/*Inflate a buffer. Inflate is the decompression step of deflate. Out buffer must be freed after use.*/
unsigned lodepng_inflate(unsigned char** out, size_t* outsize,
						 const unsigned char* in, size_t insize,
						 const LodePNGDecompressSettings* settings);

/*
Decompresses Zlib data. Reallocates the out buffer and appends the data. The
data must be according to the zlib specification.
Either, *out must be NULL and *outsize must be 0, or, *out must be a valid
buffer and *outsize its size in bytes. out must be freed by user after usage.
*/
unsigned lodepng_zlib_decompress(unsigned char** out, size_t* outsize,
								 const unsigned char* in, size_t insize,
								 const LodePNGDecompressSettings* settings);
#endif /*LODEPNG_COMPILE_DECODER*/

#ifdef LODEPNG_COMPILE_ENCODER
/*
Compresses data with Zlib. Reallocates the out buffer and appends the data.
Zlib adds a small header and trailer around the deflate data.
The data is output in the format of the zlib specification.
Either, *out must be NULL and *outsize must be 0, or, *out must be a valid
buffer and *outsize its size in bytes. out must be freed by user after usage.
*/
unsigned lodepng_zlib_compress(unsigned char** out, size_t* outsize,
							   const unsigned char* in, size_t insize,
							   const LodePNGCompressSettings* settings);

/*
Find length-limited Huffman code for given frequencies. This function is in the
public interface only for tests, it's used internally by lodepng_deflate.
*/
unsigned lodepng_huffman_code_lengths(unsigned* lengths, const unsigned* frequencies,
									  size_t numcodes, unsigned maxbitlen);

/*Compress a buffer with deflate. See RFC 1951. Out buffer must be freed after use.*/
unsigned lodepng_deflate(unsigned char** out, size_t* outsize,
						 const unsigned char* in, size_t insize,
						 const LodePNGCompressSettings* settings);

#endif /*LODEPNG_COMPILE_ENCODER*/
#endif /*LODEPNG_COMPILE_ZLIB*/

#ifdef LODEPNG_COMPILE_DISK
/*
Load a file from disk into buffer. The function allocates the out buffer, and
after usage you should free it.
out: output parameter, contains pointer to loaded buffer.
outsize: output parameter, size of the allocated out buffer
filename: the path to the file to load
return value: error code (0 means ok)
*/
unsigned lodepng_load_file(unsigned char** out, size_t* outsize, const char* filename);

/*
Save a file from buffer to disk. Warning, if it exists, this function overwrites
the file without warning!
buffer: the buffer to write
buffersize: size of the buffer to write
filename: the path to the file to save to
return value: error code (0 means ok)
*/
unsigned lodepng_save_file(const unsigned char* buffer, size_t buffersize, const char* filename);
#endif /*LODEPNG_COMPILE_DISK*/

#ifdef LODEPNG_COMPILE_CPP
//The LodePNG C++ wrapper uses std::vectors instead of manually allocated memory buffers.
namespace lodepng
{
#ifdef LODEPNG_COMPILE_PNG
class State : public LodePNGState
{
  public:
	State();
	State(const State& other);
	virtual ~State();
	State& operator=(const State& other);
};

#ifdef LODEPNG_COMPILE_DECODER
//Same as other lodepng::decode, but using a State for more settings and information.
unsigned decode(std::vector<unsigned char>& out, unsigned& w, unsigned& h,
				State& state,
				const unsigned char* in, size_t insize);
unsigned decode(std::vector<unsigned char>& out, unsigned& w, unsigned& h,
				State& state,
				const std::vector<unsigned char>& in);
#endif /*LODEPNG_COMPILE_DECODER*/

#ifdef LODEPNG_COMPILE_ENCODER
//Same as other lodepng::encode, but using a State for more settings and information.
unsigned encode(std::vector<unsigned char>& out,
				const unsigned char* in, unsigned w, unsigned h,
				State& state);
unsigned encode(std::vector<unsigned char>& out,
				const std::vector<unsigned char>& in, unsigned w, unsigned h,
				State& state);
#endif /*LODEPNG_COMPILE_ENCODER*/

#ifdef LODEPNG_COMPILE_DISK
/*
Load a file from disk into an std::vector. If the vector is empty, then either
the file doesn't exist or is an empty file.
*/
void load_file(std::vector<unsigned char>& buffer, const std::string& filename);

/*
Save the binary data in an std::vector to a file on disk. The file is overwritten
without warning.
*/
void save_file(const std::vector<unsigned char>& buffer, const std::string& filename);
#endif //LODEPNG_COMPILE_DISK
#endif //LODEPNG_COMPILE_PNG

#ifdef LODEPNG_COMPILE_ZLIB
#ifdef LODEPNG_COMPILE_DECODER
//Zlib-decompress an unsigned char buffer
unsigned decompress(std::vector<unsigned char>& out, const unsigned char* in, size_t insize,
					const LodePNGDecompressSettings& settings = lodepng_default_decompress_settings);

//Zlib-decompress an std::vector
unsigned decompress(std::vector<unsigned char>& out, const std::vector<unsigned char>& in,
					const LodePNGDecompressSettings& settings = lodepng_default_decompress_settings);
#endif //LODEPNG_COMPILE_DECODER

#ifdef LODEPNG_COMPILE_ENCODER
//Zlib-compress an unsigned char buffer
unsigned compress(std::vector<unsigned char>& out, const unsigned char* in, size_t insize,
				  const LodePNGCompressSettings& settings = lodepng_default_compress_settings);

//Zlib-compress an std::vector
unsigned compress(std::vector<unsigned char>& out, const std::vector<unsigned char>& in,
				  const LodePNGCompressSettings& settings = lodepng_default_compress_settings);
#endif //LODEPNG_COMPILE_ENCODER
#endif //LODEPNG_COMPILE_ZLIB
} //namespace lodepng
#endif /*LODEPNG_COMPILE_CPP*/

/*
TODO:
[.] test if there are no memory leaks or security exploits - done a lot but needs to be checked often
[.] check compatibility with vareous compilers  - done but needs to be redone for every newer version
[X] converting color to 16-bit per channel types
[ ] read all public PNG chunk types (but never let the color profile and gamma ones touch RGB values)
[ ] make sure encoder generates no chunks with size > (2^31)-1
[ ] partial decoding (stream processing)
[X] let the "isFullyOpaque" function check color keys and transparent palettes too
[X] better name for the variables "codes", "codesD", "codelengthcodes", "clcl" and "lldl"
[ ] don't stop decoding on errors like 69, 57, 58 (make warnings)
[ ] let the C++ wrapper catch exceptions coming from the standard library and return LodePNG error codes
[ ] allow user to provide custom color conversion functions, e.g. for premultiplied alpha, padding bits or not, ...
*/

#endif /*LODEPNG_H inclusion guard*/

/*
LodePNG Documentation
---------------------

0. table of contents
--------------------

  1. about
   1.1. supported features
   1.2. features not supported
  2. C and C++ version
  3. security
  4. decoding
  5. encoding
  6. color conversions
	6.1. PNG color types
	6.2. color conversions
	6.3. padding bits
	6.4. A note about 16-bits per channel and endianness
  7. error values
  8. chunks and PNG editing
  9. compiler support
  10. examples
   10.1. decoder C++ example
   10.2. decoder C example
  11. changes
  12. contact information

1. about
--------

PNG is a file format to store raster images losslessly with good compression,
supporting different color types and alpha channel.

LodePNG is a PNG codec according to the Portable Network Graphics (PNG)
Specification (Second Edition) - W3C Recommendation 10 November 2003.

The specifications used are:

*) Portable Network Graphics (PNG) Specification (Second Edition):
	 http://www.w3.org/TR/2003/REC-PNG-20031110
*) RFC 1950 ZLIB Compressed Data Format version 3.3:
	 http://www.gzip.org/zlib/rfc-zlib.html
*) RFC 1951 DEFLATE Compressed Data Format Specification ver 1.3:
	 http://www.gzip.org/zlib/rfc-deflate.html

The most recent version of LodePNG can currently be found at
http://lodev.org/lodepng/

LodePNG works both in C (ISO C90) and C++, with a C++ wrapper that adds
extra functionality.

LodePNG exists out of two files:
-lodepng.h: the header file for both C and C++
-lodepng.c(pp): give it the name lodepng.c or lodepng.cpp (or .cc) depending on your usage

If you want to start using LodePNG right away without reading this doc, get the
examples from the LodePNG website to see how to use it in code, or check the
smaller examples in chapter 13 here.

LodePNG is simple but only supports the basic requirements. To achieve
simplicity, the following design choices were made: There are no dependencies
on any external library. There are functions to decode and encode a PNG with
a single function call, and extended versions of these functions taking a
LodePNGState struct allowing to specify or get more information. By default
the colors of the raw image are always RGB or RGBA, no matter what color type
the PNG file uses. To read and write files, there are simple functions to
convert the files to/from buffers in memory.

This all makes LodePNG suitable for loading textures in games, demos and small
programs, ... It's less suitable for full fledged image editors, loading PNGs
over network (it requires all the image data to be available before decoding can
begin), life-critical systems, ...

1.1. supported features
-----------------------

The following features are supported by the decoder:

*) decoding of PNGs with any color type, bit depth and interlace mode, to a 24- or 32-bit color raw image,
   or the same color type as the PNG
*) encoding of PNGs, from any raw image to 24- or 32-bit color, or the same color type as the raw image
*) Adam7 interlace and deinterlace for any color type
*) loading the image from harddisk or decoding it from a buffer from other sources than harddisk
*) support for alpha channels, including RGBA color model, translucent palettes and color keying
*) zlib decompression (inflate)
*) zlib compression (deflate)
*) CRC32 and ADLER32 checksums
*) handling of unknown chunks, allowing making a PNG editor that stores custom and unknown chunks.
*) the following chunks are supported (generated/interpreted) by both encoder and decoder:
	IHDR: header information
	PLTE: color palette
	IDAT: pixel data
	IEND: the final chunk
	tRNS: transparency for palettized images
	tEXt: textual information
	zTXt: compressed textual information
	iTXt: international textual information
	bKGD: suggested background color
	pHYs: physical dimensions
	tIME: modification time

1.2. features not supported
---------------------------

The following features are _not_ supported:

*) some features needed to make a conformant PNG-Editor might be still missing.
*) partial loading/stream processing. All data must be available and is processed in one call.
*) The following public chunks are not supported but treated as unknown chunks by LodePNG
	cHRM, gAMA, iCCP, sRGB, sBIT, hIST, sPLT
   Some of these are not supported on purpose: LodePNG wants to provide the RGB values
   stored in the pixels, not values modified by system dependent gamma or color models.

2. C and C++ version
--------------------

The C version uses buffers allocated with alloc that you need to free()
yourself. You need to use init and cleanup functions for each struct whenever
using a struct from the C version to avoid exploits and memory leaks.

The C++ version has extra functions with std::vectors in the interface and the
lodepng::State class which is a LodePNGState with constructor and destructor.

These files work without modification for both C and C++ compilers because all
the additional C++ code is in "#ifdef __cplusplus" blocks that make C-compilers
ignore it, and the C code is made to compile both with strict ISO C90 and C++.

To use the C++ version, you need to rename the source file to lodepng.cpp
(instead of lodepng.c), and compile it with a C++ compiler.

To use the C version, you need to rename the source file to lodepng.c (instead
of lodepng.cpp), and compile it with a C compiler.

3. Security
-----------

Even if carefully designed, it's always possible that LodePNG contains possible
exploits. If you discover one, please let me know, and it will be fixed.

When using LodePNG, care has to be taken with the C version of LodePNG, as well
as the C-style structs when working with C++. The following conventions are used
for all C-style structs:

-if a struct has a corresponding init function, always call the init function when making a new one
-if a struct has a corresponding cleanup function, call it before the struct disappears to avoid memory leaks
-if a struct has a corresponding copy function, use the copy function instead of "=".
 The destination must also be inited already.

4. Decoding
-----------

Decoding converts a PNG compressed image to a raw pixel buffer.

Most documentation on using the decoder is at its declarations in the header
above. For C, simple decoding can be done with functions such as
lodepng_decode32, and more advanced decoding can be done with the struct
LodePNGState and lodepng_decode. For C++, all decoding can be done with the
various lodepng::decode functions, and lodepng::State can be used for advanced
features.

When using the LodePNGState, it uses the following fields for decoding:
*) LodePNGInfo info_png: it stores extra information about the PNG (the input) in here
*) LodePNGColorMode info_raw: here you can say what color mode of the raw image (the output) you want to get
*) LodePNGDecoderSettings decoder: you can specify a few extra settings for the decoder to use

LodePNGInfo info_png
--------------------

After decoding, this contains extra information of the PNG image, except the actual
pixels, width and height because these are already gotten directly from the decoder
functions.

It contains for example the original color type of the PNG image, text comments,
suggested background color, etc... More details about the LodePNGInfo struct are
at its declaration documentation.

LodePNGColorMode info_raw
-------------------------

When decoding, here you can specify which color type you want
the resulting raw image to be. If this is different from the colortype of the
PNG, then the decoder will automatically convert the result. This conversion
always works, except if you want it to convert a color PNG to greyscale or to
a palette with missing colors.

By default, 32-bit color is used for the result.

LodePNGDecoderSettings decoder
------------------------------

The settings can be used to ignore the errors created by invalid CRC and Adler32
chunks, and to disable the decoding of tEXt chunks.

There's also a setting color_convert, true by default. If false, no conversion
is done, the resulting data will be as it was in the PNG (after decompression)
and you'll have to puzzle the colors of the pixels together yourself using the
color type information in the LodePNGInfo.

5. Encoding
-----------

Encoding converts a raw pixel buffer to a PNG compressed image.

Most documentation on using the encoder is at its declarations in the header
above. For C, simple encoding can be done with functions such as
lodepng_encode32, and more advanced decoding can be done with the struct
LodePNGState and lodepng_encode. For C++, all encoding can be done with the
various lodepng::encode functions, and lodepng::State can be used for advanced
features.

Like the decoder, the encoder can also give errors. However it gives less errors
since the encoder input is trusted, the decoder input (a PNG image that could
be forged by anyone) is not trusted.

When using the LodePNGState, it uses the following fields for encoding:
*) LodePNGInfo info_png: here you specify how you want the PNG (the output) to be.
*) LodePNGColorMode info_raw: here you say what color type of the raw image (the input) has
*) LodePNGEncoderSettings encoder: you can specify a few settings for the encoder to use

LodePNGInfo info_png
--------------------

When encoding, you use this the opposite way as when decoding: for encoding,
you fill in the values you want the PNG to have before encoding. By default it's
not needed to specify a color type for the PNG since it's automatically chosen,
but it's possible to choose it yourself given the right settings.

The encoder will not always exactly match the LodePNGInfo struct you give,
it tries as close as possible. Some things are ignored by the encoder. The
encoder uses, for example, the following settings from it when applicable:
colortype and bitdepth, text chunks, time chunk, the color key, the palette, the
background color, the interlace method, unknown chunks, ...

When encoding to a PNG with colortype 3, the encoder will generate a PLTE chunk.
If the palette contains any colors for which the alpha channel is not 255 (so
there are translucent colors in the palette), it'll add a tRNS chunk.

LodePNGColorMode info_raw
-------------------------

You specify the color type of the raw image that you give to the input here,
including a possible transparent color key and palette you happen to be using in
your raw image data.

By default, 32-bit color is assumed, meaning your input has to be in RGBA
format with 4 bytes (unsigned chars) per pixel.

LodePNGEncoderSettings encoder
------------------------------

The following settings are supported (some are in sub-structs):
*) auto_convert: when this option is enabled, the encoder will
automatically choose the smallest possible color mode (including color key) that
can encode the colors of all pixels without information loss.
*) btype: the block type for LZ77. 0 = uncompressed, 1 = fixed huffman tree,
   2 = dynamic huffman tree (best compression). Should be 2 for proper
   compression.
*) use_lz77: whether or not to use LZ77 for compressed block types. Should be
   true for proper compression.
*) windowsize: the window size used by the LZ77 encoder (1 - 32768). Has value
   2048 by default, but can be set to 32768 for better, but slow, compression.
*) force_palette: if colortype is 2 or 6, you can make the encoder write a PLTE
   chunk if force_palette is true. This can used as suggested palette to convert
   to by viewers that don't support more than 256 colors (if those still exist)
*) add_id: add text chunk "Encoder: LodePNG <version>" to the image.
*) text_compression: default 1. If 1, it'll store texts as zTXt instead of tEXt chunks.
  zTXt chunks use zlib compression on the text. This gives a smaller result on
  large texts but a larger result on small texts (such as a single program name).
  It's all tEXt or all zTXt though, there's no separate setting per text yet.

6. color conversions
--------------------

An important thing to note about LodePNG, is that the color type of the PNG, and
the color type of the raw image, are completely independent. By default, when
you decode a PNG, you get the result as a raw image in the color type you want,
no matter whether the PNG was encoded with a palette, greyscale or RGBA color.
And if you encode an image, by default LodePNG will automatically choose the PNG
color type that gives good compression based on the values of colors and amount
of colors in the image. It can be configured to let you control it instead as
well, though.

To be able to do this, LodePNG does conversions from one color mode to another.
It can convert from almost any color type to any other color type, except the
following conversions: RGB to greyscale is not supported, and converting to a
palette when the palette doesn't have a required color is not supported. This is
not supported on purpose: this is information loss which requires a color
reduction algorithm that is beyong the scope of a PNG encoder (yes, RGB to grey
is easy, but there are multiple ways if you want to give some channels more
weight).

By default, when decoding, you get the raw image in 32-bit RGBA or 24-bit RGB
color, no matter what color type the PNG has. And by default when encoding,
LodePNG automatically picks the best color model for the output PNG, and expects
the input image to be 32-bit RGBA or 24-bit RGB. So, unless you want to control
the color format of the images yourself, you can skip this chapter.

6.1. PNG color types
--------------------

A PNG image can have many color types, ranging from 1-bit color to 64-bit color,
as well as palettized color modes. After the zlib decompression and unfiltering
in the PNG image is done, the raw pixel data will have that color type and thus
a certain amount of bits per pixel. If you want the output raw image after
decoding to have another color type, a conversion is done by LodePNG.

The PNG specification gives the following color types:

0: greyscale, bit depths 1, 2, 4, 8, 16
2: RGB, bit depths 8 and 16
3: palette, bit depths 1, 2, 4 and 8
4: greyscale with alpha, bit depths 8 and 16
6: RGBA, bit depths 8 and 16

Bit depth is the amount of bits per pixel per color channel. So the total amount
of bits per pixel is: amount of channels * bitdepth.

6.2. color conversions
----------------------

As explained in the sections about the encoder and decoder, you can specify
color types and bit depths in info_png and info_raw to change the default
behaviour.

If, when decoding, you want the raw image to be something else than the default,
you need to set the color type and bit depth you want in the LodePNGColorMode,
or the parameters colortype and bitdepth of the simple decoding function.

If, when encoding, you use another color type than the default in the raw input
image, you need to specify its color type and bit depth in the LodePNGColorMode
of the raw image, or use the parameters colortype and bitdepth of the simple
encoding function.

If, when encoding, you don't want LodePNG to choose the output PNG color type
but control it yourself, you need to set auto_convert in the encoder settings
to false, and specify the color type you want in the LodePNGInfo of the
encoder (including palette: it can generate a palette if auto_convert is true,
otherwise not).

If the input and output color type differ (whether user chosen or auto chosen),
LodePNG will do a color conversion, which follows the rules below, and may
sometimes result in an error.

To avoid some confusion:
-the decoder converts from PNG to raw image
-the encoder converts from raw image to PNG
-the colortype and bitdepth in LodePNGColorMode info_raw, are those of the raw image
-the colortype and bitdepth in the color field of LodePNGInfo info_png, are those of the PNG
-when encoding, the color type in LodePNGInfo is ignored if auto_convert
 is enabled, it is automatically generated instead
-when decoding, the color type in LodePNGInfo is set by the decoder to that of the original
 PNG image, but it can be ignored since the raw image has the color type you requested instead
-if the color type of the LodePNGColorMode and PNG image aren't the same, a conversion
 between the color types is done if the color types are supported. If it is not
 supported, an error is returned. If the types are the same, no conversion is done.
-even though some conversions aren't supported, LodePNG supports loading PNGs from any
 colortype and saving PNGs to any colortype, sometimes it just requires preparing
 the raw image correctly before encoding.
-both encoder and decoder use the same color converter.

Non supported color conversions:
-color to greyscale: no error is thrown, but the result will look ugly because
only the red channel is taken
-anything to palette when that palette does not have that color in it: in this
case an error is thrown

Supported color conversions:
-anything to 8-bit RGB, 8-bit RGBA, 16-bit RGB, 16-bit RGBA
-any grey or grey+alpha, to grey or grey+alpha
-anything to a palette, as long as the palette has the requested colors in it
-removing alpha channel
-higher to smaller bitdepth, and vice versa

If you want no color conversion to be done (e.g. for speed or control):
-In the encoder, you can make it save a PNG with any color type by giving the
raw color mode and LodePNGInfo the same color mode, and setting auto_convert to
false.
-In the decoder, you can make it store the pixel data in the same color type
as the PNG has, by setting the color_convert setting to false. Settings in
info_raw are then ignored.

The function lodepng_convert does the color conversion. It is available in the
interface but normally isn't needed since the encoder and decoder already call
it.

6.3. padding bits
-----------------

In the PNG file format, if a less than 8-bit per pixel color type is used and the scanlines
have a bit amount that isn't a multiple of 8, then padding bits are used so that each
scanline starts at a fresh byte. But that is NOT true for the LodePNG raw input and output.
The raw input image you give to the encoder, and the raw output image you get from the decoder
will NOT have these padding bits, e.g. in the case of a 1-bit image with a width
of 7 pixels, the first pixel of the second scanline will the the 8th bit of the first byte,
not the first bit of a new byte.

6.4. A note about 16-bits per channel and endianness
----------------------------------------------------

LodePNG uses unsigned char arrays for 16-bit per channel colors too, just like
for any other color format. The 16-bit values are stored in big endian (most
significant byte first) in these arrays. This is the opposite order of the
little endian used by x86 CPU's.

LodePNG always uses big endian because the PNG file format does so internally.
Conversions to other formats than PNG uses internally are not supported by
LodePNG on purpose, there are myriads of formats, including endianness of 16-bit
colors, the order in which you store R, G, B and A, and so on. Supporting and
converting to/from all that is outside the scope of LodePNG.

This may mean that, depending on your use case, you may want to convert the big
endian output of LodePNG to little endian with a for loop. This is certainly not
always needed, many applications and libraries support big endian 16-bit colors
anyway, but it means you cannot simply cast the unsigned char* buffer to an
unsigned short* buffer on x86 CPUs.

7. error values
---------------

All functions in LodePNG that return an error code, return 0 if everything went
OK, or a non-zero code if there was an error.

The meaning of the LodePNG error values can be retrieved with the function
lodepng_error_text: given the numerical error code, it returns a description
of the error in English as a string.

Check the implementation of lodepng_error_text to see the meaning of each code.

8. chunks and PNG editing
-------------------------

If you want to add extra chunks to a PNG you encode, or use LodePNG for a PNG
editor that should follow the rules about handling of unknown chunks, or if your
program is able to read other types of chunks than the ones handled by LodePNG,
then that's possible with the chunk functions of LodePNG.

A PNG chunk has the following layout:

4 bytes length
4 bytes type name
length bytes data
4 bytes CRC

8.1. iterating through chunks
-----------------------------

If you have a buffer containing the PNG image data, then the first chunk (the
IHDR chunk) starts at byte number 8 of that buffer. The first 8 bytes are the
signature of the PNG and are not part of a chunk. But if you start at byte 8
then you have a chunk, and can check the following things of it.

NOTE: none of these functions check for memory buffer boundaries. To avoid
exploits, always make sure the buffer contains all the data of the chunks.
When using lodepng_chunk_next, make sure the returned value is within the
allocated memory.

unsigned lodepng_chunk_length(const unsigned char* chunk):

Get the length of the chunk's data. The total chunk length is this length + 12.

void lodepng_chunk_type(char type[5], const unsigned char* chunk):
unsigned char lodepng_chunk_type_equals(const unsigned char* chunk, const char* type):

Get the type of the chunk or compare if it's a certain type

unsigned char lodepng_chunk_critical(const unsigned char* chunk):
unsigned char lodepng_chunk_private(const unsigned char* chunk):
unsigned char lodepng_chunk_safetocopy(const unsigned char* chunk):

Check if the chunk is critical in the PNG standard (only IHDR, PLTE, IDAT and IEND are).
Check if the chunk is private (public chunks are part of the standard, private ones not).
Check if the chunk is safe to copy. If it's not, then, when modifying data in a critical
chunk, unsafe to copy chunks of the old image may NOT be saved in the new one if your
program doesn't handle that type of unknown chunk.

unsigned char* lodepng_chunk_data(unsigned char* chunk):
const unsigned char* lodepng_chunk_data_const(const unsigned char* chunk):

Get a pointer to the start of the data of the chunk.

unsigned lodepng_chunk_check_crc(const unsigned char* chunk):
void lodepng_chunk_generate_crc(unsigned char* chunk):

Check if the crc is correct or generate a correct one.

unsigned char* lodepng_chunk_next(unsigned char* chunk):
const unsigned char* lodepng_chunk_next_const(const unsigned char* chunk):

Iterate to the next chunk. This works if you have a buffer with consecutive chunks. Note that these
functions do no boundary checking of the allocated data whatsoever, so make sure there is enough
data available in the buffer to be able to go to the next chunk.

unsigned lodepng_chunk_append(unsigned char** out, size_t* outlength, const unsigned char* chunk):
unsigned lodepng_chunk_create(unsigned char** out, size_t* outlength, unsigned length,
							  const char* type, const unsigned char* data):

These functions are used to create new chunks that are appended to the data in *out that has
length *outlength. The append function appends an existing chunk to the new data. The create
function creates a new chunk with the given parameters and appends it. Type is the 4-letter
name of the chunk.

8.2. chunks in info_png
-----------------------

The LodePNGInfo struct contains fields with the unknown chunk in it. It has 3
buffers (each with size) to contain 3 types of unknown chunks:
the ones that come before the PLTE chunk, the ones that come between the PLTE
and the IDAT chunks, and the ones that come after the IDAT chunks.
It's necessary to make the distionction between these 3 cases because the PNG
standard forces to keep the ordering of unknown chunks compared to the critical
chunks, but does not force any other ordering rules.

info_png.unknown_chunks_data[0] is the chunks before PLTE
info_png.unknown_chunks_data[1] is the chunks after PLTE, before IDAT
info_png.unknown_chunks_data[2] is the chunks after IDAT

The chunks in these 3 buffers can be iterated through and read by using the same
way described in the previous subchapter.

When using the decoder to decode a PNG, you can make it store all unknown chunks
if you set the option settings.remember_unknown_chunks to 1. By default, this
option is off (0).

The encoder will always encode unknown chunks that are stored in the info_png.
If you need it to add a particular chunk that isn't known by LodePNG, you can
use lodepng_chunk_append or lodepng_chunk_create to the chunk data in
info_png.unknown_chunks_data[x].

Chunks that are known by LodePNG should not be added in that way. E.g. to make
LodePNG add a bKGD chunk, set background_defined to true and add the correct
parameters there instead.

9. compiler support
-------------------

No libraries other than the current standard C library are needed to compile
LodePNG. For the C++ version, only the standard C++ library is needed on top.
Add the files lodepng.c(pp) and lodepng.h to your project, include
lodepng.h where needed, and your program can read/write PNG files.

It is compatible with C90 and up, and C++03 and up.

If performance is important, use optimization when compiling! For both the
encoder and decoder, this makes a large difference.

Make sure that LodePNG is compiled with the same compiler of the same version
and with the same settings as the rest of the program, or the interfaces with
std::vectors and std::strings in C++ can be incompatible.

CHAR_BITS must be 8 or higher, because LodePNG uses unsigned chars for octets.

*) gcc and g++

LodePNG is developed in gcc so this compiler is natively supported. It gives no
warnings with compiler options "-Wall -Wextra -pedantic -ansi", with gcc and g++
version 4.7.1 on Linux, 32-bit and 64-bit.

*) Clang

Fully supported and warning-free.

*) Mingw

The Mingw compiler (a port of gcc for Windows) should be fully supported by
LodePNG.

*) Visual Studio and Visual C++ Express Edition

LodePNG should be warning-free with warning level W4. Two warnings were disabled
with pragmas though: warning 4244 about implicit conversions, and warning 4996
where it wants to use a non-standard function fopen_s instead of the standard C
fopen.

Visual Studio may want "stdafx.h" files to be included in each source file and
give an error "unexpected end of file while looking for precompiled header".
This is not standard C++ and will not be added to the stock LodePNG. You can
disable it for lodepng.cpp only by right clicking it, Properties, C/C++,
Precompiled Headers, and set it to Not Using Precompiled Headers there.

NOTE: Modern versions of VS should be fully supported, but old versions, e.g.
VS6, are not guaranteed to work.

*) Compilers on Macintosh

LodePNG has been reported to work both with gcc and LLVM for Macintosh, both for
C and C++.

*) Other Compilers

If you encounter problems on any compilers, feel free to let me know and I may
try to fix it if the compiler is modern and standards complient.

10. examples
------------

This decoder example shows the most basic usage of LodePNG. More complex
examples can be found on the LodePNG website.

10.1. decoder C++ example
-------------------------

#include <iostream>

int main(int argc, char *argv[])
{
  const char* filename = argc > 1 ? argv[1] : "test.png";

  //load and decode
  std::vector<unsigned char> image;
  unsigned width, height;
  unsigned error = lodepng::decode(image, width, height, filename);

  //if there's an error, display it
  if(error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

  //the pixels are now in the vector "image", 4 bytes per pixel, ordered RGBARGBA..., use it as texture, draw it, ...
}

10.2. decoder C example
-----------------------

int main(int argc, char *argv[])
{
  unsigned error;
  unsigned char* image;
  size_t width, height;
  const char* filename = argc > 1 ? argv[1] : "test.png";

  error = lodepng_decode32_file(&image, &width, &height, filename);

  if(error) printf("decoder error %u: %s\n", error, lodepng_error_text(error));

  / * use image here * /

  free(image);
  return 0;
}

11. changes
-----------

The version number of LodePNG is the date of the change given in the format
yyyymmdd.

Some changes aren't backwards compatible. Those are indicated with a (!)
symbol.

*) 23 aug 2014: Reduced needless memory usage of decoder.
*) 28 jun 2014: Removed fix_png setting, always support palette OOB for
	simplicity. Made ColorProfile public.
*) 09 jun 2014: Faster encoder by fixing hash bug and more zeros optimization.
*) 22 dec 2013: Power of two windowsize required for optimization.
*) 15 apr 2013: Fixed bug with LAC_ALPHA and color key.
*) 25 mar 2013: Added an optional feature to ignore some PNG errors (fix_png).
*) 11 mar 2013 (!): Bugfix with custom free. Changed from "my" to "lodepng_"
	prefix for the custom allocators and made it possible with a new #define to
	use custom ones in your project without needing to change lodepng's code.
*) 28 jan 2013: Bugfix with color key.
*) 27 okt 2012: Tweaks in text chunk keyword length error handling.
*) 8 okt 2012 (!): Added new filter strategy (entropy) and new auto color mode.
	(no palette). Better deflate tree encoding. New compression tweak settings.
	Faster color conversions while decoding. Some internal cleanups.
*) 23 sep 2012: Reduced warnings in Visual Studio a little bit.
*) 1 sep 2012 (!): Removed #define's for giving custom (de)compression functions
	and made it work with function pointers instead.
*) 23 jun 2012: Added more filter strategies. Made it easier to use custom alloc
	and free functions and toggle #defines from compiler flags. Small fixes.
*) 6 may 2012 (!): Made plugging in custom zlib/deflate functions more flexible.
*) 22 apr 2012 (!): Made interface more consistent, renaming a lot. Removed
	redundant C++ codec classes. Reduced amount of structs. Everything changed,
	but it is cleaner now imho and functionality remains the same. Also fixed
	several bugs and shrinked the implementation code. Made new samples.
*) 6 nov 2011 (!): By default, the encoder now automatically chooses the best
	PNG color model and bit depth, based on the amount and type of colors of the
	raw image. For this, autoLeaveOutAlphaChannel replaced by auto_choose_color.
*) 9 okt 2011: simpler hash chain implementation for the encoder.
*) 8 sep 2011: lz77 encoder lazy matching instead of greedy matching.
*) 23 aug 2011: tweaked the zlib compression parameters after benchmarking.
	A bug with the PNG filtertype heuristic was fixed, so that it chooses much
	better ones (it's quite significant). A setting to do an experimental, slow,
	brute force search for PNG filter types is added.
*) 17 aug 2011 (!): changed some C zlib related function names.
*) 16 aug 2011: made the code less wide (max 120 characters per line).
*) 17 apr 2011: code cleanup. Bugfixes. Convert low to 16-bit per sample colors.
*) 21 feb 2011: fixed compiling for C90. Fixed compiling with sections disabled.
*) 11 dec 2010: encoding is made faster, based on suggestion by Peter Eastman
	to optimize long sequences of zeros.
*) 13 nov 2010: added LodePNG_InfoColor_hasPaletteAlpha and
	LodePNG_InfoColor_canHaveAlpha functions for convenience.
*) 7 nov 2010: added LodePNG_error_text function to get error code description.
*) 30 okt 2010: made decoding slightly faster
*) 26 okt 2010: (!) changed some C function and struct names (more consistent).
	 Reorganized the documentation and the declaration order in the header.
*) 08 aug 2010: only changed some comments and external samples.
*) 05 jul 2010: fixed bug thanks to warnings in the new gcc version.
*) 14 mar 2010: fixed bug where too much memory was allocated for char buffers.
*) 02 sep 2008: fixed bug where it could create empty tree that linux apps could
	read by ignoring the problem but windows apps couldn't.
*) 06 jun 2008: added more error checks for out of memory cases.
*) 26 apr 2008: added a few more checks here and there to ensure more safety.
*) 06 mar 2008: crash with encoding of strings fixed
*) 02 feb 2008: support for international text chunks added (iTXt)
*) 23 jan 2008: small cleanups, and #defines to divide code in sections
*) 20 jan 2008: support for unknown chunks allowing using LodePNG for an editor.
*) 18 jan 2008: support for tIME and pHYs chunks added to encoder and decoder.
*) 17 jan 2008: ability to encode and decode compressed zTXt chunks added
	Also vareous fixes, such as in the deflate and the padding bits code.
*) 13 jan 2008: Added ability to encode Adam7-interlaced images. Improved
	filtering code of encoder.
*) 07 jan 2008: (!) changed LodePNG to use ISO C90 instead of C++. A
	C++ wrapper around this provides an interface almost identical to before.
	Having LodePNG be pure ISO C90 makes it more portable. The C and C++ code
	are together in these files but it works both for C and C++ compilers.
*) 29 dec 2007: (!) changed most integer types to unsigned int + other tweaks
*) 30 aug 2007: bug fixed which makes this Borland C++ compatible
*) 09 aug 2007: some VS2005 warnings removed again
*) 21 jul 2007: deflate code placed in new namespace separate from zlib code
*) 08 jun 2007: fixed bug with 2- and 4-bit color, and small interlaced images
*) 04 jun 2007: improved support for Visual Studio 2005: crash with accessing
	invalid std::vector element [0] fixed, and level 3 and 4 warnings removed
*) 02 jun 2007: made the encoder add a tag with version by default
*) 27 may 2007: zlib and png code separated (but still in the same file),
	simple encoder/decoder functions added for more simple usage cases
*) 19 may 2007: minor fixes, some code cleaning, new error added (error 69),
	moved some examples from here to lodepng_examples.cpp
*) 12 may 2007: palette decoding bug fixed
*) 24 apr 2007: changed the license from BSD to the zlib license
*) 11 mar 2007: very simple addition: ability to encode bKGD chunks.
*) 04 mar 2007: (!) tEXt chunk related fixes, and support for encoding
	palettized PNG images. Plus little interface change with palette and texts.
*) 03 mar 2007: Made it encode dynamic Huffman shorter with repeat codes.
	Fixed a bug where the end code of a block had length 0 in the Huffman tree.
*) 26 feb 2007: Huffman compression with dynamic trees (BTYPE 2) now implemented
	and supported by the encoder, resulting in smaller PNGs at the output.
*) 27 jan 2007: Made the Adler-32 test faster so that a timewaste is gone.
*) 24 jan 2007: gave encoder an error interface. Added color conversion from any
	greyscale type to 8-bit greyscale with or without alpha.
*) 21 jan 2007: (!) Totally changed the interface. It allows more color types
	to convert to and is more uniform. See the manual for how it works now.
*) 07 jan 2007: Some cleanup & fixes, and a few changes over the last days:
	encode/decode custom tEXt chunks, separate classes for zlib & deflate, and
	at last made the decoder give errors for incorrect Adler32 or Crc.
*) 01 jan 2007: Fixed bug with encoding PNGs with less than 8 bits per channel.
*) 29 dec 2006: Added support for encoding images without alpha channel, and
	cleaned out code as well as making certain parts faster.
*) 28 dec 2006: Added "Settings" to the encoder.
*) 26 dec 2006: The encoder now does LZ77 encoding and produces much smaller files now.
	Removed some code duplication in the decoder. Fixed little bug in an example.
*) 09 dec 2006: (!) Placed output parameters of public functions as first parameter.
	Fixed a bug of the decoder with 16-bit per color.
*) 15 okt 2006: Changed documentation structure
*) 09 okt 2006: Encoder class added. It encodes a valid PNG image from the
	given image buffer, however for now it's not compressed.
*) 08 sep 2006: (!) Changed to interface with a Decoder class
*) 30 jul 2006: (!) LodePNG_InfoPng , width and height are now retrieved in different
	way. Renamed decodePNG to decodePNGGeneric.
*) 29 jul 2006: (!) Changed the interface: image info is now returned as a
	struct of type LodePNG::LodePNG_Info, instead of a vector, which was a bit clumsy.
*) 28 jul 2006: Cleaned the code and added new error checks.
	Corrected terminology "deflate" into "inflate".
*) 23 jun 2006: Added SDL example in the documentation in the header, this
	example allows easy debugging by displaying the PNG and its transparency.
*) 22 jun 2006: (!) Changed way to obtain error value. Added
	loadFile function for convenience. Made decodePNG32 faster.
*) 21 jun 2006: (!) Changed type of info vector to unsigned.
	Changed position of palette in info vector. Fixed an important bug that
	happened on PNGs with an uncompressed block.
*) 16 jun 2006: Internally changed unsigned into unsigned where
	needed, and performed some optimizations.
*) 07 jun 2006: (!) Renamed functions to decodePNG and placed them
	in LodePNG namespace. Changed the order of the parameters. Rewrote the
	documentation in the header. Renamed files to lodepng.cpp and lodepng.h
*) 22 apr 2006: Optimized and improved some code
*) 07 sep 2005: (!) Changed to std::vector interface
*) 12 aug 2005: Initial release (C++, decoder only)

12. contact information
-----------------------

Feel free to contact me with suggestions, problems, comments, ... concerning
LodePNG. If you encounter a PNG image that doesn't work properly with this
decoder, feel free to send it and I'll use it to find and fix the problem.

My email address is (puzzle the account and domain together with an @ symbol):
Domain: gmail dot com.
Account: lode dot vandevenne.

Copyright (c) 2005-2014 Lode Vandevenne
*/


//#line 1 "lodepng.cpp"
/*
The manual and changelog are in the header file "lodepng.h"
Rename this file to lodepng.cpp to use it for C++, or to lodepng.c to use it for C.
*/

#include <stdio.h>
#include <stdlib.h>

#ifdef LODEPNG_COMPILE_CPP
#include <fstream>
#endif /*LODEPNG_COMPILE_CPP*/

#if defined(_MSC_VER) && (_MSC_VER >= 1310) /*Visual Studio: A few warning types are not desired here.*/
#pragma warning( disable : 4244 ) /*implicit conversions: not warned by gcc -Wall -Wextra and requires too much casts*/
#pragma warning( disable : 4996 ) /*VS does not like fopen, but fopen_s is not standard C so unusable here*/
#endif /*_MSC_VER */

/*
This source file is built up in the following large parts. The code sections
with the "LODEPNG_COMPILE_" #defines divide this up further in an intermixed way.
-Tools for C and common code for PNG and Zlib
-C Code for Zlib (huffman, deflate, ...)
-C Code for PNG (file format chunks, adam7, PNG filters, color conversions, ...)
-The C++ wrapper around all of the above
*/

/*The malloc, realloc and free functions defined here with "lodepng_" in front
of the name, so that you can easily change them to others related to your
platform if needed. Everything else in the code calls these. Pass
-DLODEPNG_NO_COMPILE_ALLOCATORS to the compiler, or comment out
#define LODEPNG_COMPILE_ALLOCATORS in the header, to disable the ones here and
define them in your own project's source files without needing to change
lodepng source code. Don't forget to remove "static" if you copypaste them
from here.*/

#ifdef LODEPNG_COMPILE_ALLOCATORS
static void* lodepng_malloc(size_t size)
{
  return malloc(size);
}

static void* lodepng_realloc(void* ptr, size_t new_size)
{
  return realloc(ptr, new_size);
}

static void lodepng_free(void* ptr)
{
  free(ptr);
}
#else /*LODEPNG_COMPILE_ALLOCATORS*/
void* lodepng_malloc(size_t size);
void* lodepng_realloc(void* ptr, size_t new_size);
void lodepng_free(void* ptr);
#endif /*LODEPNG_COMPILE_ALLOCATORS*/

/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */
/* // Tools for C, and common code for PNG and Zlib.                       // */
/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */

/*
Often in case of an error a value is assigned to a variable and then it breaks
out of a loop (to go to the cleanup phase of a function). This macro does that.
It makes the error handling code shorter and more readable.

Example: if(!uivector_resizev(&frequencies_ll, 286, 0)) ERROR_BREAK(83);
*/
#define CERROR_BREAK(errorvar, code)\
{\
  errorvar = code;\
  break;\
}

/*version of CERROR_BREAK that assumes the common case where the error variable is named "error"*/
#define ERROR_BREAK(code) CERROR_BREAK(error, code)

/*Set error var to the error code, and return it.*/
#define CERROR_RETURN_ERROR(errorvar, code)\
{\
  errorvar = code;\
  return code;\
}

/*Try the code, if it returns error, also return the error.*/
#define CERROR_TRY_RETURN(call)\
{\
  unsigned error = call;\
  if(error) return error;\
}

/*Set error var to the error code, and return from the void function.*/
#define CERROR_RETURN(errorvar, code)\
{\
  errorvar = code;\
  return;\
}

/*
About uivector, ucvector and string:
-All of them wrap dynamic arrays or text strings in a similar way.
-LodePNG was originally written in C++. The vectors replace the std::vectors that were used in the C++ version.
-The string tools are made to avoid problems with compilers that declare things like strncat as deprecated.
-They're not used in the interface, only internally in this file as static functions.
-As with many other structs in this file, the init and cleanup functions serve as ctor and dtor.
*/

#ifdef LODEPNG_COMPILE_ZLIB
/*dynamic vector of unsigned ints*/
typedef struct uivector
{
  unsigned* data;
  size_t size; /*size in number of unsigned longs*/
  size_t allocsize; /*allocated size in bytes*/
} uivector;

static void uivector_cleanup(void* p)
{
  ((uivector*)p)->size = ((uivector*)p)->allocsize = 0;
  lodepng_free(((uivector*)p)->data);
  ((uivector*)p)->data = NULL;
}

/*returns 1 if success, 0 if failure ==> nothing done*/
static unsigned uivector_reserve(uivector* p, size_t allocsize)
{
  if(allocsize > p->allocsize)
  {
	size_t newsize = (allocsize > p->allocsize * 2) ? allocsize : (allocsize * 3 / 2);
	void* data = lodepng_realloc(p->data, newsize);
	if(data)
	{
	  p->allocsize = newsize;
	  p->data = (unsigned*)data;
	}
	else return 0; /*error: not enough memory*/
  }
  return 1;
}

/*returns 1 if success, 0 if failure ==> nothing done*/
static unsigned uivector_resize(uivector* p, size_t size)
{
  if(!uivector_reserve(p, size * sizeof(unsigned))) return 0;
  p->size = size;
  return 1; /*success*/
}

/*resize and give all new elements the value*/
static unsigned uivector_resizev(uivector* p, size_t size, unsigned value)
{
  size_t oldsize = p->size, i;
  if(!uivector_resize(p, size)) return 0;
  for(i = oldsize; i < size; ++i) p->data[i] = value;
  return 1;
}

static void uivector_init(uivector* p)
{
  p->data = NULL;
  p->size = p->allocsize = 0;
}

#ifdef LODEPNG_COMPILE_ENCODER
/*returns 1 if success, 0 if failure ==> nothing done*/
static unsigned uivector_push_back(uivector* p, unsigned c)
{
  if(!uivector_resize(p, p->size + 1)) return 0;
  p->data[p->size - 1] = c;
  return 1;
}

/*copy q to p, returns 1 if success, 0 if failure ==> nothing done*/
static unsigned uivector_copy(uivector* p, const uivector* q)
{
  size_t i;
  if(!uivector_resize(p, q->size)) return 0;
  for(i = 0; i != q->size; ++i) p->data[i] = q->data[i];
  return 1;
}
#endif /*LODEPNG_COMPILE_ENCODER*/
#endif /*LODEPNG_COMPILE_ZLIB*/

/* /////////////////////////////////////////////////////////////////////////// */

/*dynamic vector of unsigned chars*/
typedef struct ucvector
{
  unsigned char* data;
  size_t size; /*used size*/
  size_t allocsize; /*allocated size*/
} ucvector;

/*returns 1 if success, 0 if failure ==> nothing done*/
static unsigned ucvector_reserve(ucvector* p, size_t allocsize)
{
  if(allocsize > p->allocsize)
  {
	size_t newsize = (allocsize > p->allocsize * 2) ? allocsize : (allocsize * 3 / 2);
	void* data = lodepng_realloc(p->data, newsize);
	if(data)
	{
	  p->allocsize = newsize;
	  p->data = (unsigned char*)data;
	}
	else return 0; /*error: not enough memory*/
  }
  return 1;
}

/*returns 1 if success, 0 if failure ==> nothing done*/
static unsigned ucvector_resize(ucvector* p, size_t size)
{
  if(!ucvector_reserve(p, size * sizeof(unsigned char))) return 0;
  p->size = size;
  return 1; /*success*/
}

#ifdef LODEPNG_COMPILE_PNG

static void ucvector_cleanup(void* p)
{
  ((ucvector*)p)->size = ((ucvector*)p)->allocsize = 0;
  lodepng_free(((ucvector*)p)->data);
  ((ucvector*)p)->data = NULL;
}

static void ucvector_init(ucvector* p)
{
  p->data = NULL;
  p->size = p->allocsize = 0;
}

#ifdef LODEPNG_COMPILE_DECODER
/*resize and give all new elements the value*/
static unsigned ucvector_resizev(ucvector* p, size_t size, unsigned char value)
{
  size_t oldsize = p->size, i;
  if(!ucvector_resize(p, size)) return 0;
  for(i = oldsize; i < size; ++i) p->data[i] = value;
  return 1;
}
#endif /*LODEPNG_COMPILE_DECODER*/
#endif /*LODEPNG_COMPILE_PNG*/

#ifdef LODEPNG_COMPILE_ZLIB
/*you can both convert from vector to buffer&size and vica versa. If you use
init_buffer to take over a buffer and size, it is not needed to use cleanup*/
static void ucvector_init_buffer(ucvector* p, unsigned char* buffer, size_t size)
{
  p->data = buffer;
  p->allocsize = p->size = size;
}
#endif /*LODEPNG_COMPILE_ZLIB*/

#if (defined(LODEPNG_COMPILE_PNG) && defined(LODEPNG_COMPILE_ANCILLARY_CHUNKS)) || defined(LODEPNG_COMPILE_ENCODER)
/*returns 1 if success, 0 if failure ==> nothing done*/
static unsigned ucvector_push_back(ucvector* p, unsigned char c)
{
  if(!ucvector_resize(p, p->size + 1)) return 0;
  p->data[p->size - 1] = c;
  return 1;
}
#endif /*defined(LODEPNG_COMPILE_PNG) || defined(LODEPNG_COMPILE_ENCODER)*/

/* ////////////////////////////////////////////////////////////////////////// */

#ifdef LODEPNG_COMPILE_PNG
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
/*returns 1 if success, 0 if failure ==> nothing done*/
static unsigned string_resize(char** out, size_t size)
{
  char* data = (char*)lodepng_realloc(*out, size + 1);
  if(data)
  {
	data[size] = 0; /*null termination char*/
	*out = data;
  }
  return data != 0;
}

/*init a {char*, size_t} pair for use as string*/
static void string_init(char** out)
{
  *out = NULL;
  string_resize(out, 0);
}

/*free the above pair again*/
static void string_cleanup(char** out)
{
  lodepng_free(*out);
  *out = NULL;
}

static void string_set(char** out, const char* in)
{
  size_t insize = strlen(in), i;
  if(string_resize(out, insize))
  {
	for(i = 0; i != insize; ++i)
	{
	  (*out)[i] = in[i];
	}
  }
}
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
#endif /*LODEPNG_COMPILE_PNG*/

/* ////////////////////////////////////////////////////////////////////////// */

unsigned lodepng_read32bitInt(const unsigned char* buffer)
{
  return (unsigned)((buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3]);
}

#if defined(LODEPNG_COMPILE_PNG) || defined(LODEPNG_COMPILE_ENCODER)
/*buffer must have at least 4 allocated bytes available*/
static void lodepng_set32bitInt(unsigned char* buffer, unsigned value)
{
  buffer[0] = (unsigned char)((value >> 24) & 0xff);
  buffer[1] = (unsigned char)((value >> 16) & 0xff);
  buffer[2] = (unsigned char)((value >>  8) & 0xff);
  buffer[3] = (unsigned char)((value      ) & 0xff);
}
#endif /*defined(LODEPNG_COMPILE_PNG) || defined(LODEPNG_COMPILE_ENCODER)*/

#ifdef LODEPNG_COMPILE_ENCODER
static void lodepng_add32bitInt(ucvector* buffer, unsigned value)
{
  ucvector_resize(buffer, buffer->size + 4); /*todo: give error if resize failed*/
  lodepng_set32bitInt(&buffer->data[buffer->size - 4], value);
}
#endif /*LODEPNG_COMPILE_ENCODER*/

/* ////////////////////////////////////////////////////////////////////////// */
/* / File IO                                                                / */
/* ////////////////////////////////////////////////////////////////////////// */

#ifdef LODEPNG_COMPILE_DISK

unsigned lodepng_load_file(unsigned char** out, size_t* outsize, const char* filename)
{
  FILE* file;
  long size;

  /*provide some proper output values if error will happen*/
  *out = 0;
  *outsize = 0;

  file = fopen(filename, "rb");
  if(!file) return 78;

  /*get filesize:*/
  fseek(file , 0 , SEEK_END);
  size = ftell(file);
  rewind(file);

  /*read contents of the file into the vector*/
  *outsize = 0;
  *out = (unsigned char*)lodepng_malloc((size_t)size);
  if(size && (*out)) (*outsize) = fread(*out, 1, (size_t)size, file);

  fclose(file);
  if(!(*out) && size) return 83; /*the above malloc failed*/
  return 0;
}

/*write given buffer to the file, overwriting the file, it doesn't append to it.*/
unsigned lodepng_save_file(const unsigned char* buffer, size_t buffersize, const char* filename)
{
  FILE* file;
  file = fopen(filename, "wb" );
  if(!file) return 79;
  fwrite((char*)buffer , 1 , buffersize, file);
  fclose(file);
  return 0;
}

#endif /*LODEPNG_COMPILE_DISK*/

/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */
/* // End of common code and tools. Begin of Zlib related code.            // */
/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */

#ifdef LODEPNG_COMPILE_ZLIB
#ifdef LODEPNG_COMPILE_ENCODER
/*TODO: this ignores potential out of memory errors*/
#define addBitToStream(/*size_t**/ bitpointer, /*ucvector**/ bitstream, /*unsigned char*/ bit)\
{\
  /*add a new byte at the end*/\
  if(((*bitpointer) & 7) == 0) ucvector_push_back(bitstream, (unsigned char)0);\
  /*earlier bit of huffman code is in a lesser significant bit of an earlier byte*/\
  (bitstream->data[bitstream->size - 1]) |= (bit << ((*bitpointer) & 0x7));\
  ++(*bitpointer);\
}

static void addBitsToStream(size_t* bitpointer, ucvector* bitstream, unsigned value, size_t nbits)
{
  size_t i;
  for(i = 0; i != nbits; ++i) addBitToStream(bitpointer, bitstream, (unsigned char)((value >> i) & 1));
}

static void addBitsToStreamReversed(size_t* bitpointer, ucvector* bitstream, unsigned value, size_t nbits)
{
  size_t i;
  for(i = 0; i != nbits; ++i) addBitToStream(bitpointer, bitstream, (unsigned char)((value >> (nbits - 1 - i)) & 1));
}
#endif /*LODEPNG_COMPILE_ENCODER*/

#ifdef LODEPNG_COMPILE_DECODER

#define READBIT(bitpointer, bitstream) ((bitstream[bitpointer >> 3] >> (bitpointer & 0x7)) & (unsigned char)1)

static unsigned char readBitFromStream(size_t* bitpointer, const unsigned char* bitstream)
{
  unsigned char result = (unsigned char)(READBIT(*bitpointer, bitstream));
  ++(*bitpointer);
  return result;
}

static unsigned readBitsFromStream(size_t* bitpointer, const unsigned char* bitstream, size_t nbits)
{
  unsigned result = 0, i;
  for(i = 0; i != nbits; ++i)
  {
	result += ((unsigned)READBIT(*bitpointer, bitstream)) << i;
	++(*bitpointer);
  }
  return result;
}
#endif /*LODEPNG_COMPILE_DECODER*/

/* ////////////////////////////////////////////////////////////////////////// */
/* / Deflate - Huffman                                                      / */
/* ////////////////////////////////////////////////////////////////////////// */

#define FIRST_LENGTH_CODE_INDEX 257
#define LAST_LENGTH_CODE_INDEX 285
/*256 literals, the end code, some length codes, and 2 unused codes*/
#define NUM_DEFLATE_CODE_SYMBOLS 288
/*the distance codes have their own symbols, 30 used, 2 unused*/
#define NUM_DISTANCE_SYMBOLS 32
/*the code length codes. 0-15: code lengths, 16: copy previous 3-6 times, 17: 3-10 zeros, 18: 11-138 zeros*/
#define NUM_CODE_LENGTH_CODES 19

/*the base lengths represented by codes 257-285*/
static const unsigned LENGTHBASE[29]
  = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59,
	 67, 83, 99, 115, 131, 163, 195, 227, 258};

/*the extra bits used by codes 257-285 (added to base length)*/
static const unsigned LENGTHEXTRA[29]
  = {0, 0, 0, 0, 0, 0, 0,  0,  1,  1,  1,  1,  2,  2,  2,  2,  3,  3,  3,  3,
	  4,  4,  4,   4,   5,   5,   5,   5,   0};

/*the base backwards distances (the bits of distance codes appear after length codes and use their own huffman tree)*/
static const unsigned DISTANCEBASE[30]
  = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513,
	 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};

/*the extra bits of backwards distances (added to base)*/
static const unsigned DISTANCEEXTRA[30]
  = {0, 0, 0, 0, 1, 1, 2,  2,  3,  3,  4,  4,  5,  5,   6,   6,   7,   7,   8,
	   8,    9,    9,   10,   10,   11,   11,   12,    12,    13,    13};

/*the order in which "code length alphabet code lengths" are stored, out of this
the huffman tree of the dynamic huffman tree lengths is generated*/
static const unsigned CLCL_ORDER[NUM_CODE_LENGTH_CODES]
  = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

/* ////////////////////////////////////////////////////////////////////////// */

/*
Huffman tree struct, containing multiple representations of the tree
*/
typedef struct HuffmanTree
{
  unsigned* tree2d;
  unsigned* tree1d;
  unsigned* lengths; /*the lengths of the codes of the 1d-tree*/
  unsigned maxbitlen; /*maximum number of bits a single code can get*/
  unsigned numcodes; /*number of symbols in the alphabet = number of codes*/
} HuffmanTree;

/*function used for debug purposes to draw the tree in ascii art with C++*/
/*
static void HuffmanTree_draw(HuffmanTree* tree)
{
  std::cout << "tree. length: " << tree->numcodes << " maxbitlen: " << tree->maxbitlen << std::endl;
  for(size_t i = 0; i != tree->tree1d.size; ++i)
  {
	if(tree->lengths.data[i])
	  std::cout << i << " " << tree->tree1d.data[i] << " " << tree->lengths.data[i] << std::endl;
  }
  std::cout << std::endl;
}*/

static void HuffmanTree_init(HuffmanTree* tree)
{
  tree->tree2d = 0;
  tree->tree1d = 0;
  tree->lengths = 0;
}

static void HuffmanTree_cleanup(HuffmanTree* tree)
{
  lodepng_free(tree->tree2d);
  lodepng_free(tree->tree1d);
  lodepng_free(tree->lengths);
}

/*the tree representation used by the decoder. return value is error*/
static unsigned HuffmanTree_make2DTree(HuffmanTree* tree)
{
  unsigned nodefilled = 0; /*up to which node it is filled*/
  unsigned treepos = 0; /*position in the tree (1 of the numcodes columns)*/
  unsigned n, i;

  tree->tree2d = (unsigned*)lodepng_malloc(tree->numcodes * 2 * sizeof(unsigned));
  if(!tree->tree2d) return 83; /*alloc fail*/

  /*
  convert tree1d[] to tree2d[][]. In the 2D array, a value of 32767 means
  uninited, a value >= numcodes is an address to another bit, a value < numcodes
  is a code. The 2 rows are the 2 possible bit values (0 or 1), there are as
  many columns as codes - 1.
  A good huffmann tree has N * 2 - 1 nodes, of which N - 1 are internal nodes.
  Here, the internal nodes are stored (what their 0 and 1 option point to).
  There is only memory for such good tree currently, if there are more nodes
  (due to too long length codes), error 55 will happen
  */
  for(n = 0; n < tree->numcodes * 2; ++n)
  {
	tree->tree2d[n] = 32767; /*32767 here means the tree2d isn't filled there yet*/
  }

  for(n = 0; n < tree->numcodes; ++n) /*the codes*/
  {
	for(i = 0; i != tree->lengths[n]; ++i) /*the bits for this code*/
	{
	  unsigned char bit = (unsigned char)((tree->tree1d[n] >> (tree->lengths[n] - i - 1)) & 1);
	  /*oversubscribed, see comment in lodepng_error_text*/
	  if(treepos > 2147483647 || treepos + 2 > tree->numcodes) return 55;
	  if(tree->tree2d[2 * treepos + bit] == 32767) /*not yet filled in*/
	  {
		if(i + 1 == tree->lengths[n]) /*last bit*/
		{
		  tree->tree2d[2 * treepos + bit] = n; /*put the current code in it*/
		  treepos = 0;
		}
		else
		{
		  /*put address of the next step in here, first that address has to be found of course
		  (it's just nodefilled + 1)...*/
		  ++nodefilled;
		  /*addresses encoded with numcodes added to it*/
		  tree->tree2d[2 * treepos + bit] = nodefilled + tree->numcodes;
		  treepos = nodefilled;
		}
	  }
	  else treepos = tree->tree2d[2 * treepos + bit] - tree->numcodes;
	}
  }

  for(n = 0; n < tree->numcodes * 2; ++n)
  {
	if(tree->tree2d[n] == 32767) tree->tree2d[n] = 0; /*remove possible remaining 32767's*/
  }

  return 0;
}

/*
Second step for the ...makeFromLengths and ...makeFromFrequencies functions.
numcodes, lengths and maxbitlen must already be filled in correctly. return
value is error.
*/
static unsigned HuffmanTree_makeFromLengths2(HuffmanTree* tree)
{
  uivector blcount;
  uivector nextcode;
  unsigned error = 0;
  unsigned bits, n;

  uivector_init(&blcount);
  uivector_init(&nextcode);

  tree->tree1d = (unsigned*)lodepng_malloc(tree->numcodes * sizeof(unsigned));
  if(!tree->tree1d) error = 83; /*alloc fail*/

  if(!uivector_resizev(&blcount, tree->maxbitlen + 1, 0)
  || !uivector_resizev(&nextcode, tree->maxbitlen + 1, 0))
	error = 83; /*alloc fail*/

  if(!error)
  {
	/*step 1: count number of instances of each code length*/
	for(bits = 0; bits != tree->numcodes; ++bits) ++blcount.data[tree->lengths[bits]];
	/*step 2: generate the nextcode values*/
	for(bits = 1; bits <= tree->maxbitlen; ++bits)
	{
	  nextcode.data[bits] = (nextcode.data[bits - 1] + blcount.data[bits - 1]) << 1;
	}
	/*step 3: generate all the codes*/
	for(n = 0; n != tree->numcodes; ++n)
	{
	  if(tree->lengths[n] != 0) tree->tree1d[n] = nextcode.data[tree->lengths[n]]++;
	}
  }

  uivector_cleanup(&blcount);
  uivector_cleanup(&nextcode);

  if(!error) return HuffmanTree_make2DTree(tree);
  else return error;
}

/*
given the code lengths (as stored in the PNG file), generate the tree as defined
by Deflate. maxbitlen is the maximum bits that a code in the tree can have.
return value is error.
*/
static unsigned HuffmanTree_makeFromLengths(HuffmanTree* tree, const unsigned* bitlen,
											size_t numcodes, unsigned maxbitlen)
{
  unsigned i;
  tree->lengths = (unsigned*)lodepng_malloc(numcodes * sizeof(unsigned));
  if(!tree->lengths) return 83; /*alloc fail*/
  for(i = 0; i != numcodes; ++i) tree->lengths[i] = bitlen[i];
  tree->numcodes = (unsigned)numcodes; /*number of symbols*/
  tree->maxbitlen = maxbitlen;
  return HuffmanTree_makeFromLengths2(tree);
}

#ifdef LODEPNG_COMPILE_ENCODER

/*
A coin, this is the terminology used for the package-merge algorithm and the
coin collector's problem. This is used to generate the huffman tree.
A coin can be multiple coins (when they're merged)
*/
typedef struct Coin
{
  uivector symbols;
  float weight; /*the sum of all weights in this coin*/
} Coin;

static void coin_init(Coin* c)
{
  uivector_init(&c->symbols);
}

/*argument c is void* so that this dtor can be given as function pointer to the vector resize function*/
static void coin_cleanup(void* c)
{
  uivector_cleanup(&((Coin*)c)->symbols);
}

static void coin_copy(Coin* c1, const Coin* c2)
{
  c1->weight = c2->weight;
  uivector_copy(&c1->symbols, &c2->symbols);
}

static void add_coins(Coin* c1, const Coin* c2)
{
  size_t i;
  for(i = 0; i != c2->symbols.size; ++i) uivector_push_back(&c1->symbols, c2->symbols.data[i]);
  c1->weight += c2->weight;
}

static void init_coins(Coin* coins, size_t num)
{
  size_t i;
  for(i = 0; i != num; ++i) coin_init(&coins[i]);
}

static void cleanup_coins(Coin* coins, size_t num)
{
  size_t i;
  for(i = 0; i != num; ++i) coin_cleanup(&coins[i]);
}

static int coin_compare(const void* a, const void* b) {
  float wa = ((const Coin*)a)->weight;
  float wb = ((const Coin*)b)->weight;
  return wa > wb ? 1 : wa < wb ? -1 : 0;
}

static unsigned append_symbol_coins(Coin* coins, const unsigned* frequencies, unsigned numcodes, size_t sum)
{
  unsigned i;
  unsigned j = 0; /*index of present symbols*/
  for(i = 0; i != numcodes; ++i)
  {
	if(frequencies[i] != 0) /*only include symbols that are present*/
	{
	  coins[j].weight = frequencies[i] / (float)sum;
	  uivector_push_back(&coins[j].symbols, i);
	  ++j;
	}
  }
  return 0;
}

unsigned lodepng_huffman_code_lengths(unsigned* lengths, const unsigned* frequencies,
									  size_t numcodes, unsigned maxbitlen)
{
  unsigned i, j;
  size_t sum = 0, numpresent = 0;
  unsigned error = 0;
  Coin* coins; /*the coins of the currently calculated row*/
  Coin* prev_row; /*the previous row of coins*/
  size_t numcoins;
  size_t coinmem;

  if(numcodes == 0) return 80; /*error: a tree of 0 symbols is not supposed to be made*/

  for(i = 0; i != numcodes; ++i)
  {
	if(frequencies[i] > 0)
	{
	  ++numpresent;
	  sum += frequencies[i];
	}
  }

  for(i = 0; i != numcodes; ++i) lengths[i] = 0;

  /*ensure at least two present symbols. There should be at least one symbol
  according to RFC 1951 section 3.2.7. To decoders incorrectly require two. To
  make these work as well ensure there are at least two symbols. The
  Package-Merge code below also doesn't work correctly if there's only one
  symbol, it'd give it the theoritical 0 bits but in practice zlib wants 1 bit*/
  if(numpresent == 0)
  {
	lengths[0] = lengths[1] = 1; /*note that for RFC 1951 section 3.2.7, only lengths[0] = 1 is needed*/
  }
  else if(numpresent == 1)
  {
	for(i = 0; i != numcodes; ++i)
	{
	  if(frequencies[i])
	  {
		lengths[i] = 1;
		lengths[i == 0 ? 1 : 0] = 1;
		break;
	  }
	}
  }
  else
  {
	/*Package-Merge algorithm represented by coin collector's problem
	For every symbol, maxbitlen coins will be created*/

	coinmem = numpresent * 2; /*max amount of coins needed with the current algo*/
	coins = (Coin*)lodepng_malloc(sizeof(Coin) * coinmem);
	prev_row = (Coin*)lodepng_malloc(sizeof(Coin) * coinmem);
	if(!coins || !prev_row)
	{
	  lodepng_free(coins);
	  lodepng_free(prev_row);
	  return 83; /*alloc fail*/
	}
	init_coins(coins, coinmem);
	init_coins(prev_row, coinmem);

	/*first row, lowest denominator*/
	error = append_symbol_coins(coins, frequencies, numcodes, sum);
	numcoins = numpresent;
	qsort(coins, numcoins, sizeof(Coin), coin_compare);
	if(!error)
	{
	  unsigned numprev = 0;
	  for(j = 1; j <= maxbitlen && !error; ++j) /*each of the remaining rows*/
	  {
		unsigned tempnum;
		Coin* tempcoins;
		/*swap prev_row and coins, and their amounts*/
		tempcoins = prev_row; prev_row = coins; coins = tempcoins;
		tempnum = numprev; numprev = numcoins; numcoins = tempnum;

		cleanup_coins(coins, numcoins);
		init_coins(coins, numcoins);

		numcoins = 0;

		/*fill in the merged coins of the previous row*/
		for(i = 0; i + 1 < numprev; i += 2)
		{
		  /*merge prev_row[i] and prev_row[i + 1] into new coin*/
		  Coin* coin = &coins[numcoins++];
		  coin_copy(coin, &prev_row[i]);
		  add_coins(coin, &prev_row[i + 1]);
		}
		/*fill in all the original symbols again*/
		if(j < maxbitlen)
		{
		  error = append_symbol_coins(coins + numcoins, frequencies, numcodes, sum);
		  numcoins += numpresent;
		}
		qsort(coins, numcoins, sizeof(Coin), coin_compare);
	  }
	}

	if(!error)
	{
	  /*calculate the lengths of each symbol, as the amount of times a coin of each symbol is used*/
	  for(i = 0; i + 1 < numpresent; ++i)
	  {
		Coin* coin = &coins[i];
		for(j = 0; j < coin->symbols.size; ++j) ++lengths[coin->symbols.data[j]];
	  }
	}

	cleanup_coins(coins, coinmem);
	lodepng_free(coins);
	cleanup_coins(prev_row, coinmem);
	lodepng_free(prev_row);
  }

  return error;
}

/*Create the Huffman tree given the symbol frequencies*/
static unsigned HuffmanTree_makeFromFrequencies(HuffmanTree* tree, const unsigned* frequencies,
												size_t mincodes, size_t numcodes, unsigned maxbitlen)
{
  unsigned error = 0;
  while(!frequencies[numcodes - 1] && numcodes > mincodes) --numcodes; /*trim zeroes*/
  tree->maxbitlen = maxbitlen;
  tree->numcodes = (unsigned)numcodes; /*number of symbols*/
  tree->lengths = (unsigned*)lodepng_realloc(tree->lengths, numcodes * sizeof(unsigned));
  if(!tree->lengths) return 83; /*alloc fail*/
  /*initialize all lengths to 0*/
  memset(tree->lengths, 0, numcodes * sizeof(unsigned));

  error = lodepng_huffman_code_lengths(tree->lengths, frequencies, numcodes, maxbitlen);
  if(!error) error = HuffmanTree_makeFromLengths2(tree);
  return error;
}

static unsigned HuffmanTree_getCode(const HuffmanTree* tree, unsigned index)
{
  return tree->tree1d[index];
}

static unsigned HuffmanTree_getLength(const HuffmanTree* tree, unsigned index)
{
  return tree->lengths[index];
}
#endif /*LODEPNG_COMPILE_ENCODER*/

/*get the literal and length code tree of a deflated block with fixed tree, as per the deflate specification*/
static unsigned generateFixedLitLenTree(HuffmanTree* tree)
{
  unsigned i, error = 0;
  unsigned* bitlen = (unsigned*)lodepng_malloc(NUM_DEFLATE_CODE_SYMBOLS * sizeof(unsigned));
  if(!bitlen) return 83; /*alloc fail*/

  /*288 possible codes: 0-255=literals, 256=endcode, 257-285=lengthcodes, 286-287=unused*/
  for(i =   0; i <= 143; ++i) bitlen[i] = 8;
  for(i = 144; i <= 255; ++i) bitlen[i] = 9;
  for(i = 256; i <= 279; ++i) bitlen[i] = 7;
  for(i = 280; i <= 287; ++i) bitlen[i] = 8;

  error = HuffmanTree_makeFromLengths(tree, bitlen, NUM_DEFLATE_CODE_SYMBOLS, 15);

  lodepng_free(bitlen);
  return error;
}

/*get the distance code tree of a deflated block with fixed tree, as specified in the deflate specification*/
static unsigned generateFixedDistanceTree(HuffmanTree* tree)
{
  unsigned i, error = 0;
  unsigned* bitlen = (unsigned*)lodepng_malloc(NUM_DISTANCE_SYMBOLS * sizeof(unsigned));
  if(!bitlen) return 83; /*alloc fail*/

  /*there are 32 distance codes, but 30-31 are unused*/
  for(i = 0; i != NUM_DISTANCE_SYMBOLS; ++i) bitlen[i] = 5;
  error = HuffmanTree_makeFromLengths(tree, bitlen, NUM_DISTANCE_SYMBOLS, 15);

  lodepng_free(bitlen);
  return error;
}

#ifdef LODEPNG_COMPILE_DECODER

/*
returns the code, or (unsigned)(-1) if error happened
inbitlength is the length of the complete buffer, in bits (so its byte length times 8)
*/
static unsigned huffmanDecodeSymbol(const unsigned char* in, size_t* bp,
									const HuffmanTree* codetree, size_t inbitlength)
{
  unsigned treepos = 0, ct;
  for(;;)
  {
	if(*bp >= inbitlength) return (unsigned)(-1); /*error: end of input memory reached without endcode*/
	/*
	decode the symbol from the tree. The "readBitFromStream" code is inlined in
	the expression below because this is the biggest bottleneck while decoding
	*/
	ct = codetree->tree2d[(treepos << 1) + READBIT(*bp, in)];
	++(*bp);
	if(ct < codetree->numcodes) return ct; /*the symbol is decoded, return it*/
	else treepos = ct - codetree->numcodes; /*symbol not yet decoded, instead move tree position*/

	if(treepos >= codetree->numcodes) return (unsigned)(-1); /*error: it appeared outside the codetree*/
  }
}
#endif /*LODEPNG_COMPILE_DECODER*/

#ifdef LODEPNG_COMPILE_DECODER

/* ////////////////////////////////////////////////////////////////////////// */
/* / Inflator (Decompressor)                                                / */
/* ////////////////////////////////////////////////////////////////////////// */

/*get the tree of a deflated block with fixed tree, as specified in the deflate specification*/
static void getTreeInflateFixed(HuffmanTree* tree_ll, HuffmanTree* tree_d)
{
  /*TODO: check for out of memory errors*/
  generateFixedLitLenTree(tree_ll);
  generateFixedDistanceTree(tree_d);
}

/*get the tree of a deflated block with dynamic tree, the tree itself is also Huffman compressed with a known tree*/
static unsigned getTreeInflateDynamic(HuffmanTree* tree_ll, HuffmanTree* tree_d,
									  const unsigned char* in, size_t* bp, size_t inlength)
{
  /*make sure that length values that aren't filled in will be 0, or a wrong tree will be generated*/
  unsigned error = 0;
  unsigned n, HLIT, HDIST, HCLEN, i;
  size_t inbitlength = inlength * 8;

  /*see comments in deflateDynamic for explanation of the context and these variables, it is analogous*/
  unsigned* bitlen_ll = 0; /*lit,len code lengths*/
  unsigned* bitlen_d = 0; /*dist code lengths*/
  /*code length code lengths ("clcl"), the bit lengths of the huffman tree used to compress bitlen_ll and bitlen_d*/
  unsigned* bitlen_cl = 0;
  HuffmanTree tree_cl; /*the code tree for code length codes (the huffman tree for compressed huffman trees)*/

  if((*bp) + 14 > (inlength << 3)) return 49; /*error: the bit pointer is or will go past the memory*/

  /*number of literal/length codes + 257. Unlike the spec, the value 257 is added to it here already*/
  HLIT =  readBitsFromStream(bp, in, 5) + 257;
  /*number of distance codes. Unlike the spec, the value 1 is added to it here already*/
  HDIST = readBitsFromStream(bp, in, 5) + 1;
  /*number of code length codes. Unlike the spec, the value 4 is added to it here already*/
  HCLEN = readBitsFromStream(bp, in, 4) + 4;

  if((*bp) + HCLEN * 3 > (inlength << 3)) return 50; /*error: the bit pointer is or will go past the memory*/

  HuffmanTree_init(&tree_cl);

  while(!error)
  {
	/*read the code length codes out of 3 * (amount of code length codes) bits*/

	bitlen_cl = (unsigned*)lodepng_malloc(NUM_CODE_LENGTH_CODES * sizeof(unsigned));
	if(!bitlen_cl) ERROR_BREAK(83 /*alloc fail*/);

	for(i = 0; i != NUM_CODE_LENGTH_CODES; ++i)
	{
	  if(i < HCLEN) bitlen_cl[CLCL_ORDER[i]] = readBitsFromStream(bp, in, 3);
	  else bitlen_cl[CLCL_ORDER[i]] = 0; /*if not, it must stay 0*/
	}

	error = HuffmanTree_makeFromLengths(&tree_cl, bitlen_cl, NUM_CODE_LENGTH_CODES, 7);
	if(error) break;

	/*now we can use this tree to read the lengths for the tree that this function will return*/
	bitlen_ll = (unsigned*)lodepng_malloc(NUM_DEFLATE_CODE_SYMBOLS * sizeof(unsigned));
	bitlen_d = (unsigned*)lodepng_malloc(NUM_DISTANCE_SYMBOLS * sizeof(unsigned));
	if(!bitlen_ll || !bitlen_d) ERROR_BREAK(83 /*alloc fail*/);
	for(i = 0; i != NUM_DEFLATE_CODE_SYMBOLS; ++i) bitlen_ll[i] = 0;
	for(i = 0; i != NUM_DISTANCE_SYMBOLS; ++i) bitlen_d[i] = 0;

	/*i is the current symbol we're reading in the part that contains the code lengths of lit/len and dist codes*/
	i = 0;
	while(i < HLIT + HDIST)
	{
	  unsigned code = huffmanDecodeSymbol(in, bp, &tree_cl, inbitlength);
	  if(code <= 15) /*a length code*/
	  {
		if(i < HLIT) bitlen_ll[i] = code;
		else bitlen_d[i - HLIT] = code;
		++i;
	  }
	  else if(code == 16) /*repeat previous*/
	  {
		unsigned replength = 3; /*read in the 2 bits that indicate repeat length (3-6)*/
		unsigned value; /*set value to the previous code*/

		if (i == 0) ERROR_BREAK(54); /*can't repeat previous if i is 0*/

		if((*bp + 2) > inbitlength) ERROR_BREAK(50); /*error, bit pointer jumps past memory*/
		replength += readBitsFromStream(bp, in, 2);

		if(i < HLIT + 1) value = bitlen_ll[i - 1];
		else value = bitlen_d[i - HLIT - 1];
		/*repeat this value in the next lengths*/
		for(n = 0; n < replength; ++n)
		{
		  if(i >= HLIT + HDIST) ERROR_BREAK(13); /*error: i is larger than the amount of codes*/
		  if(i < HLIT) bitlen_ll[i] = value;
		  else bitlen_d[i - HLIT] = value;
		  ++i;
		}
	  }
	  else if(code == 17) /*repeat "0" 3-10 times*/
	  {
		unsigned replength = 3; /*read in the bits that indicate repeat length*/
		if((*bp + 3) > inbitlength) ERROR_BREAK(50); /*error, bit pointer jumps past memory*/
		replength += readBitsFromStream(bp, in, 3);

		/*repeat this value in the next lengths*/
		for(n = 0; n < replength; ++n)
		{
		  if(i >= HLIT + HDIST) ERROR_BREAK(14); /*error: i is larger than the amount of codes*/

		  if(i < HLIT) bitlen_ll[i] = 0;
		  else bitlen_d[i - HLIT] = 0;
		  ++i;
		}
	  }
	  else if(code == 18) /*repeat "0" 11-138 times*/
	  {
		unsigned replength = 11; /*read in the bits that indicate repeat length*/
		if((*bp + 7) > inbitlength) ERROR_BREAK(50); /*error, bit pointer jumps past memory*/
		replength += readBitsFromStream(bp, in, 7);

		/*repeat this value in the next lengths*/
		for(n = 0; n < replength; ++n)
		{
		  if(i >= HLIT + HDIST) ERROR_BREAK(15); /*error: i is larger than the amount of codes*/

		  if(i < HLIT) bitlen_ll[i] = 0;
		  else bitlen_d[i - HLIT] = 0;
		  ++i;
		}
	  }
	  else /*if(code == (unsigned)(-1))*/ /*huffmanDecodeSymbol returns (unsigned)(-1) in case of error*/
	  {
		if(code == (unsigned)(-1))
		{
		  /*return error code 10 or 11 depending on the situation that happened in huffmanDecodeSymbol
		  (10=no endcode, 11=wrong jump outside of tree)*/
		  error = (*bp) > inbitlength ? 10 : 11;
		}
		else error = 16; /*unexisting code, this can never happen*/
		break;
	  }
	}
	if(error) break;

	if(bitlen_ll[256] == 0) ERROR_BREAK(64); /*the length of the end code 256 must be larger than 0*/

	/*now we've finally got HLIT and HDIST, so generate the code trees, and the function is done*/
	error = HuffmanTree_makeFromLengths(tree_ll, bitlen_ll, NUM_DEFLATE_CODE_SYMBOLS, 15);
	if(error) break;
	error = HuffmanTree_makeFromLengths(tree_d, bitlen_d, NUM_DISTANCE_SYMBOLS, 15);

	break; /*end of error-while*/
  }

  lodepng_free(bitlen_cl);
  lodepng_free(bitlen_ll);
  lodepng_free(bitlen_d);
  HuffmanTree_cleanup(&tree_cl);

  return error;
}

/*inflate a block with dynamic of fixed Huffman tree*/
static unsigned inflateHuffmanBlock(ucvector* out, const unsigned char* in, size_t* bp,
									size_t* pos, size_t inlength, unsigned btype)
{
  unsigned error = 0;
  HuffmanTree tree_ll; /*the huffman tree for literal and length codes*/
  HuffmanTree tree_d; /*the huffman tree for distance codes*/
  size_t inbitlength = inlength * 8;

  HuffmanTree_init(&tree_ll);
  HuffmanTree_init(&tree_d);

  if(btype == 1) getTreeInflateFixed(&tree_ll, &tree_d);
  else if(btype == 2) error = getTreeInflateDynamic(&tree_ll, &tree_d, in, bp, inlength);

  while(!error) /*decode all symbols until end reached, breaks at end code*/
  {
	/*code_ll is literal, length or end code*/
	unsigned code_ll = huffmanDecodeSymbol(in, bp, &tree_ll, inbitlength);
	if(code_ll <= 255) /*literal symbol*/
	{
	  /*ucvector_push_back would do the same, but for some reason the two lines below run 10% faster*/
	  if(!ucvector_resize(out, (*pos) + 1)) ERROR_BREAK(83 /*alloc fail*/);
	  out->data[*pos] = (unsigned char)code_ll;
	  ++(*pos);
	}
	else if(code_ll >= FIRST_LENGTH_CODE_INDEX && code_ll <= LAST_LENGTH_CODE_INDEX) /*length code*/
	{
	  unsigned code_d, distance;
	  unsigned numextrabits_l, numextrabits_d; /*extra bits for length and distance*/
	  size_t start, forward, backward, length;

	  /*part 1: get length base*/
	  length = LENGTHBASE[code_ll - FIRST_LENGTH_CODE_INDEX];

	  /*part 2: get extra bits and add the value of that to length*/
	  numextrabits_l = LENGTHEXTRA[code_ll - FIRST_LENGTH_CODE_INDEX];
	  if((*bp + numextrabits_l) > inbitlength) ERROR_BREAK(51); /*error, bit pointer will jump past memory*/
	  length += readBitsFromStream(bp, in, numextrabits_l);

	  /*part 3: get distance code*/
	  code_d = huffmanDecodeSymbol(in, bp, &tree_d, inbitlength);
	  if(code_d > 29)
	  {
		if(code_ll == (unsigned)(-1)) /*huffmanDecodeSymbol returns (unsigned)(-1) in case of error*/
		{
		  /*return error code 10 or 11 depending on the situation that happened in huffmanDecodeSymbol
		  (10=no endcode, 11=wrong jump outside of tree)*/
		  error = (*bp) > inlength * 8 ? 10 : 11;
		}
		else error = 18; /*error: invalid distance code (30-31 are never used)*/
		break;
	  }
	  distance = DISTANCEBASE[code_d];

	  /*part 4: get extra bits from distance*/
	  numextrabits_d = DISTANCEEXTRA[code_d];
	  if((*bp + numextrabits_d) > inbitlength) ERROR_BREAK(51); /*error, bit pointer will jump past memory*/
	  distance += readBitsFromStream(bp, in, numextrabits_d);

	  /*part 5: fill in all the out[n] values based on the length and dist*/
	  start = (*pos);
	  if(distance > start) ERROR_BREAK(52); /*too long backward distance*/
	  backward = start - distance;

	  if(!ucvector_resize(out, (*pos) + length)) ERROR_BREAK(83 /*alloc fail*/);
	  for(forward = 0; forward < length; ++forward)
	  {
		out->data[(*pos)] = out->data[backward];
		++(*pos);
		++backward;
		if(backward >= start) backward = start - distance;
	  }
	}
	else if(code_ll == 256)
	{
	  break; /*end code, break the loop*/
	}
	else /*if(code == (unsigned)(-1))*/ /*huffmanDecodeSymbol returns (unsigned)(-1) in case of error*/
	{
	  /*return error code 10 or 11 depending on the situation that happened in huffmanDecodeSymbol
	  (10=no endcode, 11=wrong jump outside of tree)*/
	  error = ((*bp) > inlength * 8) ? 10 : 11;
	  break;
	}
  }

  HuffmanTree_cleanup(&tree_ll);
  HuffmanTree_cleanup(&tree_d);

  return error;
}

static unsigned inflateNoCompression(ucvector* out, const unsigned char* in, size_t* bp, size_t* pos, size_t inlength)
{
  size_t p;
  unsigned LEN, NLEN, n, error = 0;

  /*go to first boundary of byte*/
  while(((*bp) & 0x7) != 0) ++(*bp);
  p = (*bp) / 8; /*byte position*/

  /*read LEN (2 bytes) and NLEN (2 bytes)*/
  if(p + 4 >= inlength) return 52; /*error, bit pointer will jump past memory*/
  LEN = in[p] + 256u * in[p + 1]; p += 2;
  NLEN = in[p] + 256u * in[p + 1]; p += 2;

  /*check if 16-bit NLEN is really the one's complement of LEN*/
  if(LEN + NLEN != 65535) return 21; /*error: NLEN is not one's complement of LEN*/

  if(!ucvector_resize(out, (*pos) + LEN)) return 83; /*alloc fail*/

  /*read the literal data: LEN bytes are now stored in the out buffer*/
  if(p + LEN > inlength) return 23; /*error: reading outside of in buffer*/
  for(n = 0; n < LEN; ++n) out->data[(*pos)++] = in[p++];

  (*bp) = p * 8;

  return error;
}

static unsigned lodepng_inflatev(ucvector* out,
								 const unsigned char* in, size_t insize,
								 const LodePNGDecompressSettings* settings)
{
  /*bit pointer in the "in" data, current byte is bp >> 3, current bit is bp & 0x7 (from lsb to msb of the byte)*/
  size_t bp = 0;
  unsigned BFINAL = 0;
  size_t pos = 0; /*byte position in the out buffer*/
  unsigned error = 0;

  (void)settings;

  while(!BFINAL)
  {
	unsigned BTYPE;
	if(bp + 2 >= insize * 8) return 52; /*error, bit pointer will jump past memory*/
	BFINAL = readBitFromStream(&bp, in);
	BTYPE = 1u * readBitFromStream(&bp, in);
	BTYPE += 2u * readBitFromStream(&bp, in);

	if(BTYPE == 3) return 20; /*error: invalid BTYPE*/
	else if(BTYPE == 0) error = inflateNoCompression(out, in, &bp, &pos, insize); /*no compression*/
	else error = inflateHuffmanBlock(out, in, &bp, &pos, insize, BTYPE); /*compression, BTYPE 01 or 10*/

	if(error) return error;
  }

  return error;
}

unsigned lodepng_inflate(unsigned char** out, size_t* outsize,
						 const unsigned char* in, size_t insize,
						 const LodePNGDecompressSettings* settings)
{
  unsigned error;
  ucvector v;
  ucvector_init_buffer(&v, *out, *outsize);
  error = lodepng_inflatev(&v, in, insize, settings);
  *out = v.data;
  *outsize = v.size;
  return error;
}

static unsigned inflate(unsigned char** out, size_t* outsize,
						const unsigned char* in, size_t insize,
						const LodePNGDecompressSettings* settings)
{
  if(settings->custom_inflate)
  {
	return settings->custom_inflate(out, outsize, in, insize, settings);
  }
  else
  {
	return lodepng_inflate(out, outsize, in, insize, settings);
  }
}

#endif /*LODEPNG_COMPILE_DECODER*/

#ifdef LODEPNG_COMPILE_ENCODER

/* ////////////////////////////////////////////////////////////////////////// */
/* / Deflator (Compressor)                                                  / */
/* ////////////////////////////////////////////////////////////////////////// */

static const size_t MAX_SUPPORTED_DEFLATE_LENGTH = 258;

/*bitlen is the size in bits of the code*/
static void addHuffmanSymbol(size_t* bp, ucvector* compressed, unsigned code, unsigned bitlen)
{
  addBitsToStreamReversed(bp, compressed, code, bitlen);
}

/*search the index in the array, that has the largest value smaller than or equal to the given value,
given array must be sorted (if no value is smaller, it returns the size of the given array)*/
static size_t searchCodeIndex(const unsigned* array, size_t array_size, size_t value)
{
  /*linear search implementation*/
  /*for(size_t i = 1; i < array_size; ++i) if(array[i] > value) return i - 1;
  return array_size - 1;*/

  /*binary search implementation (not that much faster) (precondition: array_size > 0)*/
  size_t left  = 1;
  size_t right = array_size - 1;
  while(left <= right)
  {
	size_t mid = (left + right) / 2;
	if(array[mid] <= value) left = mid + 1; /*the value to find is more to the right*/
	else if(array[mid - 1] > value) right = mid - 1; /*the value to find is more to the left*/
	else return mid - 1;
  }
  return array_size - 1;
}

static void addLengthDistance(uivector* values, size_t length, size_t distance)
{
  /*values in encoded vector are those used by deflate:
  0-255: literal bytes
  256: end
  257-285: length/distance pair (length code, followed by extra length bits, distance code, extra distance bits)
  286-287: invalid*/

  unsigned length_code = (unsigned)searchCodeIndex(LENGTHBASE, 29, length);
  unsigned extra_length = (unsigned)(length - LENGTHBASE[length_code]);
  unsigned dist_code = (unsigned)searchCodeIndex(DISTANCEBASE, 30, distance);
  unsigned extra_distance = (unsigned)(distance - DISTANCEBASE[dist_code]);

  uivector_push_back(values, length_code + FIRST_LENGTH_CODE_INDEX);
  uivector_push_back(values, extra_length);
  uivector_push_back(values, dist_code);
  uivector_push_back(values, extra_distance);
}

/*3 bytes of data get encoded into two bytes. The hash cannot use more than 3
bytes as input because 3 is the minimum match length for deflate*/
static const unsigned HASH_NUM_VALUES = 65536;
static const unsigned HASH_BIT_MASK = 65535; /*HASH_NUM_VALUES - 1, but C90 does not like that as initializer*/

typedef struct Hash
{
  int* head; /*hash value to head circular pos - can be outdated if went around window*/
  /*circular pos to prev circular pos*/
  unsigned short* chain;
  int* val; /*circular pos to hash value*/

  /*TODO: do this not only for zeros but for any repeated byte. However for PNG
  it's always going to be the zeros that dominate, so not important for PNG*/
  int* headz; /*similar to head, but for chainz*/
  unsigned short* chainz; /*those with same amount of zeros*/
  unsigned short* zeros; /*length of zeros streak, used as a second hash chain*/
} Hash;

static unsigned hash_init(Hash* hash, unsigned windowsize)
{
  unsigned i;
  hash->head = (int*)lodepng_malloc(sizeof(int) * HASH_NUM_VALUES);
  hash->val = (int*)lodepng_malloc(sizeof(int) * windowsize);
  hash->chain = (unsigned short*)lodepng_malloc(sizeof(unsigned short) * windowsize);

  hash->zeros = (unsigned short*)lodepng_malloc(sizeof(unsigned short) * windowsize);
  hash->headz = (int*)lodepng_malloc(sizeof(int) * (MAX_SUPPORTED_DEFLATE_LENGTH + 1));
  hash->chainz = (unsigned short*)lodepng_malloc(sizeof(unsigned short) * windowsize);

  if(!hash->head || !hash->chain || !hash->val  || !hash->headz|| !hash->chainz || !hash->zeros)
  {
	return 83; /*alloc fail*/
  }

  /*initialize hash table*/
  for(i = 0; i != HASH_NUM_VALUES; ++i) hash->head[i] = -1;
  for(i = 0; i != windowsize; ++i) hash->val[i] = -1;
  for(i = 0; i != windowsize; ++i) hash->chain[i] = i; /*same value as index indicates uninitialized*/

  for(i = 0; i <= MAX_SUPPORTED_DEFLATE_LENGTH; ++i) hash->headz[i] = -1;
  for(i = 0; i != windowsize; ++i) hash->chainz[i] = i; /*same value as index indicates uninitialized*/

  return 0;
}

static void hash_cleanup(Hash* hash)
{
  lodepng_free(hash->head);
  lodepng_free(hash->val);
  lodepng_free(hash->chain);

  lodepng_free(hash->zeros);
  lodepng_free(hash->headz);
  lodepng_free(hash->chainz);
}

static unsigned getHash(const unsigned char* data, size_t size, size_t pos)
{
  unsigned result = 0;
  if (pos + 2 < size)
  {
	/*A simple shift and xor hash is used. Since the data of PNGs is dominated
	by zeroes due to the filters, a better hash does not have a significant
	effect on speed in traversing the chain, and causes more time spend on
	calculating the hash.*/
	result ^= (unsigned)(data[pos + 0] << 0u);
	result ^= (unsigned)(data[pos + 1] << 4u);
	result ^= (unsigned)(data[pos + 2] << 8u);
  } else {
	size_t amount, i;
	if(pos >= size) return 0;
	amount = size - pos;
	for(i = 0; i != amount; ++i) result ^= (unsigned)(data[pos + i] << (i * 8u));
  }
  return result & HASH_BIT_MASK;
}

static unsigned countZeros(const unsigned char* data, size_t size, size_t pos)
{
  const unsigned char* start = data + pos;
  const unsigned char* end = start + MAX_SUPPORTED_DEFLATE_LENGTH;
  if(end > data + size) end = data + size;
  data = start;
  while (data != end && *data == 0) ++data;
  /*subtracting two addresses returned as 32-bit number (max value is MAX_SUPPORTED_DEFLATE_LENGTH)*/
  return (unsigned)(data - start);
}

/*wpos = pos & (windowsize - 1)*/
static void updateHashChain(Hash* hash, size_t wpos, unsigned hashval, unsigned short numzeros)
{
  hash->val[wpos] = (int)hashval;
  if(hash->head[hashval] != -1) hash->chain[wpos] = hash->head[hashval];
  hash->head[hashval] = wpos;

  hash->zeros[wpos] = numzeros;
  if(hash->headz[numzeros] != -1) hash->chainz[wpos] = hash->headz[numzeros];
  hash->headz[numzeros] = wpos;
}

/*
LZ77-encode the data. Return value is error code. The input are raw bytes, the output
is in the form of unsigned integers with codes representing for example literal bytes, or
length/distance pairs.
It uses a hash table technique to let it encode faster. When doing LZ77 encoding, a
sliding window (of windowsize) is used, and all past bytes in that window can be used as
the "dictionary". A brute force search through all possible distances would be slow, and
this hash technique is one out of several ways to speed this up.
*/
static unsigned encodeLZ77(uivector* out, Hash* hash,
						   const unsigned char* in, size_t inpos, size_t insize, unsigned windowsize,
						   unsigned minmatch, unsigned nicematch, unsigned lazymatching)
{
  size_t pos;
  unsigned i, error = 0;
  /*for large window lengths, assume the user wants no compression loss. Otherwise, max hash chain length speedup.*/
  unsigned maxchainlength = windowsize >= 8192 ? windowsize : windowsize / 8;
  unsigned maxlazymatch = windowsize >= 8192 ? MAX_SUPPORTED_DEFLATE_LENGTH : 64;

  unsigned usezeros = 1; /*not sure if setting it to false for windowsize < 8192 is better or worse*/
  unsigned numzeros = 0;

  unsigned offset; /*the offset represents the distance in LZ77 terminology*/
  unsigned length;
  unsigned lazy = 0;
  unsigned lazylength = 0, lazyoffset = 0;
  unsigned hashval;
  unsigned current_offset, current_length;
  unsigned prev_offset;
  const unsigned char *lastptr, *foreptr, *backptr;
  unsigned hashpos;

  if(windowsize == 0 || windowsize > 32768) return 60; /*error: windowsize smaller/larger than allowed*/
  if((windowsize & (windowsize - 1)) != 0) return 90; /*error: must be power of two*/

  if(nicematch > MAX_SUPPORTED_DEFLATE_LENGTH) nicematch = MAX_SUPPORTED_DEFLATE_LENGTH;

  for(pos = inpos; pos < insize; ++pos)
  {
	size_t wpos = pos & (windowsize - 1); /*position for in 'circular' hash buffers*/
	unsigned chainlength = 0;

	hashval = getHash(in, insize, pos);

	if(usezeros && hashval == 0)
	{
	  if (numzeros == 0) numzeros = countZeros(in, insize, pos);
	  else if (pos + numzeros > insize || in[pos + numzeros - 1] != 0) --numzeros;
	}
	else
	{
	  numzeros = 0;
	}

	updateHashChain(hash, wpos, hashval, numzeros);

	/*the length and offset found for the current position*/
	length = 0;
	offset = 0;

	hashpos = hash->chain[wpos];

	lastptr = &in[insize < pos + MAX_SUPPORTED_DEFLATE_LENGTH ? insize : pos + MAX_SUPPORTED_DEFLATE_LENGTH];

	/*search for the longest string*/
	prev_offset = 0;
	for(;;)
	{
	  if(chainlength++ >= maxchainlength) break;
	  current_offset = hashpos <= wpos ? wpos - hashpos : wpos - hashpos + windowsize;

	  if(current_offset < prev_offset) break; /*stop when went completely around the circular buffer*/
	  prev_offset = current_offset;
	  if(current_offset > 0)
	  {
		/*test the next characters*/
		foreptr = &in[pos];
		backptr = &in[pos - current_offset];

		/*common case in PNGs is lots of zeros. Quickly skip over them as a speedup*/
		if(numzeros >= 3)
		{
		  unsigned skip = hash->zeros[hashpos];
		  if(skip > numzeros) skip = numzeros;
		  backptr += skip;
		  foreptr += skip;
		}

		while(foreptr != lastptr && *backptr == *foreptr) /*maximum supported length by deflate is max length*/
		{
		  ++backptr;
		  ++foreptr;
		}
		current_length = (unsigned)(foreptr - &in[pos]);

		if(current_length > length)
		{
		  length = current_length; /*the longest length*/
		  offset = current_offset; /*the offset that is related to this longest length*/
		  /*jump out once a length of max length is found (speed gain). This also jumps
		  out if length is MAX_SUPPORTED_DEFLATE_LENGTH*/
		  if(current_length >= nicematch) break;
		}
	  }

	  if(hashpos == hash->chain[hashpos]) break;

	  if(numzeros >= 3 && length > numzeros) {
		hashpos = hash->chainz[hashpos];
		if(hash->zeros[hashpos] != numzeros) break;
	  } else {
		hashpos = hash->chain[hashpos];
		/*outdated hash value, happens if particular value was not encountered in whole last window*/
		if(hash->val[hashpos] != (int)hashval) break;
	  }
	}

	if(lazymatching)
	{
	  if(!lazy && length >= 3 && length <= maxlazymatch && length < MAX_SUPPORTED_DEFLATE_LENGTH)
	  {
		lazy = 1;
		lazylength = length;
		lazyoffset = offset;
		continue; /*try the next byte*/
	  }
	  if(lazy)
	  {
		lazy = 0;
		if(pos == 0) ERROR_BREAK(81);
		if(length > lazylength + 1)
		{
		  /*push the previous character as literal*/
		  if(!uivector_push_back(out, in[pos - 1])) ERROR_BREAK(83 /*alloc fail*/);
		}
		else
		{
		  length = lazylength;
		  offset = lazyoffset;
		  hash->head[hashval] = -1; /*the same hashchain update will be done, this ensures no wrong alteration*/
		  hash->headz[numzeros] = -1; /*idem*/
		  --pos;
		}
	  }
	}
	if(length >= 3 && offset > windowsize) ERROR_BREAK(86 /*too big (or overflown negative) offset*/);

	/*encode it as length/distance pair or literal value*/
	if(length < 3) /*only lengths of 3 or higher are supported as length/distance pair*/
	{
	  if(!uivector_push_back(out, in[pos])) ERROR_BREAK(83 /*alloc fail*/);
	}
	else if(length < minmatch || (length == 3 && offset > 4096))
	{
	  /*compensate for the fact that longer offsets have more extra bits, a
	  length of only 3 may be not worth it then*/
	  if(!uivector_push_back(out, in[pos])) ERROR_BREAK(83 /*alloc fail*/);
	}
	else
	{
	  addLengthDistance(out, length, offset);
	  for(i = 1; i < length; ++i)
	  {
		++pos;
		wpos = pos & (windowsize - 1);
		hashval = getHash(in, insize, pos);
		if(usezeros && hashval == 0)
		{
		  if (numzeros == 0) numzeros = countZeros(in, insize, pos);
		  else if (pos + numzeros > insize || in[pos + numzeros - 1] != 0) --numzeros;
		}
		else
		{
		  numzeros = 0;
		}
		updateHashChain(hash, wpos, hashval, numzeros);
	  }
	}
  } /*end of the loop through each character of input*/

  return error;
}

/* /////////////////////////////////////////////////////////////////////////// */

static unsigned deflateNoCompression(ucvector* out, const unsigned char* data, size_t datasize)
{
  /*non compressed deflate block data: 1 bit BFINAL,2 bits BTYPE,(5 bits): it jumps to start of next byte,
  2 bytes LEN, 2 bytes NLEN, LEN bytes literal DATA*/

  size_t i, j, numdeflateblocks = (datasize + 65534) / 65535;
  unsigned datapos = 0;
  for(i = 0; i != numdeflateblocks; ++i)
  {
	unsigned BFINAL, BTYPE, LEN, NLEN;
	unsigned char firstbyte;

	BFINAL = (i == numdeflateblocks - 1);
	BTYPE = 0;

	firstbyte = (unsigned char)(BFINAL + ((BTYPE & 1) << 1) + ((BTYPE & 2) << 1));
	ucvector_push_back(out, firstbyte);

	LEN = 65535;
	if(datasize - datapos < 65535) LEN = (unsigned)datasize - datapos;
	NLEN = 65535 - LEN;

	ucvector_push_back(out, (unsigned char)(LEN % 256));
	ucvector_push_back(out, (unsigned char)(LEN / 256));
	ucvector_push_back(out, (unsigned char)(NLEN % 256));
	ucvector_push_back(out, (unsigned char)(NLEN / 256));

	/*Decompressed data*/
	for(j = 0; j < 65535 && datapos < datasize; ++j)
	{
	  ucvector_push_back(out, data[datapos++]);
	}
  }

  return 0;
}

/*
write the lz77-encoded data, which has lit, len and dist codes, to compressed stream using huffman trees.
tree_ll: the tree for lit and len codes.
tree_d: the tree for distance codes.
*/
static void writeLZ77data(size_t* bp, ucvector* out, const uivector* lz77_encoded,
						  const HuffmanTree* tree_ll, const HuffmanTree* tree_d)
{
  size_t i = 0;
  for(i = 0; i != lz77_encoded->size; ++i)
  {
	unsigned val = lz77_encoded->data[i];
	addHuffmanSymbol(bp, out, HuffmanTree_getCode(tree_ll, val), HuffmanTree_getLength(tree_ll, val));
	if(val > 256) /*for a length code, 3 more things have to be added*/
	{
	  unsigned length_index = val - FIRST_LENGTH_CODE_INDEX;
	  unsigned n_length_extra_bits = LENGTHEXTRA[length_index];
	  unsigned length_extra_bits = lz77_encoded->data[++i];

	  unsigned distance_code = lz77_encoded->data[++i];

	  unsigned distance_index = distance_code;
	  unsigned n_distance_extra_bits = DISTANCEEXTRA[distance_index];
	  unsigned distance_extra_bits = lz77_encoded->data[++i];

	  addBitsToStream(bp, out, length_extra_bits, n_length_extra_bits);
	  addHuffmanSymbol(bp, out, HuffmanTree_getCode(tree_d, distance_code),
					   HuffmanTree_getLength(tree_d, distance_code));
	  addBitsToStream(bp, out, distance_extra_bits, n_distance_extra_bits);
	}
  }
}

/*Deflate for a block of type "dynamic", that is, with freely, optimally, created huffman trees*/
static unsigned deflateDynamic(ucvector* out, size_t* bp, Hash* hash,
							   const unsigned char* data, size_t datapos, size_t dataend,
							   const LodePNGCompressSettings* settings, unsigned final)
{
  unsigned error = 0;

  /*
  A block is compressed as follows: The PNG data is lz77 encoded, resulting in
  literal bytes and length/distance pairs. This is then huffman compressed with
  two huffman trees. One huffman tree is used for the lit and len values ("ll"),
  another huffman tree is used for the dist values ("d"). These two trees are
  stored using their code lengths, and to compress even more these code lengths
  are also run-length encoded and huffman compressed. This gives a huffman tree
  of code lengths "cl". The code lenghts used to describe this third tree are
  the code length code lengths ("clcl").
  */

  /*The lz77 encoded data, represented with integers since there will also be length and distance codes in it*/
  uivector lz77_encoded;
  HuffmanTree tree_ll; /*tree for lit,len values*/
  HuffmanTree tree_d; /*tree for distance codes*/
  HuffmanTree tree_cl; /*tree for encoding the code lengths representing tree_ll and tree_d*/
  uivector frequencies_ll; /*frequency of lit,len codes*/
  uivector frequencies_d; /*frequency of dist codes*/
  uivector frequencies_cl; /*frequency of code length codes*/
  uivector bitlen_lld; /*lit,len,dist code lenghts (int bits), literally (without repeat codes).*/
  uivector bitlen_lld_e; /*bitlen_lld encoded with repeat codes (this is a rudemtary run length compression)*/
  /*bitlen_cl is the code length code lengths ("clcl"). The bit lengths of codes to represent tree_cl
  (these are written as is in the file, it would be crazy to compress these using yet another huffman
  tree that needs to be represented by yet another set of code lengths)*/
  uivector bitlen_cl;
  size_t datasize = dataend - datapos;

  /*
  Due to the huffman compression of huffman tree representations ("two levels"), there are some anologies:
  bitlen_lld is to tree_cl what data is to tree_ll and tree_d.
  bitlen_lld_e is to bitlen_lld what lz77_encoded is to data.
  bitlen_cl is to bitlen_lld_e what bitlen_lld is to lz77_encoded.
  */

  unsigned BFINAL = final;
  size_t numcodes_ll, numcodes_d, i;
  unsigned HLIT, HDIST, HCLEN;

  uivector_init(&lz77_encoded);
  HuffmanTree_init(&tree_ll);
  HuffmanTree_init(&tree_d);
  HuffmanTree_init(&tree_cl);
  uivector_init(&frequencies_ll);
  uivector_init(&frequencies_d);
  uivector_init(&frequencies_cl);
  uivector_init(&bitlen_lld);
  uivector_init(&bitlen_lld_e);
  uivector_init(&bitlen_cl);

  /*This while loop never loops due to a break at the end, it is here to
  allow breaking out of it to the cleanup phase on error conditions.*/
  while(!error)
  {
	if(settings->use_lz77)
	{
	  error = encodeLZ77(&lz77_encoded, hash, data, datapos, dataend, settings->windowsize,
						 settings->minmatch, settings->nicematch, settings->lazymatching);
	  if(error) break;
	}
	else
	{
	  if(!uivector_resize(&lz77_encoded, datasize)) ERROR_BREAK(83 /*alloc fail*/);
	  for(i = datapos; i < dataend; ++i) lz77_encoded.data[i] = data[i]; /*no LZ77, but still will be Huffman compressed*/
	}

	if(!uivector_resizev(&frequencies_ll, 286, 0)) ERROR_BREAK(83 /*alloc fail*/);
	if(!uivector_resizev(&frequencies_d, 30, 0)) ERROR_BREAK(83 /*alloc fail*/);

	/*Count the frequencies of lit, len and dist codes*/
	for(i = 0; i != lz77_encoded.size; ++i)
	{
	  unsigned symbol = lz77_encoded.data[i];
	  ++frequencies_ll.data[symbol];
	  if(symbol > 256)
	  {
		unsigned dist = lz77_encoded.data[i + 2];
		++frequencies_d.data[dist];
		i += 3;
	  }
	}
	frequencies_ll.data[256] = 1; /*there will be exactly 1 end code, at the end of the block*/

	/*Make both huffman trees, one for the lit and len codes, one for the dist codes*/
	error = HuffmanTree_makeFromFrequencies(&tree_ll, frequencies_ll.data, 257, frequencies_ll.size, 15);
	if(error) break;
	/*2, not 1, is chosen for mincodes: some buggy PNG decoders require at least 2 symbols in the dist tree*/
	error = HuffmanTree_makeFromFrequencies(&tree_d, frequencies_d.data, 2, frequencies_d.size, 15);
	if(error) break;

	numcodes_ll = tree_ll.numcodes; if(numcodes_ll > 286) numcodes_ll = 286;
	numcodes_d = tree_d.numcodes; if(numcodes_d > 30) numcodes_d = 30;
	/*store the code lengths of both generated trees in bitlen_lld*/
	for(i = 0; i != numcodes_ll; ++i) uivector_push_back(&bitlen_lld, HuffmanTree_getLength(&tree_ll, (unsigned)i));
	for(i = 0; i != numcodes_d; ++i) uivector_push_back(&bitlen_lld, HuffmanTree_getLength(&tree_d, (unsigned)i));

	/*run-length compress bitlen_ldd into bitlen_lld_e by using repeat codes 16 (copy length 3-6 times),
	17 (3-10 zeroes), 18 (11-138 zeroes)*/
	for(i = 0; i != (unsigned)bitlen_lld.size; ++i)
	{
	  unsigned j = 0; /*amount of repititions*/
	  while(i + j + 1 < (unsigned)bitlen_lld.size && bitlen_lld.data[i + j + 1] == bitlen_lld.data[i]) ++j;

	  if(bitlen_lld.data[i] == 0 && j >= 2) /*repeat code for zeroes*/
	  {
		++j; /*include the first zero*/
		if(j <= 10) /*repeat code 17 supports max 10 zeroes*/
		{
		  uivector_push_back(&bitlen_lld_e, 17);
		  uivector_push_back(&bitlen_lld_e, j - 3);
		}
		else /*repeat code 18 supports max 138 zeroes*/
		{
		  if(j > 138) j = 138;
		  uivector_push_back(&bitlen_lld_e, 18);
		  uivector_push_back(&bitlen_lld_e, j - 11);
		}
		i += (j - 1);
	  }
	  else if(j >= 3) /*repeat code for value other than zero*/
	  {
		size_t k;
		unsigned num = j / 6, rest = j % 6;
		uivector_push_back(&bitlen_lld_e, bitlen_lld.data[i]);
		for(k = 0; k < num; ++k)
		{
		  uivector_push_back(&bitlen_lld_e, 16);
		  uivector_push_back(&bitlen_lld_e, 6 - 3);
		}
		if(rest >= 3)
		{
		  uivector_push_back(&bitlen_lld_e, 16);
		  uivector_push_back(&bitlen_lld_e, rest - 3);
		}
		else j -= rest;
		i += j;
	  }
	  else /*too short to benefit from repeat code*/
	  {
		uivector_push_back(&bitlen_lld_e, bitlen_lld.data[i]);
	  }
	}

	/*generate tree_cl, the huffmantree of huffmantrees*/

	if(!uivector_resizev(&frequencies_cl, NUM_CODE_LENGTH_CODES, 0)) ERROR_BREAK(83 /*alloc fail*/);
	for(i = 0; i != bitlen_lld_e.size; ++i)
	{
	  ++frequencies_cl.data[bitlen_lld_e.data[i]];
	  /*after a repeat code come the bits that specify the number of repetitions,
	  those don't need to be in the frequencies_cl calculation*/
	  if(bitlen_lld_e.data[i] >= 16) ++i;
	}

	error = HuffmanTree_makeFromFrequencies(&tree_cl, frequencies_cl.data,
											frequencies_cl.size, frequencies_cl.size, 7);
	if(error) break;

	if(!uivector_resize(&bitlen_cl, tree_cl.numcodes)) ERROR_BREAK(83 /*alloc fail*/);
	for(i = 0; i != tree_cl.numcodes; ++i)
	{
	  /*lenghts of code length tree is in the order as specified by deflate*/
	  bitlen_cl.data[i] = HuffmanTree_getLength(&tree_cl, CLCL_ORDER[i]);
	}
	while(bitlen_cl.data[bitlen_cl.size - 1] == 0 && bitlen_cl.size > 4)
	{
	  /*remove zeros at the end, but minimum size must be 4*/
	  if(!uivector_resize(&bitlen_cl, bitlen_cl.size - 1)) ERROR_BREAK(83 /*alloc fail*/);
	}
	if(error) break;

	/*
	Write everything into the output

	After the BFINAL and BTYPE, the dynamic block consists out of the following:
	- 5 bits HLIT, 5 bits HDIST, 4 bits HCLEN
	- (HCLEN+4)*3 bits code lengths of code length alphabet
	- HLIT + 257 code lenghts of lit/length alphabet (encoded using the code length
	  alphabet, + possible repetition codes 16, 17, 18)
	- HDIST + 1 code lengths of distance alphabet (encoded using the code length
	  alphabet, + possible repetition codes 16, 17, 18)
	- compressed data
	- 256 (end code)
	*/

	/*Write block type*/
	addBitToStream(bp, out, BFINAL);
	addBitToStream(bp, out, 0); /*first bit of BTYPE "dynamic"*/
	addBitToStream(bp, out, 1); /*second bit of BTYPE "dynamic"*/

	/*write the HLIT, HDIST and HCLEN values*/
	HLIT = (unsigned)(numcodes_ll - 257);
	HDIST = (unsigned)(numcodes_d - 1);
	HCLEN = (unsigned)bitlen_cl.size - 4;
	/*trim zeroes for HCLEN. HLIT and HDIST were already trimmed at tree creation*/
	while(!bitlen_cl.data[HCLEN + 4 - 1] && HCLEN > 0) --HCLEN;
	addBitsToStream(bp, out, HLIT, 5);
	addBitsToStream(bp, out, HDIST, 5);
	addBitsToStream(bp, out, HCLEN, 4);

	/*write the code lenghts of the code length alphabet*/
	for(i = 0; i != HCLEN + 4; ++i) addBitsToStream(bp, out, bitlen_cl.data[i], 3);

	/*write the lenghts of the lit/len AND the dist alphabet*/
	for(i = 0; i != bitlen_lld_e.size; ++i)
	{
	  addHuffmanSymbol(bp, out, HuffmanTree_getCode(&tree_cl, bitlen_lld_e.data[i]),
					   HuffmanTree_getLength(&tree_cl, bitlen_lld_e.data[i]));
	  /*extra bits of repeat codes*/
	  if(bitlen_lld_e.data[i] == 16) addBitsToStream(bp, out, bitlen_lld_e.data[++i], 2);
	  else if(bitlen_lld_e.data[i] == 17) addBitsToStream(bp, out, bitlen_lld_e.data[++i], 3);
	  else if(bitlen_lld_e.data[i] == 18) addBitsToStream(bp, out, bitlen_lld_e.data[++i], 7);
	}

	/*write the compressed data symbols*/
	writeLZ77data(bp, out, &lz77_encoded, &tree_ll, &tree_d);
	/*error: the length of the end code 256 must be larger than 0*/
	if(HuffmanTree_getLength(&tree_ll, 256) == 0) ERROR_BREAK(64);

	/*write the end code*/
	addHuffmanSymbol(bp, out, HuffmanTree_getCode(&tree_ll, 256), HuffmanTree_getLength(&tree_ll, 256));

	break; /*end of error-while*/
  }

  /*cleanup*/
  uivector_cleanup(&lz77_encoded);
  HuffmanTree_cleanup(&tree_ll);
  HuffmanTree_cleanup(&tree_d);
  HuffmanTree_cleanup(&tree_cl);
  uivector_cleanup(&frequencies_ll);
  uivector_cleanup(&frequencies_d);
  uivector_cleanup(&frequencies_cl);
  uivector_cleanup(&bitlen_lld_e);
  uivector_cleanup(&bitlen_lld);
  uivector_cleanup(&bitlen_cl);

  return error;
}

static unsigned deflateFixed(ucvector* out, size_t* bp, Hash* hash,
							 const unsigned char* data,
							 size_t datapos, size_t dataend,
							 const LodePNGCompressSettings* settings, unsigned final)
{
  HuffmanTree tree_ll; /*tree for literal values and length codes*/
  HuffmanTree tree_d; /*tree for distance codes*/

  unsigned BFINAL = final;
  unsigned error = 0;
  size_t i;

  HuffmanTree_init(&tree_ll);
  HuffmanTree_init(&tree_d);

  generateFixedLitLenTree(&tree_ll);
  generateFixedDistanceTree(&tree_d);

  addBitToStream(bp, out, BFINAL);
  addBitToStream(bp, out, 1); /*first bit of BTYPE*/
  addBitToStream(bp, out, 0); /*second bit of BTYPE*/

  if(settings->use_lz77) /*LZ77 encoded*/
  {
	uivector lz77_encoded;
	uivector_init(&lz77_encoded);
	error = encodeLZ77(&lz77_encoded, hash, data, datapos, dataend, settings->windowsize,
					   settings->minmatch, settings->nicematch, settings->lazymatching);
	if(!error) writeLZ77data(bp, out, &lz77_encoded, &tree_ll, &tree_d);
	uivector_cleanup(&lz77_encoded);
  }
  else /*no LZ77, but still will be Huffman compressed*/
  {
	for(i = datapos; i < dataend; ++i)
	{
	  addHuffmanSymbol(bp, out, HuffmanTree_getCode(&tree_ll, data[i]), HuffmanTree_getLength(&tree_ll, data[i]));
	}
  }
  /*add END code*/
  if(!error) addHuffmanSymbol(bp, out, HuffmanTree_getCode(&tree_ll, 256), HuffmanTree_getLength(&tree_ll, 256));

  /*cleanup*/
  HuffmanTree_cleanup(&tree_ll);
  HuffmanTree_cleanup(&tree_d);

  return error;
}

static unsigned lodepng_deflatev(ucvector* out, const unsigned char* in, size_t insize,
								 const LodePNGCompressSettings* settings)
{
  unsigned error = 0;
  size_t i, blocksize, numdeflateblocks;
  size_t bp = 0; /*the bit pointer*/
  Hash hash;

  if(settings->btype > 2) return 61;
  else if(settings->btype == 0) return deflateNoCompression(out, in, insize);
  else if(settings->btype == 1) blocksize = insize;
  else /*if(settings->btype == 2)*/
  {
	blocksize = insize / 8 + 8;
	if(blocksize < 65535) blocksize = 65535;
  }

  numdeflateblocks = (insize + blocksize - 1) / blocksize;
  if(numdeflateblocks == 0) numdeflateblocks = 1;

  error = hash_init(&hash, settings->windowsize);
  if(error) return error;

  for(i = 0; i != numdeflateblocks && !error; ++i)
  {
	unsigned final = (i == numdeflateblocks - 1);
	size_t start = i * blocksize;
	size_t end = start + blocksize;
	if(end > insize) end = insize;

	if(settings->btype == 1) error = deflateFixed(out, &bp, &hash, in, start, end, settings, final);
	else if(settings->btype == 2) error = deflateDynamic(out, &bp, &hash, in, start, end, settings, final);
  }

  hash_cleanup(&hash);

  return error;
}

unsigned lodepng_deflate(unsigned char** out, size_t* outsize,
						 const unsigned char* in, size_t insize,
						 const LodePNGCompressSettings* settings)
{
  unsigned error;
  ucvector v;
  ucvector_init_buffer(&v, *out, *outsize);
  error = lodepng_deflatev(&v, in, insize, settings);
  *out = v.data;
  *outsize = v.size;
  return error;
}

static unsigned deflate(unsigned char** out, size_t* outsize,
						const unsigned char* in, size_t insize,
						const LodePNGCompressSettings* settings)
{
  if(settings->custom_deflate)
  {
	return settings->custom_deflate(out, outsize, in, insize, settings);
  }
  else
  {
	return lodepng_deflate(out, outsize, in, insize, settings);
  }
}

#endif /*LODEPNG_COMPILE_DECODER*/

/* ////////////////////////////////////////////////////////////////////////// */
/* / Adler32                                                                  */
/* ////////////////////////////////////////////////////////////////////////// */

static unsigned update_adler32(unsigned adler, const unsigned char* data, unsigned len)
{
   unsigned s1 = adler & 0xffff;
   unsigned s2 = (adler >> 16) & 0xffff;

  while(len > 0)
  {
	/*at least 5550 sums can be done before the sums overflow, saving a lot of module divisions*/
	unsigned amount = len > 5550 ? 5550 : len;
	len -= amount;
	while(amount > 0)
	{
	  s1 += (*data++);
	  s2 += s1;
	  --amount;
	}
	s1 %= 65521;
	s2 %= 65521;
  }

  return (s2 << 16) | s1;
}

/*Return the adler32 of the bytes data[0..len-1]*/
static unsigned adler32(const unsigned char* data, unsigned len)
{
  return update_adler32(1L, data, len);
}

/* ////////////////////////////////////////////////////////////////////////// */
/* / Zlib                                                                   / */
/* ////////////////////////////////////////////////////////////////////////// */

#ifdef LODEPNG_COMPILE_DECODER

unsigned lodepng_zlib_decompress(unsigned char** out, size_t* outsize, const unsigned char* in,
								 size_t insize, const LodePNGDecompressSettings* settings)
{
  unsigned error = 0;
  unsigned CM, CINFO, FDICT;

  if(insize < 2) return 53; /*error, size of zlib data too small*/
  /*read information from zlib header*/
  if((in[0] * 256 + in[1]) % 31 != 0)
  {
	/*error: 256 * in[0] + in[1] must be a multiple of 31, the FCHECK value is supposed to be made that way*/
	return 24;
  }

  CM = in[0] & 15;
  CINFO = (in[0] >> 4) & 15;
  /*FCHECK = in[1] & 31;*/ /*FCHECK is already tested above*/
  FDICT = (in[1] >> 5) & 1;
  /*FLEVEL = (in[1] >> 6) & 3;*/ /*FLEVEL is not used here*/

  if(CM != 8 || CINFO > 7)
  {
	/*error: only compression method 8: inflate with sliding window of 32k is supported by the PNG spec*/
	return 25;
  }
  if(FDICT != 0)
  {
	/*error: the specification of PNG says about the zlib stream:
	  "The additional flags shall not specify a preset dictionary."*/
	return 26;
  }

  error = inflate(out, outsize, in + 2, insize - 2, settings);
  if(error) return error;

  if(!settings->ignore_adler32)
  {
	unsigned ADLER32 = lodepng_read32bitInt(&in[insize - 4]);
	unsigned checksum = adler32(*out, (unsigned)(*outsize));
	if(checksum != ADLER32) return 58; /*error, adler checksum not correct, data must be corrupted*/
  }

  return 0; /*no error*/
}

static unsigned zlib_decompress(unsigned char** out, size_t* outsize, const unsigned char* in,
								size_t insize, const LodePNGDecompressSettings* settings)
{
  if(settings->custom_zlib)
  {
	return settings->custom_zlib(out, outsize, in, insize, settings);
  }
  else
  {
	return lodepng_zlib_decompress(out, outsize, in, insize, settings);
  }
}

#endif /*LODEPNG_COMPILE_DECODER*/

#ifdef LODEPNG_COMPILE_ENCODER

unsigned lodepng_zlib_compress(unsigned char** out, size_t* outsize, const unsigned char* in,
							   size_t insize, const LodePNGCompressSettings* settings)
{
  /*initially, *out must be NULL and outsize 0, if you just give some random *out
  that's pointing to a non allocated buffer, this'll crash*/
  ucvector outv;
  size_t i;
  unsigned error;
  unsigned char* deflatedata = 0;
  size_t deflatesize = 0;

  /*zlib data: 1 byte CMF (CM+CINFO), 1 byte FLG, deflate data, 4 byte ADLER32 checksum of the Decompressed data*/
  unsigned CMF = 120; /*0b01111000: CM 8, CINFO 7. With CINFO 7, any window size up to 32768 can be used.*/
  unsigned FLEVEL = 0;
  unsigned FDICT = 0;
  unsigned CMFFLG = 256 * CMF + FDICT * 32 + FLEVEL * 64;
  unsigned FCHECK = 31 - CMFFLG % 31;
  CMFFLG += FCHECK;

  /*ucvector-controlled version of the output buffer, for dynamic array*/
  ucvector_init_buffer(&outv, *out, *outsize);

  ucvector_push_back(&outv, (unsigned char)(CMFFLG / 256));
  ucvector_push_back(&outv, (unsigned char)(CMFFLG % 256));

  error = deflate(&deflatedata, &deflatesize, in, insize, settings);

  if(!error)
  {
	unsigned ADLER32 = adler32(in, (unsigned)insize);
	for(i = 0; i != deflatesize; ++i) ucvector_push_back(&outv, deflatedata[i]);
	lodepng_free(deflatedata);
	lodepng_add32bitInt(&outv, ADLER32);
  }

  *out = outv.data;
  *outsize = outv.size;

  return error;
}

/* compress using the default or custom zlib function */
static unsigned zlib_compress(unsigned char** out, size_t* outsize, const unsigned char* in,
							  size_t insize, const LodePNGCompressSettings* settings)
{
  if(settings->custom_zlib)
  {
	return settings->custom_zlib(out, outsize, in, insize, settings);
  }
  else
  {
	return lodepng_zlib_compress(out, outsize, in, insize, settings);
  }
}

#endif /*LODEPNG_COMPILE_ENCODER*/

#else /*no LODEPNG_COMPILE_ZLIB*/

#ifdef LODEPNG_COMPILE_DECODER
static unsigned zlib_decompress(unsigned char** out, size_t* outsize, const unsigned char* in,
								size_t insize, const LodePNGDecompressSettings* settings)
{
  if (!settings->custom_zlib) return 87; /*no custom zlib function provided */
  return settings->custom_zlib(out, outsize, in, insize, settings);
}
#endif /*LODEPNG_COMPILE_DECODER*/
#ifdef LODEPNG_COMPILE_ENCODER
static unsigned zlib_compress(unsigned char** out, size_t* outsize, const unsigned char* in,
							  size_t insize, const LodePNGCompressSettings* settings)
{
  if (!settings->custom_zlib) return 87; /*no custom zlib function provided */
  return settings->custom_zlib(out, outsize, in, insize, settings);
}
#endif /*LODEPNG_COMPILE_ENCODER*/

#endif /*LODEPNG_COMPILE_ZLIB*/

/* ////////////////////////////////////////////////////////////////////////// */

#ifdef LODEPNG_COMPILE_ENCODER

/*this is a good tradeoff between speed and compression ratio*/
#define DEFAULT_WINDOWSIZE 2048

void lodepng_compress_settings_init(LodePNGCompressSettings* settings)
{
  /*compress with dynamic huffman tree (not in the mathematical sense, just not the predefined one)*/
  settings->btype = 2;
  settings->use_lz77 = 1;
  settings->windowsize = DEFAULT_WINDOWSIZE;
  settings->minmatch = 3;
  settings->nicematch = 128;
  settings->lazymatching = 1;

  settings->custom_zlib = 0;
  settings->custom_deflate = 0;
  settings->custom_context = 0;
}

const LodePNGCompressSettings lodepng_default_compress_settings = {2, 1, DEFAULT_WINDOWSIZE, 3, 128, 1, 0, 0, 0};

#endif /*LODEPNG_COMPILE_ENCODER*/

#ifdef LODEPNG_COMPILE_DECODER

void lodepng_decompress_settings_init(LodePNGDecompressSettings* settings)
{
  settings->ignore_adler32 = 0;

  settings->custom_zlib = 0;
  settings->custom_inflate = 0;
  settings->custom_context = 0;
}

const LodePNGDecompressSettings lodepng_default_decompress_settings = {0, 0, 0, 0};

#endif /*LODEPNG_COMPILE_DECODER*/

/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */
/* // End of Zlib related code. Begin of PNG related code.                 // */
/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */

#ifdef LODEPNG_COMPILE_PNG

/* ////////////////////////////////////////////////////////////////////////// */
/* / CRC32                                                                  / */
/* ////////////////////////////////////////////////////////////////////////// */

/* CRC polynomial: 0xedb88320 */
static unsigned lodepng_crc32_table[256] = {
		   0u, 1996959894u, 3993919788u, 2567524794u,  124634137u, 1886057615u, 3915621685u, 2657392035u,
   249268274u, 2044508324u, 3772115230u, 2547177864u,  162941995u, 2125561021u, 3887607047u, 2428444049u,
   498536548u, 1789927666u, 4089016648u, 2227061214u,  450548861u, 1843258603u, 4107580753u, 2211677639u,
   325883990u, 1684777152u, 4251122042u, 2321926636u,  335633487u, 1661365465u, 4195302755u, 2366115317u,
   997073096u, 1281953886u, 3579855332u, 2724688242u, 1006888145u, 1258607687u, 3524101629u, 2768942443u,
   901097722u, 1119000684u, 3686517206u, 2898065728u,  853044451u, 1172266101u, 3705015759u, 2882616665u,
   651767980u, 1373503546u, 3369554304u, 3218104598u,  565507253u, 1454621731u, 3485111705u, 3099436303u,
   671266974u, 1594198024u, 3322730930u, 2970347812u,  795835527u, 1483230225u, 3244367275u, 3060149565u,
  1994146192u,   31158534u, 2563907772u, 4023717930u, 1907459465u,  112637215u, 2680153253u, 3904427059u,
  2013776290u,  251722036u, 2517215374u, 3775830040u, 2137656763u,  141376813u, 2439277719u, 3865271297u,
  1802195444u,  476864866u, 2238001368u, 4066508878u, 1812370925u,  453092731u, 2181625025u, 4111451223u,
  1706088902u,  314042704u, 2344532202u, 4240017532u, 1658658271u,  366619977u, 2362670323u, 4224994405u,
  1303535960u,  984961486u, 2747007092u, 3569037538u, 1256170817u, 1037604311u, 2765210733u, 3554079995u,
  1131014506u,  879679996u, 2909243462u, 3663771856u, 1141124467u,  855842277u, 2852801631u, 3708648649u,
  1342533948u,  654459306u, 3188396048u, 3373015174u, 1466479909u,  544179635u, 3110523913u, 3462522015u,
  1591671054u,  702138776u, 2966460450u, 3352799412u, 1504918807u,  783551873u, 3082640443u, 3233442989u,
  3988292384u, 2596254646u,   62317068u, 1957810842u, 3939845945u, 2647816111u,   81470997u, 1943803523u,
  3814918930u, 2489596804u,  225274430u, 2053790376u, 3826175755u, 2466906013u,  167816743u, 2097651377u,
  4027552580u, 2265490386u,  503444072u, 1762050814u, 4150417245u, 2154129355u,  426522225u, 1852507879u,
  4275313526u, 2312317920u,  282753626u, 1742555852u, 4189708143u, 2394877945u,  397917763u, 1622183637u,
  3604390888u, 2714866558u,  953729732u, 1340076626u, 3518719985u, 2797360999u, 1068828381u, 1219638859u,
  3624741850u, 2936675148u,  906185462u, 1090812512u, 3747672003u, 2825379669u,  829329135u, 1181335161u,
  3412177804u, 3160834842u,  628085408u, 1382605366u, 3423369109u, 3138078467u,  570562233u, 1426400815u,
  3317316542u, 2998733608u,  733239954u, 1555261956u, 3268935591u, 3050360625u,  752459403u, 1541320221u,
  2607071920u, 3965973030u, 1969922972u,   40735498u, 2617837225u, 3943577151u, 1913087877u,   83908371u,
  2512341634u, 3803740692u, 2075208622u,  213261112u, 2463272603u, 3855990285u, 2094854071u,  198958881u,
  2262029012u, 4057260610u, 1759359992u,  534414190u, 2176718541u, 4139329115u, 1873836001u,  414664567u,
  2282248934u, 4279200368u, 1711684554u,  285281116u, 2405801727u, 4167216745u, 1634467795u,  376229701u,
  2685067896u, 3608007406u, 1308918612u,  956543938u, 2808555105u, 3495958263u, 1231636301u, 1047427035u,
  2932959818u, 3654703836u, 1088359270u,  936918000u, 2847714899u, 3736837829u, 1202900863u,  817233897u,
  3183342108u, 3401237130u, 1404277552u,  615818150u, 3134207493u, 3453421203u, 1423857449u,  601450431u,
  3009837614u, 3294710456u, 1567103746u,  711928724u, 3020668471u, 3272380065u, 1510334235u,  755167117u
};

/*Return the CRC of the bytes buf[0..len-1].*/
unsigned lodepng_crc32(const unsigned char* buf, size_t len)
{
  unsigned c = 0xffffffffL;
  size_t n;

  for(n = 0; n < len; ++n)
  {
	c = lodepng_crc32_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
  }
  return c ^ 0xffffffffL;
}

/* ////////////////////////////////////////////////////////////////////////// */
/* / Reading and writing single bits and bytes from/to stream for LodePNG   / */
/* ////////////////////////////////////////////////////////////////////////// */

static unsigned char readBitFromReversedStream(size_t* bitpointer, const unsigned char* bitstream)
{
  unsigned char result = (unsigned char)((bitstream[(*bitpointer) >> 3] >> (7 - ((*bitpointer) & 0x7))) & 1);
  ++(*bitpointer);
  return result;
}

static unsigned readBitsFromReversedStream(size_t* bitpointer, const unsigned char* bitstream, size_t nbits)
{
  unsigned result = 0;
  size_t i;
  for(i = nbits - 1; i < nbits; --i)
  {
	result += (unsigned)readBitFromReversedStream(bitpointer, bitstream) << i;
  }
  return result;
}

#ifdef LODEPNG_COMPILE_DECODER
static void setBitOfReversedStream0(size_t* bitpointer, unsigned char* bitstream, unsigned char bit)
{
  /*the current bit in bitstream must be 0 for this to work*/
  if(bit)
  {
	/*earlier bit of huffman code is in a lesser significant bit of an earlier byte*/
	bitstream[(*bitpointer) >> 3] |= (bit << (7 - ((*bitpointer) & 0x7)));
  }
  ++(*bitpointer);
}
#endif /*LODEPNG_COMPILE_DECODER*/

static void setBitOfReversedStream(size_t* bitpointer, unsigned char* bitstream, unsigned char bit)
{
  /*the current bit in bitstream may be 0 or 1 for this to work*/
  if(bit == 0) bitstream[(*bitpointer) >> 3] &=  (unsigned char)(~(1 << (7 - ((*bitpointer) & 0x7))));
  else         bitstream[(*bitpointer) >> 3] |=  (1 << (7 - ((*bitpointer) & 0x7)));
  ++(*bitpointer);
}

/* ////////////////////////////////////////////////////////////////////////// */
/* / PNG chunks                                                             / */
/* ////////////////////////////////////////////////////////////////////////// */

unsigned lodepng_chunk_length(const unsigned char* chunk)
{
  return lodepng_read32bitInt(&chunk[0]);
}

void lodepng_chunk_type(char type[5], const unsigned char* chunk)
{
  unsigned i;
  for(i = 0; i != 4; ++i) type[i] = (char)chunk[4 + i];
  type[4] = 0; /*null termination char*/
}

unsigned char lodepng_chunk_type_equals(const unsigned char* chunk, const char* type)
{
  if(strlen(type) != 4) return 0;
  return (chunk[4] == type[0] && chunk[5] == type[1] && chunk[6] == type[2] && chunk[7] == type[3]);
}

unsigned char lodepng_chunk_ancillary(const unsigned char* chunk)
{
  return((chunk[4] & 32) != 0);
}

unsigned char lodepng_chunk_private(const unsigned char* chunk)
{
  return((chunk[6] & 32) != 0);
}

unsigned char lodepng_chunk_safetocopy(const unsigned char* chunk)
{
  return((chunk[7] & 32) != 0);
}

unsigned char* lodepng_chunk_data(unsigned char* chunk)
{
  return &chunk[8];
}

const unsigned char* lodepng_chunk_data_const(const unsigned char* chunk)
{
  return &chunk[8];
}

unsigned lodepng_chunk_check_crc(const unsigned char* chunk)
{
  unsigned length = lodepng_chunk_length(chunk);
  unsigned CRC = lodepng_read32bitInt(&chunk[length + 8]);
  /*the CRC is taken of the data and the 4 chunk type letters, not the length*/
  unsigned checksum = lodepng_crc32(&chunk[4], length + 4);
  if(CRC != checksum) return 1;
  else return 0;
}

void lodepng_chunk_generate_crc(unsigned char* chunk)
{
  unsigned length = lodepng_chunk_length(chunk);
  unsigned CRC = lodepng_crc32(&chunk[4], length + 4);
  lodepng_set32bitInt(chunk + 8 + length, CRC);
}

unsigned char* lodepng_chunk_next(unsigned char* chunk)
{
  unsigned total_chunk_length = lodepng_chunk_length(chunk) + 12;
  return &chunk[total_chunk_length];
}

const unsigned char* lodepng_chunk_next_const(const unsigned char* chunk)
{
  unsigned total_chunk_length = lodepng_chunk_length(chunk) + 12;
  return &chunk[total_chunk_length];
}

unsigned lodepng_chunk_append(unsigned char** out, size_t* outlength, const unsigned char* chunk)
{
  unsigned i;
  unsigned total_chunk_length = lodepng_chunk_length(chunk) + 12;
  unsigned char *chunk_start, *new_buffer;
  size_t new_length = (*outlength) + total_chunk_length;
  if(new_length < total_chunk_length || new_length < (*outlength)) return 77; /*integer overflow happened*/

  new_buffer = (unsigned char*)lodepng_realloc(*out, new_length);
  if(!new_buffer) return 83; /*alloc fail*/
  (*out) = new_buffer;
  (*outlength) = new_length;
  chunk_start = &(*out)[new_length - total_chunk_length];

  for(i = 0; i != total_chunk_length; ++i) chunk_start[i] = chunk[i];

  return 0;
}

unsigned lodepng_chunk_create(unsigned char** out, size_t* outlength, unsigned length,
							  const char* type, const unsigned char* data)
{
  unsigned i;
  unsigned char *chunk, *new_buffer;
  size_t new_length = (*outlength) + length + 12;
  if(new_length < length + 12 || new_length < (*outlength)) return 77; /*integer overflow happened*/
  new_buffer = (unsigned char*)lodepng_realloc(*out, new_length);
  if(!new_buffer) return 83; /*alloc fail*/
  (*out) = new_buffer;
  (*outlength) = new_length;
  chunk = &(*out)[(*outlength) - length - 12];

  /*1: length*/
  lodepng_set32bitInt(chunk, (unsigned)length);

  /*2: chunk name (4 letters)*/
  chunk[4] = (unsigned char)type[0];
  chunk[5] = (unsigned char)type[1];
  chunk[6] = (unsigned char)type[2];
  chunk[7] = (unsigned char)type[3];

  /*3: the data*/
  for(i = 0; i != length; ++i) chunk[8 + i] = data[i];

  /*4: CRC (of the chunkname characters and the data)*/
  lodepng_chunk_generate_crc(chunk);

  return 0;
}

/* ////////////////////////////////////////////////////////////////////////// */
/* / Color types and such                                                   / */
/* ////////////////////////////////////////////////////////////////////////// */

/*return type is a LodePNG error code*/
static unsigned checkColorValidity(LodePNGColorType colortype, unsigned bd) /*bd = bitdepth*/
{
  switch(colortype)
  {
	case 0: if(!(bd == 1 || bd == 2 || bd == 4 || bd == 8 || bd == 16)) return 37; break; /*grey*/
	case 2: if(!(                                 bd == 8 || bd == 16)) return 37; break; /*RGB*/
	case 3: if(!(bd == 1 || bd == 2 || bd == 4 || bd == 8            )) return 37; break; /*palette*/
	case 4: if(!(                                 bd == 8 || bd == 16)) return 37; break; /*grey + alpha*/
	case 6: if(!(                                 bd == 8 || bd == 16)) return 37; break; /*RGBA*/
	default: return 31;
  }
  return 0; /*allowed color type / bits combination*/
}

static unsigned getNumColorChannels(LodePNGColorType colortype)
{
  switch(colortype)
  {
	case 0: return 1; /*grey*/
	case 2: return 3; /*RGB*/
	case 3: return 1; /*palette*/
	case 4: return 2; /*grey + alpha*/
	case 6: return 4; /*RGBA*/
  }
  return 0; /*unexisting color type*/
}

static unsigned lodepng_get_bpp_lct(LodePNGColorType colortype, unsigned bitdepth)
{
  /*bits per pixel is amount of channels * bits per channel*/
  return getNumColorChannels(colortype) * bitdepth;
}

/* ////////////////////////////////////////////////////////////////////////// */

void lodepng_color_mode_init(LodePNGColorMode* info)
{
  info->key_defined = 0;
  info->key_r = info->key_g = info->key_b = 0;
  info->colortype = LCT_RGBA;
  info->bitdepth = 8;
  info->palette = 0;
  info->palettesize = 0;
}

void lodepng_color_mode_cleanup(LodePNGColorMode* info)
{
  lodepng_palette_clear(info);
}

unsigned lodepng_color_mode_copy(LodePNGColorMode* dest, const LodePNGColorMode* source)
{
  size_t i;
  lodepng_color_mode_cleanup(dest);
  *dest = *source;
  if(source->palette)
  {
	dest->palette = (unsigned char*)lodepng_malloc(1024);
	if(!dest->palette && source->palettesize) return 83; /*alloc fail*/
	for(i = 0; i != source->palettesize * 4; ++i) dest->palette[i] = source->palette[i];
  }
  return 0;
}

static int lodepng_color_mode_equal(const LodePNGColorMode* a, const LodePNGColorMode* b)
{
  size_t i;
  if(a->colortype != b->colortype) return 0;
  if(a->bitdepth != b->bitdepth) return 0;
  if(a->key_defined != b->key_defined) return 0;
  if(a->key_defined)
  {
	if(a->key_r != b->key_r) return 0;
	if(a->key_g != b->key_g) return 0;
	if(a->key_b != b->key_b) return 0;
  }
  if(a->palettesize != b->palettesize) return 0;
  for(i = 0; i != a->palettesize * 4; ++i)
  {
	if(a->palette[i] != b->palette[i]) return 0;
  }
  return 1;
}

void lodepng_palette_clear(LodePNGColorMode* info)
{
  if(info->palette) lodepng_free(info->palette);
  info->palette = 0;
  info->palettesize = 0;
}

unsigned lodepng_palette_add(LodePNGColorMode* info,
							 unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
  unsigned char* data;
  /*the same resize technique as C++ std::vectors is used, and here it's made so that for a palette with
  the max of 256 colors, it'll have the exact alloc size*/
  if(!info->palette) /*allocate palette if empty*/
  {
	/*room for 256 colors with 4 bytes each*/
	data = (unsigned char*)lodepng_realloc(info->palette, 1024);
	if(!data) return 83; /*alloc fail*/
	else info->palette = data;
  }
  info->palette[4 * info->palettesize + 0] = r;
  info->palette[4 * info->palettesize + 1] = g;
  info->palette[4 * info->palettesize + 2] = b;
  info->palette[4 * info->palettesize + 3] = a;
  ++info->palettesize;
  return 0;
}

unsigned lodepng_get_bpp(const LodePNGColorMode* info)
{
  /*calculate bits per pixel out of colortype and bitdepth*/
  return lodepng_get_bpp_lct(info->colortype, info->bitdepth);
}

unsigned lodepng_get_channels(const LodePNGColorMode* info)
{
  return getNumColorChannels(info->colortype);
}

unsigned lodepng_is_greyscale_type(const LodePNGColorMode* info)
{
  return info->colortype == LCT_GREY || info->colortype == LCT_GREY_ALPHA;
}

unsigned lodepng_is_alpha_type(const LodePNGColorMode* info)
{
  return (info->colortype & 4) != 0; /*4 or 6*/
}

unsigned lodepng_is_palette_type(const LodePNGColorMode* info)
{
  return info->colortype == LCT_PALETTE;
}

unsigned lodepng_has_palette_alpha(const LodePNGColorMode* info)
{
  size_t i;
  for(i = 0; i != info->palettesize; ++i)
  {
	if(info->palette[i * 4 + 3] < 255) return 1;
  }
  return 0;
}

unsigned lodepng_can_have_alpha(const LodePNGColorMode* info)
{
  return info->key_defined
	  || lodepng_is_alpha_type(info)
	  || lodepng_has_palette_alpha(info);
}

size_t lodepng_get_raw_size(unsigned w, unsigned h, const LodePNGColorMode* color)
{
  return (w * h * lodepng_get_bpp(color) + 7) / 8;
}

size_t lodepng_get_raw_size_lct(unsigned w, unsigned h, LodePNGColorType colortype, unsigned bitdepth)
{
  return (w * h * lodepng_get_bpp_lct(colortype, bitdepth) + 7) / 8;
}

#ifdef LODEPNG_COMPILE_PNG
#ifdef LODEPNG_COMPILE_DECODER
/*in an idat chunk, each scanline is a multiple of 8 bits, unlike the lodepng output buffer*/
static size_t lodepng_get_raw_size_idat(unsigned w, unsigned h, const LodePNGColorMode* color)
{
  return h * ((w * lodepng_get_bpp(color) + 7) / 8);
}
#endif /*LODEPNG_COMPILE_DECODER*/
#endif /*LODEPNG_COMPILE_PNG*/

#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS

static void LodePNGUnknownChunks_init(LodePNGInfo* info)
{
  unsigned i;
  for(i = 0; i != 3; ++i) info->unknown_chunks_data[i] = 0;
  for(i = 0; i != 3; ++i) info->unknown_chunks_size[i] = 0;
}

static void LodePNGUnknownChunks_cleanup(LodePNGInfo* info)
{
  unsigned i;
  for(i = 0; i != 3; ++i) lodepng_free(info->unknown_chunks_data[i]);
}

static unsigned LodePNGUnknownChunks_copy(LodePNGInfo* dest, const LodePNGInfo* src)
{
  unsigned i;

  LodePNGUnknownChunks_cleanup(dest);

  for(i = 0; i != 3; ++i)
  {
	size_t j;
	dest->unknown_chunks_size[i] = src->unknown_chunks_size[i];
	dest->unknown_chunks_data[i] = (unsigned char*)lodepng_malloc(src->unknown_chunks_size[i]);
	if(!dest->unknown_chunks_data[i] && dest->unknown_chunks_size[i]) return 83; /*alloc fail*/
	for(j = 0; j < src->unknown_chunks_size[i]; ++j)
	{
	  dest->unknown_chunks_data[i][j] = src->unknown_chunks_data[i][j];
	}
  }

  return 0;
}

/******************************************************************************/

static void LodePNGText_init(LodePNGInfo* info)
{
  info->text_num = 0;
  info->text_keys = NULL;
  info->text_strings = NULL;
}

static void LodePNGText_cleanup(LodePNGInfo* info)
{
  size_t i;
  for(i = 0; i != info->text_num; ++i)
  {
	string_cleanup(&info->text_keys[i]);
	string_cleanup(&info->text_strings[i]);
  }
  lodepng_free(info->text_keys);
  lodepng_free(info->text_strings);
}

static unsigned LodePNGText_copy(LodePNGInfo* dest, const LodePNGInfo* source)
{
  size_t i = 0;
  dest->text_keys = 0;
  dest->text_strings = 0;
  dest->text_num = 0;
  for(i = 0; i != source->text_num; ++i)
  {
	CERROR_TRY_RETURN(lodepng_add_text(dest, source->text_keys[i], source->text_strings[i]));
  }
  return 0;
}

void lodepng_clear_text(LodePNGInfo* info)
{
  LodePNGText_cleanup(info);
}

unsigned lodepng_add_text(LodePNGInfo* info, const char* key, const char* str)
{
  char** new_keys = (char**)(lodepng_realloc(info->text_keys, sizeof(char*) * (info->text_num + 1)));
  char** new_strings = (char**)(lodepng_realloc(info->text_strings, sizeof(char*) * (info->text_num + 1)));
  if(!new_keys || !new_strings)
  {
	lodepng_free(new_keys);
	lodepng_free(new_strings);
	return 83; /*alloc fail*/
  }

  ++info->text_num;
  info->text_keys = new_keys;
  info->text_strings = new_strings;

  string_init(&info->text_keys[info->text_num - 1]);
  string_set(&info->text_keys[info->text_num - 1], key);

  string_init(&info->text_strings[info->text_num - 1]);
  string_set(&info->text_strings[info->text_num - 1], str);

  return 0;
}

/******************************************************************************/

static void LodePNGIText_init(LodePNGInfo* info)
{
  info->itext_num = 0;
  info->itext_keys = NULL;
  info->itext_langtags = NULL;
  info->itext_transkeys = NULL;
  info->itext_strings = NULL;
}

static void LodePNGIText_cleanup(LodePNGInfo* info)
{
  size_t i;
  for(i = 0; i != info->itext_num; ++i)
  {
	string_cleanup(&info->itext_keys[i]);
	string_cleanup(&info->itext_langtags[i]);
	string_cleanup(&info->itext_transkeys[i]);
	string_cleanup(&info->itext_strings[i]);
  }
  lodepng_free(info->itext_keys);
  lodepng_free(info->itext_langtags);
  lodepng_free(info->itext_transkeys);
  lodepng_free(info->itext_strings);
}

static unsigned LodePNGIText_copy(LodePNGInfo* dest, const LodePNGInfo* source)
{
  size_t i = 0;
  dest->itext_keys = 0;
  dest->itext_langtags = 0;
  dest->itext_transkeys = 0;
  dest->itext_strings = 0;
  dest->itext_num = 0;
  for(i = 0; i != source->itext_num; ++i)
  {
	CERROR_TRY_RETURN(lodepng_add_itext(dest, source->itext_keys[i], source->itext_langtags[i],
										source->itext_transkeys[i], source->itext_strings[i]));
  }
  return 0;
}

void lodepng_clear_itext(LodePNGInfo* info)
{
  LodePNGIText_cleanup(info);
}

unsigned lodepng_add_itext(LodePNGInfo* info, const char* key, const char* langtag,
						   const char* transkey, const char* str)
{
  char** new_keys = (char**)(lodepng_realloc(info->itext_keys, sizeof(char*) * (info->itext_num + 1)));
  char** new_langtags = (char**)(lodepng_realloc(info->itext_langtags, sizeof(char*) * (info->itext_num + 1)));
  char** new_transkeys = (char**)(lodepng_realloc(info->itext_transkeys, sizeof(char*) * (info->itext_num + 1)));
  char** new_strings = (char**)(lodepng_realloc(info->itext_strings, sizeof(char*) * (info->itext_num + 1)));
  if(!new_keys || !new_langtags || !new_transkeys || !new_strings)
  {
	lodepng_free(new_keys);
	lodepng_free(new_langtags);
	lodepng_free(new_transkeys);
	lodepng_free(new_strings);
	return 83; /*alloc fail*/
  }

  ++info->itext_num;
  info->itext_keys = new_keys;
  info->itext_langtags = new_langtags;
  info->itext_transkeys = new_transkeys;
  info->itext_strings = new_strings;

  string_init(&info->itext_keys[info->itext_num - 1]);
  string_set(&info->itext_keys[info->itext_num - 1], key);

  string_init(&info->itext_langtags[info->itext_num - 1]);
  string_set(&info->itext_langtags[info->itext_num - 1], langtag);

  string_init(&info->itext_transkeys[info->itext_num - 1]);
  string_set(&info->itext_transkeys[info->itext_num - 1], transkey);

  string_init(&info->itext_strings[info->itext_num - 1]);
  string_set(&info->itext_strings[info->itext_num - 1], str);

  return 0;
}
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/

void lodepng_info_init(LodePNGInfo* info)
{
  lodepng_color_mode_init(&info->color);
  info->interlace_method = 0;
  info->compression_method = 0;
  info->filter_method = 0;
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
  info->background_defined = 0;
  info->background_r = info->background_g = info->background_b = 0;

  LodePNGText_init(info);
  LodePNGIText_init(info);

  info->time_defined = 0;
  info->phys_defined = 0;

  LodePNGUnknownChunks_init(info);
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
}

void lodepng_info_cleanup(LodePNGInfo* info)
{
  lodepng_color_mode_cleanup(&info->color);
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
  LodePNGText_cleanup(info);
  LodePNGIText_cleanup(info);

  LodePNGUnknownChunks_cleanup(info);
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
}

unsigned lodepng_info_copy(LodePNGInfo* dest, const LodePNGInfo* source)
{
  lodepng_info_cleanup(dest);
  *dest = *source;
  lodepng_color_mode_init(&dest->color);
  CERROR_TRY_RETURN(lodepng_color_mode_copy(&dest->color, &source->color));

#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
  CERROR_TRY_RETURN(LodePNGText_copy(dest, source));
  CERROR_TRY_RETURN(LodePNGIText_copy(dest, source));

  LodePNGUnknownChunks_init(dest);
  CERROR_TRY_RETURN(LodePNGUnknownChunks_copy(dest, source));
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
  return 0;
}

void lodepng_info_swap(LodePNGInfo* a, LodePNGInfo* b)
{
  LodePNGInfo temp = *a;
  *a = *b;
  *b = temp;
}

/* ////////////////////////////////////////////////////////////////////////// */

/*index: bitgroup index, bits: bitgroup size(1, 2 or 4), in: bitgroup value, out: octet array to add bits to*/
static void addColorBits(unsigned char* out, size_t index, unsigned bits, unsigned in)
{
  unsigned m = bits == 1 ? 7 : bits == 2 ? 3 : 1; /*8 / bits - 1*/
  /*p = the partial index in the byte, e.g. with 4 palettebits it is 0 for first half or 1 for second half*/
  unsigned p = index & m;
  in &= (1u << bits) - 1u; /*filter out any other bits of the input value*/
  in = in << (bits * (m - p));
  if(p == 0) out[index * bits / 8] = in;
  else out[index * bits / 8] |= in;
}

typedef struct ColorTree ColorTree;

/*
One node of a color tree
This is the data structure used to count the number of unique colors and to get a palette
index for a color. It's like an octree, but because the alpha channel is used too, each
node has 16 instead of 8 children.
*/
struct ColorTree
{
  ColorTree* children[16]; /*up to 16 pointers to ColorTree of next level*/
  int index; /*the payload. Only has a meaningful value if this is in the last level*/
};

static void color_tree_init(ColorTree* tree)
{
  int i;
  for(i = 0; i != 16; ++i) tree->children[i] = 0;
  tree->index = -1;
}

static void color_tree_cleanup(ColorTree* tree)
{
  int i;
  for(i = 0; i != 16; ++i)
  {
	if(tree->children[i])
	{
	  color_tree_cleanup(tree->children[i]);
	  lodepng_free(tree->children[i]);
	}
  }
}

/*returns -1 if color not present, its index otherwise*/
static int color_tree_get(ColorTree* tree, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
  int bit = 0;
  for(bit = 0; bit < 8; ++bit)
  {
	int i = 8 * ((r >> bit) & 1) + 4 * ((g >> bit) & 1) + 2 * ((b >> bit) & 1) + 1 * ((a >> bit) & 1);
	if(!tree->children[i]) return -1;
	else tree = tree->children[i];
  }
  return tree ? tree->index : -1;
}

#ifdef LODEPNG_COMPILE_ENCODER
static int color_tree_has(ColorTree* tree, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
  return color_tree_get(tree, r, g, b, a) >= 0;
}
#endif /*LODEPNG_COMPILE_ENCODER*/

/*color is not allowed to already exist.
Index should be >= 0 (it's signed to be compatible with using -1 for "doesn't exist")*/
static void color_tree_add(ColorTree* tree,
						   unsigned char r, unsigned char g, unsigned char b, unsigned char a, unsigned index)
{
  int bit;
  for(bit = 0; bit < 8; ++bit)
  {
	int i = 8 * ((r >> bit) & 1) + 4 * ((g >> bit) & 1) + 2 * ((b >> bit) & 1) + 1 * ((a >> bit) & 1);
	if(!tree->children[i])
	{
	  tree->children[i] = (ColorTree*)lodepng_malloc(sizeof(ColorTree));
	  color_tree_init(tree->children[i]);
	}
	tree = tree->children[i];
  }
  tree->index = (int)index;
}

/*put a pixel, given its RGBA color, into image of any color type*/
static unsigned rgba8ToPixel(unsigned char* out, size_t i,
							 const LodePNGColorMode* mode, ColorTree* tree /*for palette*/,
							 unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
  if(mode->colortype == LCT_GREY)
  {
	unsigned char grey = r; /*((unsigned short)r + g + b) / 3*/;
	if(mode->bitdepth == 8) out[i] = grey;
	else if(mode->bitdepth == 16) out[i * 2 + 0] = out[i * 2 + 1] = grey;
	else
	{
	  /*take the most significant bits of grey*/
	  grey = (grey >> (8 - mode->bitdepth)) & ((1 << mode->bitdepth) - 1);
	  addColorBits(out, i, mode->bitdepth, grey);
	}
  }
  else if(mode->colortype == LCT_RGB)
  {
	if(mode->bitdepth == 8)
	{
	  out[i * 3 + 0] = r;
	  out[i * 3 + 1] = g;
	  out[i * 3 + 2] = b;
	}
	else
	{
	  out[i * 6 + 0] = out[i * 6 + 1] = r;
	  out[i * 6 + 2] = out[i * 6 + 3] = g;
	  out[i * 6 + 4] = out[i * 6 + 5] = b;
	}
  }
  else if(mode->colortype == LCT_PALETTE)
  {
	int index = color_tree_get(tree, r, g, b, a);
	if(index < 0) return 82; /*color not in palette*/
	if(mode->bitdepth == 8) out[i] = index;
	else addColorBits(out, i, mode->bitdepth, (unsigned)index);
  }
  else if(mode->colortype == LCT_GREY_ALPHA)
  {
	unsigned char grey = r; /*((unsigned short)r + g + b) / 3*/;
	if(mode->bitdepth == 8)
	{
	  out[i * 2 + 0] = grey;
	  out[i * 2 + 1] = a;
	}
	else if(mode->bitdepth == 16)
	{
	  out[i * 4 + 0] = out[i * 4 + 1] = grey;
	  out[i * 4 + 2] = out[i * 4 + 3] = a;
	}
  }
  else if(mode->colortype == LCT_RGBA)
  {
	if(mode->bitdepth == 8)
	{
	  out[i * 4 + 0] = r;
	  out[i * 4 + 1] = g;
	  out[i * 4 + 2] = b;
	  out[i * 4 + 3] = a;
	}
	else
	{
	  out[i * 8 + 0] = out[i * 8 + 1] = r;
	  out[i * 8 + 2] = out[i * 8 + 3] = g;
	  out[i * 8 + 4] = out[i * 8 + 5] = b;
	  out[i * 8 + 6] = out[i * 8 + 7] = a;
	}
  }

  return 0; /*no error*/
}

/*put a pixel, given its RGBA16 color, into image of any color 16-bitdepth type*/
static void rgba16ToPixel(unsigned char* out, size_t i,
						 const LodePNGColorMode* mode,
						 unsigned short r, unsigned short g, unsigned short b, unsigned short a)
{
  if(mode->colortype == LCT_GREY)
  {
	unsigned short grey = r; /*((unsigned)r + g + b) / 3*/;
	out[i * 2 + 0] = (grey >> 8) & 255;
	out[i * 2 + 1] = grey & 255;
  }
  else if(mode->colortype == LCT_RGB)
  {
	out[i * 6 + 0] = (r >> 8) & 255;
	out[i * 6 + 1] = r & 255;
	out[i * 6 + 2] = (g >> 8) & 255;
	out[i * 6 + 3] = g & 255;
	out[i * 6 + 4] = (b >> 8) & 255;
	out[i * 6 + 5] = b & 255;
  }
  else if(mode->colortype == LCT_GREY_ALPHA)
  {
	unsigned short grey = r; /*((unsigned)r + g + b) / 3*/;
	out[i * 4 + 0] = (grey >> 8) & 255;
	out[i * 4 + 1] = grey & 255;
	out[i * 4 + 2] = (a >> 8) & 255;
	out[i * 4 + 3] = a & 255;
  }
  else if(mode->colortype == LCT_RGBA)
  {
	out[i * 8 + 0] = (r >> 8) & 255;
	out[i * 8 + 1] = r & 255;
	out[i * 8 + 2] = (g >> 8) & 255;
	out[i * 8 + 3] = g & 255;
	out[i * 8 + 4] = (b >> 8) & 255;
	out[i * 8 + 5] = b & 255;
	out[i * 8 + 6] = (a >> 8) & 255;
	out[i * 8 + 7] = a & 255;
  }
}

/*Get RGBA8 color of pixel with index i (y * width + x) from the raw image with given color type.*/
static void getPixelColorRGBA8(unsigned char* r, unsigned char* g,
							   unsigned char* b, unsigned char* a,
							   const unsigned char* in, size_t i,
							   const LodePNGColorMode* mode)
{
  if(mode->colortype == LCT_GREY)
  {
	if(mode->bitdepth == 8)
	{
	  *r = *g = *b = in[i];
	  if(mode->key_defined && *r == mode->key_r) *a = 0;
	  else *a = 255;
	}
	else if(mode->bitdepth == 16)
	{
	  *r = *g = *b = in[i * 2 + 0];
	  if(mode->key_defined && 256U * in[i * 2 + 0] + in[i * 2 + 1] == mode->key_r) *a = 0;
	  else *a = 255;
	}
	else
	{
	  unsigned highest = ((1U << mode->bitdepth) - 1U); /*highest possible value for this bit depth*/
	  size_t j = i * mode->bitdepth;
	  unsigned value = readBitsFromReversedStream(&j, in, mode->bitdepth);
	  *r = *g = *b = (value * 255) / highest;
	  if(mode->key_defined && value == mode->key_r) *a = 0;
	  else *a = 255;
	}
  }
  else if(mode->colortype == LCT_RGB)
  {
	if(mode->bitdepth == 8)
	{
	  *r = in[i * 3 + 0]; *g = in[i * 3 + 1]; *b = in[i * 3 + 2];
	  if(mode->key_defined && *r == mode->key_r && *g == mode->key_g && *b == mode->key_b) *a = 0;
	  else *a = 255;
	}
	else
	{
	  *r = in[i * 6 + 0];
	  *g = in[i * 6 + 2];
	  *b = in[i * 6 + 4];
	  if(mode->key_defined && 256U * in[i * 6 + 0] + in[i * 6 + 1] == mode->key_r
		 && 256U * in[i * 6 + 2] + in[i * 6 + 3] == mode->key_g
		 && 256U * in[i * 6 + 4] + in[i * 6 + 5] == mode->key_b) *a = 0;
	  else *a = 255;
	}
  }
  else if(mode->colortype == LCT_PALETTE)
  {
	unsigned index;
	if(mode->bitdepth == 8) index = in[i];
	else
	{
	  size_t j = i * mode->bitdepth;
	  index = readBitsFromReversedStream(&j, in, mode->bitdepth);
	}

	if(index >= mode->palettesize)
	{
	  /*This is an error according to the PNG spec, but common PNG decoders make it black instead.
	  Done here too, slightly faster due to no error handling needed.*/
	  *r = *g = *b = 0;
	  *a = 255;
	}
	else
	{
	  *r = mode->palette[index * 4 + 0];
	  *g = mode->palette[index * 4 + 1];
	  *b = mode->palette[index * 4 + 2];
	  *a = mode->palette[index * 4 + 3];
	}
  }
  else if(mode->colortype == LCT_GREY_ALPHA)
  {
	if(mode->bitdepth == 8)
	{
	  *r = *g = *b = in[i * 2 + 0];
	  *a = in[i * 2 + 1];
	}
	else
	{
	  *r = *g = *b = in[i * 4 + 0];
	  *a = in[i * 4 + 2];
	}
  }
  else if(mode->colortype == LCT_RGBA)
  {
	if(mode->bitdepth == 8)
	{
	  *r = in[i * 4 + 0];
	  *g = in[i * 4 + 1];
	  *b = in[i * 4 + 2];
	  *a = in[i * 4 + 3];
	}
	else
	{
	  *r = in[i * 8 + 0];
	  *g = in[i * 8 + 2];
	  *b = in[i * 8 + 4];
	  *a = in[i * 8 + 6];
	}
  }
}

/*Similar to getPixelColorRGBA8, but with all the for loops inside of the color
mode test cases, optimized to convert the colors much faster, when converting
to RGBA or RGB with 8 bit per cannel. buffer must be RGBA or RGB output with
enough memory, if has_alpha is true the output is RGBA. mode has the color mode
of the input buffer.*/
static void getPixelColorsRGBA8(unsigned char* buffer, size_t numpixels,
								unsigned has_alpha, const unsigned char* in,
								const LodePNGColorMode* mode)
{
  unsigned num_channels = has_alpha ? 4 : 3;
  size_t i;
  if(mode->colortype == LCT_GREY)
  {
	if(mode->bitdepth == 8)
	{
	  for(i = 0; i != numpixels; ++i, buffer += num_channels)
	  {
		buffer[0] = buffer[1] = buffer[2] = in[i];
		if(has_alpha) buffer[3] = mode->key_defined && in[i] == mode->key_r ? 0 : 255;
	  }
	}
	else if(mode->bitdepth == 16)
	{
	  for(i = 0; i != numpixels; ++i, buffer += num_channels)
	  {
		buffer[0] = buffer[1] = buffer[2] = in[i * 2];
		if(has_alpha) buffer[3] = mode->key_defined && 256U * in[i * 2 + 0] + in[i * 2 + 1] == mode->key_r ? 0 : 255;
	  }
	}
	else
	{
	  unsigned highest = ((1U << mode->bitdepth) - 1U); /*highest possible value for this bit depth*/
	  size_t j = 0;
	  for(i = 0; i != numpixels; ++i, buffer += num_channels)
	  {
		unsigned value = readBitsFromReversedStream(&j, in, mode->bitdepth);
		buffer[0] = buffer[1] = buffer[2] = (value * 255) / highest;
		if(has_alpha) buffer[3] = mode->key_defined && value == mode->key_r ? 0 : 255;
	  }
	}
  }
  else if(mode->colortype == LCT_RGB)
  {
	if(mode->bitdepth == 8)
	{
	  for(i = 0; i != numpixels; ++i, buffer += num_channels)
	  {
		buffer[0] = in[i * 3 + 0];
		buffer[1] = in[i * 3 + 1];
		buffer[2] = in[i * 3 + 2];
		if(has_alpha) buffer[3] = mode->key_defined && buffer[0] == mode->key_r
		   && buffer[1]== mode->key_g && buffer[2] == mode->key_b ? 0 : 255;
	  }
	}
	else
	{
	  for(i = 0; i != numpixels; ++i, buffer += num_channels)
	  {
		buffer[0] = in[i * 6 + 0];
		buffer[1] = in[i * 6 + 2];
		buffer[2] = in[i * 6 + 4];
		if(has_alpha) buffer[3] = mode->key_defined
		   && 256U * in[i * 6 + 0] + in[i * 6 + 1] == mode->key_r
		   && 256U * in[i * 6 + 2] + in[i * 6 + 3] == mode->key_g
		   && 256U * in[i * 6 + 4] + in[i * 6 + 5] == mode->key_b ? 0 : 255;
	  }
	}
  }
  else if(mode->colortype == LCT_PALETTE)
  {
	unsigned index;
	size_t j = 0;
	for(i = 0; i != numpixels; ++i, buffer += num_channels)
	{
	  if(mode->bitdepth == 8) index = in[i];
	  else index = readBitsFromReversedStream(&j, in, mode->bitdepth);

	  if(index >= mode->palettesize)
	  {
		/*This is an error according to the PNG spec, but most PNG decoders make it black instead.
		Done here too, slightly faster due to no error handling needed.*/
		buffer[0] = buffer[1] = buffer[2] = 0;
		if(has_alpha) buffer[3] = 255;
	  }
	  else
	  {
		buffer[0] = mode->palette[index * 4 + 0];
		buffer[1] = mode->palette[index * 4 + 1];
		buffer[2] = mode->palette[index * 4 + 2];
		if(has_alpha) buffer[3] = mode->palette[index * 4 + 3];
	  }
	}
  }
  else if(mode->colortype == LCT_GREY_ALPHA)
  {
	if(mode->bitdepth == 8)
	{
	  for(i = 0; i != numpixels; ++i, buffer += num_channels)
	  {
		buffer[0] = buffer[1] = buffer[2] = in[i * 2 + 0];
		if(has_alpha) buffer[3] = in[i * 2 + 1];
	  }
	}
	else
	{
	  for(i = 0; i != numpixels; ++i, buffer += num_channels)
	  {
		buffer[0] = buffer[1] = buffer[2] = in[i * 4 + 0];
		if(has_alpha) buffer[3] = in[i * 4 + 2];
	  }
	}
  }
  else if(mode->colortype == LCT_RGBA)
  {
	if(mode->bitdepth == 8)
	{
	  for(i = 0; i != numpixels; ++i, buffer += num_channels)
	  {
		buffer[0] = in[i * 4 + 0];
		buffer[1] = in[i * 4 + 1];
		buffer[2] = in[i * 4 + 2];
		if(has_alpha) buffer[3] = in[i * 4 + 3];
	  }
	}
	else
	{
	  for(i = 0; i != numpixels; ++i, buffer += num_channels)
	  {
		buffer[0] = in[i * 8 + 0];
		buffer[1] = in[i * 8 + 2];
		buffer[2] = in[i * 8 + 4];
		if(has_alpha) buffer[3] = in[i * 8 + 6];
	  }
	}
  }
}

/*Get RGBA16 color of pixel with index i (y * width + x) from the raw image with
given color type, but the given color type must be 16-bit itself.*/
static void getPixelColorRGBA16(unsigned short* r, unsigned short* g, unsigned short* b, unsigned short* a,
								const unsigned char* in, size_t i, const LodePNGColorMode* mode)
{
  if(mode->colortype == LCT_GREY)
  {
	*r = *g = *b = 256 * in[i * 2 + 0] + in[i * 2 + 1];
	if(mode->key_defined && 256U * in[i * 2 + 0] + in[i * 2 + 1] == mode->key_r) *a = 0;
	else *a = 65535;
  }
  else if(mode->colortype == LCT_RGB)
  {
	*r = 256 * in[i * 6 + 0] + in[i * 6 + 1];
	*g = 256 * in[i * 6 + 2] + in[i * 6 + 3];
	*b = 256 * in[i * 6 + 4] + in[i * 6 + 5];
	if(mode->key_defined && 256U * in[i * 6 + 0] + in[i * 6 + 1] == mode->key_r
	   && 256U * in[i * 6 + 2] + in[i * 6 + 3] == mode->key_g
	   && 256U * in[i * 6 + 4] + in[i * 6 + 5] == mode->key_b) *a = 0;
	else *a = 65535;
  }
  else if(mode->colortype == LCT_GREY_ALPHA)
  {
	*r = *g = *b = 256 * in[i * 4 + 0] + in[i * 4 + 1];
	*a = 256 * in[i * 4 + 2] + in[i * 4 + 3];
  }
  else if(mode->colortype == LCT_RGBA)
  {
	*r = 256 * in[i * 8 + 0] + in[i * 8 + 1];
	*g = 256 * in[i * 8 + 2] + in[i * 8 + 3];
	*b = 256 * in[i * 8 + 4] + in[i * 8 + 5];
	*a = 256 * in[i * 8 + 6] + in[i * 8 + 7];
  }
}

unsigned lodepng_convert(unsigned char* out, const unsigned char* in,
						 LodePNGColorMode* mode_out, const LodePNGColorMode* mode_in,
						 unsigned w, unsigned h)
{
  size_t i;
  ColorTree tree;
  size_t numpixels = w * h;

  if(lodepng_color_mode_equal(mode_out, mode_in))
  {
	size_t numbytes = lodepng_get_raw_size(w, h, mode_in);
	for(i = 0; i != numbytes; ++i) out[i] = in[i];
	return 0;
  }

  if(mode_out->colortype == LCT_PALETTE)
  {
	size_t palsize = 1u << mode_out->bitdepth;
	if(mode_out->palettesize < palsize) palsize = mode_out->palettesize;
	color_tree_init(&tree);
	for(i = 0; i != palsize; ++i)
	{
	  unsigned char* p = &mode_out->palette[i * 4];
	  color_tree_add(&tree, p[0], p[1], p[2], p[3], i);
	}
  }

  if(mode_in->bitdepth == 16 && mode_out->bitdepth == 16)
  {
	for(i = 0; i != numpixels; ++i)
	{
	  unsigned short r = 0, g = 0, b = 0, a = 0;
	  getPixelColorRGBA16(&r, &g, &b, &a, in, i, mode_in);
	  rgba16ToPixel(out, i, mode_out, r, g, b, a);
	}
  }
  else if(mode_out->bitdepth == 8 && mode_out->colortype == LCT_RGBA)
  {
	getPixelColorsRGBA8(out, numpixels, 1, in, mode_in);
  }
  else if(mode_out->bitdepth == 8 && mode_out->colortype == LCT_RGB)
  {
	getPixelColorsRGBA8(out, numpixels, 0, in, mode_in);
  }
  else
  {
	unsigned char r = 0, g = 0, b = 0, a = 0;
	for(i = 0; i != numpixels; ++i)
	{
	  getPixelColorRGBA8(&r, &g, &b, &a, in, i, mode_in);
	  rgba8ToPixel(out, i, mode_out, &tree, r, g, b, a);
	}
  }

  if(mode_out->colortype == LCT_PALETTE)
  {
	color_tree_cleanup(&tree);
  }

  return 0; /*no error (this function currently never has one, but maybe OOM detection added later.)*/
}

#ifdef LODEPNG_COMPILE_ENCODER

void lodepng_color_profile_init(LodePNGColorProfile* profile)
{
  profile->colored = 0;
  profile->key = 0;
  profile->alpha = 0;
  profile->key_r = profile->key_g = profile->key_b = 0;
  profile->numcolors = 0;
  profile->bits = 1;
}

/*function used for debug purposes with C++*/
/*void printColorProfile(LodePNGColorProfile* p)
{
  std::cout << "colored: " << (int)p->colored << ", ";
  std::cout << "key: " << (int)p->key << ", ";
  std::cout << "key_r: " << (int)p->key_r << ", ";
  std::cout << "key_g: " << (int)p->key_g << ", ";
  std::cout << "key_b: " << (int)p->key_b << ", ";
  std::cout << "alpha: " << (int)p->alpha << ", ";
  std::cout << "numcolors: " << (int)p->numcolors << ", ";
  std::cout << "bits: " << (int)p->bits << std::endl;
}*/

/*Returns how many bits needed to represent given value (max 8 bit)*/
static unsigned getValueRequiredBits(unsigned char value)
{
  if(value == 0 || value == 255) return 1;
  /*The scaling of 2-bit and 4-bit values uses multiples of 85 and 17*/
  if(value % 17 == 0) return value % 85 == 0 ? 2 : 4;
  return 8;
}

/*profile must already have been inited with mode.
It's ok to set some parameters of profile to done already.*/
unsigned lodepng_get_color_profile(LodePNGColorProfile* profile,
								   const unsigned char* in, unsigned w, unsigned h,
								   const LodePNGColorMode* mode)
{
  unsigned error = 0;
  size_t i;
  ColorTree tree;
  size_t numpixels = w * h;

  unsigned colored_done = lodepng_is_greyscale_type(mode) ? 1 : 0;
  unsigned alpha_done = lodepng_can_have_alpha(mode) ? 0 : 1;
  unsigned numcolors_done = 0;
  unsigned bpp = lodepng_get_bpp(mode);
  unsigned bits_done = bpp == 1 ? 1 : 0;
  unsigned maxnumcolors = 257;
  unsigned sixteen = 0;
  if(bpp <= 8) maxnumcolors = bpp == 1 ? 2 : (bpp == 2 ? 4 : (bpp == 4 ? 16 : 256));

  color_tree_init(&tree);

  /*Check if the 16-bit input is truly 16-bit*/
  if(mode->bitdepth == 16)
  {
	unsigned short r, g, b, a;
	for(i = 0; i != numpixels; ++i)
	{
	  getPixelColorRGBA16(&r, &g, &b, &a, in, i, mode);
	  if((r & 255u) != ((r >> 8) & 255u) || (g & 255u) != ((g >> 8) & 255u) ||
		 (b & 255u) != ((b >> 8) & 255u) || (a & 255u) != ((a >> 8) & 255u)) /*first and second byte differ*/
	  {
		sixteen = 1;
		break;
	  }
	}
  }

  if(sixteen)
  {
	unsigned short r = 0, g = 0, b = 0, a = 0;
	profile->bits = 16;
	bits_done = numcolors_done = 1; /*counting colors no longer useful, palette doesn't support 16-bit*/

	for(i = 0; i != numpixels; ++i)
	{
	  getPixelColorRGBA16(&r, &g, &b, &a, in, i, mode);

	  if(!colored_done && (r != g || r != b))
	  {
		profile->colored = 1;
		colored_done = 1;
	  }

	  if(!alpha_done)
	  {
		unsigned matchkey = (r == profile->key_r && g == profile->key_g && b == profile->key_b);
		if(a != 65535 && (a != 0 || (profile->key && !matchkey)))
		{
		  profile->alpha = 1;
		  alpha_done = 1;
		  if(profile->bits < 8) profile->bits = 8; /*PNG has no alphachannel modes with less than 8-bit per channel*/
		}
		else if(a == 0 && !profile->alpha && !profile->key)
		{
		  profile->key = 1;
		  profile->key_r = r;
		  profile->key_g = g;
		  profile->key_b = b;
		}
		else if(a == 65535 && profile->key && matchkey)
		{
		  /* Color key cannot be used if an opaque pixel also has that RGB color. */
		  profile->alpha = 1;
		  alpha_done = 1;
		}
	  }

	  if(alpha_done && numcolors_done && colored_done && bits_done) break;
	}
  }
  else /* < 16-bit */
  {
	for(i = 0; i != numpixels; ++i)
	{
	  unsigned char r = 0, g = 0, b = 0, a = 0;
	  getPixelColorRGBA8(&r, &g, &b, &a, in, i, mode);

	  if(!bits_done && profile->bits < 8)
	  {
		/*only r is checked, < 8 bits is only relevant for greyscale*/
		unsigned bits = getValueRequiredBits(r);
		if(bits > profile->bits) profile->bits = bits;
	  }
	  bits_done = (profile->bits >= bpp);

	  if(!colored_done && (r != g || r != b))
	  {
		profile->colored = 1;
		colored_done = 1;
		if(profile->bits < 8) profile->bits = 8; /*PNG has no colored modes with less than 8-bit per channel*/
	  }

	  if(!alpha_done)
	  {
		unsigned matchkey = (r == profile->key_r && g == profile->key_g && b == profile->key_b);
		if(a != 255 && (a != 0 || (profile->key && !matchkey)))
		{
		  profile->alpha = 1;
		  alpha_done = 1;
		  if(profile->bits < 8) profile->bits = 8; /*PNG has no alphachannel modes with less than 8-bit per channel*/
		}
		else if(a == 0 && !profile->alpha && !profile->key)
		{
		  profile->key = 1;
		  profile->key_r = r;
		  profile->key_g = g;
		  profile->key_b = b;
		}
		else if(a == 255 && profile->key && matchkey)
		{
		  /* Color key cannot be used if an opaque pixel also has that RGB color. */
		  profile->alpha = 1;
		  alpha_done = 1;
		  if(profile->bits < 8) profile->bits = 8; /*PNG has no alphachannel modes with less than 8-bit per channel*/
		}
	  }

	  if(!numcolors_done)
	  {
		if(!color_tree_has(&tree, r, g, b, a))
		{
		  color_tree_add(&tree, r, g, b, a, profile->numcolors);
		  if(profile->numcolors < 256)
		  {
			unsigned char* p = profile->palette;
			unsigned n = profile->numcolors;
			p[n * 4 + 0] = r;
			p[n * 4 + 1] = g;
			p[n * 4 + 2] = b;
			p[n * 4 + 3] = a;
		  }
		  ++profile->numcolors;
		  numcolors_done = profile->numcolors >= maxnumcolors;
		}
	  }

	  if(alpha_done && numcolors_done && colored_done && bits_done) break;
	}

	/*make the profile's key always 16-bit for consistency - repeat each byte twice*/
	profile->key_r += (profile->key_r << 8);
	profile->key_g += (profile->key_g << 8);
	profile->key_b += (profile->key_b << 8);
  }

  color_tree_cleanup(&tree);
  return error;
}

/*Automatically chooses color type that gives smallest amount of bits in the
output image, e.g. grey if there are only greyscale pixels, palette if there
are less than 256 colors, ...
Updates values of mode with a potentially smaller color model. mode_out should
contain the user chosen color model, but will be overwritten with the new chosen one.*/
unsigned lodepng_auto_choose_color(LodePNGColorMode* mode_out,
								   const unsigned char* image, unsigned w, unsigned h,
								   const LodePNGColorMode* mode_in)
{
  LodePNGColorProfile prof;
  unsigned error = 0;
  unsigned i, n, palettebits, grey_ok, palette_ok;

  lodepng_color_profile_init(&prof);
  error = lodepng_get_color_profile(&prof, image, w, h, mode_in);
  if(error) return error;
  mode_out->key_defined = 0;

  if(prof.key && w * h <= 16) {
	prof.alpha = 1; /*too few pixels to justify tRNS chunk overhead*/
	if(prof.bits < 8) prof.bits = 8; /*PNG has no alphachannel modes with less than 8-bit per channel*/
  }
  grey_ok = !prof.colored && !prof.alpha; /*grey without alpha, with potentially low bits*/
  n = prof.numcolors;
  palettebits = n <= 2 ? 1 : (n <= 4 ? 2 : (n <= 16 ? 4 : 8));
  palette_ok = n <= 256 && (n * 2 < w * h) && prof.bits <= 8;
  if(w * h < n * 2) palette_ok = 0; /*don't add palette overhead if image has only a few pixels*/
  if(grey_ok && prof.bits <= palettebits) palette_ok = 0; /*grey is less overhead*/

  if(palette_ok)
  {
	unsigned char* p = prof.palette;
	lodepng_palette_clear(mode_out); /*remove potential earlier palette*/
	for(i = 0; i != prof.numcolors; ++i)
	{
	  error = lodepng_palette_add(mode_out, p[i * 4 + 0], p[i * 4 + 1], p[i * 4 + 2], p[i * 4 + 3]);
	  if(error) break;
	}

	mode_out->colortype = LCT_PALETTE;
	mode_out->bitdepth = palettebits;

	if(mode_in->colortype == LCT_PALETTE && mode_in->palettesize >= mode_out->palettesize
		&& mode_in->bitdepth == mode_out->bitdepth)
	{
	  /*If input should have same palette colors, keep original to preserve its order and prevent conversion*/
	  lodepng_color_mode_cleanup(mode_out);
	  lodepng_color_mode_copy(mode_out, mode_in);
	}
  }
  else /*8-bit or 16-bit per channel*/
  {
	mode_out->bitdepth = prof.bits;
	mode_out->colortype = prof.alpha ? (prof.colored ? LCT_RGBA : LCT_GREY_ALPHA)
									 : (prof.colored ? LCT_RGB : LCT_GREY);

	if(prof.key && !prof.alpha)
	{
	  unsigned mask = (1u << mode_out->bitdepth) - 1u; /*profile always uses 16-bit, mask converts it*/
	  mode_out->key_r = prof.key_r & mask;
	  mode_out->key_g = prof.key_g & mask;
	  mode_out->key_b = prof.key_b & mask;
	  mode_out->key_defined = 1;
	}
  }

  return error;
}

#endif /* #ifdef LODEPNG_COMPILE_ENCODER */

/*
Paeth predicter, used by PNG filter type 4
The parameters are of type short, but should come from unsigned chars, the shorts
are only needed to make the paeth calculation correct.
*/
static unsigned char paethPredictor(short a, short b, short c)
{
  short pa = abs(b - c);
  short pb = abs(a - c);
  short pc = abs(a + b - c - c);

  if(pc < pa && pc < pb) return (unsigned char)c;
  else if(pb < pa) return (unsigned char)b;
  else return (unsigned char)a;
}

/*shared values used by multiple Adam7 related functions*/

static const unsigned ADAM7_IX[7] = { 0, 4, 0, 2, 0, 1, 0 }; /*x start values*/
static const unsigned ADAM7_IY[7] = { 0, 0, 4, 0, 2, 0, 1 }; /*y start values*/
static const unsigned ADAM7_DX[7] = { 8, 8, 4, 4, 2, 2, 1 }; /*x delta values*/
static const unsigned ADAM7_DY[7] = { 8, 8, 8, 4, 4, 2, 2 }; /*y delta values*/

/*
Outputs various dimensions and positions in the image related to the Adam7 reduced images.
passw: output containing the width of the 7 passes
passh: output containing the height of the 7 passes
filter_passstart: output containing the index of the start and end of each
 reduced image with filter bytes
padded_passstart output containing the index of the start and end of each
 reduced image when without filter bytes but with padded scanlines
passstart: output containing the index of the start and end of each reduced
 image without padding between scanlines, but still padding between the images
w, h: width and height of non-interlaced image
bpp: bits per pixel
"padded" is only relevant if bpp is less than 8 and a scanline or image does not
 end at a full byte
*/
static void Adam7_getpassvalues(unsigned passw[7], unsigned passh[7], size_t filter_passstart[8],
								size_t padded_passstart[8], size_t passstart[8], unsigned w, unsigned h, unsigned bpp)
{
  /*the passstart values have 8 values: the 8th one indicates the byte after the end of the 7th (= last) pass*/
  unsigned i;

  /*calculate width and height in pixels of each pass*/
  for(i = 0; i != 7; ++i)
  {
	passw[i] = (w + ADAM7_DX[i] - ADAM7_IX[i] - 1) / ADAM7_DX[i];
	passh[i] = (h + ADAM7_DY[i] - ADAM7_IY[i] - 1) / ADAM7_DY[i];
	if(passw[i] == 0) passh[i] = 0;
	if(passh[i] == 0) passw[i] = 0;
  }

  filter_passstart[0] = padded_passstart[0] = passstart[0] = 0;
  for(i = 0; i != 7; ++i)
  {
	/*if passw[i] is 0, it's 0 bytes, not 1 (no filtertype-byte)*/
	filter_passstart[i + 1] = filter_passstart[i]
							+ ((passw[i] && passh[i]) ? passh[i] * (1 + (passw[i] * bpp + 7) / 8) : 0);
	/*bits padded if needed to fill full byte at end of each scanline*/
	padded_passstart[i + 1] = padded_passstart[i] + passh[i] * ((passw[i] * bpp + 7) / 8);
	/*only padded at end of reduced image*/
	passstart[i + 1] = passstart[i] + (passh[i] * passw[i] * bpp + 7) / 8;
  }
}

#ifdef LODEPNG_COMPILE_DECODER

/* ////////////////////////////////////////////////////////////////////////// */
/* / PNG Decoder                                                            / */
/* ////////////////////////////////////////////////////////////////////////// */

/*read the information from the header and store it in the LodePNGInfo. return value is error*/
unsigned lodepng_inspect(unsigned* w, unsigned* h, LodePNGState* state,
						 const unsigned char* in, size_t insize)
{
  LodePNGInfo* info = &state->info_png;
  if(insize == 0 || in == 0)
  {
	CERROR_RETURN_ERROR(state->error, 48); /*error: the given data is empty*/
  }
  if(insize < 33)
  {
	CERROR_RETURN_ERROR(state->error, 27); /*error: the data length is smaller than the length of a PNG header*/
  }

  /*when decoding a new PNG image, make sure all parameters created after previous decoding are reset*/
  lodepng_info_cleanup(info);
  lodepng_info_init(info);

  if(in[0] != 137 || in[1] != 80 || in[2] != 78 || in[3] != 71
	 || in[4] != 13 || in[5] != 10 || in[6] != 26 || in[7] != 10)
  {
	CERROR_RETURN_ERROR(state->error, 28); /*error: the first 8 bytes are not the correct PNG signature*/
  }
  if(in[12] != 'I' || in[13] != 'H' || in[14] != 'D' || in[15] != 'R')
  {
	CERROR_RETURN_ERROR(state->error, 29); /*error: it doesn't start with a IHDR chunk!*/
  }

  /*read the values given in the header*/
  *w = lodepng_read32bitInt(&in[16]);
  *h = lodepng_read32bitInt(&in[20]);
  info->color.bitdepth = in[24];
  info->color.colortype = (LodePNGColorType)in[25];
  info->compression_method = in[26];
  info->filter_method = in[27];
  info->interlace_method = in[28];

  if(*w == 0 || *h == 0)
  {
	CERROR_RETURN_ERROR(state->error, 93);
  }

  if(!state->decoder.ignore_crc)
  {
	unsigned CRC = lodepng_read32bitInt(&in[29]);
	unsigned checksum = lodepng_crc32(&in[12], 17);
	if(CRC != checksum)
	{
	  CERROR_RETURN_ERROR(state->error, 57); /*invalid CRC*/
	}
  }

  /*error: only compression method 0 is allowed in the specification*/
  if(info->compression_method != 0) CERROR_RETURN_ERROR(state->error, 32);
  /*error: only filter method 0 is allowed in the specification*/
  if(info->filter_method != 0) CERROR_RETURN_ERROR(state->error, 33);
  /*error: only interlace methods 0 and 1 exist in the specification*/
  if(info->interlace_method > 1) CERROR_RETURN_ERROR(state->error, 34);

  state->error = checkColorValidity(info->color.colortype, info->color.bitdepth);
  return state->error;
}

static unsigned unfilterScanline(unsigned char* recon, const unsigned char* scanline, const unsigned char* precon,
								 size_t bytewidth, unsigned char filterType, size_t length)
{
  /*
  For PNG filter method 0
  unfilter a PNG image scanline by scanline. when the pixels are smaller than 1 byte,
  the filter works byte per byte (bytewidth = 1)
  precon is the previous unfiltered scanline, recon the result, scanline the current one
  the incoming scanlines do NOT include the filtertype byte, that one is given in the parameter filterType instead
  recon and scanline MAY be the same memory address! precon must be disjoint.
  */

  size_t i;
  switch(filterType)
  {
	case 0:
	  for(i = 0; i != length; ++i) recon[i] = scanline[i];
	  break;
	case 1:
	  for(i = 0; i != bytewidth; ++i) recon[i] = scanline[i];
	  for(i = bytewidth; i < length; ++i) recon[i] = scanline[i] + recon[i - bytewidth];
	  break;
	case 2:
	  if(precon)
	  {
		for(i = 0; i != length; ++i) recon[i] = scanline[i] + precon[i];
	  }
	  else
	  {
		for(i = 0; i != length; ++i) recon[i] = scanline[i];
	  }
	  break;
	case 3:
	  if(precon)
	  {
		for(i = 0; i != bytewidth; ++i) recon[i] = scanline[i] + precon[i] / 2;
		for(i = bytewidth; i < length; ++i) recon[i] = scanline[i] + ((recon[i - bytewidth] + precon[i]) / 2);
	  }
	  else
	  {
		for(i = 0; i != bytewidth; ++i) recon[i] = scanline[i];
		for(i = bytewidth; i < length; ++i) recon[i] = scanline[i] + recon[i - bytewidth] / 2;
	  }
	  break;
	case 4:
	  if(precon)
	  {
		for(i = 0; i != bytewidth; ++i)
		{
		  recon[i] = (scanline[i] + precon[i]); /*paethPredictor(0, precon[i], 0) is always precon[i]*/
		}
		for(i = bytewidth; i < length; ++i)
		{
		  recon[i] = (scanline[i] + paethPredictor(recon[i - bytewidth], precon[i], precon[i - bytewidth]));
		}
	  }
	  else
	  {
		for(i = 0; i != bytewidth; ++i)
		{
		  recon[i] = scanline[i];
		}
		for(i = bytewidth; i < length; ++i)
		{
		  /*paethPredictor(recon[i - bytewidth], 0, 0) is always recon[i - bytewidth]*/
		  recon[i] = (scanline[i] + recon[i - bytewidth]);
		}
	  }
	  break;
	default: return 36; /*error: unexisting filter type given*/
  }
  return 0;
}

static unsigned unfilter(unsigned char* out, const unsigned char* in, unsigned w, unsigned h, unsigned bpp)
{
  /*
  For PNG filter method 0
  this function unfilters a single image (e.g. without interlacing this is called once, with Adam7 seven times)
  out must have enough bytes allocated already, in must have the scanlines + 1 filtertype byte per scanline
  w and h are image dimensions or dimensions of reduced image, bpp is bits per pixel
  in and out are allowed to be the same memory address (but aren't the same size since in has the extra filter bytes)
  */

  unsigned y;
  unsigned char* prevline = 0;

  /*bytewidth is used for filtering, is 1 when bpp < 8, number of bytes per pixel otherwise*/
  size_t bytewidth = (bpp + 7) / 8;
  size_t linebytes = (w * bpp + 7) / 8;

  for(y = 0; y < h; ++y)
  {
	size_t outindex = linebytes * y;
	size_t inindex = (1 + linebytes) * y; /*the extra filterbyte added to each row*/
	unsigned char filterType = in[inindex];

	CERROR_TRY_RETURN(unfilterScanline(&out[outindex], &in[inindex + 1], prevline, bytewidth, filterType, linebytes));

	prevline = &out[outindex];
  }

  return 0;
}

/*
in: Adam7 interlaced image, with no padding bits between scanlines, but between
 reduced images so that each reduced image starts at a byte.
out: the same pixels, but re-ordered so that they're now a non-interlaced image with size w*h
bpp: bits per pixel
out has the following size in bits: w * h * bpp.
in is possibly bigger due to padding bits between reduced images.
out must be big enough AND must be 0 everywhere if bpp < 8 in the current implementation
(because that's likely a little bit faster)
NOTE: comments about padding bits are only relevant if bpp < 8
*/
static void Adam7_deinterlace(unsigned char* out, const unsigned char* in, unsigned w, unsigned h, unsigned bpp)
{
  unsigned passw[7], passh[7];
  size_t filter_passstart[8], padded_passstart[8], passstart[8];
  unsigned i;

  Adam7_getpassvalues(passw, passh, filter_passstart, padded_passstart, passstart, w, h, bpp);

  if(bpp >= 8)
  {
	for(i = 0; i != 7; ++i)
	{
	  unsigned x, y, b;
	  size_t bytewidth = bpp / 8;
	  for(y = 0; y < passh[i]; ++y)
	  for(x = 0; x < passw[i]; ++x)
	  {
		size_t pixelinstart = passstart[i] + (y * passw[i] + x) * bytewidth;
		size_t pixeloutstart = ((ADAM7_IY[i] + y * ADAM7_DY[i]) * w + ADAM7_IX[i] + x * ADAM7_DX[i]) * bytewidth;
		for(b = 0; b < bytewidth; ++b)
		{
		  out[pixeloutstart + b] = in[pixelinstart + b];
		}
	  }
	}
  }
  else /*bpp < 8: Adam7 with pixels < 8 bit is a bit trickier: with bit pointers*/
  {
	for(i = 0; i != 7; ++i)
	{
	  unsigned x, y, b;
	  unsigned ilinebits = bpp * passw[i];
	  unsigned olinebits = bpp * w;
	  size_t obp, ibp; /*bit pointers (for out and in buffer)*/
	  for(y = 0; y < passh[i]; ++y)
	  for(x = 0; x < passw[i]; ++x)
	  {
		ibp = (8 * passstart[i]) + (y * ilinebits + x * bpp);
		obp = (ADAM7_IY[i] + y * ADAM7_DY[i]) * olinebits + (ADAM7_IX[i] + x * ADAM7_DX[i]) * bpp;
		for(b = 0; b < bpp; ++b)
		{
		  unsigned char bit = readBitFromReversedStream(&ibp, in);
		  /*note that this function assumes the out buffer is completely 0, use setBitOfReversedStream otherwise*/
		  setBitOfReversedStream0(&obp, out, bit);
		}
	  }
	}
  }
}

static void removePaddingBits(unsigned char* out, const unsigned char* in,
							  size_t olinebits, size_t ilinebits, unsigned h)
{
  /*
  After filtering there are still padding bits if scanlines have non multiple of 8 bit amounts. They need
  to be removed (except at last scanline of (Adam7-reduced) image) before working with pure image buffers
  for the Adam7 code, the color convert code and the output to the user.
  in and out are allowed to be the same buffer, in may also be higher but still overlapping; in must
  have >= ilinebits*h bits, out must have >= olinebits*h bits, olinebits must be <= ilinebits
  also used to move bits after earlier such operations happened, e.g. in a sequence of reduced images from Adam7
  only useful if (ilinebits - olinebits) is a value in the range 1..7
  */
  unsigned y;
  size_t diff = ilinebits - olinebits;
  size_t ibp = 0, obp = 0; /*input and output bit pointers*/
  for(y = 0; y < h; ++y)
  {
	size_t x;
	for(x = 0; x < olinebits; ++x)
	{
	  unsigned char bit = readBitFromReversedStream(&ibp, in);
	  setBitOfReversedStream(&obp, out, bit);
	}
	ibp += diff;
  }
}

/*out must be buffer big enough to contain full image, and in must contain the full decompressed data from
the IDAT chunks (with filter index bytes and possible padding bits)
return value is error*/
static unsigned postProcessScanlines(unsigned char* out, unsigned char* in,
									 unsigned w, unsigned h, const LodePNGInfo* info_png)
{
  /*
  This function converts the filtered-padded-interlaced data into pure 2D image buffer with the PNG's colortype.
  Steps:
  *) if no Adam7: 1) unfilter 2) remove padding bits (= posible extra bits per scanline if bpp < 8)
  *) if adam7: 1) 7x unfilter 2) 7x remove padding bits 3) Adam7_deinterlace
  NOTE: the in buffer will be overwritten with intermediate data!
  */
  unsigned bpp = lodepng_get_bpp(&info_png->color);
  if(bpp == 0) return 31; /*error: invalid colortype*/

  if(info_png->interlace_method == 0)
  {
	if(bpp < 8 && w * bpp != ((w * bpp + 7) / 8) * 8)
	{
	  CERROR_TRY_RETURN(unfilter(in, in, w, h, bpp));
	  removePaddingBits(out, in, w * bpp, ((w * bpp + 7) / 8) * 8, h);
	}
	/*we can immediatly filter into the out buffer, no other steps needed*/
	else CERROR_TRY_RETURN(unfilter(out, in, w, h, bpp));
  }
  else /*interlace_method is 1 (Adam7)*/
  {
	unsigned passw[7], passh[7]; size_t filter_passstart[8], padded_passstart[8], passstart[8];
	unsigned i;

	Adam7_getpassvalues(passw, passh, filter_passstart, padded_passstart, passstart, w, h, bpp);

	for(i = 0; i != 7; ++i)
	{
	  CERROR_TRY_RETURN(unfilter(&in[padded_passstart[i]], &in[filter_passstart[i]], passw[i], passh[i], bpp));
	  /*TODO: possible efficiency improvement: if in this reduced image the bits fit nicely in 1 scanline,
	  move bytes instead of bits or move not at all*/
	  if(bpp < 8)
	  {
		/*remove padding bits in scanlines; after this there still may be padding
		bits between the different reduced images: each reduced image still starts nicely at a byte*/
		removePaddingBits(&in[passstart[i]], &in[padded_passstart[i]], passw[i] * bpp,
						  ((passw[i] * bpp + 7) / 8) * 8, passh[i]);
	  }
	}

	Adam7_deinterlace(out, in, w, h, bpp);
  }

  return 0;
}

static unsigned readChunk_PLTE(LodePNGColorMode* color, const unsigned char* data, size_t chunkLength)
{
  unsigned pos = 0, i;
  if(color->palette) lodepng_free(color->palette);
  color->palettesize = chunkLength / 3;
  color->palette = (unsigned char*)lodepng_malloc(4 * color->palettesize);
  if(!color->palette && color->palettesize)
  {
	color->palettesize = 0;
	return 83; /*alloc fail*/
  }
  if(color->palettesize > 256) return 38; /*error: palette too big*/

  for(i = 0; i != color->palettesize; ++i)
  {
	color->palette[4 * i + 0] = data[pos++]; /*R*/
	color->palette[4 * i + 1] = data[pos++]; /*G*/
	color->palette[4 * i + 2] = data[pos++]; /*B*/
	color->palette[4 * i + 3] = 255; /*alpha*/
  }

  return 0; /* OK */
}

static unsigned readChunk_tRNS(LodePNGColorMode* color, const unsigned char* data, size_t chunkLength)
{
  unsigned i;
  if(color->colortype == LCT_PALETTE)
  {
	/*error: more alpha values given than there are palette entries*/
	if(chunkLength > color->palettesize) return 38;

	for(i = 0; i != chunkLength; ++i) color->palette[4 * i + 3] = data[i];
  }
  else if(color->colortype == LCT_GREY)
  {
	/*error: this chunk must be 2 bytes for greyscale image*/
	if(chunkLength != 2) return 30;

	color->key_defined = 1;
	color->key_r = color->key_g = color->key_b = 256u * data[0] + data[1];
  }
  else if(color->colortype == LCT_RGB)
  {
	/*error: this chunk must be 6 bytes for RGB image*/
	if(chunkLength != 6) return 41;

	color->key_defined = 1;
	color->key_r = 256u * data[0] + data[1];
	color->key_g = 256u * data[2] + data[3];
	color->key_b = 256u * data[4] + data[5];
  }
  else return 42; /*error: tRNS chunk not allowed for other color models*/

  return 0; /* OK */
}

#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
/*background color chunk (bKGD)*/
static unsigned readChunk_bKGD(LodePNGInfo* info, const unsigned char* data, size_t chunkLength)
{
  if(info->color.colortype == LCT_PALETTE)
  {
	/*error: this chunk must be 1 byte for indexed color image*/
	if(chunkLength != 1) return 43;

	info->background_defined = 1;
	info->background_r = info->background_g = info->background_b = data[0];
  }
  else if(info->color.colortype == LCT_GREY || info->color.colortype == LCT_GREY_ALPHA)
  {
	/*error: this chunk must be 2 bytes for greyscale image*/
	if(chunkLength != 2) return 44;

	info->background_defined = 1;
	info->background_r = info->background_g = info->background_b = 256u * data[0] + data[1];
  }
  else if(info->color.colortype == LCT_RGB || info->color.colortype == LCT_RGBA)
  {
	/*error: this chunk must be 6 bytes for greyscale image*/
	if(chunkLength != 6) return 45;

	info->background_defined = 1;
	info->background_r = 256u * data[0] + data[1];
	info->background_g = 256u * data[2] + data[3];
	info->background_b = 256u * data[4] + data[5];
  }

  return 0; /* OK */
}

/*text chunk (tEXt)*/
static unsigned readChunk_tEXt(LodePNGInfo* info, const unsigned char* data, size_t chunkLength)
{
  unsigned error = 0;
  char *key = 0, *str = 0;
  unsigned i;

  while(!error) /*not really a while loop, only used to break on error*/
  {
	unsigned length, string2_begin;

	length = 0;
	while(length < chunkLength && data[length] != 0) ++length;
	/*even though it's not allowed by the standard, no error is thrown if
	there's no null termination char, if the text is empty*/
	if(length < 1 || length > 79) CERROR_BREAK(error, 89); /*keyword too short or long*/

	key = (char*)lodepng_malloc(length + 1);
	if(!key) CERROR_BREAK(error, 83); /*alloc fail*/

	key[length] = 0;
	for(i = 0; i != length; ++i) key[i] = (char)data[i];

	string2_begin = length + 1; /*skip keyword null terminator*/

	length = chunkLength < string2_begin ? 0 : chunkLength - string2_begin;
	str = (char*)lodepng_malloc(length + 1);
	if(!str) CERROR_BREAK(error, 83); /*alloc fail*/

	str[length] = 0;
	for(i = 0; i != length; ++i) str[i] = (char)data[string2_begin + i];

	error = lodepng_add_text(info, key, str);

	break;
  }

  lodepng_free(key);
  lodepng_free(str);

  return error;
}

/*compressed text chunk (zTXt)*/
static unsigned readChunk_zTXt(LodePNGInfo* info, const LodePNGDecompressSettings* zlibsettings,
							   const unsigned char* data, size_t chunkLength)
{
  unsigned error = 0;
  unsigned i;

  unsigned length, string2_begin;
  char *key = 0;
  ucvector decoded;

  ucvector_init(&decoded);

  while(!error) /*not really a while loop, only used to break on error*/
  {
	for(length = 0; length < chunkLength && data[length] != 0; ++length) ;
	if(length + 2 >= chunkLength) CERROR_BREAK(error, 75); /*no null termination, corrupt?*/
	if(length < 1 || length > 79) CERROR_BREAK(error, 89); /*keyword too short or long*/

	key = (char*)lodepng_malloc(length + 1);
	if(!key) CERROR_BREAK(error, 83); /*alloc fail*/

	key[length] = 0;
	for(i = 0; i != length; ++i) key[i] = (char)data[i];

	if(data[length + 1] != 0) CERROR_BREAK(error, 72); /*the 0 byte indicating compression must be 0*/

	string2_begin = length + 2;
	if(string2_begin > chunkLength) CERROR_BREAK(error, 75); /*no null termination, corrupt?*/

	length = chunkLength - string2_begin;
	/*will fail if zlib error, e.g. if length is too small*/
	error = zlib_decompress(&decoded.data, &decoded.size,
							(unsigned char*)(&data[string2_begin]),
							length, zlibsettings);
	if(error) break;
	ucvector_push_back(&decoded, 0);

	error = lodepng_add_text(info, key, (char*)decoded.data);

	break;
  }

  lodepng_free(key);
  ucvector_cleanup(&decoded);

  return error;
}

/*international text chunk (iTXt)*/
static unsigned readChunk_iTXt(LodePNGInfo* info, const LodePNGDecompressSettings* zlibsettings,
							   const unsigned char* data, size_t chunkLength)
{
  unsigned error = 0;
  unsigned i;

  unsigned length, begin, compressed;
  char *key = 0, *langtag = 0, *transkey = 0;
  ucvector decoded;
  ucvector_init(&decoded);

  while(!error) /*not really a while loop, only used to break on error*/
  {
	/*Quick check if the chunk length isn't too small. Even without check
	it'd still fail with other error checks below if it's too short. This just gives a different error code.*/
	if(chunkLength < 5) CERROR_BREAK(error, 30); /*iTXt chunk too short*/

	/*read the key*/
	for(length = 0; length < chunkLength && data[length] != 0; ++length) ;
	if(length + 3 >= chunkLength) CERROR_BREAK(error, 75); /*no null termination char, corrupt?*/
	if(length < 1 || length > 79) CERROR_BREAK(error, 89); /*keyword too short or long*/

	key = (char*)lodepng_malloc(length + 1);
	if(!key) CERROR_BREAK(error, 83); /*alloc fail*/

	key[length] = 0;
	for(i = 0; i != length; ++i) key[i] = (char)data[i];

	/*read the compression method*/
	compressed = data[length + 1];
	if(data[length + 2] != 0) CERROR_BREAK(error, 72); /*the 0 byte indicating compression must be 0*/

	/*even though it's not allowed by the standard, no error is thrown if
	there's no null termination char, if the text is empty for the next 3 texts*/

	/*read the langtag*/
	begin = length + 3;
	length = 0;
	for(i = begin; i < chunkLength && data[i] != 0; ++i) ++length;

	langtag = (char*)lodepng_malloc(length + 1);
	if(!langtag) CERROR_BREAK(error, 83); /*alloc fail*/

	langtag[length] = 0;
	for(i = 0; i != length; ++i) langtag[i] = (char)data[begin + i];

	/*read the transkey*/
	begin += length + 1;
	length = 0;
	for(i = begin; i < chunkLength && data[i] != 0; ++i) ++length;

	transkey = (char*)lodepng_malloc(length + 1);
	if(!transkey) CERROR_BREAK(error, 83); /*alloc fail*/

	transkey[length] = 0;
	for(i = 0; i != length; ++i) transkey[i] = (char)data[begin + i];

	/*read the actual text*/
	begin += length + 1;

	length = chunkLength < begin ? 0 : chunkLength - begin;

	if(compressed)
	{
	  /*will fail if zlib error, e.g. if length is too small*/
	  error = zlib_decompress(&decoded.data, &decoded.size,
							  (unsigned char*)(&data[begin]),
							  length, zlibsettings);
	  if(error) break;
	  if(decoded.allocsize < decoded.size) decoded.allocsize = decoded.size;
	  ucvector_push_back(&decoded, 0);
	}
	else
	{
	  if(!ucvector_resize(&decoded, length + 1)) CERROR_BREAK(error, 83 /*alloc fail*/);

	  decoded.data[length] = 0;
	  for(i = 0; i != length; ++i) decoded.data[i] = data[begin + i];
	}

	error = lodepng_add_itext(info, key, langtag, transkey, (char*)decoded.data);

	break;
  }

  lodepng_free(key);
  lodepng_free(langtag);
  lodepng_free(transkey);
  ucvector_cleanup(&decoded);

  return error;
}

static unsigned readChunk_tIME(LodePNGInfo* info, const unsigned char* data, size_t chunkLength)
{
  if(chunkLength != 7) return 73; /*invalid tIME chunk size*/

  info->time_defined = 1;
  info->time.year = 256u * data[0] + data[1];
  info->time.month = data[2];
  info->time.day = data[3];
  info->time.hour = data[4];
  info->time.minute = data[5];
  info->time.second = data[6];

  return 0; /* OK */
}

static unsigned readChunk_pHYs(LodePNGInfo* info, const unsigned char* data, size_t chunkLength)
{
  if(chunkLength != 9) return 74; /*invalid pHYs chunk size*/

  info->phys_defined = 1;
  info->phys_x = 16777216u * data[0] + 65536u * data[1] + 256u * data[2] + data[3];
  info->phys_y = 16777216u * data[4] + 65536u * data[5] + 256u * data[6] + data[7];
  info->phys_unit = data[8];

  return 0; /* OK */
}
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/

/*read a PNG, the result will be in the same color type as the PNG (hence "generic")*/
static void decodeGeneric(unsigned char** out, unsigned* w, unsigned* h,
						  LodePNGState* state,
						  const unsigned char* in, size_t insize)
{
  unsigned char IEND = 0;
  const unsigned char* chunk;
  size_t i;
  ucvector idat; /*the data from idat chunks*/
  ucvector scanlines;
  size_t predict;
  size_t numpixels;

  /*for unknown chunk order*/
  unsigned unknown = 0;
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
  unsigned critical_pos = 1; /*1 = after IHDR, 2 = after PLTE, 3 = after IDAT*/
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/

  /*provide some proper output values if error will happen*/
  *out = 0;

  state->error = lodepng_inspect(w, h, state, in, insize); /*reads header and resets other parameters in state->info_png*/
  if(state->error) return;

  numpixels = *w * *h;

  /*multiplication overflow*/
  if(*h != 0 && numpixels / *h != *w) CERROR_RETURN(state->error, 92);
  /*multiplication overflow possible further below. Allows up to 2^31-1 pixel
  bytes with 16-bit RGBA, the rest is room for filter bytes.*/
  if(numpixels > 268435455) CERROR_RETURN(state->error, 92);

  ucvector_init(&idat);
  chunk = &in[33]; /*first byte of the first chunk after the header*/

  /*loop through the chunks, ignoring unknown chunks and stopping at IEND chunk.
  IDAT data is put at the start of the in buffer*/
  while(!IEND && !state->error)
  {
	unsigned chunkLength;
	const unsigned char* data; /*the data in the chunk*/

	/*error: size of the in buffer too small to contain next chunk*/
	if((size_t)((chunk - in) + 12) > insize || chunk < in) CERROR_BREAK(state->error, 30);

	/*length of the data of the chunk, excluding the length bytes, chunk type and CRC bytes*/
	chunkLength = lodepng_chunk_length(chunk);
	/*error: chunk length larger than the max PNG chunk size*/
	if(chunkLength > 2147483647) CERROR_BREAK(state->error, 63);

	if((size_t)((chunk - in) + chunkLength + 12) > insize || (chunk + chunkLength + 12) < in)
	{
	  CERROR_BREAK(state->error, 64); /*error: size of the in buffer too small to contain next chunk*/
	}

	data = lodepng_chunk_data_const(chunk);

	/*IDAT chunk, containing compressed image data*/
	if(lodepng_chunk_type_equals(chunk, "IDAT"))
	{
	  size_t oldsize = idat.size;
	  if(!ucvector_resize(&idat, oldsize + chunkLength)) CERROR_BREAK(state->error, 83 /*alloc fail*/);
	  for(i = 0; i != chunkLength; ++i) idat.data[oldsize + i] = data[i];
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
	  critical_pos = 3;
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
	}
	/*IEND chunk*/
	else if(lodepng_chunk_type_equals(chunk, "IEND"))
	{
	  IEND = 1;
	}
	/*palette chunk (PLTE)*/
	else if(lodepng_chunk_type_equals(chunk, "PLTE"))
	{
	  state->error = readChunk_PLTE(&state->info_png.color, data, chunkLength);
	  if(state->error) break;
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
	  critical_pos = 2;
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
	}
	/*palette transparency chunk (tRNS)*/
	else if(lodepng_chunk_type_equals(chunk, "tRNS"))
	{
	  state->error = readChunk_tRNS(&state->info_png.color, data, chunkLength);
	  if(state->error) break;
	}
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
	/*background color chunk (bKGD)*/
	else if(lodepng_chunk_type_equals(chunk, "bKGD"))
	{
	  state->error = readChunk_bKGD(&state->info_png, data, chunkLength);
	  if(state->error) break;
	}
	/*text chunk (tEXt)*/
	else if(lodepng_chunk_type_equals(chunk, "tEXt"))
	{
	  if(state->decoder.read_text_chunks)
	  {
		state->error = readChunk_tEXt(&state->info_png, data, chunkLength);
		if(state->error) break;
	  }
	}
	/*compressed text chunk (zTXt)*/
	else if(lodepng_chunk_type_equals(chunk, "zTXt"))
	{
	  if(state->decoder.read_text_chunks)
	  {
		state->error = readChunk_zTXt(&state->info_png, &state->decoder.zlibsettings, data, chunkLength);
		if(state->error) break;
	  }
	}
	/*international text chunk (iTXt)*/
	else if(lodepng_chunk_type_equals(chunk, "iTXt"))
	{
	  if(state->decoder.read_text_chunks)
	  {
		state->error = readChunk_iTXt(&state->info_png, &state->decoder.zlibsettings, data, chunkLength);
		if(state->error) break;
	  }
	}
	else if(lodepng_chunk_type_equals(chunk, "tIME"))
	{
	  state->error = readChunk_tIME(&state->info_png, data, chunkLength);
	  if(state->error) break;
	}
	else if(lodepng_chunk_type_equals(chunk, "pHYs"))
	{
	  state->error = readChunk_pHYs(&state->info_png, data, chunkLength);
	  if(state->error) break;
	}
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
	else /*it's not an implemented chunk type, so ignore it: skip over the data*/
	{
	  /*error: unknown critical chunk (5th bit of first byte of chunk type is 0)*/
	  if(!lodepng_chunk_ancillary(chunk)) CERROR_BREAK(state->error, 69);

	  unknown = 1;
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
	  if(state->decoder.remember_unknown_chunks)
	  {
		state->error = lodepng_chunk_append(&state->info_png.unknown_chunks_data[critical_pos - 1],
											&state->info_png.unknown_chunks_size[critical_pos - 1], chunk);
		if(state->error) break;
	  }
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
	}

	if(!state->decoder.ignore_crc && !unknown) /*check CRC if wanted, only on known chunk types*/
	{
	  if(lodepng_chunk_check_crc(chunk)) CERROR_BREAK(state->error, 57); /*invalid CRC*/
	}

	if(!IEND) chunk = lodepng_chunk_next_const(chunk);
  }

  ucvector_init(&scanlines);
  /*predict output size, to allocate exact size for output buffer to avoid more dynamic allocation.
  If the decompressed size does not match the prediction, the image must be corrupt.*/
  if(state->info_png.interlace_method == 0)
  {
	/*The extra *h is added because this are the filter bytes every scanline starts with*/
	predict = lodepng_get_raw_size_idat(*w, *h, &state->info_png.color) + *h;
  }
  else
  {
	/*Adam-7 interlaced: predicted size is the sum of the 7 sub-images sizes*/
	const LodePNGColorMode* color = &state->info_png.color;
	predict = 0;
	predict += lodepng_get_raw_size_idat((*w + 7) / 8, (*h + 7) / 8, color) + (*h + 7) / 8;
	if(*w > 4) predict += lodepng_get_raw_size_idat((*w + 3) / 8, (*h + 7) / 8, color) + (*h + 7) / 8;
	predict += lodepng_get_raw_size_idat((*w + 3) / 4, (*h + 3) / 8, color) + (*h + 3) / 8;
	if(*w > 2) predict += lodepng_get_raw_size_idat((*w + 1) / 4, (*h + 3) / 4, color) + (*h + 3) / 4;
	predict += lodepng_get_raw_size_idat((*w + 1) / 2, (*h + 1) / 4, color) + (*h + 1) / 4;
	if(*w > 1) predict += lodepng_get_raw_size_idat((*w + 0) / 2, (*h + 1) / 2, color) + (*h + 1) / 2;
	predict += lodepng_get_raw_size_idat((*w + 0) / 1, (*h + 0) / 2, color) + (*h + 0) / 2;
  }
  if(!state->error && !ucvector_reserve(&scanlines, predict)) state->error = 83; /*alloc fail*/
  if(!state->error)
  {
	state->error = zlib_decompress(&scanlines.data, &scanlines.size, idat.data,
								   idat.size, &state->decoder.zlibsettings);
	if(!state->error && scanlines.size != predict) state->error = 91; /*decompressed size doesn't match prediction*/
  }
  ucvector_cleanup(&idat);

  if(!state->error)
  {
	size_t outsize = lodepng_get_raw_size(*w, *h, &state->info_png.color);
	ucvector outv;
	ucvector_init(&outv);
	if(!ucvector_resizev(&outv, outsize, 0)) state->error = 83; /*alloc fail*/
	if(!state->error) state->error = postProcessScanlines(outv.data, scanlines.data, *w, *h, &state->info_png);
	*out = outv.data;
  }
  ucvector_cleanup(&scanlines);
}

unsigned lodepng_decode(unsigned char** out, unsigned* w, unsigned* h,
						LodePNGState* state,
						const unsigned char* in, size_t insize)
{
  *out = 0;
  decodeGeneric(out, w, h, state, in, insize);
  if(state->error) return state->error;
  if(!state->decoder.color_convert || lodepng_color_mode_equal(&state->info_raw, &state->info_png.color))
  {
	/*same color type, no copying or converting of data needed*/
	/*store the info_png color settings on the info_raw so that the info_raw still reflects what colortype
	the raw image has to the end user*/
	if(!state->decoder.color_convert)
	{
	  state->error = lodepng_color_mode_copy(&state->info_raw, &state->info_png.color);
	  if(state->error) return state->error;
	}
  }
  else
  {
	/*color conversion needed; sort of copy of the data*/
	unsigned char* data = *out;
	size_t outsize;

	/*TODO: check if this works according to the statement in the documentation: "The converter can convert
	from greyscale input color type, to 8-bit greyscale or greyscale with alpha"*/
	if(!(state->info_raw.colortype == LCT_RGB || state->info_raw.colortype == LCT_RGBA)
	   && !(state->info_raw.bitdepth == 8))
	{
	  return 56; /*unsupported color mode conversion*/
	}

	outsize = lodepng_get_raw_size(*w, *h, &state->info_raw);
	*out = (unsigned char*)lodepng_malloc(outsize);
	if(!(*out))
	{
	  state->error = 83; /*alloc fail*/
	}
	else state->error = lodepng_convert(*out, data, &state->info_raw,
										&state->info_png.color, *w, *h);
	lodepng_free(data);
  }
  return state->error;
}

unsigned lodepng_decode_memory(unsigned char** out, unsigned* w, unsigned* h, const unsigned char* in,
							   size_t insize, LodePNGColorType colortype, unsigned bitdepth)
{
  unsigned error;
  LodePNGState state;
  lodepng_state_init(&state);
  state.info_raw.colortype = colortype;
  state.info_raw.bitdepth = bitdepth;
  error = lodepng_decode(out, w, h, &state, in, insize);
  lodepng_state_cleanup(&state);
  return error;
}

unsigned lodepng_decode32(unsigned char** out, unsigned* w, unsigned* h, const unsigned char* in, size_t insize)
{
  return lodepng_decode_memory(out, w, h, in, insize, LCT_RGBA, 8);
}

unsigned lodepng_decode24(unsigned char** out, unsigned* w, unsigned* h, const unsigned char* in, size_t insize)
{
  return lodepng_decode_memory(out, w, h, in, insize, LCT_RGB, 8);
}

#ifdef LODEPNG_COMPILE_DISK
unsigned lodepng_decode_file(unsigned char** out, unsigned* w, unsigned* h, const char* filename,
							 LodePNGColorType colortype, unsigned bitdepth)
{
  unsigned char* buffer;
  size_t buffersize;
  unsigned error;
  error = lodepng_load_file(&buffer, &buffersize, filename);
  if(!error) error = lodepng_decode_memory(out, w, h, buffer, buffersize, colortype, bitdepth);
  lodepng_free(buffer);
  return error;
}

unsigned lodepng_decode32_file(unsigned char** out, unsigned* w, unsigned* h, const char* filename)
{
  return lodepng_decode_file(out, w, h, filename, LCT_RGBA, 8);
}

unsigned lodepng_decode24_file(unsigned char** out, unsigned* w, unsigned* h, const char* filename)
{
  return lodepng_decode_file(out, w, h, filename, LCT_RGB, 8);
}
#endif /*LODEPNG_COMPILE_DISK*/

void lodepng_decoder_settings_init(LodePNGDecoderSettings* settings)
{
  settings->color_convert = 1;
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
  settings->read_text_chunks = 1;
  settings->remember_unknown_chunks = 0;
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
  settings->ignore_crc = 0;
  lodepng_decompress_settings_init(&settings->zlibsettings);
}

#endif /*LODEPNG_COMPILE_DECODER*/

#if defined(LODEPNG_COMPILE_DECODER) || defined(LODEPNG_COMPILE_ENCODER)

void lodepng_state_init(LodePNGState* state)
{
#ifdef LODEPNG_COMPILE_DECODER
  lodepng_decoder_settings_init(&state->decoder);
#endif /*LODEPNG_COMPILE_DECODER*/
#ifdef LODEPNG_COMPILE_ENCODER
  lodepng_encoder_settings_init(&state->encoder);
#endif /*LODEPNG_COMPILE_ENCODER*/
  lodepng_color_mode_init(&state->info_raw);
  lodepng_info_init(&state->info_png);
  state->error = 1;
}

void lodepng_state_cleanup(LodePNGState* state)
{
  lodepng_color_mode_cleanup(&state->info_raw);
  lodepng_info_cleanup(&state->info_png);
}

void lodepng_state_copy(LodePNGState* dest, const LodePNGState* source)
{
  lodepng_state_cleanup(dest);
  *dest = *source;
  lodepng_color_mode_init(&dest->info_raw);
  lodepng_info_init(&dest->info_png);
  dest->error = lodepng_color_mode_copy(&dest->info_raw, &source->info_raw); if(dest->error) return;
  dest->error = lodepng_info_copy(&dest->info_png, &source->info_png); if(dest->error) return;
}

#endif /* defined(LODEPNG_COMPILE_DECODER) || defined(LODEPNG_COMPILE_ENCODER) */

#ifdef LODEPNG_COMPILE_ENCODER

/* ////////////////////////////////////////////////////////////////////////// */
/* / PNG Encoder                                                            / */
/* ////////////////////////////////////////////////////////////////////////// */

/*chunkName must be string of 4 characters*/
static unsigned addChunk(ucvector* out, const char* chunkName, const unsigned char* data, size_t length)
{
  CERROR_TRY_RETURN(lodepng_chunk_create(&out->data, &out->size, (unsigned)length, chunkName, data));
  out->allocsize = out->size; /*fix the allocsize again*/
  return 0;
}

static void writeSignature(ucvector* out)
{
  /*8 bytes PNG signature, aka the magic bytes*/
  ucvector_push_back(out, 137);
  ucvector_push_back(out, 80);
  ucvector_push_back(out, 78);
  ucvector_push_back(out, 71);
  ucvector_push_back(out, 13);
  ucvector_push_back(out, 10);
  ucvector_push_back(out, 26);
  ucvector_push_back(out, 10);
}

static unsigned addChunk_IHDR(ucvector* out, unsigned w, unsigned h,
							  LodePNGColorType colortype, unsigned bitdepth, unsigned interlace_method)
{
  unsigned error = 0;
  ucvector header;
  ucvector_init(&header);

  lodepng_add32bitInt(&header, w); /*width*/
  lodepng_add32bitInt(&header, h); /*height*/
  ucvector_push_back(&header, (unsigned char)bitdepth); /*bit depth*/
  ucvector_push_back(&header, (unsigned char)colortype); /*color type*/
  ucvector_push_back(&header, 0); /*compression method*/
  ucvector_push_back(&header, 0); /*filter method*/
  ucvector_push_back(&header, interlace_method); /*interlace method*/

  error = addChunk(out, "IHDR", header.data, header.size);
  ucvector_cleanup(&header);

  return error;
}

static unsigned addChunk_PLTE(ucvector* out, const LodePNGColorMode* info)
{
  unsigned error = 0;
  size_t i;
  ucvector PLTE;
  ucvector_init(&PLTE);
  for(i = 0; i != info->palettesize * 4; ++i)
  {
	/*add all channels except alpha channel*/
	if(i % 4 != 3) ucvector_push_back(&PLTE, info->palette[i]);
  }
  error = addChunk(out, "PLTE", PLTE.data, PLTE.size);
  ucvector_cleanup(&PLTE);

  return error;
}

static unsigned addChunk_tRNS(ucvector* out, const LodePNGColorMode* info)
{
  unsigned error = 0;
  size_t i;
  ucvector tRNS;
  ucvector_init(&tRNS);
  if(info->colortype == LCT_PALETTE)
  {
	size_t amount = info->palettesize;
	/*the tail of palette values that all have 255 as alpha, does not have to be encoded*/
	for(i = info->palettesize; i != 0; --i)
	{
	  if(info->palette[4 * (i - 1) + 3] == 255) --amount;
	  else break;
	}
	/*add only alpha channel*/
	for(i = 0; i != amount; ++i) ucvector_push_back(&tRNS, info->palette[4 * i + 3]);
  }
  else if(info->colortype == LCT_GREY)
  {
	if(info->key_defined)
	{
	  ucvector_push_back(&tRNS, (unsigned char)(info->key_r / 256));
	  ucvector_push_back(&tRNS, (unsigned char)(info->key_r % 256));
	}
  }
  else if(info->colortype == LCT_RGB)
  {
	if(info->key_defined)
	{
	  ucvector_push_back(&tRNS, (unsigned char)(info->key_r / 256));
	  ucvector_push_back(&tRNS, (unsigned char)(info->key_r % 256));
	  ucvector_push_back(&tRNS, (unsigned char)(info->key_g / 256));
	  ucvector_push_back(&tRNS, (unsigned char)(info->key_g % 256));
	  ucvector_push_back(&tRNS, (unsigned char)(info->key_b / 256));
	  ucvector_push_back(&tRNS, (unsigned char)(info->key_b % 256));
	}
  }

  error = addChunk(out, "tRNS", tRNS.data, tRNS.size);
  ucvector_cleanup(&tRNS);

  return error;
}

static unsigned addChunk_IDAT(ucvector* out, const unsigned char* data, size_t datasize,
							  LodePNGCompressSettings* zlibsettings)
{
  ucvector zlibdata;
  unsigned error = 0;

  /*compress with the Zlib compressor*/
  ucvector_init(&zlibdata);
  error = zlib_compress(&zlibdata.data, &zlibdata.size, data, datasize, zlibsettings);
  if(!error) error = addChunk(out, "IDAT", zlibdata.data, zlibdata.size);
  ucvector_cleanup(&zlibdata);

  return error;
}

static unsigned addChunk_IEND(ucvector* out)
{
  unsigned error = 0;
  error = addChunk(out, "IEND", 0, 0);
  return error;
}

#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS

static unsigned addChunk_tEXt(ucvector* out, const char* keyword, const char* textstring)
{
  unsigned error = 0;
  size_t i;
  ucvector text;
  ucvector_init(&text);
  for(i = 0; keyword[i] != 0; ++i) ucvector_push_back(&text, (unsigned char)keyword[i]);
  if(i < 1 || i > 79) return 89; /*error: invalid keyword size*/
  ucvector_push_back(&text, 0); /*0 termination char*/
  for(i = 0; textstring[i] != 0; ++i) ucvector_push_back(&text, (unsigned char)textstring[i]);
  error = addChunk(out, "tEXt", text.data, text.size);
  ucvector_cleanup(&text);

  return error;
}

static unsigned addChunk_zTXt(ucvector* out, const char* keyword, const char* textstring,
							  LodePNGCompressSettings* zlibsettings)
{
  unsigned error = 0;
  ucvector data, compressed;
  size_t i, textsize = strlen(textstring);

  ucvector_init(&data);
  ucvector_init(&compressed);
  for(i = 0; keyword[i] != 0; ++i) ucvector_push_back(&data, (unsigned char)keyword[i]);
  if(i < 1 || i > 79) return 89; /*error: invalid keyword size*/
  ucvector_push_back(&data, 0); /*0 termination char*/
  ucvector_push_back(&data, 0); /*compression method: 0*/

  error = zlib_compress(&compressed.data, &compressed.size,
						(unsigned char*)textstring, textsize, zlibsettings);
  if(!error)
  {
	for(i = 0; i != compressed.size; ++i) ucvector_push_back(&data, compressed.data[i]);
	error = addChunk(out, "zTXt", data.data, data.size);
  }

  ucvector_cleanup(&compressed);
  ucvector_cleanup(&data);
  return error;
}

static unsigned addChunk_iTXt(ucvector* out, unsigned compressed, const char* keyword, const char* langtag,
							  const char* transkey, const char* textstring, LodePNGCompressSettings* zlibsettings)
{
  unsigned error = 0;
  ucvector data;
  size_t i, textsize = strlen(textstring);

  ucvector_init(&data);

  for(i = 0; keyword[i] != 0; ++i) ucvector_push_back(&data, (unsigned char)keyword[i]);
  if(i < 1 || i > 79) return 89; /*error: invalid keyword size*/
  ucvector_push_back(&data, 0); /*null termination char*/
  ucvector_push_back(&data, compressed ? 1 : 0); /*compression flag*/
  ucvector_push_back(&data, 0); /*compression method*/
  for(i = 0; langtag[i] != 0; ++i) ucvector_push_back(&data, (unsigned char)langtag[i]);
  ucvector_push_back(&data, 0); /*null termination char*/
  for(i = 0; transkey[i] != 0; ++i) ucvector_push_back(&data, (unsigned char)transkey[i]);
  ucvector_push_back(&data, 0); /*null termination char*/

  if(compressed)
  {
	ucvector compressed_data;
	ucvector_init(&compressed_data);
	error = zlib_compress(&compressed_data.data, &compressed_data.size,
						  (unsigned char*)textstring, textsize, zlibsettings);
	if(!error)
	{
	  for(i = 0; i != compressed_data.size; ++i) ucvector_push_back(&data, compressed_data.data[i]);
	}
	ucvector_cleanup(&compressed_data);
  }
  else /*not compressed*/
  {
	for(i = 0; textstring[i] != 0; ++i) ucvector_push_back(&data, (unsigned char)textstring[i]);
  }

  if(!error) error = addChunk(out, "iTXt", data.data, data.size);
  ucvector_cleanup(&data);
  return error;
}

static unsigned addChunk_bKGD(ucvector* out, const LodePNGInfo* info)
{
  unsigned error = 0;
  ucvector bKGD;
  ucvector_init(&bKGD);
  if(info->color.colortype == LCT_GREY || info->color.colortype == LCT_GREY_ALPHA)
  {
	ucvector_push_back(&bKGD, (unsigned char)(info->background_r / 256));
	ucvector_push_back(&bKGD, (unsigned char)(info->background_r % 256));
  }
  else if(info->color.colortype == LCT_RGB || info->color.colortype == LCT_RGBA)
  {
	ucvector_push_back(&bKGD, (unsigned char)(info->background_r / 256));
	ucvector_push_back(&bKGD, (unsigned char)(info->background_r % 256));
	ucvector_push_back(&bKGD, (unsigned char)(info->background_g / 256));
	ucvector_push_back(&bKGD, (unsigned char)(info->background_g % 256));
	ucvector_push_back(&bKGD, (unsigned char)(info->background_b / 256));
	ucvector_push_back(&bKGD, (unsigned char)(info->background_b % 256));
  }
  else if(info->color.colortype == LCT_PALETTE)
  {
	ucvector_push_back(&bKGD, (unsigned char)(info->background_r % 256)); /*palette index*/
  }

  error = addChunk(out, "bKGD", bKGD.data, bKGD.size);
  ucvector_cleanup(&bKGD);

  return error;
}

static unsigned addChunk_tIME(ucvector* out, const LodePNGTime* time)
{
  unsigned error = 0;
  unsigned char* data = (unsigned char*)lodepng_malloc(7);
  if(!data) return 83; /*alloc fail*/
  data[0] = (unsigned char)(time->year / 256);
  data[1] = (unsigned char)(time->year % 256);
  data[2] = (unsigned char)time->month;
  data[3] = (unsigned char)time->day;
  data[4] = (unsigned char)time->hour;
  data[5] = (unsigned char)time->minute;
  data[6] = (unsigned char)time->second;
  error = addChunk(out, "tIME", data, 7);
  lodepng_free(data);
  return error;
}

static unsigned addChunk_pHYs(ucvector* out, const LodePNGInfo* info)
{
  unsigned error = 0;
  ucvector data;
  ucvector_init(&data);

  lodepng_add32bitInt(&data, info->phys_x);
  lodepng_add32bitInt(&data, info->phys_y);
  ucvector_push_back(&data, info->phys_unit);

  error = addChunk(out, "pHYs", data.data, data.size);
  ucvector_cleanup(&data);

  return error;
}

#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/

static void filterScanline(unsigned char* out, const unsigned char* scanline, const unsigned char* prevline,
						   size_t length, size_t bytewidth, unsigned char filterType)
{
  size_t i;
  switch(filterType)
  {
	case 0: /*None*/
	  for(i = 0; i != length; ++i) out[i] = scanline[i];
	  break;
	case 1: /*Sub*/
	  for(i = 0; i != bytewidth; ++i) out[i] = scanline[i];
	  for(i = bytewidth; i < length; ++i) out[i] = scanline[i] - scanline[i - bytewidth];
	  break;
	case 2: /*Up*/
	  if(prevline)
	  {
		for(i = 0; i != length; ++i) out[i] = scanline[i] - prevline[i];
	  }
	  else
	  {
		for(i = 0; i != length; ++i) out[i] = scanline[i];
	  }
	  break;
	case 3: /*Average*/
	  if(prevline)
	  {
		for(i = 0; i != bytewidth; ++i) out[i] = scanline[i] - prevline[i] / 2;
		for(i = bytewidth; i < length; ++i) out[i] = scanline[i] - ((scanline[i - bytewidth] + prevline[i]) / 2);
	  }
	  else
	  {
		for(i = 0; i != bytewidth; ++i) out[i] = scanline[i];
		for(i = bytewidth; i < length; ++i) out[i] = scanline[i] - scanline[i - bytewidth] / 2;
	  }
	  break;
	case 4: /*Paeth*/
	  if(prevline)
	  {
		/*paethPredictor(0, prevline[i], 0) is always prevline[i]*/
		for(i = 0; i != bytewidth; ++i) out[i] = (scanline[i] - prevline[i]);
		for(i = bytewidth; i < length; ++i)
		{
		  out[i] = (scanline[i] - paethPredictor(scanline[i - bytewidth], prevline[i], prevline[i - bytewidth]));
		}
	  }
	  else
	  {
		for(i = 0; i != bytewidth; ++i) out[i] = scanline[i];
		/*paethPredictor(scanline[i - bytewidth], 0, 0) is always scanline[i - bytewidth]*/
		for(i = bytewidth; i < length; ++i) out[i] = (scanline[i] - scanline[i - bytewidth]);
	  }
	  break;
	default: return; /*unexisting filter type given*/
  }
}

/* log2 approximation. A slight bit faster than std::log. */
static float flog2(float f)
{
  float result = 0;
  while(f > 32) { result += 4; f /= 16; }
  while(f > 2) { ++result; f /= 2; }
  return result + 1.442695f * (f * f * f / 3 - 3 * f * f / 2 + 3 * f - 1.83333f);
}

static unsigned filter(unsigned char* out, const unsigned char* in, unsigned w, unsigned h,
					   const LodePNGColorMode* info, const LodePNGEncoderSettings* settings)
{
  /*
  For PNG filter method 0
  out must be a buffer with as size: h + (w * h * bpp + 7) / 8, because there are
  the scanlines with 1 extra byte per scanline
  */

  unsigned bpp = lodepng_get_bpp(info);
  /*the width of a scanline in bytes, not including the filter type*/
  size_t linebytes = (w * bpp + 7) / 8;
  /*bytewidth is used for filtering, is 1 when bpp < 8, number of bytes per pixel otherwise*/
  size_t bytewidth = (bpp + 7) / 8;
  const unsigned char* prevline = 0;
  unsigned x, y;
  unsigned error = 0;
  LodePNGFilterStrategy strategy = settings->filter_strategy;

  /*
  There is a heuristic called the minimum sum of absolute differences heuristic, suggested by the PNG standard:
   *  If the image type is Palette, or the bit depth is smaller than 8, then do not filter the image (i.e.
	  use fixed filtering, with the filter None).
   * (The other case) If the image type is Grayscale or RGB (with or without Alpha), and the bit depth is
	 not smaller than 8, then use adaptive filtering heuristic as follows: independently for each row, apply
	 all five filters and select the filter that produces the smallest sum of absolute values per row.
  This heuristic is used if filter strategy is LFS_MINSUM and filter_palette_zero is true.

  If filter_palette_zero is true and filter_strategy is not LFS_MINSUM, the above heuristic is followed,
  but for "the other case", whatever strategy filter_strategy is set to instead of the minimum sum
  heuristic is used.
  */
  if(settings->filter_palette_zero &&
	 (info->colortype == LCT_PALETTE || info->bitdepth < 8)) strategy = LFS_ZERO;

  if(bpp == 0) return 31; /*error: invalid color type*/

  if(strategy == LFS_ZERO)
  {
	for(y = 0; y != h; ++y)
	{
	  size_t outindex = (1 + linebytes) * y; /*the extra filterbyte added to each row*/
	  size_t inindex = linebytes * y;
	  out[outindex] = 0; /*filter type byte*/
	  filterScanline(&out[outindex + 1], &in[inindex], prevline, linebytes, bytewidth, 0);
	  prevline = &in[inindex];
	}
  }
  else if(strategy == LFS_MINSUM)
  {
	/*adaptive filtering*/
	size_t sum[5];
	ucvector attempt[5]; /*five filtering attempts, one for each filter type*/
	size_t smallest = 0;
	unsigned char type, bestType = 0;

	for(type = 0; type != 5; ++type)
	{
	  ucvector_init(&attempt[type]);
	  if(!ucvector_resize(&attempt[type], linebytes)) return 83; /*alloc fail*/
	}

	if(!error)
	{
	  for(y = 0; y != h; ++y)
	  {
		/*try the 5 filter types*/
		for(type = 0; type != 5; ++type)
		{
		  filterScanline(attempt[type].data, &in[y * linebytes], prevline, linebytes, bytewidth, type);

		  /*calculate the sum of the result*/
		  sum[type] = 0;
		  if(type == 0)
		  {
			for(x = 0; x != linebytes; ++x) sum[type] += (unsigned char)(attempt[type].data[x]);
		  }
		  else
		  {
			for(x = 0; x != linebytes; ++x)
			{
			  /*For differences, each byte should be treated as signed, values above 127 are negative
			  (converted to signed char). Filtertype 0 isn't a difference though, so use unsigned there.
			  This means filtertype 0 is almost never chosen, but that is justified.*/
			  unsigned char s = attempt[type].data[x];
			  sum[type] += s < 128 ? s : (255U - s);
			}
		  }

		  /*check if this is smallest sum (or if type == 0 it's the first case so always store the values)*/
		  if(type == 0 || sum[type] < smallest)
		  {
			bestType = type;
			smallest = sum[type];
		  }
		}

		prevline = &in[y * linebytes];

		/*now fill the out values*/
		out[y * (linebytes + 1)] = bestType; /*the first byte of a scanline will be the filter type*/
		for(x = 0; x != linebytes; ++x) out[y * (linebytes + 1) + 1 + x] = attempt[bestType].data[x];
	  }
	}

	for(type = 0; type != 5; ++type) ucvector_cleanup(&attempt[type]);
  }
  else if(strategy == LFS_ENTROPY)
  {
	float sum[5];
	ucvector attempt[5]; /*five filtering attempts, one for each filter type*/
	float smallest = 0;
	unsigned type, bestType = 0;
	unsigned count[256];

	for(type = 0; type != 5; ++type)
	{
	  ucvector_init(&attempt[type]);
	  if(!ucvector_resize(&attempt[type], linebytes)) return 83; /*alloc fail*/
	}

	for(y = 0; y != h; ++y)
	{
	  /*try the 5 filter types*/
	  for(type = 0; type != 5; ++type)
	  {
		filterScanline(attempt[type].data, &in[y * linebytes], prevline, linebytes, bytewidth, type);
		for(x = 0; x != 256; ++x) count[x] = 0;
		for(x = 0; x != linebytes; ++x) ++count[attempt[type].data[x]];
		++count[type]; /*the filter type itself is part of the scanline*/
		sum[type] = 0;
		for(x = 0; x != 256; ++x)
		{
		  float p = count[x] / (float)(linebytes + 1);
		  sum[type] += count[x] == 0 ? 0 : flog2(1 / p) * p;
		}
		/*check if this is smallest sum (or if type == 0 it's the first case so always store the values)*/
		if(type == 0 || sum[type] < smallest)
		{
		  bestType = type;
		  smallest = sum[type];
		}
	  }

	  prevline = &in[y * linebytes];

	  /*now fill the out values*/
	  out[y * (linebytes + 1)] = bestType; /*the first byte of a scanline will be the filter type*/
	  for(x = 0; x != linebytes; ++x) out[y * (linebytes + 1) + 1 + x] = attempt[bestType].data[x];
	}

	for(type = 0; type != 5; ++type) ucvector_cleanup(&attempt[type]);
  }
  else if(strategy == LFS_PREDEFINED)
  {
	for(y = 0; y != h; ++y)
	{
	  size_t outindex = (1 + linebytes) * y; /*the extra filterbyte added to each row*/
	  size_t inindex = linebytes * y;
	  unsigned char type = settings->predefined_filters[y];
	  out[outindex] = type; /*filter type byte*/
	  filterScanline(&out[outindex + 1], &in[inindex], prevline, linebytes, bytewidth, type);
	  prevline = &in[inindex];
	}
  }
  else if(strategy == LFS_BRUTE_FORCE)
  {
	/*brute force filter chooser.
	deflate the scanline after every filter attempt to see which one deflates best.
	This is very slow and gives only slightly smaller, sometimes even larger, result*/
	size_t size[5];
	ucvector attempt[5]; /*five filtering attempts, one for each filter type*/
	size_t smallest = 0;
	unsigned type = 0, bestType = 0;
	unsigned char* dummy;
	LodePNGCompressSettings zlibsettings = settings->zlibsettings;
	/*use fixed tree on the attempts so that the tree is not adapted to the filtertype on purpose,
	to simulate the true case where the tree is the same for the whole image. Sometimes it gives
	better result with dynamic tree anyway. Using the fixed tree sometimes gives worse, but in rare
	cases better compression. It does make this a bit less slow, so it's worth doing this.*/
	zlibsettings.btype = 1;
	/*a custom encoder likely doesn't read the btype setting and is optimized for complete PNG
	images only, so disable it*/
	zlibsettings.custom_zlib = 0;
	zlibsettings.custom_deflate = 0;
	for(type = 0; type != 5; ++type)
	{
	  ucvector_init(&attempt[type]);
	  ucvector_resize(&attempt[type], linebytes); /*todo: give error if resize failed*/
	}
	for(y = 0; y != h; ++y) /*try the 5 filter types*/
	{
	  for(type = 0; type != 5; ++type)
	  {
		unsigned testsize = attempt[type].size;
		/*if(testsize > 8) testsize /= 8;*/ /*it already works good enough by testing a part of the row*/

		filterScanline(attempt[type].data, &in[y * linebytes], prevline, linebytes, bytewidth, type);
		size[type] = 0;
		dummy = 0;
		zlib_compress(&dummy, &size[type], attempt[type].data, testsize, &zlibsettings);
		lodepng_free(dummy);
		/*check if this is smallest size (or if type == 0 it's the first case so always store the values)*/
		if(type == 0 || size[type] < smallest)
		{
		  bestType = type;
		  smallest = size[type];
		}
	  }
	  prevline = &in[y * linebytes];
	  out[y * (linebytes + 1)] = bestType; /*the first byte of a scanline will be the filter type*/
	  for(x = 0; x != linebytes; ++x) out[y * (linebytes + 1) + 1 + x] = attempt[bestType].data[x];
	}
	for(type = 0; type != 5; ++type) ucvector_cleanup(&attempt[type]);
  }
  else return 88; /* unknown filter strategy */

  return error;
}

static void addPaddingBits(unsigned char* out, const unsigned char* in,
						   size_t olinebits, size_t ilinebits, unsigned h)
{
  /*The opposite of the removePaddingBits function
  olinebits must be >= ilinebits*/
  unsigned y;
  size_t diff = olinebits - ilinebits;
  size_t obp = 0, ibp = 0; /*bit pointers*/
  for(y = 0; y != h; ++y)
  {
	size_t x;
	for(x = 0; x < ilinebits; ++x)
	{
	  unsigned char bit = readBitFromReversedStream(&ibp, in);
	  setBitOfReversedStream(&obp, out, bit);
	}
	/*obp += diff; --> no, fill in some value in the padding bits too, to avoid
	"Use of uninitialised value of size ###" warning from valgrind*/
	for(x = 0; x != diff; ++x) setBitOfReversedStream(&obp, out, 0);
  }
}

/*
in: non-interlaced image with size w*h
out: the same pixels, but re-ordered according to PNG's Adam7 interlacing, with
 no padding bits between scanlines, but between reduced images so that each
 reduced image starts at a byte.
bpp: bits per pixel
there are no padding bits, not between scanlines, not between reduced images
in has the following size in bits: w * h * bpp.
out is possibly bigger due to padding bits between reduced images
NOTE: comments about padding bits are only relevant if bpp < 8
*/
static void Adam7_interlace(unsigned char* out, const unsigned char* in, unsigned w, unsigned h, unsigned bpp)
{
  unsigned passw[7], passh[7];
  size_t filter_passstart[8], padded_passstart[8], passstart[8];
  unsigned i;

  Adam7_getpassvalues(passw, passh, filter_passstart, padded_passstart, passstart, w, h, bpp);

  if(bpp >= 8)
  {
	for(i = 0; i != 7; ++i)
	{
	  unsigned x, y, b;
	  size_t bytewidth = bpp / 8;
	  for(y = 0; y < passh[i]; ++y)
	  for(x = 0; x < passw[i]; ++x)
	  {
		size_t pixelinstart = ((ADAM7_IY[i] + y * ADAM7_DY[i]) * w + ADAM7_IX[i] + x * ADAM7_DX[i]) * bytewidth;
		size_t pixeloutstart = passstart[i] + (y * passw[i] + x) * bytewidth;
		for(b = 0; b < bytewidth; ++b)
		{
		  out[pixeloutstart + b] = in[pixelinstart + b];
		}
	  }
	}
  }
  else /*bpp < 8: Adam7 with pixels < 8 bit is a bit trickier: with bit pointers*/
  {
	for(i = 0; i != 7; ++i)
	{
	  unsigned x, y, b;
	  unsigned ilinebits = bpp * passw[i];
	  unsigned olinebits = bpp * w;
	  size_t obp, ibp; /*bit pointers (for out and in buffer)*/
	  for(y = 0; y < passh[i]; ++y)
	  for(x = 0; x < passw[i]; ++x)
	  {
		ibp = (ADAM7_IY[i] + y * ADAM7_DY[i]) * olinebits + (ADAM7_IX[i] + x * ADAM7_DX[i]) * bpp;
		obp = (8 * passstart[i]) + (y * ilinebits + x * bpp);
		for(b = 0; b < bpp; ++b)
		{
		  unsigned char bit = readBitFromReversedStream(&ibp, in);
		  setBitOfReversedStream(&obp, out, bit);
		}
	  }
	}
  }
}

/*out must be buffer big enough to contain uncompressed IDAT chunk data, and in must contain the full image.
return value is error**/
static unsigned preProcessScanlines(unsigned char** out, size_t* outsize, const unsigned char* in,
									unsigned w, unsigned h,
									const LodePNGInfo* info_png, const LodePNGEncoderSettings* settings)
{
  /*
  This function converts the pure 2D image with the PNG's colortype, into filtered-padded-interlaced data. Steps:
  *) if no Adam7: 1) add padding bits (= posible extra bits per scanline if bpp < 8) 2) filter
  *) if adam7: 1) Adam7_interlace 2) 7x add padding bits 3) 7x filter
  */
  unsigned bpp = lodepng_get_bpp(&info_png->color);
  unsigned error = 0;

  if(info_png->interlace_method == 0)
  {
	*outsize = h + (h * ((w * bpp + 7) / 8)); /*image size plus an extra byte per scanline + possible padding bits*/
	*out = (unsigned char*)lodepng_malloc(*outsize);
	if(!(*out) && (*outsize)) error = 83; /*alloc fail*/

	if(!error)
	{
	  /*non multiple of 8 bits per scanline, padding bits needed per scanline*/
	  if(bpp < 8 && w * bpp != ((w * bpp + 7) / 8) * 8)
	  {
		unsigned char* padded = (unsigned char*)lodepng_malloc(h * ((w * bpp + 7) / 8));
		if(!padded) error = 83; /*alloc fail*/
		if(!error)
		{
		  addPaddingBits(padded, in, ((w * bpp + 7) / 8) * 8, w * bpp, h);
		  error = filter(*out, padded, w, h, &info_png->color, settings);
		}
		lodepng_free(padded);
	  }
	  else
	  {
		/*we can immediatly filter into the out buffer, no other steps needed*/
		error = filter(*out, in, w, h, &info_png->color, settings);
	  }
	}
  }
  else /*interlace_method is 1 (Adam7)*/
  {
	unsigned passw[7], passh[7];
	size_t filter_passstart[8], padded_passstart[8], passstart[8];
	unsigned char* adam7;

	Adam7_getpassvalues(passw, passh, filter_passstart, padded_passstart, passstart, w, h, bpp);

	*outsize = filter_passstart[7]; /*image size plus an extra byte per scanline + possible padding bits*/
	*out = (unsigned char*)lodepng_malloc(*outsize);
	if(!(*out)) error = 83; /*alloc fail*/

	adam7 = (unsigned char*)lodepng_malloc(passstart[7]);
	if(!adam7 && passstart[7]) error = 83; /*alloc fail*/

	if(!error)
	{
	  unsigned i;

	  Adam7_interlace(adam7, in, w, h, bpp);
	  for(i = 0; i != 7; ++i)
	  {
		if(bpp < 8)
		{
		  unsigned char* padded = (unsigned char*)lodepng_malloc(padded_passstart[i + 1] - padded_passstart[i]);
		  if(!padded) ERROR_BREAK(83); /*alloc fail*/
		  addPaddingBits(padded, &adam7[passstart[i]],
						 ((passw[i] * bpp + 7) / 8) * 8, passw[i] * bpp, passh[i]);
		  error = filter(&(*out)[filter_passstart[i]], padded,
						 passw[i], passh[i], &info_png->color, settings);
		  lodepng_free(padded);
		}
		else
		{
		  error = filter(&(*out)[filter_passstart[i]], &adam7[padded_passstart[i]],
						 passw[i], passh[i], &info_png->color, settings);
		}

		if(error) break;
	  }
	}

	lodepng_free(adam7);
  }

  return error;
}

/*
palette must have 4 * palettesize bytes allocated, and given in format RGBARGBARGBARGBA...
returns 0 if the palette is opaque,
returns 1 if the palette has a single color with alpha 0 ==> color key
returns 2 if the palette is semi-translucent.
*/
static unsigned getPaletteTranslucency(const unsigned char* palette, size_t palettesize)
{
  size_t i;
  unsigned key = 0;
  unsigned r = 0, g = 0, b = 0; /*the value of the color with alpha 0, so long as color keying is possible*/
  for(i = 0; i != palettesize; ++i)
  {
	if(!key && palette[4 * i + 3] == 0)
	{
	  r = palette[4 * i + 0]; g = palette[4 * i + 1]; b = palette[4 * i + 2];
	  key = 1;
	  i = (size_t)(-1); /*restart from beginning, to detect earlier opaque colors with key's value*/
	}
	else if(palette[4 * i + 3] != 255) return 2;
	/*when key, no opaque RGB may have key's RGB*/
	else if(key && r == palette[i * 4 + 0] && g == palette[i * 4 + 1] && b == palette[i * 4 + 2]) return 2;
  }
  return key;
}

#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
static unsigned addUnknownChunks(ucvector* out, unsigned char* data, size_t datasize)
{
  unsigned char* inchunk = data;
  while((size_t)(inchunk - data) < datasize)
  {
	CERROR_TRY_RETURN(lodepng_chunk_append(&out->data, &out->size, inchunk));
	out->allocsize = out->size; /*fix the allocsize again*/
	inchunk = lodepng_chunk_next(inchunk);
  }
  return 0;
}
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/

unsigned lodepng_encode(unsigned char** out, size_t* outsize,
						const unsigned char* image, unsigned w, unsigned h,
						LodePNGState* state)
{
  LodePNGInfo info;
  ucvector outv;
  unsigned char* data = 0; /*uncompressed version of the IDAT chunk data*/
  size_t datasize = 0;

  /*provide some proper output values if error will happen*/
  *out = 0;
  *outsize = 0;
  state->error = 0;

  lodepng_info_init(&info);
  lodepng_info_copy(&info, &state->info_png);

  if((info.color.colortype == LCT_PALETTE || state->encoder.force_palette)
	  && (info.color.palettesize == 0 || info.color.palettesize > 256))
  {
	state->error = 68; /*invalid palette size, it is only allowed to be 1-256*/
	return state->error;
  }

  if(state->encoder.auto_convert)
  {
	state->error = lodepng_auto_choose_color(&info.color, image, w, h, &state->info_raw);
  }
  if(state->error) return state->error;

  if(state->encoder.zlibsettings.btype > 2)
  {
	CERROR_RETURN_ERROR(state->error, 61); /*error: unexisting btype*/
  }
  if(state->info_png.interlace_method > 1)
  {
	CERROR_RETURN_ERROR(state->error, 71); /*error: unexisting interlace mode*/
  }

  state->error = checkColorValidity(info.color.colortype, info.color.bitdepth);
  if(state->error) return state->error; /*error: unexisting color type given*/
  state->error = checkColorValidity(state->info_raw.colortype, state->info_raw.bitdepth);
  if(state->error) return state->error; /*error: unexisting color type given*/

  if(!lodepng_color_mode_equal(&state->info_raw, &info.color))
  {
	unsigned char* converted;
	size_t size = (w * h * lodepng_get_bpp(&info.color) + 7) / 8;

	converted = (unsigned char*)lodepng_malloc(size);
	if(!converted && size) state->error = 83; /*alloc fail*/
	if(!state->error)
	{
	  state->error = lodepng_convert(converted, image, &info.color, &state->info_raw, w, h);
	}
	if(!state->error) preProcessScanlines(&data, &datasize, converted, w, h, &info, &state->encoder);
	lodepng_free(converted);
  }
  else preProcessScanlines(&data, &datasize, image, w, h, &info, &state->encoder);

  ucvector_init(&outv);
  while(!state->error) /*while only executed once, to break on error*/
  {
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
	size_t i;
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
	/*write signature and chunks*/
	writeSignature(&outv);
	/*IHDR*/
	addChunk_IHDR(&outv, w, h, info.color.colortype, info.color.bitdepth, info.interlace_method);
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
	/*unknown chunks between IHDR and PLTE*/
	if(info.unknown_chunks_data[0])
	{
	  state->error = addUnknownChunks(&outv, info.unknown_chunks_data[0], info.unknown_chunks_size[0]);
	  if(state->error) break;
	}
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
	/*PLTE*/
	if(info.color.colortype == LCT_PALETTE)
	{
	  addChunk_PLTE(&outv, &info.color);
	}
	if(state->encoder.force_palette && (info.color.colortype == LCT_RGB || info.color.colortype == LCT_RGBA))
	{
	  addChunk_PLTE(&outv, &info.color);
	}
	/*tRNS*/
	if(info.color.colortype == LCT_PALETTE && getPaletteTranslucency(info.color.palette, info.color.palettesize) != 0)
	{
	  addChunk_tRNS(&outv, &info.color);
	}
	if((info.color.colortype == LCT_GREY || info.color.colortype == LCT_RGB) && info.color.key_defined)
	{
	  addChunk_tRNS(&outv, &info.color);
	}
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
	/*bKGD (must come between PLTE and the IDAt chunks*/
	if(info.background_defined) addChunk_bKGD(&outv, &info);
	/*pHYs (must come before the IDAT chunks)*/
	if(info.phys_defined) addChunk_pHYs(&outv, &info);

	/*unknown chunks between PLTE and IDAT*/
	if(info.unknown_chunks_data[1])
	{
	  state->error = addUnknownChunks(&outv, info.unknown_chunks_data[1], info.unknown_chunks_size[1]);
	  if(state->error) break;
	}
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
	/*IDAT (multiple IDAT chunks must be consecutive)*/
	state->error = addChunk_IDAT(&outv, data, datasize, &state->encoder.zlibsettings);
	if(state->error) break;
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
	/*tIME*/
	if(info.time_defined) addChunk_tIME(&outv, &info.time);
	/*tEXt and/or zTXt*/
	for(i = 0; i != info.text_num; ++i)
	{
	  if(strlen(info.text_keys[i]) > 79)
	  {
		state->error = 66; /*text chunk too large*/
		break;
	  }
	  if(strlen(info.text_keys[i]) < 1)
	  {
		state->error = 67; /*text chunk too small*/
		break;
	  }
	  if(state->encoder.text_compression)
	  {
		addChunk_zTXt(&outv, info.text_keys[i], info.text_strings[i], &state->encoder.zlibsettings);
	  }
	  else
	  {
		addChunk_tEXt(&outv, info.text_keys[i], info.text_strings[i]);
	  }
	}
	/*LodePNG version id in text chunk*/
	if(state->encoder.add_id)
	{
	  unsigned alread_added_id_text = 0;
	  for(i = 0; i != info.text_num; ++i)
	  {
		if(!strcmp(info.text_keys[i], "LodePNG"))
		{
		  alread_added_id_text = 1;
		  break;
		}
	  }
	  if(alread_added_id_text == 0)
	  {
		addChunk_tEXt(&outv, "LodePNG", LODEPNG_VERSION_STRING); /*it's shorter as tEXt than as zTXt chunk*/
	  }
	}
	/*iTXt*/
	for(i = 0; i != info.itext_num; ++i)
	{
	  if(strlen(info.itext_keys[i]) > 79)
	  {
		state->error = 66; /*text chunk too large*/
		break;
	  }
	  if(strlen(info.itext_keys[i]) < 1)
	  {
		state->error = 67; /*text chunk too small*/
		break;
	  }
	  addChunk_iTXt(&outv, state->encoder.text_compression,
					info.itext_keys[i], info.itext_langtags[i], info.itext_transkeys[i], info.itext_strings[i],
					&state->encoder.zlibsettings);
	}

	/*unknown chunks between IDAT and IEND*/
	if(info.unknown_chunks_data[2])
	{
	  state->error = addUnknownChunks(&outv, info.unknown_chunks_data[2], info.unknown_chunks_size[2]);
	  if(state->error) break;
	}
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
	addChunk_IEND(&outv);

	break; /*this isn't really a while loop; no error happened so break out now!*/
  }

  lodepng_info_cleanup(&info);
  lodepng_free(data);
  /*instead of cleaning the vector up, give it to the output*/
  *out = outv.data;
  *outsize = outv.size;

  return state->error;
}

unsigned lodepng_encode_memory(unsigned char** out, size_t* outsize, const unsigned char* image,
							   unsigned w, unsigned h, LodePNGColorType colortype, unsigned bitdepth)
{
  unsigned error;
  LodePNGState state;
  lodepng_state_init(&state);
  state.info_raw.colortype = colortype;
  state.info_raw.bitdepth = bitdepth;
  state.info_png.color.colortype = colortype;
  state.info_png.color.bitdepth = bitdepth;
  lodepng_encode(out, outsize, image, w, h, &state);
  error = state.error;
  lodepng_state_cleanup(&state);
  return error;
}

unsigned lodepng_encode_memory_std(unsigned char** out, size_t* outsize, const unsigned char* image,
							   unsigned w, unsigned h, unsigned colortype, unsigned bitdepth)
{
  return lodepng_encode_memory(out,outsize,image,w,h,(LodePNGColorType)colortype,bitdepth);
}

unsigned lodepng_encode32(unsigned char** out, size_t* outsize, const unsigned char* image, unsigned w, unsigned h)
{
  return lodepng_encode_memory(out, outsize, image, w, h, LCT_RGBA, 8);
}

unsigned lodepng_encode24(unsigned char** out, size_t* outsize, const unsigned char* image, unsigned w, unsigned h)
{
  return lodepng_encode_memory(out, outsize, image, w, h, LCT_RGB, 8);
}

#ifdef LODEPNG_COMPILE_DISK
unsigned lodepng_encode_file(const char* filename, const unsigned char* image, unsigned w, unsigned h,
							 LodePNGColorType colortype, unsigned bitdepth)
{
  unsigned char* buffer;
  size_t buffersize;
  unsigned error = lodepng_encode_memory(&buffer, &buffersize, image, w, h, colortype, bitdepth);
  if(!error) error = lodepng_save_file(buffer, buffersize, filename);
  lodepng_free(buffer);
  return error;
}

unsigned lodepng_encode32_file(const char* filename, const unsigned char* image, unsigned w, unsigned h)
{
  return lodepng_encode_file(filename, image, w, h, LCT_RGBA, 8);
}

unsigned lodepng_encode24_file(const char* filename, const unsigned char* image, unsigned w, unsigned h)
{
  return lodepng_encode_file(filename, image, w, h, LCT_RGB, 8);
}
#endif /*LODEPNG_COMPILE_DISK*/

void lodepng_encoder_settings_init(LodePNGEncoderSettings* settings)
{
  lodepng_compress_settings_init(&settings->zlibsettings);
  settings->filter_palette_zero = 1;
  settings->filter_strategy = LFS_MINSUM;
  settings->auto_convert = 1;
  settings->force_palette = 0;
  settings->predefined_filters = 0;
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
  settings->add_id = 0;
  settings->text_compression = 1;
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
}

#endif /*LODEPNG_COMPILE_ENCODER*/
#endif /*LODEPNG_COMPILE_PNG*/

#ifdef LODEPNG_COMPILE_ERROR_TEXT
/*
This returns the description of a numerical error code in English. This is also
the documentation of all the error codes.
*/
const char* lodepng_error_text(unsigned code)
{
  switch(code)
  {
	case 0: return "no error, everything went ok";
	case 1: return "nothing done yet"; /*the Encoder/Decoder has done nothing yet, error checking makes no sense yet*/
	case 10: return "end of input memory reached without huffman end code"; /*while huffman decoding*/
	case 11: return "error in code tree made it jump outside of huffman tree"; /*while huffman decoding*/
	case 13: return "problem while processing dynamic deflate block";
	case 14: return "problem while processing dynamic deflate block";
	case 15: return "problem while processing dynamic deflate block";
	case 16: return "unexisting code while processing dynamic deflate block";
	case 17: return "end of out buffer memory reached while inflating";
	case 18: return "invalid distance code while inflating";
	case 19: return "end of out buffer memory reached while inflating";
	case 20: return "invalid deflate block BTYPE encountered while decoding";
	case 21: return "NLEN is not ones complement of LEN in a deflate block";
	 /*end of out buffer memory reached while inflating:
	 This can happen if the inflated deflate data is longer than the amount of bytes required to fill up
	 all the pixels of the image, given the color depth and image dimensions. Something that doesn't
	 happen in a normal, well encoded, PNG image.*/
	case 22: return "end of out buffer memory reached while inflating";
	case 23: return "end of in buffer memory reached while inflating";
	case 24: return "invalid FCHECK in zlib header";
	case 25: return "invalid compression method in zlib header";
	case 26: return "FDICT encountered in zlib header while it's not used for PNG";
	case 27: return "PNG file is smaller than a PNG header";
	/*Checks the magic file header, the first 8 bytes of the PNG file*/
	case 28: return "incorrect PNG signature, it's no PNG or corrupted";
	case 29: return "first chunk is not the header chunk";
	case 30: return "chunk length too large, chunk broken off at end of file";
	case 31: return "illegal PNG color type or bpp";
	case 32: return "illegal PNG compression method";
	case 33: return "illegal PNG filter method";
	case 34: return "illegal PNG interlace method";
	case 35: return "chunk length of a chunk is too large or the chunk too small";
	case 36: return "illegal PNG filter type encountered";
	case 37: return "illegal bit depth for this color type given";
	case 38: return "the palette is too big"; /*more than 256 colors*/
	case 39: return "more palette alpha values given in tRNS chunk than there are colors in the palette";
	case 40: return "tRNS chunk has wrong size for greyscale image";
	case 41: return "tRNS chunk has wrong size for RGB image";
	case 42: return "tRNS chunk appeared while it was not allowed for this color type";
	case 43: return "bKGD chunk has wrong size for palette image";
	case 44: return "bKGD chunk has wrong size for greyscale image";
	case 45: return "bKGD chunk has wrong size for RGB image";
	/*the input data is empty, maybe a PNG file doesn't exist or is in the wrong path*/
	case 48: return "empty input or file doesn't exist";
	case 49: return "jumped past memory while generating dynamic huffman tree";
	case 50: return "jumped past memory while generating dynamic huffman tree";
	case 51: return "jumped past memory while inflating huffman block";
	case 52: return "jumped past memory while inflating";
	case 53: return "size of zlib data too small";
	case 54: return "repeat symbol in tree while there was no value symbol yet";
	/*jumped past tree while generating huffman tree, this could be when the
	tree will have more leaves than symbols after generating it out of the
	given lenghts. They call this an oversubscribed dynamic bit lengths tree in zlib.*/
	case 55: return "jumped past tree while generating huffman tree";
	case 56: return "given output image colortype or bitdepth not supported for color conversion";
	case 57: return "invalid CRC encountered (checking CRC can be disabled)";
	case 58: return "invalid ADLER32 encountered (checking ADLER32 can be disabled)";
	case 59: return "requested color conversion not supported";
	case 60: return "invalid window size given in the settings of the encoder (must be 0-32768)";
	case 61: return "invalid BTYPE given in the settings of the encoder (only 0, 1 and 2 are allowed)";
	/*LodePNG leaves the choice of RGB to greyscale conversion formula to the user.*/
	case 62: return "conversion from color to greyscale not supported";
	case 63: return "length of a chunk too long, max allowed for PNG is 2147483647 bytes per chunk"; /*(2^31-1)*/
	/*this would result in the inability of a deflated block to ever contain an end code. It must be at least 1.*/
	case 64: return "the length of the END symbol 256 in the Huffman tree is 0";
	case 66: return "the length of a text chunk keyword given to the encoder is longer than the maximum of 79 bytes";
	case 67: return "the length of a text chunk keyword given to the encoder is smaller than the minimum of 1 byte";
	case 68: return "tried to encode a PLTE chunk with a palette that has less than 1 or more than 256 colors";
	case 69: return "unknown chunk type with 'critical' flag encountered by the decoder";
	case 71: return "unexisting interlace mode given to encoder (must be 0 or 1)";
	case 72: return "while decoding, unexisting compression method encountering in zTXt or iTXt chunk (it must be 0)";
	case 73: return "invalid tIME chunk size";
	case 74: return "invalid pHYs chunk size";
	/*length could be wrong, or data chopped off*/
	case 75: return "no null termination char found while decoding text chunk";
	case 76: return "iTXt chunk too short to contain required bytes";
	case 77: return "integer overflow in buffer size";
	case 78: return "failed to open file for reading"; /*file doesn't exist or couldn't be opened for reading*/
	case 79: return "failed to open file for writing";
	case 80: return "tried creating a tree of 0 symbols";
	case 81: return "lazy matching at pos 0 is impossible";
	case 82: return "color conversion to palette requested while a color isn't in palette";
	case 83: return "memory allocation failed";
	case 84: return "given image too small to contain all pixels to be encoded";
	case 86: return "impossible offset in lz77 encoding (internal bug)";
	case 87: return "must provide custom zlib function pointer if LODEPNG_COMPILE_ZLIB is not defined";
	case 88: return "invalid filter strategy given for LodePNGEncoderSettings.filter_strategy";
	case 89: return "text chunk keyword too short or long: must have size 1-79";
	/*the windowsize in the LodePNGCompressSettings. Requiring POT(==> & instead of %) makes encoding 12% faster.*/
	case 90: return "windowsize must be a power of two";
	case 91: return "invalid decompressed idat size";
	case 92: return "too many pixels, not supported";
	case 93: return "zero width or height is invalid";
  }
  return "unknown error code";
}
#endif /*LODEPNG_COMPILE_ERROR_TEXT*/

/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */
/* // C++ Wrapper                                                          // */
/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */

#ifdef LODEPNG_COMPILE_CPP
namespace lodepng
{

#ifdef LODEPNG_COMPILE_DISK
void load_file(std::vector<unsigned char>& buffer, const std::string& filename)
{
  std::ifstream file(filename.c_str(), std::ios::in|std::ios::binary|std::ios::ate);

  /*get filesize*/
  std::streamsize size = 0;
  if(file.seekg(0, std::ios::end).good()) size = file.tellg();
  if(file.seekg(0, std::ios::beg).good()) size -= file.tellg();

  /*read contents of the file into the vector*/
  buffer.resize(size_t(size));
  if(size > 0) file.read((char*)(&buffer[0]), size);
}

/*write given buffer to the file, overwriting the file, it doesn't append to it.*/
void save_file(const std::vector<unsigned char>& buffer, const std::string& filename)
{
  std::ofstream file(filename.c_str(), std::ios::out|std::ios::binary);
  file.write(buffer.empty() ? 0 : (char*)&buffer[0], std::streamsize(buffer.size()));
}
#endif //LODEPNG_COMPILE_DISK

#ifdef LODEPNG_COMPILE_ZLIB
#ifdef LODEPNG_COMPILE_DECODER
unsigned decompress(std::vector<unsigned char>& out, const unsigned char* in, size_t insize,
					const LodePNGDecompressSettings& settings)
{
  unsigned char* buffer = 0;
  size_t buffersize = 0;
  unsigned error = zlib_decompress(&buffer, &buffersize, in, insize, &settings);
  if(buffer)
  {
	out.insert(out.end(), &buffer[0], &buffer[buffersize]);
	lodepng_free(buffer);
  }
  return error;
}

unsigned decompress(std::vector<unsigned char>& out, const std::vector<unsigned char>& in,
					const LodePNGDecompressSettings& settings)
{
  return decompress(out, in.empty() ? 0 : &in[0], in.size(), settings);
}
#endif //LODEPNG_COMPILE_DECODER

#ifdef LODEPNG_COMPILE_ENCODER
unsigned compress(std::vector<unsigned char>& out, const unsigned char* in, size_t insize,
				  const LodePNGCompressSettings& settings)
{
  unsigned char* buffer = 0;
  size_t buffersize = 0;
  unsigned error = zlib_compress(&buffer, &buffersize, in, insize, &settings);
  if(buffer)
  {
	out.insert(out.end(), &buffer[0], &buffer[buffersize]);
	lodepng_free(buffer);
  }
  return error;
}

unsigned compress(std::vector<unsigned char>& out, const std::vector<unsigned char>& in,
				  const LodePNGCompressSettings& settings)
{
  return compress(out, in.empty() ? 0 : &in[0], in.size(), settings);
}
#endif //LODEPNG_COMPILE_ENCODER
#endif //LODEPNG_COMPILE_ZLIB

#ifdef LODEPNG_COMPILE_PNG

State::State()
{
  lodepng_state_init(this);
}

State::State(const State& other)
{
  lodepng_state_init(this);
  lodepng_state_copy(this, &other);
}

State::~State()
{
  lodepng_state_cleanup(this);
}

State& State::operator=(const State& other)
{
  lodepng_state_copy(this, &other);
  return *this;
}

#ifdef LODEPNG_COMPILE_DECODER

unsigned decode(std::vector<unsigned char>& out, unsigned& w, unsigned& h, const unsigned char* in,
				size_t insize, LodePNGColorType colortype, unsigned bitdepth)
{
  unsigned char* buffer;
  unsigned error = lodepng_decode_memory(&buffer, &w, &h, in, insize, colortype, bitdepth);
  if(buffer && !error)
  {
	State state;
	state.info_raw.colortype = colortype;
	state.info_raw.bitdepth = bitdepth;
	size_t buffersize = lodepng_get_raw_size(w, h, &state.info_raw);
	out.insert(out.end(), &buffer[0], &buffer[buffersize]);
	lodepng_free(buffer);
  }
  return error;
}

unsigned decode(std::vector<unsigned char>& out, unsigned& w, unsigned& h,
				const std::vector<unsigned char>& in, LodePNGColorType colortype, unsigned bitdepth)
{
  return decode(out, w, h, in.empty() ? 0 : &in[0], (unsigned)in.size(), colortype, bitdepth);
}

unsigned decode(std::vector<unsigned char>& out, unsigned& w, unsigned& h,
				State& state,
				const unsigned char* in, size_t insize)
{
  unsigned char* buffer = NULL;
  unsigned error = lodepng_decode(&buffer, &w, &h, &state, in, insize);
  if(buffer && !error)
  {
	size_t buffersize = lodepng_get_raw_size(w, h, &state.info_raw);
	out.insert(out.end(), &buffer[0], &buffer[buffersize]);
  }
  lodepng_free(buffer);
  return error;
}

unsigned decode(std::vector<unsigned char>& out, unsigned& w, unsigned& h,
				State& state,
				const std::vector<unsigned char>& in)
{
  return decode(out, w, h, state, in.empty() ? 0 : &in[0], in.size());
}

#ifdef LODEPNG_COMPILE_DISK
unsigned decode(std::vector<unsigned char>& out, unsigned& w, unsigned& h, const std::string& filename,
				LodePNGColorType colortype, unsigned bitdepth)
{
  std::vector<unsigned char> buffer;
  load_file(buffer, filename);
  return decode(out, w, h, buffer, colortype, bitdepth);
}
#endif //LODEPNG_COMPILE_DECODER
#endif //LODEPNG_COMPILE_DISK

#ifdef LODEPNG_COMPILE_ENCODER
unsigned encode(std::vector<unsigned char>& out, const unsigned char* in, unsigned w, unsigned h,
				LodePNGColorType colortype, unsigned bitdepth)
{
  unsigned char* buffer;
  size_t buffersize;
  unsigned error = lodepng_encode_memory(&buffer, &buffersize, in, w, h, colortype, bitdepth);
  if(buffer)
  {
	out.insert(out.end(), &buffer[0], &buffer[buffersize]);
	lodepng_free(buffer);
  }
  return error;
}

unsigned encode(std::vector<unsigned char>& out,
				const std::vector<unsigned char>& in, unsigned w, unsigned h,
				LodePNGColorType colortype, unsigned bitdepth)
{
  if(lodepng_get_raw_size_lct(w, h, colortype, bitdepth) > in.size()) return 84;
  return encode(out, in.empty() ? 0 : &in[0], w, h, colortype, bitdepth);
}

unsigned encode(std::vector<unsigned char>& out,
				const unsigned char* in, unsigned w, unsigned h,
				State& state)
{
  unsigned char* buffer;
  size_t buffersize;
  unsigned error = lodepng_encode(&buffer, &buffersize, in, w, h, &state);
  if(buffer)
  {
	out.insert(out.end(), &buffer[0], &buffer[buffersize]);
	lodepng_free(buffer);
  }
  return error;
}

unsigned encode(std::vector<unsigned char>& out,
				const std::vector<unsigned char>& in, unsigned w, unsigned h,
				State& state)
{
  if(lodepng_get_raw_size(w, h, &state.info_raw) > in.size()) return 84;
  return encode(out, in.empty() ? 0 : &in[0], w, h, state);
}

#ifdef LODEPNG_COMPILE_DISK
unsigned encode(const std::string& filename,
				const unsigned char* in, unsigned w, unsigned h,
				LodePNGColorType colortype, unsigned bitdepth)
{
  std::vector<unsigned char> buffer;
  unsigned error = encode(buffer, in, w, h, colortype, bitdepth);
  if(!error) save_file(buffer, filename);
  return error;
}

unsigned encode(const std::string& filename,
				const std::vector<unsigned char>& in, unsigned w, unsigned h,
				LodePNGColorType colortype, unsigned bitdepth)
{
  if(lodepng_get_raw_size_lct(w, h, colortype, bitdepth) > in.size()) return 84;
  return encode(filename, in.empty() ? 0 : &in[0], w, h, colortype, bitdepth);
}
#endif //LODEPNG_COMPILE_DISK
#endif //LODEPNG_COMPILE_ENCODER
#endif //LODEPNG_COMPILE_PNG
} //namespace lodepng
#endif /*LODEPNG_COMPILE_CPP*/

#if defined(SPOT_OPENGL_HEADER)
#   include SPOT_OPENGL_HEADER

//#line 1 "SOIL2.h"
#ifndef HEADER_SIMPLE_OPENGL_IMAGE_LIBRARY
#define HEADER_SIMPLE_OPENGL_IMAGE_LIBRARY

#ifdef __cplusplus
extern "C" {
#endif

/**
	The format of images that may be loaded (force_channels).
	SOIL_LOAD_AUTO leaves the image in whatever format it was found.
	SOIL_LOAD_L forces the image to load as Luminous (greyscale)
	SOIL_LOAD_LA forces the image to load as Luminous with Alpha
	SOIL_LOAD_RGB forces the image to load as Red Green Blue
	SOIL_LOAD_RGBA forces the image to load as Red Green Blue Alpha
**/
enum
{
	SOIL_LOAD_AUTO = 0,
	SOIL_LOAD_L = 1,
	SOIL_LOAD_LA = 2,
	SOIL_LOAD_RGB = 3,
	SOIL_LOAD_RGBA = 4
};

/**
	Passed in as reuse_texture_ID, will cause SOIL to
	register a new texture ID using glGenTextures().
	If the value passed into reuse_texture_ID > 0 then
	SOIL will just re-use that texture ID (great for
	reloading image assets in-game!)
**/
enum
{
	SOIL_CREATE_NEW_ID = 0
};

/**
	flags you can pass into SOIL_load_OGL_texture()
	and SOIL_create_OGL_texture().
	(note that if SOIL_FLAG_DDS_LOAD_DIRECT is used
	the rest of the flags with the exception of
	SOIL_FLAG_TEXTURE_REPEATS will be ignored while
	loading already-compressed DDS files.)

	SOIL_FLAG_POWER_OF_TWO: force the image to be POT
	SOIL_FLAG_MIPMAPS: generate mipmaps for the texture
	SOIL_FLAG_TEXTURE_REPEATS: otherwise will clamp
	SOIL_FLAG_MULTIPLY_ALPHA: for using (GL_ONE,GL_ONE_MINUS_SRC_ALPHA) blending
	SOIL_FLAG_INVERT_Y: flip the image vertically
	SOIL_FLAG_COMPRESS_TO_DXT: if the card can display them, will convert RGB to DXT1, RGBA to DXT5
	SOIL_FLAG_DDS_LOAD_DIRECT: will load DDS files directly without _ANY_ additional processing ( if supported )
	SOIL_FLAG_NTSC_SAFE_RGB: clamps RGB components to the range [16,235]
	SOIL_FLAG_CoCg_Y: Google YCoCg; RGB=>CoYCg, RGBA=>CoCgAY
	SOIL_FLAG_TEXTURE_RECTANGE: uses ARB_texture_rectangle ; pixel indexed & no repeat or MIPmaps or cubemaps
	SOIL_FLAG_PVR_LOAD_DIRECT: will load PVR files directly without _ANY_ additional processing ( if supported )
**/
enum
{
	SOIL_FLAG_POWER_OF_TWO = 1,
	SOIL_FLAG_MIPMAPS = 2,
	SOIL_FLAG_TEXTURE_REPEATS = 4,
	SOIL_FLAG_MULTIPLY_ALPHA = 8,
	SOIL_FLAG_INVERT_Y = 16,
	SOIL_FLAG_COMPRESS_TO_DXT = 32,
	SOIL_FLAG_DDS_LOAD_DIRECT = 64,
	SOIL_FLAG_NTSC_SAFE_RGB = 128,
	SOIL_FLAG_CoCg_Y = 256,
	SOIL_FLAG_TEXTURE_RECTANGLE = 512,
	SOIL_FLAG_PVR_LOAD_DIRECT = 1024,
	SOIL_FLAG_ETC1_LOAD_DIRECT = 2048,
	SOIL_FLAG_GL_MIPMAPS = 4096
};

/**
	The types of images that may be saved.
	(TGA supports uncompressed RGB / RGBA)
	(BMP supports uncompressed RGB)
	(DDS supports DXT1 and DXT5)
	(PNG supports RGB / RGBA)
**/
enum
{
	SOIL_SAVE_TYPE_TGA = 0,
	SOIL_SAVE_TYPE_BMP = 1,
	SOIL_SAVE_TYPE_PNG = 2,
	SOIL_SAVE_TYPE_DDS = 3
};

/**
	Defines the order of faces in a DDS cubemap.
	I recommend that you use the same order in single
	image cubemap files, so they will be interchangeable
	with DDS cubemaps when using SOIL.
**/
#define SOIL_DDS_CUBEMAP_FACE_ORDER "EWUDNS"

/**
	The types of internal fake HDR representations

	SOIL_HDR_RGBE:		RGB * pow( 2.0, A - 128.0 )
	SOIL_HDR_RGBdivA:	RGB / A
	SOIL_HDR_RGBdivA2:	RGB / (A*A)
**/
enum
{
	SOIL_HDR_RGBE = 0,
	SOIL_HDR_RGBdivA = 1,
	SOIL_HDR_RGBdivA2 = 2
};

/**
	Loads an image from disk into an OpenGL texture.
	\param filename the name of the file to upload as a texture
	\param force_channels 0-image format, 1-luminous, 2-luminous/alpha, 3-RGB, 4-RGBA
	\param reuse_texture_ID 0-generate a new texture ID, otherwise reuse the texture ID (overwriting the old texture)
	\param flags can be any of SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS | SOIL_FLAG_MULTIPLY_ALPHA | SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT | SOIL_FLAG_DDS_LOAD_DIRECT
	\return 0-failed, otherwise returns the OpenGL texture handle
**/
unsigned int
	SOIL_load_OGL_texture
	(
		const char *filename,
		int force_channels,
		unsigned int reuse_texture_ID,
		unsigned int flags
	);

/**
	Loads 6 images from disk into an OpenGL cubemap texture.
	\param x_pos_file the name of the file to upload as the +x cube face
	\param x_neg_file the name of the file to upload as the -x cube face
	\param y_pos_file the name of the file to upload as the +y cube face
	\param y_neg_file the name of the file to upload as the -y cube face
	\param z_pos_file the name of the file to upload as the +z cube face
	\param z_neg_file the name of the file to upload as the -z cube face
	\param force_channels 0-image format, 1-luminous, 2-luminous/alpha, 3-RGB, 4-RGBA
	\param reuse_texture_ID 0-generate a new texture ID, otherwise reuse the texture ID (overwriting the old texture)
	\param flags can be any of SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS | SOIL_FLAG_MULTIPLY_ALPHA | SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT | SOIL_FLAG_DDS_LOAD_DIRECT
	\return 0-failed, otherwise returns the OpenGL texture handle
**/
unsigned int
	SOIL_load_OGL_cubemap
	(
		const char *x_pos_file,
		const char *x_neg_file,
		const char *y_pos_file,
		const char *y_neg_file,
		const char *z_pos_file,
		const char *z_neg_file,
		int force_channels,
		unsigned int reuse_texture_ID,
		unsigned int flags
	);

/**
	Loads 1 image from disk and splits it into an OpenGL cubemap texture.
	\param filename the name of the file to upload as a texture
	\param face_order the order of the faces in the file, any combination of NSWEUD, for North, South, Up, etc.
	\param force_channels 0-image format, 1-luminous, 2-luminous/alpha, 3-RGB, 4-RGBA
	\param reuse_texture_ID 0-generate a new texture ID, otherwise reuse the texture ID (overwriting the old texture)
	\param flags can be any of SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS | SOIL_FLAG_MULTIPLY_ALPHA | SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT | SOIL_FLAG_DDS_LOAD_DIRECT
	\return 0-failed, otherwise returns the OpenGL texture handle
**/
unsigned int
	SOIL_load_OGL_single_cubemap
	(
		const char *filename,
		const char face_order[6],
		int force_channels,
		unsigned int reuse_texture_ID,
		unsigned int flags
	);

/**
	Loads an HDR image from disk into an OpenGL texture.
	\param filename the name of the file to upload as a texture
	\param fake_HDR_format SOIL_HDR_RGBE, SOIL_HDR_RGBdivA, SOIL_HDR_RGBdivA2
	\param reuse_texture_ID 0-generate a new texture ID, otherwise reuse the texture ID (overwriting the old texture)
	\param flags can be any of SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS | SOIL_FLAG_MULTIPLY_ALPHA | SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT
	\return 0-failed, otherwise returns the OpenGL texture handle
**/
unsigned int
	SOIL_load_OGL_HDR_texture
	(
		const char *filename,
		int fake_HDR_format,
		int rescale_to_max,
		unsigned int reuse_texture_ID,
		unsigned int flags
	);

/**
	Loads an image from RAM into an OpenGL texture.
	\param buffer the image data in RAM just as if it were still in a file
	\param buffer_length the size of the buffer in bytes
	\param force_channels 0-image format, 1-luminous, 2-luminous/alpha, 3-RGB, 4-RGBA
	\param reuse_texture_ID 0-generate a new texture ID, otherwise reuse the texture ID (overwriting the old texture)
	\param flags can be any of SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS | SOIL_FLAG_MULTIPLY_ALPHA | SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT | SOIL_FLAG_DDS_LOAD_DIRECT
	\return 0-failed, otherwise returns the OpenGL texture handle
**/
unsigned int
	SOIL_load_OGL_texture_from_memory
	(
		const unsigned char *const buffer,
		int buffer_length,
		int force_channels,
		unsigned int reuse_texture_ID,
		unsigned int flags
	);

/**
	Loads 6 images from memory into an OpenGL cubemap texture.
	\param x_pos_buffer the image data in RAM to upload as the +x cube face
	\param x_pos_buffer_length the size of the above buffer
	\param x_neg_buffer the image data in RAM to upload as the +x cube face
	\param x_neg_buffer_length the size of the above buffer
	\param y_pos_buffer the image data in RAM to upload as the +x cube face
	\param y_pos_buffer_length the size of the above buffer
	\param y_neg_buffer the image data in RAM to upload as the +x cube face
	\param y_neg_buffer_length the size of the above buffer
	\param z_pos_buffer the image data in RAM to upload as the +x cube face
	\param z_pos_buffer_length the size of the above buffer
	\param z_neg_buffer the image data in RAM to upload as the +x cube face
	\param z_neg_buffer_length the size of the above buffer
	\param force_channels 0-image format, 1-luminous, 2-luminous/alpha, 3-RGB, 4-RGBA
	\param reuse_texture_ID 0-generate a new texture ID, otherwise reuse the texture ID (overwriting the old texture)
	\param flags can be any of SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS | SOIL_FLAG_MULTIPLY_ALPHA | SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT | SOIL_FLAG_DDS_LOAD_DIRECT
	\return 0-failed, otherwise returns the OpenGL texture handle
**/
unsigned int
	SOIL_load_OGL_cubemap_from_memory
	(
		const unsigned char *const x_pos_buffer,
		int x_pos_buffer_length,
		const unsigned char *const x_neg_buffer,
		int x_neg_buffer_length,
		const unsigned char *const y_pos_buffer,
		int y_pos_buffer_length,
		const unsigned char *const y_neg_buffer,
		int y_neg_buffer_length,
		const unsigned char *const z_pos_buffer,
		int z_pos_buffer_length,
		const unsigned char *const z_neg_buffer,
		int z_neg_buffer_length,
		int force_channels,
		unsigned int reuse_texture_ID,
		unsigned int flags
	);

/**
	Loads 1 image from RAM and splits it into an OpenGL cubemap texture.
	\param buffer the image data in RAM just as if it were still in a file
	\param buffer_length the size of the buffer in bytes
	\param face_order the order of the faces in the file, any combination of NSWEUD, for North, South, Up, etc.
	\param force_channels 0-image format, 1-luminous, 2-luminous/alpha, 3-RGB, 4-RGBA
	\param reuse_texture_ID 0-generate a new texture ID, otherwise reuse the texture ID (overwriting the old texture)
	\param flags can be any of SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS | SOIL_FLAG_MULTIPLY_ALPHA | SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT | SOIL_FLAG_DDS_LOAD_DIRECT
	\return 0-failed, otherwise returns the OpenGL texture handle
**/
unsigned int
	SOIL_load_OGL_single_cubemap_from_memory
	(
		const unsigned char *const buffer,
		int buffer_length,
		const char face_order[6],
		int force_channels,
		unsigned int reuse_texture_ID,
		unsigned int flags
	);

/**
	Creates a 2D OpenGL texture from raw image data.  Note that the raw data is
	_NOT_ freed after the upload (so the user can load various versions).
	\param data the raw data to be uploaded as an OpenGL texture
	\param width the pointer of the width of the image in pixels ( if the texture size change, width will be overrided with the new width )
	\param height the pointer of the height of the image in pixels ( if the texture size change, height will be overrided with the new height )
	\param channels the number of channels: 1-luminous, 2-luminous/alpha, 3-RGB, 4-RGBA
	\param reuse_texture_ID 0-generate a new texture ID, otherwise reuse the texture ID (overwriting the old texture)
	\param flags can be any of SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS | SOIL_FLAG_MULTIPLY_ALPHA | SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT
	\return 0-failed, otherwise returns the OpenGL texture handle
**/
unsigned int
	SOIL_create_OGL_texture
	(
		const unsigned char *const data,
		int *width, int *height, int channels,
		unsigned int reuse_texture_ID,
		unsigned int flags
	);

/**
	Creates an OpenGL cubemap texture by splitting up 1 image into 6 parts.
	\param data the raw data to be uploaded as an OpenGL texture
	\param width the width of the image in pixels
	\param height the height of the image in pixels
	\param channels the number of channels: 1-luminous, 2-luminous/alpha, 3-RGB, 4-RGBA
	\param face_order the order of the faces in the file, and combination of NSWEUD, for North, South, Up, etc.
	\param reuse_texture_ID 0-generate a new texture ID, otherwise reuse the texture ID (overwriting the old texture)
	\param flags can be any of SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS | SOIL_FLAG_MULTIPLY_ALPHA | SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT | SOIL_FLAG_DDS_LOAD_DIRECT
	\return 0-failed, otherwise returns the OpenGL texture handle
**/
unsigned int
	SOIL_create_OGL_single_cubemap
	(
		const unsigned char *const data,
		int width, int height, int channels,
		const char face_order[6],
		unsigned int reuse_texture_ID,
		unsigned int flags
	);

/**
	Captures the OpenGL window (RGB) and saves it to disk
	\return 0 if it failed, otherwise returns 1
**/
int
	SOIL_save_screenshot
	(
		const char *filename,
		int image_type,
		int x, int y,
		int width, int height
	);

/**
	Loads an image from disk into an array of unsigned chars.
	Note that *channels return the original channel count of the
	image.  If force_channels was other than SOIL_LOAD_AUTO,
	the resulting image has force_channels, but *channels may be
	different (if the original image had a different channel
	count).
	\return 0 if failed, otherwise returns 1
**/
unsigned char*
	SOIL_load_image
	(
		const char *filename,
		int *width, int *height, int *channels,
		int force_channels
	);

/**
	Loads an image from memory into an array of unsigned chars.
	Note that *channels return the original channel count of the
	image.  If force_channels was other than SOIL_LOAD_AUTO,
	the resulting image has force_channels, but *channels may be
	different (if the original image had a different channel
	count).
	\return 0 if failed, otherwise returns 1
**/
unsigned char*
	SOIL_load_image_from_memory
	(
		const unsigned char *const buffer,
		int buffer_length,
		int *width, int *height, int *channels,
		int force_channels
	);

/**
	Saves an image from an array of unsigned chars (RGBA) to disk
	\return 0 if failed, otherwise returns 1
**/
int
	SOIL_save_image
	(
		const char *filename,
		int image_type,
		int width, int height, int channels,
		const unsigned char *const data
	);

/**
	Frees the image data (note, this is just C's "free()"...this function is
	present mostly so C++ programmers don't forget to use "free()" and call
	"delete []" instead [8^)
**/
void
	SOIL_free_image_data
	(
		unsigned char *img_data
	);

/**
	This function resturn a pointer to a string describing the last thing
	that happened inside SOIL.  It can be used to determine why an image
	failed to load.
**/
const char*
	SOIL_last_result
	(
		void
	);

/** @return The address of the GL function proc, or NULL if the function is not found. */
void *
	SOIL_GL_GetProcAddress
	(
		const char *proc
	);

/** @return 1 if an OpenGL extension is supported for the current context, 0 otherwise. */
int
	SOIL_GL_ExtensionSupported
	(
		const char *extension
	);

/** Loads the DDS texture directly to the GPU memory ( if supported ) */
unsigned int SOIL_direct_load_DDS(
		const char *filename,
		unsigned int reuse_texture_ID,
		int flags,
		int loading_as_cubemap );

/** Loads the DDS texture directly to the GPU memory ( if supported ) */
unsigned int SOIL_direct_load_DDS_from_memory(
		const unsigned char *const buffer,
		int buffer_length,
		unsigned int reuse_texture_ID,
		int flags,
		int loading_as_cubemap );

/** Loads the PVR texture directly to the GPU memory ( if supported ) */
unsigned int SOIL_direct_load_PVR(
		const char *filename,
		unsigned int reuse_texture_ID,
		int flags,
		int loading_as_cubemap );

/** Loads the PVR texture directly to the GPU memory ( if supported ) */
unsigned int SOIL_direct_load_PVR_from_memory(
		const unsigned char *const buffer,
		int buffer_length,
		unsigned int reuse_texture_ID,
		int flags,
		int loading_as_cubemap );

/** Loads the PVR texture directly to the GPU memory ( if supported ) */
unsigned int SOIL_direct_load_ETC1(const char *filename,
		unsigned int reuse_texture_ID,
		int flags );

/** Loads the PVR texture directly to the GPU memory ( if supported ) */
unsigned int SOIL_direct_load_ETC1_from_memory(const unsigned char *const buffer,
		int buffer_length,
		unsigned int reuse_texture_ID,
		int flags );

#ifdef __cplusplus
}
#endif

#endif /* HEADER_SIMPLE_OPENGL_IMAGE_LIBRARY	*/



//#line 1 "SOIL2.c"
#define SOIL_CHECK_FOR_GL_ERRORS 0

#if defined( __APPLE_CC__ ) || defined ( __APPLE__ )
	#include <TargetConditionals.h>

	#if defined( __IPHONE__ ) || ( defined( TARGET_OS_IPHONE ) && TARGET_OS_IPHONE ) || ( defined( TARGET_IPHONE_SIMULATOR ) && TARGET_IPHONE_SIMULATOR )
		#define SOIL_PLATFORM_IOS
		#include <dlfcn.h>
	#else
		#define SOIL_PLATFORM_OSX
	#endif
#elif defined( __ANDROID__ ) || defined( ANDROID )
	#define SOIL_PLATFORM_ANDROID
#elif ( defined ( linux ) || defined( __linux__ ) || defined( __FreeBSD__ ) || defined(__OpenBSD__) || defined( __NetBSD__ ) || defined( __DragonFly__ ) || defined( __SVR4 ) )
	#define SOIL_X11_PLATFORM
#endif

#if ( defined( SOIL_PLATFORM_IOS ) || defined( SOIL_PLATFORM_ANDROID ) ) && ( !defined( SOIL_GLES1 ) && !defined( SOIL_GLES2 ) )
	#define SOIL_GLES2
#endif

#if ( defined( SOIL_GLES2 ) || defined( SOIL_GLES1 ) ) && !defined( SOIL_NO_EGL ) && !defined( SOIL_PLATFORM_IOS )
	#include <EGL/egl.h>
#endif

#if defined( SOIL_GLES2 )
	#ifdef SOIL_PLATFORM_IOS
		#include <OpenGLES/ES2/gl.h>
		#include <OpenGLES/ES2/glext.h>
	#else
		#include <GLES2/gl2.h>
		#include <GLES2/gl2ext.h>
	#endif

	#define APIENTRY GL_APIENTRY
#elif defined( SOIL_GLES1 )
	#ifndef GL_GLEXT_PROTOTYPES
	#define GL_GLEXT_PROTOTYPES
	#endif
	#ifdef SOIL_PLATFORM_IOS
		#include <OpenGLES/ES1/gl.h>
		#include <OpenGLES/ES1/glext.h>
	#else
		#include <GLES/gl.h>
		#include <GLES/glext.h>
	#endif

	#define APIENTRY GL_APIENTRY
#else

#if defined( __WIN32__ ) || defined( _WIN32 ) || defined( WIN32 )
	#define SOIL_PLATFORM_WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <wingdi.h>
	#include <GL/gl.h>

	#ifndef GL_UNSIGNED_SHORT_4_4_4_4
	#define GL_UNSIGNED_SHORT_4_4_4_4 0x8033
	#endif
	#ifndef GL_UNSIGNED_SHORT_5_5_5_1
	#define GL_UNSIGNED_SHORT_5_5_5_1 0x8034
	#endif
	#ifndef GL_UNSIGNED_SHORT_5_6_5
	#define GL_UNSIGNED_SHORT_5_6_5 0x8363
	#endif
#elif defined(__APPLE__) || defined(__APPLE_CC__)
	/*	I can't test this Apple stuff!	*/
	#include <OpenGL/gl.h>
	#include <Carbon/Carbon.h>
	#define APIENTRY
#elif defined( SOIL_X11_PLATFORM )
	#include <GL/gl.h>
	#include <GL/glx.h>
#else
	#include <GL/gl.h>
#endif

#endif

#ifndef GL_BGRA
#define GL_BGRA                                             0x80E1
#endif

#ifndef GL_RG
#define GL_RG                             0x8227
#endif

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>

/*	error reporting	*/
const char *result_string_pointer = "SOIL initialized";

/*	for loading cube maps	*/
enum{
	SOIL_CAPABILITY_UNKNOWN = -1,
	SOIL_CAPABILITY_NONE = 0,
	SOIL_CAPABILITY_PRESENT = 1
};
static int has_cubemap_capability = SOIL_CAPABILITY_UNKNOWN;
int query_cubemap_capability( void );
#define SOIL_TEXTURE_WRAP_R					0x8072
#define SOIL_CLAMP_TO_EDGE					0x812F
#define SOIL_NORMAL_MAP						0x8511
#define SOIL_REFLECTION_MAP					0x8512
#define SOIL_TEXTURE_CUBE_MAP				0x8513
#define SOIL_TEXTURE_BINDING_CUBE_MAP		0x8514
#define SOIL_TEXTURE_CUBE_MAP_POSITIVE_X	0x8515
#define SOIL_TEXTURE_CUBE_MAP_NEGATIVE_X	0x8516
#define SOIL_TEXTURE_CUBE_MAP_POSITIVE_Y	0x8517
#define SOIL_TEXTURE_CUBE_MAP_NEGATIVE_Y	0x8518
#define SOIL_TEXTURE_CUBE_MAP_POSITIVE_Z	0x8519
#define SOIL_TEXTURE_CUBE_MAP_NEGATIVE_Z	0x851A
#define SOIL_PROXY_TEXTURE_CUBE_MAP			0x851B
#define SOIL_MAX_CUBE_MAP_TEXTURE_SIZE		0x851C
/*	for non-power-of-two texture	*/
#define SOIL_IS_POW2( v ) ( ( v & ( v - 1 ) ) == 0 )
static int has_NPOT_capability = SOIL_CAPABILITY_UNKNOWN;
int query_NPOT_capability( void );
/*	for texture rectangles	*/
static int has_tex_rectangle_capability = SOIL_CAPABILITY_UNKNOWN;
int query_tex_rectangle_capability( void );
#define SOIL_TEXTURE_RECTANGLE_ARB				0x84F5
#define SOIL_MAX_RECTANGLE_TEXTURE_SIZE_ARB		0x84F8
/*	for using DXT compression	*/
static int has_DXT_capability = SOIL_CAPABILITY_UNKNOWN;
int query_DXT_capability( void );
#define SOIL_RGB_S3TC_DXT1		0x83F0
#define SOIL_RGBA_S3TC_DXT1		0x83F1
#define SOIL_RGBA_S3TC_DXT3		0x83F2
#define SOIL_RGBA_S3TC_DXT5		0x83F3
typedef void (APIENTRY * P_SOIL_GLCOMPRESSEDTEXIMAGE2DPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid * data);
static P_SOIL_GLCOMPRESSEDTEXIMAGE2DPROC soilGlCompressedTexImage2D = NULL;

typedef void (APIENTRY *P_SOIL_GLGENERATEMIPMAPPROC)(GLenum target);
static P_SOIL_GLGENERATEMIPMAPPROC soilGlGenerateMipmap = NULL;

static int has_gen_mipmap_capability = SOIL_CAPABILITY_UNKNOWN;
static int query_gen_mipmap_capability( void );

static int has_PVR_capability = SOIL_CAPABILITY_UNKNOWN;
int query_PVR_capability( void );
static int has_BGRA8888_capability = SOIL_CAPABILITY_UNKNOWN;
int query_BGRA8888_capability( void );
static int has_ETC1_capability = SOIL_CAPABILITY_UNKNOWN;
int query_ETC1_capability( void );

/* GL_IMG_texture_compression_pvrtc */
#define SOIL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG                      0x8C00
#define SOIL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG                      0x8C01
#define SOIL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG                     0x8C02
#define SOIL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG                     0x8C03
#define SOIL_GL_ETC1_RGB8_OES                                     0x8D64

#if defined( SOIL_X11_PLATFORM ) || defined( SOIL_PLATFORM_WIN32 ) || defined( SOIL_PLATFORM_OSX )
typedef const GLubyte *(APIENTRY * P_SOIL_glGetStringiFunc) (GLenum, GLuint);
static P_SOIL_glGetStringiFunc soilGlGetStringiFunc = NULL;

static int isAtLeastGL3()
{
	static int is_gl3 = SOIL_CAPABILITY_UNKNOWN;

	if ( SOIL_CAPABILITY_UNKNOWN == is_gl3 )
	{
		const char * verstr	= (const char *) glGetString( GL_VERSION );
		is_gl3				= ( verstr && ( atoi(verstr) >= 3 ) );
	}

	return is_gl3;
}
#endif

#ifdef SOIL_PLATFORM_WIN32
static int soilTestWinProcPointer(const PROC pTest)
{
	ptrdiff_t iTest;
	if(!pTest) return 0;
	iTest = (ptrdiff_t)pTest;
	if(iTest == 1 || iTest == 2 || iTest == 3 || iTest == -1) return 0;
	return 1;
}
#endif

void * SOIL_GL_GetProcAddress(const char *proc)
{
	void *func = NULL;

#if defined( SOIL_PLATFORM_IOS )
	func = dlsym( RTLD_DEFAULT, proc );
#elif defined( SOIL_GLES2 ) || defined( SOIL_GLES1 )
	#ifndef SOIL_NO_EGL
		func = eglGetProcAddress( proc );
	#else
		func = NULL;
	#endif
#elif defined( SOIL_PLATFORM_WIN32 )
	func =  wglGetProcAddress( proc );

	if (!soilTestWinProcPointer((const PROC)func))
		func = NULL;
#elif defined( SOIL_PLATFORM_OSX )
	/*	I can't test this Apple stuff!	*/
	CFBundleRef bundle;
	CFURLRef bundleURL =
	CFURLCreateWithFileSystemPath(
								  kCFAllocatorDefault,
								  CFSTR("/System/Library/Frameworks/OpenGL.framework"),
								  kCFURLPOSIXPathStyle,
								  true );
	CFStringRef extensionName =
	CFStringCreateWithCString(
							  kCFAllocatorDefault,
							  proc,
							  kCFStringEncodingASCII );
	bundle = CFBundleCreate( kCFAllocatorDefault, bundleURL );
	assert( bundle != NULL );

	func = CFBundleGetFunctionPointerForName( bundle, extensionName );

	CFRelease( bundleURL );
	CFRelease( extensionName );
	CFRelease( bundle );
#elif defined( SOIL_X11_PLATFORM )
	func =
#if !defined(GLX_VERSION_1_4)
	glXGetProcAddressARB
#else
	glXGetProcAddress
#endif
	( (const GLubyte *)proc );
#endif

	return func;
}

/* Based on the SDL2 implementation */
int SOIL_GL_ExtensionSupported(const char *extension)
{
	const char *extensions;
	const char *start;
	const char *where, *terminator;

	/* Extension names should not have spaces. */
	where = strchr(extension, ' ');

	if (where || *extension == '\0')
	{
		return 0;
	}

	#if defined( SOIL_X11_PLATFORM ) || defined( SOIL_PLATFORM_WIN32 ) || defined( SOIL_PLATFORM_OSX )
	/* Lookup the available extensions */
	if ( isAtLeastGL3() )
	{
		GLint num_exts = 0;
		GLint i;

		if ( NULL == soilGlGetStringiFunc )
		{
			soilGlGetStringiFunc = (P_SOIL_glGetStringiFunc)SOIL_GL_GetProcAddress("glGetStringi");

			if ( NULL == soilGlGetStringiFunc )
			{
				return 0;
			}
		}

		#ifndef GL_NUM_EXTENSIONS
		#define GL_NUM_EXTENSIONS 0x821D
		#endif
		glGetIntegerv(GL_NUM_EXTENSIONS, &num_exts);
		for (i = 0; i < num_exts; i++)
		{
			const char *thisext = (const char *) soilGlGetStringiFunc(GL_EXTENSIONS, i);

			if (strcmp(thisext, extension) == 0)
			{
				return 1;
			}
		}

		return 0;
	}
	#endif

	/* Try the old way with glGetString(GL_EXTENSIONS) ... */
	extensions = (const char *) glGetString(GL_EXTENSIONS);

	if (!extensions)
	{
		return 0;
	}

	/*
	 * It takes a bit of care to be fool-proof about parsing the OpenGL
	 * extensions string. Don't be fooled by sub-strings, etc.
	 */
	start = extensions;

	for (;;) {
		where = strstr(start, extension);

		if (!where)
			break;

		terminator = where + strlen(extension);

		if (where == start || *(where - 1) == ' ')
			if (*terminator == ' ' || *terminator == '\0')
				return 1;

		start = terminator;
	}

	return 0;
}

/*	other functions	*/
unsigned int
	SOIL_internal_create_OGL_texture
	(
		const unsigned char *const data,
		int *width, int *height, int channels,
		unsigned int reuse_texture_ID,
		unsigned int flags,
		unsigned int opengl_texture_type,
		unsigned int opengl_texture_target,
		unsigned int texture_check_size_enum
	);

/*	and the code magic begins here [8^)	*/
unsigned int
	SOIL_load_OGL_texture
	(
		const char *filename,
		int force_channels,
		unsigned int reuse_texture_ID,
		unsigned int flags
	)
{
	/*	variables	*/
	unsigned char* img;
	int width, height, channels;
	unsigned int tex_id;
	/*	does the user want direct uploading of the image as a DDS file?	*/
	if( flags & SOIL_FLAG_DDS_LOAD_DIRECT )
	{
		/*	1st try direct loading of the image as a DDS file
			note: direct uploading will only load what is in the
			DDS file, no MIPmaps will be generated, the image will
			not be flipped, etc.	*/
		tex_id = SOIL_direct_load_DDS( filename, reuse_texture_ID, flags, 0 );
		if( tex_id )
		{
			/*	hey, it worked!!	*/
			return tex_id;
		}
	}

	if( flags & SOIL_FLAG_PVR_LOAD_DIRECT )
	{
		tex_id = SOIL_direct_load_PVR( filename, reuse_texture_ID, flags, 0 );
		if( tex_id )
		{
			/*	hey, it worked!!	*/
			return tex_id;
		}
	}

	if( flags & SOIL_FLAG_ETC1_LOAD_DIRECT )
	{
		tex_id = SOIL_direct_load_ETC1( filename, reuse_texture_ID, flags );
		if( tex_id )
		{
			/*	hey, it worked!!	*/
			return tex_id;
		}
	}

	/*	try to load the image	*/
	img = SOIL_load_image( filename, &width, &height, &channels, force_channels );
	/*	channels holds the original number of channels, which may have been forced	*/
	if( (force_channels >= 1) && (force_channels <= 4) )
	{
		channels = force_channels;
	}
	if( NULL == img )
	{
		/*	image loading failed	*/
		result_string_pointer = stbi_failure_reason();
		return 0;
	}
	/*	OK, make it a texture!	*/
	tex_id = SOIL_internal_create_OGL_texture(
			img, &width, &height, channels,
			reuse_texture_ID, flags,
			GL_TEXTURE_2D, GL_TEXTURE_2D,
			GL_MAX_TEXTURE_SIZE );
	/*	and nuke the image data	*/
	SOIL_free_image_data( img );
	/*	and return the handle, such as it is	*/
	return tex_id;
}

unsigned int
	SOIL_load_OGL_HDR_texture
	(
		const char *filename,
		int fake_HDR_format,
		int rescale_to_max,
		unsigned int reuse_texture_ID,
		unsigned int flags
	)
{
	/*	variables	*/
	unsigned char* img = NULL;
	int width, height, channels;
	unsigned int tex_id;
	/*	no direct uploading of the image as a DDS file	*/
	/* error check */
	if( (fake_HDR_format != SOIL_HDR_RGBE) &&
		(fake_HDR_format != SOIL_HDR_RGBdivA) &&
		(fake_HDR_format != SOIL_HDR_RGBdivA2) )
	{
		result_string_pointer = "Invalid fake HDR format specified";
		return 0;
	}

	/* check if the image is HDR */
	if ( stbi_is_hdr( filename ) )
	{
		/*	try to load the image (only the HDR type) */
		img = stbi_load( filename, &width, &height, &channels, 4 );
	}

	/*	channels holds the original number of channels, which may have been forced	*/
	if( NULL == img )
	{
		/*	image loading failed	*/
		result_string_pointer = stbi_failure_reason();
		return 0;
	}
	/* the load worked, do I need to convert it? */
	if( fake_HDR_format == SOIL_HDR_RGBdivA )
	{
		RGBE_to_RGBdivA( img, width, height, rescale_to_max );
	} else if( fake_HDR_format == SOIL_HDR_RGBdivA2 )
	{
		RGBE_to_RGBdivA2( img, width, height, rescale_to_max );
	}
	/*	OK, make it a texture!	*/
	tex_id = SOIL_internal_create_OGL_texture(
			img, &width, &height, channels,
			reuse_texture_ID, flags,
			GL_TEXTURE_2D, GL_TEXTURE_2D,
			GL_MAX_TEXTURE_SIZE );
	/*	and nuke the image data	*/
	SOIL_free_image_data( img );
	/*	and return the handle, such as it is	*/
	return tex_id;
}

unsigned int
	SOIL_load_OGL_texture_from_memory
	(
		const unsigned char *const buffer,
		int buffer_length,
		int force_channels,
		unsigned int reuse_texture_ID,
		unsigned int flags
	)
{
	/*	variables	*/
	unsigned char* img;
	int width, height, channels;
	unsigned int tex_id;
	/*	does the user want direct uploading of the image as a DDS file?	*/
	if( flags & SOIL_FLAG_DDS_LOAD_DIRECT )
	{
		/*	1st try direct loading of the image as a DDS file
			note: direct uploading will only load what is in the
			DDS file, no MIPmaps will be generated, the image will
			not be flipped, etc.	*/
		tex_id = SOIL_direct_load_DDS_from_memory(
				buffer, buffer_length,
				reuse_texture_ID, flags, 0 );
		if( tex_id )
		{
			/*	hey, it worked!!	*/
			return tex_id;
		}
	}

	if( flags & SOIL_FLAG_PVR_LOAD_DIRECT )
	{
		tex_id = SOIL_direct_load_PVR_from_memory(
				buffer, buffer_length,
				reuse_texture_ID, flags, 0 );
		if( tex_id )
		{
			/*	hey, it worked!!	*/
			return tex_id;
		}
	}

	if( flags & SOIL_FLAG_ETC1_LOAD_DIRECT )
	{
		tex_id = SOIL_direct_load_ETC1_from_memory(
				buffer, buffer_length,
				reuse_texture_ID, flags );
		if( tex_id )
		{
			/*	hey, it worked!!	*/
			return tex_id;
		}
	}

	/*	try to load the image	*/
	img = SOIL_load_image_from_memory(
					buffer, buffer_length,
					&width, &height, &channels,
					force_channels );
	/*	channels holds the original number of channels, which may have been forced	*/
	if( (force_channels >= 1) && (force_channels <= 4) )
	{
		channels = force_channels;
	}
	if( NULL == img )
	{
		/*	image loading failed	*/
		result_string_pointer = stbi_failure_reason();
		return 0;
	}
	/*	OK, make it a texture!	*/
	tex_id = SOIL_internal_create_OGL_texture(
			img, &width, &height, channels,
			reuse_texture_ID, flags,
			GL_TEXTURE_2D, GL_TEXTURE_2D,
			GL_MAX_TEXTURE_SIZE );
	/*	and nuke the image data	*/
	SOIL_free_image_data( img );
	/*	and return the handle, such as it is	*/
	return tex_id;
}

unsigned int
	SOIL_load_OGL_cubemap
	(
		const char *x_pos_file,
		const char *x_neg_file,
		const char *y_pos_file,
		const char *y_neg_file,
		const char *z_pos_file,
		const char *z_neg_file,
		int force_channels,
		unsigned int reuse_texture_ID,
		unsigned int flags
	)
{
	/*	variables	*/
	unsigned char* img;
	int width, height, channels;
	unsigned int tex_id;
	/*	error checking	*/
	if( (x_pos_file == NULL) ||
		(x_neg_file == NULL) ||
		(y_pos_file == NULL) ||
		(y_neg_file == NULL) ||
		(z_pos_file == NULL) ||
		(z_neg_file == NULL) )
	{
		result_string_pointer = "Invalid cube map files list";
		return 0;
	}
	/*	capability checking	*/
	if( query_cubemap_capability() != SOIL_CAPABILITY_PRESENT )
	{
		result_string_pointer = "No cube map capability present";
		return 0;
	}
	/*	1st face: try to load the image	*/
	img = SOIL_load_image( x_pos_file, &width, &height, &channels, force_channels );
	/*	channels holds the original number of channels, which may have been forced	*/
	if( (force_channels >= 1) && (force_channels <= 4) )
	{
		channels = force_channels;
	}
	if( NULL == img )
	{
		/*	image loading failed	*/
		result_string_pointer = stbi_failure_reason();
		return 0;
	}
	/*	upload the texture, and create a texture ID if necessary	*/
	tex_id = SOIL_internal_create_OGL_texture(
			img, &width, &height, channels,
			reuse_texture_ID, flags,
			SOIL_TEXTURE_CUBE_MAP, SOIL_TEXTURE_CUBE_MAP_POSITIVE_X,
			SOIL_MAX_CUBE_MAP_TEXTURE_SIZE );
	/*	and nuke the image data	*/
	SOIL_free_image_data( img );
	/*	continue?	*/
	if( tex_id != 0 )
	{
		/*	1st face: try to load the image	*/
		img = SOIL_load_image( x_neg_file, &width, &height, &channels, force_channels );
		/*	channels holds the original number of channels, which may have been forced	*/
		if( (force_channels >= 1) && (force_channels <= 4) )
		{
			channels = force_channels;
		}
		if( NULL == img )
		{
			/*	image loading failed	*/
			result_string_pointer = stbi_failure_reason();
			return 0;
		}
		/*	upload the texture, but reuse the assigned texture ID	*/
		tex_id = SOIL_internal_create_OGL_texture(
				img, &width, &height, channels,
				tex_id, flags,
				SOIL_TEXTURE_CUBE_MAP, SOIL_TEXTURE_CUBE_MAP_NEGATIVE_X,
				SOIL_MAX_CUBE_MAP_TEXTURE_SIZE );
		/*	and nuke the image data	*/
		SOIL_free_image_data( img );
	}
	/*	continue?	*/
	if( tex_id != 0 )
	{
		/*	1st face: try to load the image	*/
		img = SOIL_load_image( y_pos_file, &width, &height, &channels, force_channels );
		/*	channels holds the original number of channels, which may have been forced	*/
		if( (force_channels >= 1) && (force_channels <= 4) )
		{
			channels = force_channels;
		}
		if( NULL == img )
		{
			/*	image loading failed	*/
			result_string_pointer = stbi_failure_reason();
			return 0;
		}
		/*	upload the texture, but reuse the assigned texture ID	*/
		tex_id = SOIL_internal_create_OGL_texture(
				img, &width, &height, channels,
				tex_id, flags,
				SOIL_TEXTURE_CUBE_MAP, SOIL_TEXTURE_CUBE_MAP_POSITIVE_Y,
				SOIL_MAX_CUBE_MAP_TEXTURE_SIZE );
		/*	and nuke the image data	*/
		SOIL_free_image_data( img );
	}
	/*	continue?	*/
	if( tex_id != 0 )
	{
		/*	1st face: try to load the image	*/
		img = SOIL_load_image( y_neg_file, &width, &height, &channels, force_channels );
		/*	channels holds the original number of channels, which may have been forced	*/
		if( (force_channels >= 1) && (force_channels <= 4) )
		{
			channels = force_channels;
		}
		if( NULL == img )
		{
			/*	image loading failed	*/
			result_string_pointer = stbi_failure_reason();
			return 0;
		}
		/*	upload the texture, but reuse the assigned texture ID	*/
		tex_id = SOIL_internal_create_OGL_texture(
				img, &width, &height, channels,
				tex_id, flags,
				SOIL_TEXTURE_CUBE_MAP, SOIL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
				SOIL_MAX_CUBE_MAP_TEXTURE_SIZE );
		/*	and nuke the image data	*/
		SOIL_free_image_data( img );
	}
	/*	continue?	*/
	if( tex_id != 0 )
	{
		/*	1st face: try to load the image	*/
		img = SOIL_load_image( z_pos_file, &width, &height, &channels, force_channels );
		/*	channels holds the original number of channels, which may have been forced	*/
		if( (force_channels >= 1) && (force_channels <= 4) )
		{
			channels = force_channels;
		}
		if( NULL == img )
		{
			/*	image loading failed	*/
			result_string_pointer = stbi_failure_reason();
			return 0;
		}
		/*	upload the texture, but reuse the assigned texture ID	*/
		tex_id = SOIL_internal_create_OGL_texture(
				img, &width, &height, channels,
				tex_id, flags,
				SOIL_TEXTURE_CUBE_MAP, SOIL_TEXTURE_CUBE_MAP_POSITIVE_Z,
				SOIL_MAX_CUBE_MAP_TEXTURE_SIZE );
		/*	and nuke the image data	*/
		SOIL_free_image_data( img );
	}
	/*	continue?	*/
	if( tex_id != 0 )
	{
		/*	1st face: try to load the image	*/
		img = SOIL_load_image( z_neg_file, &width, &height, &channels, force_channels );
		/*	channels holds the original number of channels, which may have been forced	*/
		if( (force_channels >= 1) && (force_channels <= 4) )
		{
			channels = force_channels;
		}
		if( NULL == img )
		{
			/*	image loading failed	*/
			result_string_pointer = stbi_failure_reason();
			return 0;
		}
		/*	upload the texture, but reuse the assigned texture ID	*/
		tex_id = SOIL_internal_create_OGL_texture(
				img, &width, &height, channels,
				tex_id, flags,
				SOIL_TEXTURE_CUBE_MAP, SOIL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
				SOIL_MAX_CUBE_MAP_TEXTURE_SIZE );
		/*	and nuke the image data	*/
		SOIL_free_image_data( img );
	}
	/*	and return the handle, such as it is	*/
	return tex_id;
}

unsigned int
	SOIL_load_OGL_cubemap_from_memory
	(
		const unsigned char *const x_pos_buffer,
		int x_pos_buffer_length,
		const unsigned char *const x_neg_buffer,
		int x_neg_buffer_length,
		const unsigned char *const y_pos_buffer,
		int y_pos_buffer_length,
		const unsigned char *const y_neg_buffer,
		int y_neg_buffer_length,
		const unsigned char *const z_pos_buffer,
		int z_pos_buffer_length,
		const unsigned char *const z_neg_buffer,
		int z_neg_buffer_length,
		int force_channels,
		unsigned int reuse_texture_ID,
		unsigned int flags
	)
{
	/*	variables	*/
	unsigned char* img;
	int width, height, channels;
	unsigned int tex_id;
	/*	error checking	*/
	if( (x_pos_buffer == NULL) ||
		(x_neg_buffer == NULL) ||
		(y_pos_buffer == NULL) ||
		(y_neg_buffer == NULL) ||
		(z_pos_buffer == NULL) ||
		(z_neg_buffer == NULL) )
	{
		result_string_pointer = "Invalid cube map buffers list";
		return 0;
	}
	/*	capability checking	*/
	if( query_cubemap_capability() != SOIL_CAPABILITY_PRESENT )
	{
		result_string_pointer = "No cube map capability present";
		return 0;
	}
	/*	1st face: try to load the image	*/
	img = SOIL_load_image_from_memory(
			x_pos_buffer, x_pos_buffer_length,
			&width, &height, &channels, force_channels );
	/*	channels holds the original number of channels, which may have been forced	*/
	if( (force_channels >= 1) && (force_channels <= 4) )
	{
		channels = force_channels;
	}
	if( NULL == img )
	{
		/*	image loading failed	*/
		result_string_pointer = stbi_failure_reason();
		return 0;
	}
	/*	upload the texture, and create a texture ID if necessary	*/
	tex_id = SOIL_internal_create_OGL_texture(
			img, &width, &height, channels,
			reuse_texture_ID, flags,
			SOIL_TEXTURE_CUBE_MAP, SOIL_TEXTURE_CUBE_MAP_POSITIVE_X,
			SOIL_MAX_CUBE_MAP_TEXTURE_SIZE );
	/*	and nuke the image data	*/
	SOIL_free_image_data( img );
	/*	continue?	*/
	if( tex_id != 0 )
	{
		/*	1st face: try to load the image	*/
		img = SOIL_load_image_from_memory(
				x_neg_buffer, x_neg_buffer_length,
				&width, &height, &channels, force_channels );
		/*	channels holds the original number of channels, which may have been forced	*/
		if( (force_channels >= 1) && (force_channels <= 4) )
		{
			channels = force_channels;
		}
		if( NULL == img )
		{
			/*	image loading failed	*/
			result_string_pointer = stbi_failure_reason();
			return 0;
		}
		/*	upload the texture, but reuse the assigned texture ID	*/
		tex_id = SOIL_internal_create_OGL_texture(
				img, &width, &height, channels,
				tex_id, flags,
				SOIL_TEXTURE_CUBE_MAP, SOIL_TEXTURE_CUBE_MAP_NEGATIVE_X,
				SOIL_MAX_CUBE_MAP_TEXTURE_SIZE );
		/*	and nuke the image data	*/
		SOIL_free_image_data( img );
	}
	/*	continue?	*/
	if( tex_id != 0 )
	{
		/*	1st face: try to load the image	*/
		img = SOIL_load_image_from_memory(
				y_pos_buffer, y_pos_buffer_length,
				&width, &height, &channels, force_channels );
		/*	channels holds the original number of channels, which may have been forced	*/
		if( (force_channels >= 1) && (force_channels <= 4) )
		{
			channels = force_channels;
		}
		if( NULL == img )
		{
			/*	image loading failed	*/
			result_string_pointer = stbi_failure_reason();
			return 0;
		}
		/*	upload the texture, but reuse the assigned texture ID	*/
		tex_id = SOIL_internal_create_OGL_texture(
				img, &width, &height, channels,
				tex_id, flags,
				SOIL_TEXTURE_CUBE_MAP, SOIL_TEXTURE_CUBE_MAP_POSITIVE_Y,
				SOIL_MAX_CUBE_MAP_TEXTURE_SIZE );
		/*	and nuke the image data	*/
		SOIL_free_image_data( img );
	}
	/*	continue?	*/
	if( tex_id != 0 )
	{
		/*	1st face: try to load the image	*/
		img = SOIL_load_image_from_memory(
				y_neg_buffer, y_neg_buffer_length,
				&width, &height, &channels, force_channels );
		/*	channels holds the original number of channels, which may have been forced	*/
		if( (force_channels >= 1) && (force_channels <= 4) )
		{
			channels = force_channels;
		}
		if( NULL == img )
		{
			/*	image loading failed	*/
			result_string_pointer = stbi_failure_reason();
			return 0;
		}
		/*	upload the texture, but reuse the assigned texture ID	*/
		tex_id = SOIL_internal_create_OGL_texture(
				img, &width, &height, channels,
				tex_id, flags,
				SOIL_TEXTURE_CUBE_MAP, SOIL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
				SOIL_MAX_CUBE_MAP_TEXTURE_SIZE );
		/*	and nuke the image data	*/
		SOIL_free_image_data( img );
	}
	/*	continue?	*/
	if( tex_id != 0 )
	{
		/*	1st face: try to load the image	*/
		img = SOIL_load_image_from_memory(
				z_pos_buffer, z_pos_buffer_length,
				&width, &height, &channels, force_channels );
		/*	channels holds the original number of channels, which may have been forced	*/
		if( (force_channels >= 1) && (force_channels <= 4) )
		{
			channels = force_channels;
		}
		if( NULL == img )
		{
			/*	image loading failed	*/
			result_string_pointer = stbi_failure_reason();
			return 0;
		}
		/*	upload the texture, but reuse the assigned texture ID	*/
		tex_id = SOIL_internal_create_OGL_texture(
				img, &width, &height, channels,
				tex_id, flags,
				SOIL_TEXTURE_CUBE_MAP, SOIL_TEXTURE_CUBE_MAP_POSITIVE_Z,
				SOIL_MAX_CUBE_MAP_TEXTURE_SIZE );
		/*	and nuke the image data	*/
		SOIL_free_image_data( img );
	}
	/*	continue?	*/
	if( tex_id != 0 )
	{
		/*	1st face: try to load the image	*/
		img = SOIL_load_image_from_memory(
				z_neg_buffer, z_neg_buffer_length,
				&width, &height, &channels, force_channels );
		/*	channels holds the original number of channels, which may have been forced	*/
		if( (force_channels >= 1) && (force_channels <= 4) )
		{
			channels = force_channels;
		}
		if( NULL == img )
		{
			/*	image loading failed	*/
			result_string_pointer = stbi_failure_reason();
			return 0;
		}
		/*	upload the texture, but reuse the assigned texture ID	*/
		tex_id = SOIL_internal_create_OGL_texture(
				img, &width, &height, channels,
				tex_id, flags,
				SOIL_TEXTURE_CUBE_MAP, SOIL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
				SOIL_MAX_CUBE_MAP_TEXTURE_SIZE );
		/*	and nuke the image data	*/
		SOIL_free_image_data( img );
	}
	/*	and return the handle, such as it is	*/
	return tex_id;
}

unsigned int
	SOIL_load_OGL_single_cubemap
	(
		const char *filename,
		const char face_order[6],
		int force_channels,
		unsigned int reuse_texture_ID,
		unsigned int flags
	)
{
	/*	variables	*/
	unsigned char* img;
	int width, height, channels, i;
	unsigned int tex_id = 0;
	/*	error checking	*/
	if( filename == NULL )
	{
		result_string_pointer = "Invalid single cube map file name";
		return 0;
	}
	/*	does the user want direct uploading of the image as a DDS file?	*/
	if( flags & SOIL_FLAG_DDS_LOAD_DIRECT )
	{
		/*	1st try direct loading of the image as a DDS file
			note: direct uploading will only load what is in the
			DDS file, no MIPmaps will be generated, the image will
			not be flipped, etc.	*/
		tex_id = SOIL_direct_load_DDS( filename, reuse_texture_ID, flags, 1 );
		if( tex_id )
		{
			/*	hey, it worked!!	*/
			return tex_id;
		}
	}

	if ( flags & SOIL_FLAG_PVR_LOAD_DIRECT )
	{
		tex_id = SOIL_direct_load_PVR( filename, reuse_texture_ID, flags, 1 );
		if( tex_id )
		{
			/*	hey, it worked!!	*/
			return tex_id;
		}
	}

	if ( flags & SOIL_FLAG_ETC1_LOAD_DIRECT )
	{
		return 0;
	}

	/*	face order checking	*/
	for( i = 0; i < 6; ++i )
	{
		if( (face_order[i] != 'N') &&
			(face_order[i] != 'S') &&
			(face_order[i] != 'W') &&
			(face_order[i] != 'E') &&
			(face_order[i] != 'U') &&
			(face_order[i] != 'D') )
		{
			result_string_pointer = "Invalid single cube map face order";
			return 0;
		};
	}
	/*	capability checking	*/
	if( query_cubemap_capability() != SOIL_CAPABILITY_PRESENT )
	{
		result_string_pointer = "No cube map capability present";
		return 0;
	}
	/*	1st off, try to load the full image	*/
	img = SOIL_load_image( filename, &width, &height, &channels, force_channels );
	/*	channels holds the original number of channels, which may have been forced	*/
	if( (force_channels >= 1) && (force_channels <= 4) )
	{
		channels = force_channels;
	}
	if( NULL == img )
	{
		/*	image loading failed	*/
		result_string_pointer = stbi_failure_reason();
		return 0;
	}
	/*	now, does this image have the right dimensions?	*/
	if( (width != 6*height) &&
		(6*width != height) )
	{
		SOIL_free_image_data( img );
		result_string_pointer = "Single cubemap image must have a 6:1 ratio";
		return 0;
	}
	/*	try the image split and create	*/
	tex_id = SOIL_create_OGL_single_cubemap(
			img, width, height, channels,
			face_order, reuse_texture_ID, flags
			);
	/*	nuke the temporary image data and return the texture handle	*/
	SOIL_free_image_data( img );
	return tex_id;
}

unsigned int
	SOIL_load_OGL_single_cubemap_from_memory
	(
		const unsigned char *const buffer,
		int buffer_length,
		const char face_order[6],
		int force_channels,
		unsigned int reuse_texture_ID,
		unsigned int flags
	)
{
	/*	variables	*/
	unsigned char* img;
	int width, height, channels, i;
	unsigned int tex_id = 0;
	/*	error checking	*/
	if( buffer == NULL )
	{
		result_string_pointer = "Invalid single cube map buffer";
		return 0;
	}
	/*	does the user want direct uploading of the image as a DDS file?	*/
	if( flags & SOIL_FLAG_DDS_LOAD_DIRECT )
	{
		/*	1st try direct loading of the image as a DDS file
			note: direct uploading will only load what is in the
			DDS file, no MIPmaps will be generated, the image will
			not be flipped, etc.	*/
		tex_id = SOIL_direct_load_DDS_from_memory(
				buffer, buffer_length,
				reuse_texture_ID, flags, 1 );
		if( tex_id )
		{
			/*	hey, it worked!!	*/
			return tex_id;
		}
	}

	if ( flags & SOIL_FLAG_PVR_LOAD_DIRECT )
	{
		tex_id = SOIL_direct_load_PVR_from_memory(
				buffer, buffer_length,
				reuse_texture_ID, flags, 1 );
		if ( tex_id )
		{
			/*	hey, it worked!!	*/
			return tex_id;
		}
	}

	if ( flags & SOIL_FLAG_ETC1_LOAD_DIRECT )
	{
		return 0;
	}

	/*	face order checking	*/
	for( i = 0; i < 6; ++i )
	{
		if( (face_order[i] != 'N') &&
			(face_order[i] != 'S') &&
			(face_order[i] != 'W') &&
			(face_order[i] != 'E') &&
			(face_order[i] != 'U') &&
			(face_order[i] != 'D') )
		{
			result_string_pointer = "Invalid single cube map face order";
			return 0;
		};
	}
	/*	capability checking	*/
	if( query_cubemap_capability() != SOIL_CAPABILITY_PRESENT )
	{
		result_string_pointer = "No cube map capability present";
		return 0;
	}
	/*	1st off, try to load the full image	*/
	img = SOIL_load_image_from_memory(
			buffer, buffer_length,
			&width, &height, &channels,
			force_channels );
	/*	channels holds the original number of channels, which may have been forced	*/
	if( (force_channels >= 1) && (force_channels <= 4) )
	{
		channels = force_channels;
	}
	if( NULL == img )
	{
		/*	image loading failed	*/
		result_string_pointer = stbi_failure_reason();
		return 0;
	}
	/*	now, does this image have the right dimensions?	*/
	if( (width != 6*height) &&
		(6*width != height) )
	{
		SOIL_free_image_data( img );
		result_string_pointer = "Single cubemap image must have a 6:1 ratio";
		return 0;
	}
	/*	try the image split and create	*/
	tex_id = SOIL_create_OGL_single_cubemap(
			img, width, height, channels,
			face_order, reuse_texture_ID, flags
			);
	/*	nuke the temporary image data and return the texture handle	*/
	SOIL_free_image_data( img );
	return tex_id;
}

unsigned int
	SOIL_create_OGL_single_cubemap
	(
		const unsigned char *const data,
		int width, int height, int channels,
		const char face_order[6],
		unsigned int reuse_texture_ID,
		unsigned int flags
	)
{
	/*	variables	*/
	unsigned char* sub_img;
	int dw, dh, sz, i;
	unsigned int tex_id;
	/*	error checking	*/
	if( data == NULL )
	{
		result_string_pointer = "Invalid single cube map image data";
		return 0;
	}
	/*	face order checking	*/
	for( i = 0; i < 6; ++i )
	{
		if( (face_order[i] != 'N') &&
			(face_order[i] != 'S') &&
			(face_order[i] != 'W') &&
			(face_order[i] != 'E') &&
			(face_order[i] != 'U') &&
			(face_order[i] != 'D') )
		{
			result_string_pointer = "Invalid single cube map face order";
			return 0;
		};
	}
	/*	capability checking	*/
	if( query_cubemap_capability() != SOIL_CAPABILITY_PRESENT )
	{
		result_string_pointer = "No cube map capability present";
		return 0;
	}
	/*	now, does this image have the right dimensions?	*/
	if( (width != 6*height) &&
		(6*width != height) )
	{
		result_string_pointer = "Single cubemap image must have a 6:1 ratio";
		return 0;
	}
	/*	which way am I stepping?	*/
	if( width > height )
	{
		dw = height;
		dh = 0;
	} else
	{
		dw = 0;
		dh = width;
	}
	sz = dw+dh;
	sub_img = (unsigned char *)malloc( sz*sz*channels );
	/*	do the splitting and uploading	*/
	tex_id = reuse_texture_ID;
	for( i = 0; i < 6; ++i )
	{
		int x, y, idx = 0;
		unsigned int cubemap_target = 0;
		/*	copy in the sub-image	*/
		for( y = i*dh; y < i*dh+sz; ++y )
		{
			for( x = i*dw*channels; x < (i*dw+sz)*channels; ++x )
			{
				sub_img[idx++] = data[y*width*channels+x];
			}
		}
		/*	what is my texture target?
			remember, this coordinate system is
			LHS if viewed from inside the cube!	*/
		switch( face_order[i] )
		{
		case 'N':
			cubemap_target = SOIL_TEXTURE_CUBE_MAP_POSITIVE_Z;
			break;
		case 'S':
			cubemap_target = SOIL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
			break;
		case 'W':
			cubemap_target = SOIL_TEXTURE_CUBE_MAP_NEGATIVE_X;
			break;
		case 'E':
			cubemap_target = SOIL_TEXTURE_CUBE_MAP_POSITIVE_X;
			break;
		case 'U':
			cubemap_target = SOIL_TEXTURE_CUBE_MAP_POSITIVE_Y;
			break;
		case 'D':
			cubemap_target = SOIL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
			break;
		}
		/*	upload it as a texture	*/
		tex_id = SOIL_internal_create_OGL_texture(
				sub_img, &sz, &sz, channels,
				tex_id, flags,
				SOIL_TEXTURE_CUBE_MAP,
				cubemap_target,
				SOIL_MAX_CUBE_MAP_TEXTURE_SIZE );
	}
	/*	and nuke the image and sub-image data	*/
	SOIL_free_image_data( sub_img );
	/*	and return the handle, such as it is	*/
	return tex_id;
}

unsigned int
	SOIL_create_OGL_texture
	(
		const unsigned char *const data,
		int *width, int *height, int channels,
		unsigned int reuse_texture_ID,
		unsigned int flags
	)
{
	/*	wrapper function for 2D textures	*/
	return SOIL_internal_create_OGL_texture(
				data, width, height, channels,
				reuse_texture_ID, flags,
				GL_TEXTURE_2D, GL_TEXTURE_2D,
				GL_MAX_TEXTURE_SIZE );
}

#if SOIL_CHECK_FOR_GL_ERRORS
void check_for_GL_errors( const char *calling_location )
{
	/*	check for errors	*/
	GLenum err_code = glGetError();
	while( GL_NO_ERROR != err_code )
	{
		printf( "OpenGL Error @ %s: %i", calling_location, err_code );
		err_code = glGetError();
	}
}
#else
void check_for_GL_errors( const char *calling_location )
{
	/*	no check for errors	*/
}
#endif

static void createMipmaps(const unsigned char *const img,
		int width, int height, int channels,
		unsigned int flags,
		unsigned int opengl_texture_target,
		unsigned int internal_texture_format,
		unsigned int original_texture_format,
		int DXT_mode)
{
	if ( ( flags & SOIL_FLAG_GL_MIPMAPS ) && query_gen_mipmap_capability() == SOIL_CAPABILITY_PRESENT )
	{
		soilGlGenerateMipmap(opengl_texture_target);
	}
	else
	{
		int MIPlevel = 1;
		int MIPwidth = (width+1) / 2;
		int MIPheight = (height+1) / 2;
		unsigned char *resampled = (unsigned char*)malloc( channels*MIPwidth*MIPheight );

		while( ((1<<MIPlevel) <= width) || ((1<<MIPlevel) <= height) )
		{
			/*	do this MIPmap level	*/
			mipmap_image(
					img, width, height, channels,
					resampled,
					(1 << MIPlevel), (1 << MIPlevel) );

			/*  upload the MIPmaps	*/
			if( DXT_mode == SOIL_CAPABILITY_PRESENT )
			{
				/*	user wants me to do the DXT conversion!	*/
				int DDS_size;
				unsigned char *DDS_data = NULL;
				if( (channels & 1) == 1 )
				{
					/*	RGB, use DXT1	*/
					DDS_data = convert_image_to_DXT1(
							resampled, MIPwidth, MIPheight, channels, &DDS_size );
				} else
				{
					/*	RGBA, use DXT5	*/
					DDS_data = convert_image_to_DXT5(
							resampled, MIPwidth, MIPheight, channels, &DDS_size );
				}
				if( DDS_data )
				{
					soilGlCompressedTexImage2D(
						opengl_texture_target, MIPlevel,
						internal_texture_format, MIPwidth, MIPheight, 0,
						DDS_size, DDS_data );
					check_for_GL_errors( "glCompressedTexImage2D" );
					SOIL_free_image_data( DDS_data );
				} else
				{
					/*	my compression failed, try the OpenGL driver's version	*/
					glTexImage2D(
						opengl_texture_target, MIPlevel,
						internal_texture_format, MIPwidth, MIPheight, 0,
						original_texture_format, GL_UNSIGNED_BYTE, resampled );
					check_for_GL_errors( "glTexImage2D" );
				}
			} else
			{
				/*	user want OpenGL to do all the work!	*/
				glTexImage2D(
					opengl_texture_target, MIPlevel,
					internal_texture_format, MIPwidth, MIPheight, 0,
					original_texture_format, GL_UNSIGNED_BYTE, resampled );
				check_for_GL_errors( "glTexImage2D" );
			}
			/*	prep for the next level	*/
			++MIPlevel;
			MIPwidth = (MIPwidth + 1) / 2;
			MIPheight = (MIPheight + 1) / 2;
		}

		SOIL_free_image_data( resampled );
	}
}

unsigned int
	SOIL_internal_create_OGL_texture
	(
		const unsigned char *const data,
		int *width, int *height, int channels,
		unsigned int reuse_texture_ID,
		unsigned int flags,
		unsigned int opengl_texture_type,
		unsigned int opengl_texture_target,
		unsigned int texture_check_size_enum
	)
{
	/*	variables	*/
	unsigned char* img = NULL;
	unsigned int tex_id;
	unsigned int internal_texture_format = 0, original_texture_format = 0;
	int DXT_mode = SOIL_CAPABILITY_UNKNOWN;
	int max_supported_size;
	int iwidth = *width;
	int iheight = *height;
	int needCopy;
	GLint unpack_aligment;

	/*	how large of a texture can this OpenGL implementation handle?	*/
	/*	texture_check_size_enum will be GL_MAX_TEXTURE_SIZE or SOIL_MAX_CUBE_MAP_TEXTURE_SIZE	*/
	glGetIntegerv( texture_check_size_enum, &max_supported_size );

	/*	If the user wants to use the texture rectangle I kill a few flags	*/
	if( flags & SOIL_FLAG_TEXTURE_RECTANGLE )
	{
		/*	well, the user asked for it, can we do that?	*/
		if( query_tex_rectangle_capability() == SOIL_CAPABILITY_PRESENT )
		{
			/*	only allow this if the user in _NOT_ trying to do a cubemap!	*/
			if( opengl_texture_type == GL_TEXTURE_2D )
			{
				/*	clean out the flags that cannot be used with texture rectangles	*/
				flags &= ~(
						SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_MIPMAPS |
						SOIL_FLAG_TEXTURE_REPEATS
					);
				/*	and change my target	*/
				opengl_texture_target = SOIL_TEXTURE_RECTANGLE_ARB;
				opengl_texture_type = SOIL_TEXTURE_RECTANGLE_ARB;
			} else
			{
				/*	not allowed for any other uses (yes, I'm looking at you, cubemaps!)	*/
				flags &= ~SOIL_FLAG_TEXTURE_RECTANGLE;
			}

		} else
		{
			/*	can't do it, and that is a breakable offense (uv coords use pixels instead of [0,1]!)	*/
			result_string_pointer = "Texture Rectangle extension unsupported";
			return 0;
		}
	}

	/*	if the user can't support NPOT textures, make sure we force the POT option	*/
	if( (query_NPOT_capability() == SOIL_CAPABILITY_NONE) &&
		!(flags & SOIL_FLAG_TEXTURE_RECTANGLE) )
	{
		/*	add in the POT flag */
		flags |= SOIL_FLAG_POWER_OF_TWO;
	}

	needCopy = ( ( flags & SOIL_FLAG_INVERT_Y ) ||
				 ( flags & SOIL_FLAG_NTSC_SAFE_RGB ) ||
				 ( flags & SOIL_FLAG_MULTIPLY_ALPHA ) ||
				 ( flags & SOIL_FLAG_CoCg_Y )
				);

	/*	create a copy the image data only if needed */
	if ( needCopy ) {
		img = (unsigned char*)malloc( iwidth*iheight*channels );
		memcpy( img, data, iwidth*iheight*channels );
	}

	/*	does the user want me to invert the image?	*/
	if( flags & SOIL_FLAG_INVERT_Y )
	{
		int i, j;
		for( j = 0; j*2 < iheight; ++j )
		{
			int index1 = j * iwidth * channels;
			int index2 = (iheight - 1 - j) * iwidth * channels;
			for( i = iwidth * channels; i > 0; --i )
			{
				unsigned char temp = img[index1];
				img[index1] = img[index2];
				img[index2] = temp;
				++index1;
				++index2;
			}
		}
	}
	/*	does the user want me to scale the colors into the NTSC safe RGB range?	*/
	if( flags & SOIL_FLAG_NTSC_SAFE_RGB )
	{
		scale_image_RGB_to_NTSC_safe( img, iwidth, iheight, channels );
	}
	/*	does the user want me to convert from straight to pre-multiplied alpha?
		(and do we even _have_ alpha?)	*/
	if( flags & SOIL_FLAG_MULTIPLY_ALPHA )
	{
		int i;
		switch( channels )
		{
		case 2:
			for( i = 0; i < 2*iwidth*iheight; i += 2 )
			{
				img[i] = (img[i] * img[i+1] + 128) >> 8;
			}
			break;
		case 4:
			for( i = 0; i < 4*iwidth*iheight; i += 4 )
			{
				img[i+0] = (img[i+0] * img[i+3] + 128) >> 8;
				img[i+1] = (img[i+1] * img[i+3] + 128) >> 8;
				img[i+2] = (img[i+2] * img[i+3] + 128) >> 8;
			}
			break;
		default:
			/*	no other number of channels contains alpha data	*/
			break;
		}
	}

	/*	do I need to make it a power of 2?	*/
	if(
		( ( flags & SOIL_FLAG_POWER_OF_TWO) && ( !SOIL_IS_POW2(iwidth) || !SOIL_IS_POW2(iheight) ) ) ||	/*	user asked for it and the texture is not power of 2	*/
		( (flags & SOIL_FLAG_MIPMAPS)&& !( ( flags & SOIL_FLAG_GL_MIPMAPS ) &&
										   query_gen_mipmap_capability() == SOIL_CAPABILITY_PRESENT &&
										   query_NPOT_capability() == SOIL_CAPABILITY_PRESENT ) ) ||	/*	need it for the MIP-maps when mipmaps required
																											and not GL mipmaps required and supported	*/
		(iwidth > max_supported_size) ||		/*	it's too big, (make sure it's	*/
		(iheight > max_supported_size) )		/*	2^n for later down-sampling)	*/
	{
		int new_width = 1;
		int new_height = 1;
		while( new_width < iwidth )
		{
			new_width *= 2;
		}
		while( new_height < iheight )
		{
			new_height *= 2;
		}
		/*	still?	*/
		if( (new_width != iwidth) || (new_height != iheight) )
		{
			/*	yep, resize	*/
			unsigned char *resampled = (unsigned char*)malloc( channels*new_width*new_height );
			up_scale_image(
					NULL != img ? img : data, iwidth, iheight, channels,
					resampled, new_width, new_height );

			/*	nuke the old guy ( if a copy exists ), then point it at the new guy	*/
			SOIL_free_image_data( img );
			img = resampled;
			*width = new_width;
			*height = new_height;
			iwidth = new_width;
			iheight = new_height;
		}
	}
	/*	now, if it is too large...	*/
	if( (iwidth > max_supported_size) || (iheight > max_supported_size) )
	{
		/*	I've already made it a power of two, so simply use the MIPmapping
			code to reduce its size to the allowable maximum.	*/
		unsigned char *resampled;
		int reduce_block_x = 1, reduce_block_y = 1;
		int new_width, new_height;
		if( iwidth > max_supported_size )
		{
			reduce_block_x = iwidth / max_supported_size;
		}
		if( iheight > max_supported_size )
		{
			reduce_block_y = iheight / max_supported_size;
		}
		new_width = iwidth / reduce_block_x;
		new_height = iheight / reduce_block_y;
		resampled = (unsigned char*)malloc( channels*new_width*new_height );
		/*	perform the actual reduction	*/
		mipmap_image( NULL != img ? img : data, iwidth, iheight, channels,
						resampled, reduce_block_x, reduce_block_y );
		/*	nuke the old guy, then point it at the new guy	*/
		SOIL_free_image_data( img );
		img = resampled;
		*width = new_width;
		*height = new_height;
		iwidth = new_width;
		iheight = new_height;
	}
	/*	does the user want us to use YCoCg color space?	*/
	if( flags & SOIL_FLAG_CoCg_Y )
	{
		/*	this will only work with RGB and RGBA images */
		convert_RGB_to_YCoCg( img, iwidth, iheight, channels );
	}
	/*	create the OpenGL texture ID handle
		(note: allowing a forced texture ID lets me reload a texture)	*/
	tex_id = reuse_texture_ID;
	if( tex_id == 0 )
	{
		glGenTextures( 1, &tex_id );
	}
	check_for_GL_errors( "glGenTextures" );
	/* Note: sometimes glGenTextures fails (usually no OpenGL context)	*/
	if( tex_id )
	{
		/*	and what type am I using as the internal texture format?	*/
		switch( channels )
		{
		case 1:
			#if defined( SOIL_X11_PLATFORM ) || defined( SOIL_PLATFORM_WIN32 ) || defined( SOIL_PLATFORM_OSX )
			original_texture_format = isAtLeastGL3() ? GL_RED : GL_LUMINANCE;
			#else
			original_texture_format = GL_LUMINANCE;
			#endif
			break;
		case 2:
			#if defined( SOIL_X11_PLATFORM ) || defined( SOIL_PLATFORM_WIN32 ) || defined( SOIL_PLATFORM_OSX )
			original_texture_format = isAtLeastGL3() ? GL_RG : GL_LUMINANCE_ALPHA;
			#else
			original_texture_format = GL_LUMINANCE_ALPHA;
			#endif
			break;
		case 3:
			original_texture_format = GL_RGB;
			break;
		case 4:
			original_texture_format = GL_RGBA;
			break;
		}
		internal_texture_format = original_texture_format;
		/*	does the user want me to, and can I, save as DXT?	*/
		if( flags & SOIL_FLAG_COMPRESS_TO_DXT )
		{
			DXT_mode = query_DXT_capability();
			if( DXT_mode == SOIL_CAPABILITY_PRESENT )
			{
				/*	I can use DXT, whether I compress it or OpenGL does	*/
				if( (channels & 1) == 1 )
				{
					/*	1 or 3 channels = DXT1	*/
					internal_texture_format = SOIL_RGB_S3TC_DXT1;
				} else
				{
					/*	2 or 4 channels = DXT5	*/
					internal_texture_format = SOIL_RGBA_S3TC_DXT5;
				}
			}
		}
		/*  bind an OpenGL texture ID	*/
		glBindTexture( opengl_texture_type, tex_id );
		check_for_GL_errors( "glBindTexture" );

		/* set the unpack aligment */
		glGetIntegerv(GL_UNPACK_ALIGNMENT, &unpack_aligment);
		if ( 1 != unpack_aligment )
		{
			glPixelStorei(GL_UNPACK_ALIGNMENT,1);
		}

		/*  upload the main image	*/
		if( DXT_mode == SOIL_CAPABILITY_PRESENT )
		{
			/*	user wants me to do the DXT conversion!	*/
			int DDS_size;
			unsigned char *DDS_data = NULL;
			if( (channels & 1) == 1 )
			{
				/*	RGB, use DXT1	*/
				DDS_data = convert_image_to_DXT1( NULL != img ? img : data, iwidth, iheight, channels, &DDS_size );
			} else
			{
				/*	RGBA, use DXT5	*/
				DDS_data = convert_image_to_DXT5( NULL != img ? img : data, iwidth, iheight, channels, &DDS_size );
			}
			if( DDS_data )
			{
				soilGlCompressedTexImage2D(
					opengl_texture_target, 0,
					internal_texture_format, iwidth, iheight, 0,
					DDS_size, DDS_data );
				check_for_GL_errors( "glCompressedTexImage2D" );
				SOIL_free_image_data( DDS_data );
				/*	printf( "Internal DXT compressor\n" );	*/
			} else
			{
				/*	my compression failed, try the OpenGL driver's version	*/
				glTexImage2D(
					opengl_texture_target, 0,
					internal_texture_format, iwidth, iheight, 0,
					original_texture_format, GL_UNSIGNED_BYTE, NULL != img ? img : data );
				check_for_GL_errors( "glTexImage2D" );
				/*	printf( "OpenGL DXT compressor\n" );	*/
			}
		} else
		{
			/*	user want OpenGL to do all the work!	*/
			glTexImage2D(
				opengl_texture_target, 0,
				internal_texture_format, iwidth, iheight, 0,
				original_texture_format, GL_UNSIGNED_BYTE, NULL != img ? img : data );

			check_for_GL_errors( "glTexImage2D" );
			/*printf( "OpenGL DXT compressor\n" );	*/
		}

		/*	are any MIPmaps desired?	*/
		if( flags & SOIL_FLAG_MIPMAPS || flags & SOIL_FLAG_GL_MIPMAPS )
		{
			createMipmaps( NULL != img ? img : data, iwidth, iheight, channels, flags, opengl_texture_target, internal_texture_format, original_texture_format, DXT_mode );

			/*	instruct OpenGL to use the MIPmaps	*/
			glTexParameteri( opengl_texture_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTexParameteri( opengl_texture_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
			check_for_GL_errors( "GL_TEXTURE_MIN/MAG_FILTER" );
		} else
		{
			/*	instruct OpenGL _NOT_ to use the MIPmaps	*/
			glTexParameteri( opengl_texture_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTexParameteri( opengl_texture_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			check_for_GL_errors( "GL_TEXTURE_MIN/MAG_FILTER" );
		}

		/* recover the unpack aligment */
		if ( 1 != unpack_aligment )
		{
			glPixelStorei(GL_UNPACK_ALIGNMENT, unpack_aligment);
		}

		/*	does the user want clamping, or wrapping?	*/
		if( flags & SOIL_FLAG_TEXTURE_REPEATS )
		{
			glTexParameteri( opengl_texture_type, GL_TEXTURE_WRAP_S, GL_REPEAT );
			glTexParameteri( opengl_texture_type, GL_TEXTURE_WRAP_T, GL_REPEAT );
			if( opengl_texture_type == SOIL_TEXTURE_CUBE_MAP )
			{
				/*	SOIL_TEXTURE_WRAP_R is invalid if cubemaps aren't supported	*/
				glTexParameteri( opengl_texture_type, SOIL_TEXTURE_WRAP_R, GL_REPEAT );
			}
			check_for_GL_errors( "GL_TEXTURE_WRAP_*" );
		} else
		{
			unsigned int clamp_mode = SOIL_CLAMP_TO_EDGE;
			/* unsigned int clamp_mode = GL_CLAMP; */
			glTexParameteri( opengl_texture_type, GL_TEXTURE_WRAP_S, clamp_mode );
			glTexParameteri( opengl_texture_type, GL_TEXTURE_WRAP_T, clamp_mode );
			if( opengl_texture_type == SOIL_TEXTURE_CUBE_MAP )
			{
				/*	SOIL_TEXTURE_WRAP_R is invalid if cubemaps aren't supported	*/
				glTexParameteri( opengl_texture_type, SOIL_TEXTURE_WRAP_R, clamp_mode );
			}
			check_for_GL_errors( "GL_TEXTURE_WRAP_*" );
		}
		/*	done	*/
		result_string_pointer = "Image loaded as an OpenGL texture";
	} else
	{
		/*	failed	*/
		result_string_pointer = "Failed to generate an OpenGL texture name; missing OpenGL context?";
	}

	SOIL_free_image_data( img );

	return tex_id;
}

int
	SOIL_save_screenshot
	(
		const char *filename,
		int image_type,
		int x, int y,
		int width, int height
	)
{
	unsigned char *pixel_data;
	int i, j;
	int save_result;

	/*	error checks	*/
	if( (width < 1) || (height < 1) )
	{
		result_string_pointer = "Invalid screenshot dimensions";
		return 0;
	}
	if( (x < 0) || (y < 0) )
	{
		result_string_pointer = "Invalid screenshot location";
		return 0;
	}
	if( filename == NULL )
	{
		result_string_pointer = "Invalid screenshot filename";
		return 0;
	}

	/*  Get the data from OpenGL	*/
	pixel_data = (unsigned char*)malloc( 3*width*height );
	glReadPixels (x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixel_data);

	/*	invert the image	*/
	for( j = 0; j*2 < height; ++j )
	{
		int index1 = j * width * 3;
		int index2 = (height - 1 - j) * width * 3;
		for( i = width * 3; i > 0; --i )
		{
			unsigned char temp = pixel_data[index1];
			pixel_data[index1] = pixel_data[index2];
			pixel_data[index2] = temp;
			++index1;
			++index2;
		}
	}

	/*	save the image	*/
	save_result = SOIL_save_image( filename, image_type, width, height, 3, pixel_data);

	/*	And free the memory	*/
	SOIL_free_image_data( pixel_data );
	return save_result;
}

unsigned char*
	SOIL_load_image
	(
		const char *filename,
		int *width, int *height, int *channels,
		int force_channels
	)
{
	unsigned char *result = stbi_load( filename,
			width, height, channels, force_channels );
	if( result == NULL )
	{
		result_string_pointer = stbi_failure_reason();
	} else
	{
		result_string_pointer = "Image loaded";
	}
	return result;
}

unsigned char*
	SOIL_load_image_from_memory
	(
		const unsigned char *const buffer,
		int buffer_length,
		int *width, int *height, int *channels,
		int force_channels
	)
{
	unsigned char *result = stbi_load_from_memory(
				buffer, buffer_length,
				width, height, channels,
				force_channels );
	if( result == NULL )
	{
		result_string_pointer = stbi_failure_reason();
	} else
	{
		result_string_pointer = "Image loaded from memory";
	}
	return result;
}

int
	SOIL_save_image
	(
		const char *filename,
		int image_type,
		int width, int height, int channels,
		const unsigned char *const data
	)
{
	int save_result;

	/*	error check	*/
	if( (width < 1) || (height < 1) ||
		(channels < 1) || (channels > 4) ||
		(data == NULL) ||
		(filename == NULL) )
	{
		return 0;
	}
	if( image_type == SOIL_SAVE_TYPE_BMP )
	{
		save_result = stbi_write_bmp( filename,
				width, height, channels, (void*)data );
	} else
	if( image_type == SOIL_SAVE_TYPE_TGA )
	{
		save_result = stbi_write_tga( filename,
				width, height, channels, (void*)data );
	} else
	if( image_type == SOIL_SAVE_TYPE_DDS )
	{
		save_result = save_image_as_DDS( filename,
				width, height, channels, (const unsigned char *const)data );
	} else
	if( image_type == SOIL_SAVE_TYPE_PNG )
	{
		save_result = stbi_write_png( filename,
				width, height, channels, (const unsigned char *const)data, 0 );
	} else
	{
		save_result = 0;
	}

	if( save_result == 0 )
	{
		result_string_pointer = "Saving the image failed";
	} else
	{
		result_string_pointer = "Image saved";
	}
	return save_result;
}

void
	SOIL_free_image_data
	(
		unsigned char *img_data
	)
{
	if ( img_data )
		free( (void*)img_data );
}

const char*
	SOIL_last_result
	(
		void
	)
{
	return result_string_pointer;
}

unsigned int SOIL_direct_load_DDS_from_memory(
		const unsigned char *const buffer,
		int buffer_length,
		unsigned int reuse_texture_ID,
		int flags,
		int loading_as_cubemap )
{
	/*	variables	*/
	DDS_header header;
	unsigned int buffer_index = 0;
	unsigned int tex_ID = 0;
	/*	file reading variables	*/
	unsigned int S3TC_type = 0;
	unsigned char *DDS_data;
	unsigned int DDS_main_size;
	unsigned int DDS_full_size;
	unsigned int width, height;
	int mipmaps, cubemap, uncompressed, block_size = 16;
	unsigned int flag;
	unsigned int cf_target, ogl_target_start, ogl_target_end;
	unsigned int opengl_texture_type;
	int i;
	/*	1st off, does the filename even exist?	*/
	if( NULL == buffer )
	{
		/*	we can't do it!	*/
		result_string_pointer = "NULL buffer";
		return 0;
	}
	if( buffer_length < sizeof( DDS_header ) )
	{
		/*	we can't do it!	*/
		result_string_pointer = "DDS file was too small to contain the DDS header";
		return 0;
	}
	/*	try reading in the header	*/
	memcpy ( (void*)(&header), (const void *)buffer, sizeof( DDS_header ) );
	buffer_index = sizeof( DDS_header );
	/*	guilty until proven innocent	*/
	result_string_pointer = "Failed to read a known DDS header";
	/*	validate the header (warning, "goto"'s ahead, shield your eyes!!)	*/
	flag = ('D'<<0)|('D'<<8)|('S'<<16)|(' '<<24);
	if( header.dwMagic != flag ) {goto quick_exit;}
	if( header.dwSize != 124 ) {goto quick_exit;}
	/*	I need all of these	*/
	flag = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
	if( (header.dwFlags & flag) != flag ) {goto quick_exit;}
	/*	According to the MSDN spec, the dwFlags should contain
		DDSD_LINEARSIZE if it's compressed, or DDSD_PITCH if
		uncompressed.  Some DDS writers do not conform to the
		spec, so I need to make my reader more tolerant	*/
	/*	I need one of these	*/
	flag = DDPF_FOURCC | DDPF_RGB;
	if( (header.sPixelFormat.dwFlags & flag) == 0 ) {goto quick_exit;}
	if( header.sPixelFormat.dwSize != 32 ) {goto quick_exit;}
	if( (header.sCaps.dwCaps1 & DDSCAPS_TEXTURE) == 0 ) {goto quick_exit;}
	/*	make sure it is a type we can upload	*/
	if( (header.sPixelFormat.dwFlags & DDPF_FOURCC) &&
		!(
		(header.sPixelFormat.dwFourCC == (('D'<<0)|('X'<<8)|('T'<<16)|('1'<<24))) ||
		(header.sPixelFormat.dwFourCC == (('D'<<0)|('X'<<8)|('T'<<16)|('3'<<24))) ||
		(header.sPixelFormat.dwFourCC == (('D'<<0)|('X'<<8)|('T'<<16)|('5'<<24)))
		) )
	{
		goto quick_exit;
	}
	/*	OK, validated the header, let's load the image data	*/
	result_string_pointer = "DDS header loaded and validated";
	width = header.dwWidth;
	height = header.dwHeight;
	uncompressed = 1 - (header.sPixelFormat.dwFlags & DDPF_FOURCC) / DDPF_FOURCC;
	cubemap = (header.sCaps.dwCaps2 & DDSCAPS2_CUBEMAP) / DDSCAPS2_CUBEMAP;
	if( uncompressed )
	{
		S3TC_type = GL_RGB;
		block_size = 3;
		if( header.sPixelFormat.dwFlags & DDPF_ALPHAPIXELS )
		{
			S3TC_type = GL_RGBA;
			block_size = 4;
		}
		DDS_main_size = width * height * block_size;
	} else
	{
		/*	can we even handle direct uploading to OpenGL DXT compressed images?	*/
		if( query_DXT_capability() != SOIL_CAPABILITY_PRESENT )
		{
			/*	we can't do it!	*/
			result_string_pointer = "Direct upload of S3TC images not supported by the OpenGL driver";
			return 0;
		}
		/*	well, we know it is DXT1/3/5, because we checked above	*/
		switch( (header.sPixelFormat.dwFourCC >> 24) - '0' )
		{
		case 1:
			S3TC_type = SOIL_RGBA_S3TC_DXT1;
			block_size = 8;
			break;
		case 3:
			S3TC_type = SOIL_RGBA_S3TC_DXT3;
			block_size = 16;
			break;
		case 5:
			S3TC_type = SOIL_RGBA_S3TC_DXT5;
			block_size = 16;
			break;
		}
		DDS_main_size = ((width+3)>>2)*((height+3)>>2)*block_size;
	}
	if( cubemap )
	{
		/* does the user want a cubemap?	*/
		if( !loading_as_cubemap )
		{
			/*	we can't do it!	*/
			result_string_pointer = "DDS image was a cubemap";
			return 0;
		}
		/*	can we even handle cubemaps with the OpenGL driver?	*/
		if( query_cubemap_capability() != SOIL_CAPABILITY_PRESENT )
		{
			/*	we can't do it!	*/
			result_string_pointer = "Direct upload of cubemap images not supported by the OpenGL driver";
			return 0;
		}
		ogl_target_start = SOIL_TEXTURE_CUBE_MAP_POSITIVE_X;
		ogl_target_end =   SOIL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
		opengl_texture_type = SOIL_TEXTURE_CUBE_MAP;
	} else
	{
		/* does the user want a non-cubemap?	*/
		if( loading_as_cubemap )
		{
			/*	we can't do it!	*/
			result_string_pointer = "DDS image was not a cubemap";
			return 0;
		}
		ogl_target_start = GL_TEXTURE_2D;
		ogl_target_end =   GL_TEXTURE_2D;
		opengl_texture_type = GL_TEXTURE_2D;
	}
	if( (header.sCaps.dwCaps1 & DDSCAPS_MIPMAP) && (header.dwMipMapCount > 1) )
	{
		int shift_offset;
		mipmaps = header.dwMipMapCount - 1;
		DDS_full_size = DDS_main_size;
		if( uncompressed )
		{
			/*	uncompressed DDS, simple MIPmap size calculation	*/
			shift_offset = 0;
		} else
		{
			/*	compressed DDS, MIPmap size calculation is block based	*/
			shift_offset = 2;
		}
		for( i = 1; i <= mipmaps; ++ i )
		{
			int w, h;
			w = width >> (shift_offset + i);
			h = height >> (shift_offset + i);
			if( w < 1 )
			{
				w = 1;
			}
			if( h < 1 )
			{
				h = 1;
			}
			DDS_full_size += w*h*block_size;
		}
	} else
	{
		mipmaps = 0;
		DDS_full_size = DDS_main_size;
	}
	DDS_data = (unsigned char*)malloc( DDS_full_size );
	/*	got the image data RAM, create or use an existing OpenGL texture handle	*/
	tex_ID = reuse_texture_ID;
	if( tex_ID == 0 )
	{
		glGenTextures( 1, &tex_ID );
	}
	/*  bind an OpenGL texture ID	*/
	glBindTexture( opengl_texture_type, tex_ID );
	/*	do this for each face of the cubemap!	*/
	for( cf_target = ogl_target_start; cf_target <= ogl_target_end; ++cf_target )
	{
		if( buffer_index + DDS_full_size <= (unsigned int)buffer_length )
		{
			unsigned int byte_offset = DDS_main_size;
			memcpy( (void*)DDS_data, (const void*)(&buffer[buffer_index]), DDS_full_size );
			buffer_index += DDS_full_size;
			/*	upload the main chunk	*/
			if( uncompressed )
			{
				/*	and remember, DXT uncompressed uses BGR(A),
					so swap to RGB(A) for ALL MIPmap levels	*/
				for( i = 0; i < (int)DDS_full_size; i += block_size )
				{
					unsigned char temp = DDS_data[i];
					DDS_data[i] = DDS_data[i+2];
					DDS_data[i+2] = temp;
				}
				glTexImage2D(
					cf_target, 0,
					S3TC_type, width, height, 0,
					S3TC_type, GL_UNSIGNED_BYTE, DDS_data );
			} else
			{
				soilGlCompressedTexImage2D(
					cf_target, 0,
					S3TC_type, width, height, 0,
					DDS_main_size, DDS_data );
			}
			/*	upload the mipmaps, if we have them	*/
			for( i = 1; i <= mipmaps; ++i )
			{
				int w, h, mip_size;
				w = width >> i;
				h = height >> i;
				if( w < 1 )
				{
					w = 1;
				}
				if( h < 1 )
				{
					h = 1;
				}
				/*	upload this mipmap	*/
				if( uncompressed )
				{
					mip_size = w*h*block_size;
					glTexImage2D(
						cf_target, i,
						S3TC_type, w, h, 0,
						S3TC_type, GL_UNSIGNED_BYTE, &DDS_data[byte_offset] );
				} else
				{
					mip_size = ((w+3)/4)*((h+3)/4)*block_size;
					soilGlCompressedTexImage2D(
						cf_target, i,
						S3TC_type, w, h, 0,
						mip_size, &DDS_data[byte_offset] );
				}
				/*	and move to the next mipmap	*/
				byte_offset += mip_size;
			}
			/*	it worked!	*/
			result_string_pointer = "DDS file loaded";
		} else
		{
			glDeleteTextures( 1, & tex_ID );
			tex_ID = 0;
			cf_target = ogl_target_end + 1;
			result_string_pointer = "DDS file was too small for expected image data";
		}
	}/* end reading each face */
	SOIL_free_image_data( DDS_data );
	if( tex_ID )
	{
		/*	did I have MIPmaps?	*/
		if( mipmaps > 0 )
		{
			/*	instruct OpenGL to use the MIPmaps	*/
			glTexParameteri( opengl_texture_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTexParameteri( opengl_texture_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		} else
		{
			/*	instruct OpenGL _NOT_ to use the MIPmaps	*/
			glTexParameteri( opengl_texture_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTexParameteri( opengl_texture_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		}
		/*	does the user want clamping, or wrapping?	*/
		if( flags & SOIL_FLAG_TEXTURE_REPEATS )
		{
			glTexParameteri( opengl_texture_type, GL_TEXTURE_WRAP_S, GL_REPEAT );
			glTexParameteri( opengl_texture_type, GL_TEXTURE_WRAP_T, GL_REPEAT );
			glTexParameteri( opengl_texture_type, SOIL_TEXTURE_WRAP_R, GL_REPEAT );
		} else
		{
			unsigned int clamp_mode = SOIL_CLAMP_TO_EDGE;
			/* unsigned int clamp_mode = GL_CLAMP; */
			glTexParameteri( opengl_texture_type, GL_TEXTURE_WRAP_S, clamp_mode );
			glTexParameteri( opengl_texture_type, GL_TEXTURE_WRAP_T, clamp_mode );
			glTexParameteri( opengl_texture_type, SOIL_TEXTURE_WRAP_R, clamp_mode );
		}
	}

quick_exit:
	/*	report success or failure	*/
	return tex_ID;
}

unsigned int SOIL_direct_load_DDS(
		const char *filename,
		unsigned int reuse_texture_ID,
		int flags,
		int loading_as_cubemap )
{
	FILE *f;
	unsigned char *buffer;
	size_t buffer_length, bytes_read;
	unsigned int tex_ID = 0;
	/*	error checks	*/
	if( NULL == filename )
	{
		result_string_pointer = "NULL filename";
		return 0;
	}
	f = fopen( filename, "rb" );
	if( NULL == f )
	{
		/*	the file doesn't seem to exist (or be open-able)	*/
		result_string_pointer = "Can not find DDS file";
		return 0;
	}
	fseek( f, 0, SEEK_END );
	buffer_length = ftell( f );
	fseek( f, 0, SEEK_SET );
	buffer = (unsigned char *) malloc( buffer_length );
	if( NULL == buffer )
	{
		result_string_pointer = "malloc failed";
		fclose( f );
		return 0;
	}
	bytes_read = fread( (void*)buffer, 1, buffer_length, f );
	fclose( f );
	if( bytes_read < buffer_length )
	{
		/*	huh?	*/
		buffer_length = bytes_read;
	}
	/*	now try to do the loading	*/
	tex_ID = SOIL_direct_load_DDS_from_memory(
		(const unsigned char *const)buffer, (int)buffer_length,
		reuse_texture_ID, flags, loading_as_cubemap );
	SOIL_free_image_data( buffer );
	return tex_ID;
}

unsigned int SOIL_direct_load_PVR_from_memory(
		const unsigned char *const buffer,
		int buffer_length,
		unsigned int reuse_texture_ID,
		int flags,
		int loading_as_cubemap )
{
	PVR_Texture_Header* header = (PVR_Texture_Header*)buffer;
	int num_surfs = 1;
	GLuint tex_ID = 0;
	GLenum PVR_format = 0;
	GLenum PVR_type = GL_RGB;
	unsigned int opengl_texture_type = loading_as_cubemap ? SOIL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
	int is_PVRTC_supported = query_PVR_capability() == SOIL_CAPABILITY_PRESENT;
	int is_BGRA8888_supported  = query_BGRA8888_capability() == SOIL_CAPABILITY_PRESENT;
	int is_compressed_format_supported = 0;
	int is_compressed_format = 0;
	int mipmaps = 0;
	int i;
	GLint unpack_aligment;

	// Check the header size
	if ( header->dwHeaderSize != sizeof(PVR_Texture_Header) ) {
		if ( header->dwHeaderSize == PVRTEX_V1_HEADER_SIZE ) {
			result_string_pointer = "this is an old pvr ( update the PVR file )";

			if ( loading_as_cubemap ) {
				if( header->dwpfFlags & PVRTEX_CUBEMAP ) {
					num_surfs = 6;
				} else {
					result_string_pointer = "tried to load a non-cubemap PVR as cubemap";
					return 0;
				}
			}
		} else {
			result_string_pointer = "invalid PVR header";

			return 0;
		}
	} else {
		if ( loading_as_cubemap ) {
			// Header V2
			if( header->dwNumSurfs < 1 ) {
				if( header->dwpfFlags & PVRTEX_CUBEMAP ) {
					num_surfs = 6;
				} else {
					result_string_pointer = "tried to load a non-cubemap PVR as cubemap";
					return 0;
				}
			} else {
				num_surfs = header->dwNumSurfs;
			}
		}
	}

	// Check the magic identifier
	if ( header->dwPVR != PVRTEX_IDENTIFIER ) {
		result_string_pointer = "invalid PVR header";
		return 0;
	}

	/* Only accept untwiddled data UNLESS texture format is PVRTC */
	if ( ((header->dwpfFlags & PVRTEX_TWIDDLE) == PVRTEX_TWIDDLE)
		&& ((header->dwpfFlags & PVRTEX_PIXELTYPE)!=OGL_PVRTC2)
		&& ((header->dwpfFlags & PVRTEX_PIXELTYPE)!=OGL_PVRTC4) )
	{
		// We need to load untwiddled textures -- hw will twiddle for us.
		result_string_pointer = "pvr is not compressed ( untwiddled texture )";
		return 0;
	}

	switch( header->dwpfFlags & PVRTEX_PIXELTYPE )
	{
		case OGL_RGBA_4444:
			PVR_format = GL_UNSIGNED_SHORT_4_4_4_4;
			PVR_type = GL_RGBA;
			break;
		case OGL_RGBA_5551:
			PVR_format = GL_UNSIGNED_SHORT_5_5_5_1;
			PVR_type = GL_RGBA;
			break;
		case OGL_RGBA_8888:
			PVR_format = GL_UNSIGNED_BYTE;
			PVR_type = GL_RGBA;
			break;
		case OGL_RGB_565:
			PVR_format = GL_UNSIGNED_SHORT_5_6_5;
			PVR_type = GL_RGB;
			break;
		case OGL_RGB_555:
			result_string_pointer = "failed: pixel type OGL_RGB_555 not supported.";
			return 0;
		case OGL_RGB_888:
			PVR_format = GL_UNSIGNED_BYTE;
			PVR_type = GL_RGB;
			break;
		case OGL_I_8:
			PVR_format = GL_UNSIGNED_BYTE;
			PVR_type = GL_LUMINANCE;
			break;
		case OGL_AI_88:
			PVR_format = GL_UNSIGNED_BYTE;
			PVR_type = GL_LUMINANCE_ALPHA;
			break;
		case MGLPT_PVRTC2:
		case OGL_PVRTC2:
			if(is_PVRTC_supported) {
				is_compressed_format_supported = is_compressed_format = 1;
				PVR_format = header->dwAlphaBitMask==0 ? SOIL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG : SOIL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG ;	// PVRTC2
			} else {
				result_string_pointer = "error: PVRTC2 not supported.Decompress the texture first.";
				return 0;
			}
			break;
		case MGLPT_PVRTC4:
		case OGL_PVRTC4:
			if(is_PVRTC_supported) {
				is_compressed_format_supported = is_compressed_format = 1;
				PVR_format = header->dwAlphaBitMask==0 ? SOIL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG : SOIL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG ;	// PVRTC4
			} else {
				result_string_pointer = "error: PVRTC4 not supported. Decompress the texture first.";
				return 0;
			}
			break;
		case OGL_BGRA_8888:
			if(is_BGRA8888_supported) {
				PVR_format = GL_UNSIGNED_BYTE;
				PVR_type   = GL_BGRA;
				break;
			} else {
				result_string_pointer = "Unable to load GL_BGRA texture as extension GL_IMG_texture_format_BGRA8888 is unsupported.";
				return 0;
			}
		default:											// NOT SUPPORTED
			result_string_pointer = "failed: pixel type not supported.";
			return 0;
	}

	#ifdef SOIL_GLES1
	//  check that this data is cube map data or not.
	if( loading_as_cubemap ) {
		result_string_pointer = "cube map textures are not available in GLES1.x.";
		return 0;
	}
	#endif

	// load the texture up
	tex_ID = reuse_texture_ID;
	if( tex_ID == 0 )
	{
		glGenTextures( 1, &tex_ID );
	}

	glBindTexture( opengl_texture_type, tex_ID );

	if( glGetError() ) {
		result_string_pointer = "failed: glBindTexture() failed.";
		return 0;
	}

	glGetIntegerv(GL_UNPACK_ALIGNMENT, &unpack_aligment);
	if ( 1 != unpack_aligment )
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT,1);				// Never have row-aligned in headers
	}

	#define _MAX( a, b ) (( a <= b )? b : a)
	for(i=0; i<num_surfs; i++) {
		char *texture_ptr = (char*)buffer + header->dwHeaderSize + header->dwTextureDataSize * i;
		char *cur_texture_ptr = 0;
		int	mipmap_level;
		unsigned int width= header->dwWidth, height = header->dwHeight;
		unsigned int compressed_image_size = 0;

		mipmaps = ( ( flags & SOIL_FLAG_MIPMAPS ) && (header->dwpfFlags & PVRTEX_MIPMAP) ) ? header->dwMipMapCount : 0;

		for(mipmap_level = 0; mipmap_level <= mipmaps; width = _MAX(width/2, (unsigned int)1), height = _MAX(height/2, (unsigned int)1), mipmap_level++ ) {
			// Do Alpha-swap if needed
			cur_texture_ptr = texture_ptr;

			// Load the Texture
			/* If the texture is PVRTC then use GLCompressedTexImage2D */
			if( is_compressed_format ) {
				/* Calculate how many bytes this MIP level occupies */
				if ((header->dwpfFlags & PVRTEX_PIXELTYPE)==OGL_PVRTC2) {
					compressed_image_size = ( _MAX(width, PVRTC2_MIN_TEXWIDTH) * _MAX(height, PVRTC2_MIN_TEXHEIGHT) * header->dwBitCount + 7 ) / 8;
				} else {// PVRTC4 case
					compressed_image_size = ( _MAX(width, PVRTC4_MIN_TEXWIDTH) * _MAX(height, PVRTC4_MIN_TEXHEIGHT) * header->dwBitCount + 7 ) / 8;
				}

				if ( is_compressed_format_supported ) {
					/* Load compressed texture data at selected MIP level */
					if ( loading_as_cubemap ) {
						soilGlCompressedTexImage2D( SOIL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mipmap_level, PVR_format, width, height, 0, compressed_image_size, cur_texture_ptr );
					} else {
						soilGlCompressedTexImage2D( opengl_texture_type, mipmap_level, PVR_format, width, height, 0, compressed_image_size, cur_texture_ptr );
					}
				} else {
					result_string_pointer = "failed: GPU doesnt support compressed textures";
				}
			} else {
				/* Load uncompressed texture data at selected MIP level */
				if ( loading_as_cubemap ) {
					glTexImage2D( SOIL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mipmap_level, PVR_type, width, height, 0, PVR_type, PVR_format, cur_texture_ptr );
				} else {
					glTexImage2D( opengl_texture_type, mipmap_level, PVR_type, width, height, 0, PVR_type, PVR_format, cur_texture_ptr );
				}
			}

			if( glGetError() ) {
				result_string_pointer = "failed: glCompressedTexImage2D() failed.";
				if ( 1 != unpack_aligment )
				{
					glPixelStorei(GL_UNPACK_ALIGNMENT, unpack_aligment);
				}
				return 0;
			}

			// offset the texture pointer by one mip-map level
			/* PVRTC case */
			if ( is_compressed_format ) {
				texture_ptr += compressed_image_size;
			} else {
				/* New formula that takes into account bit counts inferior to 8 (e.g. 1 bpp) */
				texture_ptr += (width * height * header->dwBitCount + 7) / 8;
			}
		}
	}
	#undef _MAX

	if ( 1 != unpack_aligment )
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, unpack_aligment);
	}

	if( tex_ID )
	{
		/*	did I have MIPmaps?	*/
		if( mipmaps )
		{
			/*	instruct OpenGL to use the MIPmaps	*/
			glTexParameteri( opengl_texture_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTexParameteri( opengl_texture_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		} else
		{
			/*	instruct OpenGL _NOT_ to use the MIPmaps	*/
			glTexParameteri( opengl_texture_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTexParameteri( opengl_texture_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		}

		/*	does the user want clamping, or wrapping?	*/
		if( flags & SOIL_FLAG_TEXTURE_REPEATS )
		{
			glTexParameteri( opengl_texture_type, GL_TEXTURE_WRAP_S, GL_REPEAT );
			glTexParameteri( opengl_texture_type, GL_TEXTURE_WRAP_T, GL_REPEAT );
			glTexParameteri( opengl_texture_type, SOIL_TEXTURE_WRAP_R, GL_REPEAT );
		} else
		{
			unsigned int clamp_mode = SOIL_CLAMP_TO_EDGE;
			/* unsigned int clamp_mode = GL_CLAMP; */
			glTexParameteri( opengl_texture_type, GL_TEXTURE_WRAP_S, clamp_mode );
			glTexParameteri( opengl_texture_type, GL_TEXTURE_WRAP_T, clamp_mode );
			glTexParameteri( opengl_texture_type, SOIL_TEXTURE_WRAP_R, clamp_mode );
		}
	}

	return tex_ID;
}

unsigned int SOIL_direct_load_PVR(
		const char *filename,
		unsigned int reuse_texture_ID,
		int flags,
		int loading_as_cubemap )
{
	FILE *f;
	unsigned char *buffer;
	size_t buffer_length, bytes_read;
	unsigned int tex_ID = 0;
	/*	error checks	*/
	if( NULL == filename )
	{
		result_string_pointer = "NULL filename";
		return 0;
	}
	f = fopen( filename, "rb" );
	if( NULL == f )
	{
		/*	the file doesn't seem to exist (or be open-able)	*/
		result_string_pointer = "Can not find PVR file";
		return 0;
	}
	fseek( f, 0, SEEK_END );
	buffer_length = ftell( f );
	fseek( f, 0, SEEK_SET );
	buffer = (unsigned char *) malloc( buffer_length );
	if( NULL == buffer )
	{
		result_string_pointer = "malloc failed";
		fclose( f );
		return 0;
	}
	bytes_read = fread( (void*)buffer, 1, buffer_length, f );
	fclose( f );
	if( bytes_read < buffer_length )
	{
		/*	huh?	*/
		buffer_length = bytes_read;
	}
	/*	now try to do the loading	*/
	tex_ID = SOIL_direct_load_PVR_from_memory(
		(const unsigned char *const)buffer, (int)buffer_length,
		reuse_texture_ID, flags, loading_as_cubemap );
	SOIL_free_image_data( buffer );
	return tex_ID;
}

unsigned int SOIL_direct_load_ETC1_from_memory(
		const unsigned char *const buffer,
		int buffer_length,
		unsigned int reuse_texture_ID,
		int flags )
{
	GLuint tex_ID = 0;
	PKMHeader* header = (PKMHeader*)buffer;
	unsigned int opengl_texture_type = GL_TEXTURE_2D;
	unsigned int width;
	unsigned int height;
	unsigned long compressed_image_size = buffer_length - PKM_HEADER_SIZE;
	char *texture_ptr = (char*)buffer + PKM_HEADER_SIZE;
	GLint unpack_aligment;

	if ( query_ETC1_capability() != SOIL_CAPABILITY_PRESENT ) {
		result_string_pointer = "error: ETC1 not supported. Decompress the texture first.";
		return 0;
	}

	if ( 0 != strcmp( header->aName, "PKM 10" ) ) {
		result_string_pointer = "error: PKM 10 header not found.";
		return 0;
	}

	width = (header->iWidthMSB << 8) | header->iWidthLSB;
	height = (header->iHeightMSB << 8) | header->iHeightLSB;
	compressed_image_size = (((width + 3) & ~3) * ((height + 3) & ~3)) >> 1;

	// load the texture up
	tex_ID = reuse_texture_ID;
	if( tex_ID == 0 )
	{
		glGenTextures( 1, &tex_ID );
	}

	glBindTexture( opengl_texture_type, tex_ID );

	if( glGetError() ) {
		result_string_pointer = "failed: glBindTexture() failed.";
		return 0;
	}

	glGetIntegerv(GL_UNPACK_ALIGNMENT, &unpack_aligment);
	if ( 1 != unpack_aligment )
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT,1);				// Never have row-aligned in headers
	}

	soilGlCompressedTexImage2D( opengl_texture_type, 0, SOIL_GL_ETC1_RGB8_OES, width, height, 0, compressed_image_size, texture_ptr );

	if( glGetError() ) {
		result_string_pointer = "failed: glCompressedTexImage2D() failed.";

		if ( 1 != unpack_aligment )
		{
			glPixelStorei(GL_UNPACK_ALIGNMENT, unpack_aligment);
		}
		return 0;
	}

	if ( 1 != unpack_aligment )
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, unpack_aligment);
	}

	if( tex_ID )
	{
		/* No MIPmaps for ETC1 */
		glTexParameteri( opengl_texture_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri( opengl_texture_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

		/*	does the user want clamping, or wrapping?	*/
		if( flags & SOIL_FLAG_TEXTURE_REPEATS )
		{
			glTexParameteri( opengl_texture_type, GL_TEXTURE_WRAP_S, GL_REPEAT );
			glTexParameteri( opengl_texture_type, GL_TEXTURE_WRAP_T, GL_REPEAT );
			glTexParameteri( opengl_texture_type, SOIL_TEXTURE_WRAP_R, GL_REPEAT );
		} else
		{
			unsigned int clamp_mode = SOIL_CLAMP_TO_EDGE;
			/* unsigned int clamp_mode = GL_CLAMP; */
			glTexParameteri( opengl_texture_type, GL_TEXTURE_WRAP_S, clamp_mode );
			glTexParameteri( opengl_texture_type, GL_TEXTURE_WRAP_T, clamp_mode );
			glTexParameteri( opengl_texture_type, SOIL_TEXTURE_WRAP_R, clamp_mode );
		}
	}

	return tex_ID;
}

unsigned int SOIL_direct_load_ETC1(const char *filename,
		unsigned int reuse_texture_ID,
		int flags )
{
	FILE *f;
	unsigned char *buffer;
	size_t buffer_length, bytes_read;
	unsigned int tex_ID = 0;
	/*	error checks	*/
	if( NULL == filename )
	{
		result_string_pointer = "NULL filename";
		return 0;
	}
	f = fopen( filename, "rb" );
	if( NULL == f )
	{
		/*	the file doesn't seem to exist (or be open-able)	*/
		result_string_pointer = "Can not find PVR file";
		return 0;
	}
	fseek( f, 0, SEEK_END );
	buffer_length = ftell( f );
	fseek( f, 0, SEEK_SET );
	buffer = (unsigned char *) malloc( buffer_length );
	if( NULL == buffer )
	{
		result_string_pointer = "malloc failed";
		fclose( f );
		return 0;
	}
	bytes_read = fread( (void*)buffer, 1, buffer_length, f );
	fclose( f );
	if( bytes_read < buffer_length )
	{
		/*	huh?	*/
		buffer_length = bytes_read;
	}
	/*	now try to do the loading	*/
	tex_ID = SOIL_direct_load_ETC1_from_memory(
		(const unsigned char *const)buffer, (int)buffer_length,
		reuse_texture_ID, flags );
	SOIL_free_image_data( buffer );
	return tex_ID;
}

int query_NPOT_capability( void )
{
	/*	check for the capability	*/
	if( has_NPOT_capability == SOIL_CAPABILITY_UNKNOWN )
	{
		/*	we haven't yet checked for the capability, do so	*/
		if(
			(0 == SOIL_GL_ExtensionSupported(
				"GL_ARB_texture_non_power_of_two" ) )
		&&
			(0 == SOIL_GL_ExtensionSupported(
				"GL_OES_texture_npot" ) )
			)
		{
			/*	not there, flag the failure	*/
			has_NPOT_capability = SOIL_CAPABILITY_NONE;
		} else
		{
			/*	it's there!	*/
			has_NPOT_capability = SOIL_CAPABILITY_PRESENT;
		}
	}
	/*	let the user know if we can do non-power-of-two textures or not	*/
	return has_NPOT_capability;
}

int query_tex_rectangle_capability( void )
{
	/*	check for the capability	*/
	if( has_tex_rectangle_capability == SOIL_CAPABILITY_UNKNOWN )
	{
		/*	we haven't yet checked for the capability, do so	*/
		if(
			(0 == SOIL_GL_ExtensionSupported(
				"GL_ARB_texture_rectangle" ) )
		&&
			(0 == SOIL_GL_ExtensionSupported(
				"GL_EXT_texture_rectangle" ) )
		&&
			(0 == SOIL_GL_ExtensionSupported(
				"GL_NV_texture_rectangle" ) )
			)
		{
			/*	not there, flag the failure	*/
			has_tex_rectangle_capability = SOIL_CAPABILITY_NONE;
		} else
		{
			/*	it's there!	*/
			has_tex_rectangle_capability = SOIL_CAPABILITY_PRESENT;
		}
	}
	/*	let the user know if we can do texture rectangles or not	*/
	return has_tex_rectangle_capability;
}

int query_cubemap_capability( void )
{
	/*	check for the capability	*/
	if( has_cubemap_capability == SOIL_CAPABILITY_UNKNOWN )
	{
		/*	we haven't yet checked for the capability, do so	*/
		if(
			(0 == SOIL_GL_ExtensionSupported(
				"GL_ARB_texture_cube_map" ) )
		&&
			(0 == SOIL_GL_ExtensionSupported(
				"GL_EXT_texture_cube_map" ) )
			)
		{
			/*	not there, flag the failure	*/
			has_cubemap_capability = SOIL_CAPABILITY_NONE;
		} else
		{
			/*	it's there!	*/
			has_cubemap_capability = SOIL_CAPABILITY_PRESENT;
		}
	}
	/*	let the user know if we can do cubemaps or not	*/
	return has_cubemap_capability;
}

static P_SOIL_GLCOMPRESSEDTEXIMAGE2DPROC get_glCompressedTexImage2D_addr()
{
	/*	and find the address of the extension function	*/
	P_SOIL_GLCOMPRESSEDTEXIMAGE2DPROC ext_addr = NULL;

#if defined( SOIL_PLATFORM_WIN32 ) || defined( SOIL_PLATFORM_OSX ) || defined( SOIL_X11_PLATFORM )
	ext_addr = (P_SOIL_GLCOMPRESSEDTEXIMAGE2DPROC)SOIL_GL_GetProcAddress( "glCompressedTexImage2D" );
#else
	ext_addr = (P_SOIL_GLCOMPRESSEDTEXIMAGE2DPROC)&glCompressedTexImage2D;
#endif

	return ext_addr;
}

int query_DXT_capability( void )
{
	/*	check for the capability	*/
	if( has_DXT_capability == SOIL_CAPABILITY_UNKNOWN )
	{
		/*	we haven't yet checked for the capability, do so	*/
		if (	0 == SOIL_GL_ExtensionSupported(
					"GL_EXT_texture_compression_s3tc" )  &&
				0 == SOIL_GL_ExtensionSupported(
					"WEBGL_compressed_texture_s3tc ") &&
				0 == SOIL_GL_ExtensionSupported(
					"WEBKIT_WEBGL_compressed_texture_s3tc") &&
				0 == SOIL_GL_ExtensionSupported(
					"MOZ_WEBGL_compressed_texture_s3tc"
				)
			)
		{
			/*	not there, flag the failure	*/
			has_DXT_capability = SOIL_CAPABILITY_NONE;
		} else
		{
			P_SOIL_GLCOMPRESSEDTEXIMAGE2DPROC ext_addr = get_glCompressedTexImage2D_addr();

			/*	Flag it so no checks needed later	*/
			if( NULL == ext_addr )
			{
				/*	hmm, not good!!  This should not happen, but does on my
					laptop's VIA chipset.  The GL_EXT_texture_compression_s3tc
					spec requires that ARB_texture_compression be present too.
					this means I can upload and have the OpenGL drive do the
					conversion, but I can't use my own routines or load DDS files
					from disk and upload them directly [8^(	*/
				has_DXT_capability = SOIL_CAPABILITY_NONE;
			} else
			{
				/*	all's well!	*/
				soilGlCompressedTexImage2D = ext_addr;
				has_DXT_capability = SOIL_CAPABILITY_PRESENT;
			}
		}
	}
	/*	let the user know if we can do DXT or not	*/
	return has_DXT_capability;
}

int query_PVR_capability( void )
{
	/*	check for the capability	*/
	if( has_PVR_capability == SOIL_CAPABILITY_UNKNOWN )
	{
		/*	we haven't yet checked for the capability, do so	*/
		if (0 == SOIL_GL_ExtensionSupported(
				"GL_IMG_texture_compression_pvrtc" ) )
		{
			/*	not there, flag the failure	*/
			has_PVR_capability = SOIL_CAPABILITY_NONE;
		} else
		{
			if ( NULL == soilGlCompressedTexImage2D ) {
				soilGlCompressedTexImage2D = get_glCompressedTexImage2D_addr();
			}

			/*	it's there!	*/
			has_PVR_capability = SOIL_CAPABILITY_PRESENT;
		}
	}
	/*	let the user know if we can do cubemaps or not	*/
	return has_PVR_capability;
}

int query_BGRA8888_capability( void )
{
	/*	check for the capability	*/
	if( has_BGRA8888_capability == SOIL_CAPABILITY_UNKNOWN )
	{
		/*	we haven't yet checked for the capability, do so	*/
		if (0 == SOIL_GL_ExtensionSupported(
				"GL_IMG_texture_format_BGRA8888" ) )
		{
			/*	not there, flag the failure	*/
			has_BGRA8888_capability = SOIL_CAPABILITY_NONE;
		} else
		{
			/*	it's there!	*/
			has_BGRA8888_capability = SOIL_CAPABILITY_PRESENT;
		}
	}
	/*	let the user know if we can do cubemaps or not	*/
	return has_BGRA8888_capability;
}

int query_ETC1_capability( void )
{
	/*	check for the capability	*/
	if( has_ETC1_capability == SOIL_CAPABILITY_UNKNOWN )
	{
		/*	we haven't yet checked for the capability, do so	*/
		if (0 == SOIL_GL_ExtensionSupported(
				"GL_OES_compressed_ETC1_RGB8_texture" ) )
		{
			/*	not there, flag the failure	*/
			has_ETC1_capability = SOIL_CAPABILITY_NONE;
		} else
		{
			if ( NULL == soilGlCompressedTexImage2D ) {
				soilGlCompressedTexImage2D = get_glCompressedTexImage2D_addr();
			}

			/*	it's there!	*/
			has_ETC1_capability = SOIL_CAPABILITY_PRESENT;
		}
	}
	/*	let the user know if we can do cubemaps or not	*/
	return has_ETC1_capability;
}

int query_gen_mipmap_capability( void )
{
	/* check for the capability   */
	P_SOIL_GLGENERATEMIPMAPPROC ext_addr = NULL;

	if( has_gen_mipmap_capability == SOIL_CAPABILITY_UNKNOWN )
	{
		if (	0 == SOIL_GL_ExtensionSupported(
					"GL_ARB_framebuffer_object" )
			&&
				0 == SOIL_GL_ExtensionSupported(
					"GL_EXT_framebuffer_object" )
			&&  0 == SOIL_GL_ExtensionSupported(
					"GL_OES_framebuffer_object" )
			)
		{
			/* not there, flag the failure */
			has_gen_mipmap_capability = SOIL_CAPABILITY_NONE;
		}
		else
		{
			#if !defined( SOIL_GLES1 ) && !defined( SOIL_GLES2 )

			ext_addr = (P_SOIL_GLGENERATEMIPMAPPROC)SOIL_GL_GetProcAddress("glGenerateMipmap");

			if(ext_addr == NULL)
			{
				ext_addr = (P_SOIL_GLGENERATEMIPMAPPROC)SOIL_GL_GetProcAddress("glGenerateMipmapEXT");
			}

			#elif defined( SOIL_GLES2 )
				ext_addr = 	&glGenerateMipmap;
			#else /** SOIL_GLES1 */
				ext_addr = &glGenerateMipmapOES;
			#endif
		}

		if(ext_addr == NULL)
		{
			/* this should never happen */
			has_gen_mipmap_capability = SOIL_CAPABILITY_NONE;
		} else
		{
			/* it's there! */
			has_gen_mipmap_capability = SOIL_CAPABILITY_PRESENT;
			soilGlGenerateMipmap = ext_addr;
		}
	}

	return has_gen_mipmap_capability;
}

#endif

#include <stdint.h>

extern "C"
int stbi_write_dds( char const *filename, int w, int h, int comp, const void *data ) {
   return save_image_as_DDS( filename, w, h, comp, (const unsigned char *const) data );
}

extern "C"
int WebPGetInfo(const uint8_t* data, size_t data_size, int* width, int* height);

extern "C"
uint8_t* WebPDecodeRGBA(const uint8_t* data, size_t data_size, int* width, int* height);

extern "C"
size_t WebPEncodeRGBA(const unsigned char* rgba, int width, int height, int stride, float quality_factor, unsigned char** output);

namespace spot
{
	namespace internals {

		int32_t swapbe( int32_t v ) {
			// swap bytes on big endian only; little remains unchanged
			union autodetect {
				int word;
				char byte[ sizeof(int) ];
				autodetect() : word(1)
				{}
			} _;
			bool is_big = _.byte[0] == 0;
			if( is_big ) {
				unsigned char *p = (unsigned char *)&v;
				std::swap( p[0], p[3] );
				std::swap( p[1], p[2] );
			}
			return v;
		}

		std::string encode_png( unsigned w, unsigned h, const void *data, unsigned stride ) {
			if( w && h && data && stride ) {
				auto mode = LCT_RGBA;
				/**/ if( stride == 3 ) mode = LCT_RGB;
				else if( stride == 2 ) mode = LCT_GREY_ALPHA;
				else if( stride == 1 ) mode = LCT_GREY;
				unsigned char* png;
				size_t pngsize;
				unsigned bpp = 8;
				unsigned error = lodepng_encode_memory_std( &png, &pngsize, (const unsigned char *)data, w, h, mode, bpp );
				if( !error && pngsize ) {
					std::string buf;
					buf.resize(pngsize);
					memcpy(&buf[0],png,pngsize);
					free(png);
					return buf;
				}
			}
			return std::string();
		}

		std::string encode_jpg( unsigned w, unsigned h, const void *data, unsigned quality ) {
			if( w && h && data && quality ) {
				std::string buf( 1024 + w * h * 3, '\0' );
				jpge::params p;
				p.m_quality = (int)quality;
				p.m_two_pass_flag = true; // slower but slighty smaller
				int buf_size = (int)buf.size();
				if( jpge::compress_image_to_jpeg_file_in_memory(&buf[0], buf_size, w, h, 4, (const jpge::uint8 *)data, p) ) {
					if( buf_size > 0 ) {
						buf.resize((unsigned)(buf_size));
						return buf;
					}
				}
			}
			return std::string();
		}

		std::string encode_wbp( unsigned w, unsigned h, const void *data, unsigned quality ) {
			if( w && h && data && quality ) {
				unsigned char *buf = 0;
				size_t buflen = WebPEncodeRGBA( (const unsigned char *)data, w, h, w * 4, quality, &buf );
				if( buflen && buf ) {
					std::string s;
					s.resize(buflen);
					memcpy(&s[0],buf,buflen);
					free(buf);
					return s;
				}
			}
			return std::string();
		}

		std::string encode_pug( unsigned w, unsigned h, const void *data, unsigned quality ) {
			if( w && h && data && quality ) {
				// encode color as jpg
				std::string jpg = encode_jpg( w, h, data, quality );
				// encode alpha as png (gray, lum8)
				std::vector<unsigned char> alpha( w * h );
				unsigned char *ptr = ((unsigned char *)data) + 3;
				for( unsigned x = 0; x < w * h; ++x, ptr += 4 ) alpha[ x ] = *ptr;
				std::string png = encode_png( w, h, alpha.data(), 1 );
				// glue and footer
				int32_t size24 = internals::swapbe( int32_t(jpg.size()) );
				int32_t size08 = internals::swapbe( int32_t(png.size()) );
				if( size24 && size08 ) {
					std::stringstream ss;
					ss.write( &jpg[0], size24 );
					ss.write( &png[0], size08 );
					ss.write( (const char *)&size24, 4 );
					ss.write( (const char *)&size08, 4 );
					ss.write( "pug1", 4 );
					if( ss.good() ) {
						return ss.str();
					}
				}
			}
			return std::string();
		}

		bool writefile( const std::string &filename, const std::string &data ) {
			if( !data.empty() ) {
				std::ofstream ofs( filename.c_str(), std::ios::binary );
				ofs.write( &data[0], data.size() );
				return ofs.good();
			}
			return false;
		}
	}

	std::vector<std::string> list_supported_inputs() {
		const char *str[] = { "bmp", "dds", "gif", "hdr", "jpg", "pic", "pkm", "png", "psd", "pvr", "svg", "tga", "webp", "pnm", "pug", 0 };
		std::vector<std::string> list;
		for( int i = 0; str[i]; ++i ) {
			list.push_back( str[i] );
		}
		return list;
	}
	std::vector<std::string> list_supported_outputs() {
		const char *str[] = { "bmp", "dds", "jpg", "png", "tga", "webp", "pug", 0 };
		std::vector<std::string> list;
		for( int i = 0; str[i]; ++i ) {
			list.push_back( str[i] );
		}
		return list;
	}

	std::vector<unsigned char> decode8( const void *ptr, size_t size, size_t *w, size_t *h, std::string *error, bool make_squared, bool mirror_w, bool mirror_h ) {
		std::vector<unsigned char> temp;

		if( !ptr ) {
			if( error ) *error = "Error: invalid pointer provided";
			return std::vector<unsigned char>();
		}

		if( !size ) {
			if( error ) *error = "Error: invalid size provided";
			return std::vector<unsigned char>();
		}

		// decode
		int imageWidth = 0, imageHeight = 0, imageBpp = 0;
		enum { STBI_DELETER, FREE_DELETER, NEW_DELETER, NEW_ARRAY_DELETER, NO_DELETER } deleter = NO_DELETER;
		stbi_uc *imageuc = 0;

		if( !imageuc )
		{
			imageuc = stbi_load_from_memory( (const unsigned char *)ptr, size, &imageWidth, &imageHeight, &imageBpp, 4 );
			deleter = STBI_DELETER;

			if( imageuc ) {
				// decode alpha if it is a .pug file
				const char *magic = (const char *)ptr + size - 4;
				if( magic[0] == 'p' && magic[1] == 'u' && magic[2] == 'g' && magic[3] == '1' ) {
					const int32_t color_size = internals::swapbe( *(const int32_t *)((const char *)ptr + size - 12) );
					const int32_t alpha_size = internals::swapbe( *(const int32_t *)((const char *)ptr + size - 8) );
					int w2 = 0, h2 = 0, bpp2 = 0;
					stbi_uc *alpha = stbi_load_from_memory( (const unsigned char *)ptr + color_size, alpha_size, &w2, &h2, &bpp2, 1 );
					if( alpha ) {
						stbi_uc *src = &alpha[ 0 ], *end = &alpha[ w2*h2 ], *dst = &imageuc[ 3 ];
						while( src < end ) {
							*dst = *src++;
							dst += 4;
						}
						stbi_image_free( alpha );
					}
				}
			}
		}

		if( !imageuc )
		{
			bool ok = 0 != WebPGetInfo( (const uint8_t *)ptr, size, &imageWidth, &imageHeight );
			if( ok ) {
				imageuc = (stbi_uc *) WebPDecodeRGBA( (const uint8_t *)ptr, size, &imageWidth, &imageHeight );
				deleter = FREE_DELETER;
				imageBpp = 4;
			}
		}

		if( !imageuc )
		{
			// Load SVG, parse and rasterize
			char *str = new char[ size + 1 ];
			memcpy( str, ptr, size );
			str[size] = '\0';

			NSVGimage *image = (str[0] == '<' || str[0] == ' ' || str[0] == '\t' ? nsvgParse( str, "px" /*units*/, 96.f /* dpi */ ) : (NSVGimage *)0 );
			if( image ) {
				// Create rasterizer (can be used to render multiple images).
				static struct install {
					NSVGrasterizer *rasterizer;
					 install() { rasterizer = nsvgCreateRasterizer(); }
					~install() { if( rasterizer ) nsvgDeleteRasterizer( rasterizer ); rasterizer = 0; }
				} local;
				// Allocate memory for image
				int w = image->width;
				int h = image->height;

				double scale = 1.0;
				imageWidth = image->width * scale;
				imageHeight = image->height * scale;
				imageBpp = 4;

				imageuc = (stbi_uc *)malloc(w*h*4);
				deleter = FREE_DELETER;
				if( imageuc ) {
					// Rasterizes SVG image, returns RGBA image (non-premultiplied alpha)
					//   r - pointer to rasterizer context
					//   image - pointer to image to rasterize
					//   tx,ty - image offset (applied after scaling)
					//   scale - image scale
					//   dst - pointer to destination image data, 4 bytes per pixel (RGBA)
					//   w - width of the image to render
					//   h - height of the image to render
					//   stride - number of bytes per scaleline in the destination buffer
					nsvgRasterize( local.rasterizer, image, 0,0,scale, imageuc, w, h, w*scale*4 );
				}
				nsvgDelete(image);
			}

			delete [] str;
		}

		if( !imageuc )
		{
			// assert( false );
			// return yellow/black texture instead?
			if( error ) *error = "Error! unable to decode image";
			return std::vector<unsigned char>();
		}

		if( make_squared )
		{
			bool is_power_of_two_w = imageWidth && !(imageWidth & (imageWidth - 1));
			bool is_power_of_two_h = imageHeight && !(imageHeight & (imageHeight - 1));

			if( is_power_of_two_w && is_power_of_two_h )
			{
				temp.resize( imageWidth * imageHeight * 4 );
				memcpy( &temp[0], imageuc, imageWidth * imageHeight * 4 );
			}
			else
			{
				int nw = 1, nh = 1, atx, aty;
				while( nw < imageWidth ) nw <<= 1;
				while( nh < imageHeight ) nh <<= 1;

				// squared as well, cos we want them pixel perfect
				if( nh > nw ) nw = nh; else nh = nw;

				temp.resize( nw * nh * 4, 0 );
				atx = (nw - imageWidth) / 2;
				aty = (nh - imageHeight) / 2;

				//std::cout << wire::string( "\1x\2 -> \3x\4 @(\5,\6)\n", imageWidth, imageHeight, nw, nh, atx, aty);

				for( int y = 0; y < imageHeight; ++y )
					memcpy( &((stbi_uc*)&temp[0])[ ((atx)+(aty+y)*nw)*4 ], &imageuc[ (y*imageWidth) * 4 ], imageWidth * 4 );

				imageWidth = nw;
				imageHeight = nh;
			}
		}
		else
		{
				temp.resize( imageWidth * imageHeight * 4 );
				memcpy( &temp[0], imageuc, imageWidth * imageHeight * 4 );
		}

		/****/ if( deleter == STBI_DELETER ) {
			stbi_image_free( imageuc );
		} else if( deleter == FREE_DELETER ) {
			free(imageuc);
		} else if( deleter == NEW_DELETER ) {
			delete imageuc;
		} else if( deleter == NEW_ARRAY_DELETER ) {
			delete [] (imageuc);
		}
		imageuc = 0;

		if( mirror_w && !mirror_h )
		{
			std::vector<stbi_uc> mirrored( temp.size() );

			for( int c = 0, y = 0; y < imageHeight; ++y )
			for( int x = imageWidth - 1; x >= 0; --x )
				mirrored[ c++ ] = temp[ ( x + y * imageWidth ) * 4 + 0 ],
				mirrored[ c++ ] = temp[ ( x + y * imageWidth ) * 4 + 1 ],
				mirrored[ c++ ] = temp[ ( x + y * imageWidth ) * 4 + 2 ],
				mirrored[ c++ ] = temp[ ( x + y * imageWidth ) * 4 + 3 ];

			temp = mirrored;
		}
		else
		if( mirror_h && !mirror_w )
		{
			std::vector<stbi_uc> mirrored( temp.size() );

			for( int c = 0, y = imageHeight - 1; y >= 0; --y )
			for( int x = 0; x < imageWidth; ++x )
				mirrored[ c++ ] = temp[ ( x + y * imageWidth ) * 4 + 0 ],
				mirrored[ c++ ] = temp[ ( x + y * imageWidth ) * 4 + 1 ],
				mirrored[ c++ ] = temp[ ( x + y * imageWidth ) * 4 + 2 ],
				mirrored[ c++ ] = temp[ ( x + y * imageWidth ) * 4 + 3 ];

			temp = mirrored;
		}
		else
		if( mirror_w && mirror_h )
		{
			std::vector<stbi_uc> mirrored( temp.size() );

			for( int c = 0, y = imageHeight - 1; y >= 0; --y )
			for( int x = imageWidth - 1; x >= 0; --x )
				mirrored[ c++ ] = temp[ ( x + y * imageWidth ) * 4 + 0 ],
				mirrored[ c++ ] = temp[ ( x + y * imageWidth ) * 4 + 1 ],
				mirrored[ c++ ] = temp[ ( x + y * imageWidth ) * 4 + 2 ],
				mirrored[ c++ ] = temp[ ( x + y * imageWidth ) * 4 + 3 ];

			temp = mirrored;

			std::swap( imageWidth, imageHeight );
		}

		if( w ) *w = imageWidth;
		if( h ) *h = imageHeight;

		return temp;
	}

	std::vector<unsigned char> decode8( const std::string &filename, size_t *w, size_t *h, std::string *error, bool make_squared, bool mirror_w, bool mirror_h ) {
		std::ifstream ifs( filename.c_str(), std::ios::binary );
		std::vector<char> buffer( (std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
		return decode8( (const unsigned char *)buffer.data(), buffer.size(), w, h, error, make_squared, mirror_w, mirror_h );
	}

	std::vector<unsigned int> decode32( const void *ptr, size_t size, size_t *w, size_t *h, std::string *error, bool make_squared, bool mirror_w, bool mirror_h ) {
		std::vector<unsigned char> decoded = decode8( ptr, size, w, h, error, make_squared, mirror_w, mirror_h );
		std::vector<unsigned int> out;
		if( !decoded.empty() ) {
			out.reserve( decoded.size() / 4 );
			for( unsigned char *data8 = &decoded[0], *end8 = data8 + decoded.size(); data8 != end8; ) {
				pixel p;
				p.r = *data8++;
				p.g = *data8++;
				p.b = *data8++;
				p.a = *data8++;
				out.push_back( p.rgba );
			}
			return out;
		}
		return out;
	}

	std::vector<unsigned int> decode32( const std::string &filename, size_t *w, size_t *h, std::string *error, bool make_squared, bool mirror_w, bool mirror_h ) {
		std::ifstream ifs( filename.c_str(), std::ios::binary );
		std::vector<char> buffer( (std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
		return decode32( (const unsigned char *)buffer.data(), buffer.size(), w, h, error, make_squared, mirror_w, mirror_h );
	}

	void hsl2rgb( const float *hsl, float *rgb )
	{
		// Given H,S,L in range of 0-1
		// Returns a Color (RGB struct) in range of 0-1

		const float &h = hsl[0];
		const float &s = hsl[1];
		const float &l = hsl[2];

		float &r = rgb[0];
		float &g = rgb[1];
		float &b = rgb[2];

		float v;

		r = l;   // default to gray
		g = l;
		b = l;
		v = (l <= 0.5f) ? (l * (1.f + s)) : (l + s - l * s);

		if (v > 0)
		{
			  float m = l + l - v;
			  float sv = (v - m ) / v;
			  float h6 = h * 6.f;
			  int sextant = (int)h6;
			  float fract = h6 - sextant;
			  float vsf = v * sv * fract;
			  float mid1 = m + vsf;
			  float mid2 = v - vsf;

			  switch (sextant)
			  {
					default:
					case 0:
						  r = v;
						  g = mid1;
						  b = m;
						  break;
					case 1:
						  r = mid2;
						  g = v;
						  b = m;
						  break;
					case 2:
						  r = m;
						  g = v;
						  b = mid1;
						  break;
					case 3:
						  r = m;
						  g = mid2;
						  b = v;
						  break;
					case 4:
						  r = mid1;
						  g = m;
						  b = v;
						  break;
					case 5:
						  r = v;
						  g = m;
						  b = mid2;
						  break;
			  }
		}
	}

	void rgb2hsl( const float *rgb, float *hsl )
	{
		// Given a Color (RGB Struct) in range of 0-1
		// Return H,S,L in range of 0-1

		const float &r = rgb[0];
		const float &g = rgb[1];
		const float &b = rgb[2];
		float v;
		float m;
		float vm;
		float r2, g2, b2;

		float &h = hsl[0];
		float &s = hsl[1];
		float &l = hsl[2];

		h = 0; // default to black
		s = 0;
		l = 0;
		v = ( r > g ? r : g );
		v = ( v > b ? v : b );
		m = ( r < g ? r : g );
		m = ( m < b ? m : b );
		l = (m + v) / 2.f;
		if (l <= 0.f)
		{
			  return;
		}
		vm = v - m;
		s = vm;
		if (s > 0.f)
		{
			  s /= (l <= 0.5f) ? (v + m ) : (2.f - v - m) ;
		}
		else
		{
			  return;
		}
		r2 = (v - r) / vm;
		g2 = (v - g) / vm;
		b2 = (v - b) / vm;
		if (r == v)
		{
			  h = (g == m ? 5.f + b2 : 1.f - g2);
		}
		else if (g == v)
		{
			  h = (b == m ? 1.f + r2 : 3.f - b2);
		}
		else
		{
			  h = (r == m ? 3.f + g2 : 5.f - r2);
		}
		h /= 6.f;
	}

	pixel::operator color() const {
		pixel c = this->clamp().to_hsla();
		return spot::color(c.r / 255.f, c.g / 255.f, c.b / 255.f, c.a / 255.f);
	}
}

