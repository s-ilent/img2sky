#include "main.h"

#include <fstream>
#include <sstream>

#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include <vector>
#include <algorithm>

#include "tri_stripper.h"
#include "mat4_transform.h"
#include "main.h"

#ifndef PI
#define PI 3.1415926535897932384626433832795
#endif



static std::ostream *output_stream;

/////////////////////////////////////////////////////////////////////////////////////

extern double radius;
extern unsigned make_geometry;
static unsigned ply_faces = 0;

static void ply_face(Triangle& T, void *closure)
{
    std::ostream& out = *output_stream;
    xarray2<int>& vert_id = *(xarray2<int> *)closure;

    const Vec2& p1 = T.point1();
    const Vec2& p2 = T.point2();
    const Vec2& p3 = T.point3();

    out << 3 << " ";
    out << vert_id((int)p1[X], (int)p1[Y]) << " ";
    out << vert_id((int)p2[X], (int)p2[Y]) << " ";
    out << vert_id((int)p3[X], (int)p3[Y]) << std::endl;

    ply_faces++;
}

static void ply_vertex(::ostream& out, int x, int y, int z = 0)
{
    TGAPixel32 p = TGA->GetPixelA(x, y);

    if( make_geometry == sphere || make_geometry == hemisphere ) {
        double &R = radius;
        double LAT, LON;

        if( 0 ) {
            double u = double(x) / (DEM->width-1);
            double v = double(y) / (DEM->height-1);

            double xy = -2 * PI * u; // xy angle
            double zz = PI * v;      // z  angle

            double px = R * cos(xy) * sin(zz);
            double py = R * sin(xy) * sin(zz);
            double pz = R * cos(zz);

            out << px << " " << py << " " << pz << " " << int(p.R) << " " << int(p.G) << " " << int(p.B) << " " << int(p.A) << std::endl;
            return;
        }



        if(0) {
            // mercator projection
            LON = x / R;
            LAT = 2 * atan(exp(y/R)) - PI/2;
        } else {
            // this is very naive :s
            double lx = ( double(x) / (DEM->width-1) ) * 360 - 180;
            double ly = ( double(y) / (DEM->height-1) ) * 180 - 90;
            if( make_geometry != sphere ) {
                // assume a hemisphere
                lx = ( double(x) / (DEM->width-1) ) * 180 - 90;
            }
            LAT = ly * PI/180;
            LON = lx * PI/180;
        }

        // latlong to sphere
        // The horizontal coordinate represents the longitude, the vertical coordinate the latitude.
        // Latitude is between -90 and 90, and longitude between -180 and 180
        double px = -R * cos(LAT) * cos(LON);
        double py = -R * sin(LAT);
        double pz =  R * cos(LAT) * sin(LON);
        out << px << " " << py << " " << pz << " " << int(p.R) << " " << int(p.G) << " " << int(p.B) << " " << int(p.A) << std::endl;

        // also:
        // vec2 vec3latlon(vec3 pos, float sphereRadius) {
        // float px,py,pz;
        // float lat = acos(position.Y / sphereRadius);
        // float lon = atan(position.X / position.Z);
        // return vec2( lat, lon );
        // }
    } else {

        if(make_geometry == curve) {
            double fx = double(x) / DEM->width * 2.0 - 1.0;     // map [-1..1]
            double fy = double(y) / DEM->height * 2.0 - 1.0;    // map [-1..1]
            double fd = sqrt((fx*fx + fy*fy) / 2.0);            // normalized length from center
            double lz = radius * fd * fd;

            out << x << " " << y << " " << lz << " " << int(p.R) << " " << int(p.G) << " " << int(p.B) << " " << int(p.A) << std::endl;
        } else {
            out << x << " " << y << " " << z << " " << int(p.R) << " " << int(p.G) << " " << int(p.B) << " " << int(p.A) << std::endl;
        }
        return;
    }
}

bool output_ply(const char *filename)
{
    std::stringstream sout;
    output_stream = &sout;

    int width = DEM->width;
    int height = DEM->height;

    xarray2<int> vert_id(width, height);
    int index = 0;

    for(int x=0; x<width; x++) {
            for(int y=0; y<height; y++) {
            if(mesh->is_used(x,y) == DATA_POINT_USED) {
                vert_id(x,y) = index++;
                ply_vertex(sout, x,y);
            } else {
                vert_id(x,y) = 0;
            }
        }
    }

    ply_faces = 0;
    mesh->overFaces(ply_face, &vert_id);

    ::ofstream out(filename);
    out << "ply" << std::endl;
    out << "format ascii 1.0" << std::endl;
    out << "comment author: " << machine_name() << std::endl;
    out << "comment object: vertex-color mesh" << std::endl;
    out << "element vertex " << index << std::endl;
    out << "property float x" << std::endl;
    out << "property float y" << std::endl;
    out << "property float z" << std::endl;
    out << "property uchar red" << std::endl;
    out << "property uchar green" << std::endl;
    out << "property uchar blue" << std::endl;
    out << "property uchar alpha" << std::endl;
    out << "element face " << ply_faces << std::endl;
    out << "property list uchar int vertex_index" << std::endl;
    out << "end_header" << std::endl;
    out << sout.rdbuf();
    std::cout << " (tris: "<< ply_faces << ") (" << (out.tellp() / 1024) << " KiB)";
    out.close();

    return out.good();
}

