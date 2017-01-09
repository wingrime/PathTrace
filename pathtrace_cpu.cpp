#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include <future>
#include <chrono>
#include <iostream>
using namespace std;

#define u32 uint32_t
#define u16 uint16_t
#define s32 int32_t
#define s16 int16_t
#define u8  uint8_t
#define s8  int8_t

struct t_color {
    float r;
    float g;
    float b;
};

/*24 bit saved color*/
struct t_rgbc {
    u8 b;
    u8 g;
    u8 r;
    u8 reserved;
}__attribute__((__packed__));

 struct __attribute__((packed)) t_bmp_header {
  uint16_t bfType; /*signature = 4D42 */
  uint32_t bfSize; /*file size in bytes*/
  uint16_t bfReserved1;
  uint16_t bfReserved2;
  uint32_t bfOffBits; /*image offset*/
};
struct  t_bmp_info_header {
    uint32_t biSize; /*header size bytes= 40*/
    int32_t biWidth; /*pixel width*/
    int32_t biHeight; /*pixel height*/
    uint16_t biPlanes; /*planes = 1*/
    uint16_t bitCount; /* 24 - true color  */
    uint32_t biCompression; /*0 for uncompressed*/
    uint32_t biSizeImage; /* o for uncompressed*/
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;

}__attribute__((__packed__));
/* micro vector library*/
struct vec3 {

    float x;
    float y;
    float z;

    vec3 inline operator + (const vec3& v2) const {
      vec3 rv = {x+v2.x ,y+v2.y , z+v2.z};
    return rv;
    };
    vec3 inline operator - (const vec3& v2) const {
      vec3 rv = {x-v2.x ,y-v2.y , z-v2.z};
    return rv;
    };
    vec3 inline operator - () const {
      vec3 rv = {-x ,-y ,-z};
    return rv;
    };

};

vec3 inline operator * (const vec3& v1 , float s){

    vec3 rv = {v1.x*s , v1.y*s , v1.z*s};
    return rv;
}
vec3 inline operator * (float s,const vec3& v1 ){

    vec3 rv = {v1.x*s , v1.y*s , v1.z*s};
    return rv;
}
float inline dot(const vec3& v1 ,const  vec3& v2) {

    return (v1.x*v2.x +  v1.y*v2.y + v1.z*v2.z);
}
float inline length(const vec3& v1) {

    return sqrt(dot(v1,v1));
}

float f_invsqrt(float x)
{
  float xhalf = 0.5f*x;
  int i = *(int*)&x; // get bits for floating value
  i = 0x5f375a86- (i>>1); // gives initial guess y0
  x = *(float*)&i; // convert bits back to float
  x = x*(1.5f-xhalf*x*x); // Newton step, repeating increases accuracy
  return x;
}
vec3 inline normalize(const  vec3& v1) {

    float len = 1.0/sqrt(dot(v1,v1));

    //float len = f_invsqrt ( dot(v1,v1) );
    vec3 rv = len*v1;

    return rv;
}



float inline max(float a, float b) {
    if (a > b )
        return a;
    else
        return b;
}

/*ray result*/
struct t_rr {
    float r;
    vec3 n;
    t_color c;
};
struct t_rastr {
    u32 w;
    u32 h;
    t_rgbc * bits;
    u32 ppb;
};

struct t_sphere {

    vec3 p;
    float r;
};
#define INFI ((float)1000000.0)
#define EPS ((float)0.000001)
float inline iSphere ( const t_sphere& s, const vec3& ro, const vec3& rd)  {
    vec3 oc = ro-s.p;
    float b = dot(rd,oc);
    float c = dot(oc,oc) - s.r*s.r;
    float t = b*b - c;
    if (t > 0.0)
        t = -b - sqrt(t);

    if (t < EPS)
        return INFI;
    return t;
}
struct t_plane {
    vec3 n;
    float d;
};

float inline iPlane(const t_plane& plane, const vec3& ro ,const vec3& rd) {
    float s = (-(dot(plane.n,ro)+plane.d))/(dot(plane.n,rd)+EPS);

    //printf("plane{ n= {%f,%f,%f},%f }\n rd = { %f,%f,%f} ro = {%f,%f,%f}\nres=%f \n" , plane.n.x,plane.n.y,plane.n.z \
    //       , plane.d , rd.x,rd.y,rd.z , ro.x,ro.y ,ro.z, s);

    if (s < (EPS*2.0) || isnan(s))
        return INFI;

    return s;
}

int writeBMP(const t_rastr& b) {
   FILE *f = fopen("test_write_bmp.bmp","wb");
   if (!f) { printf("Write failed!\n"); return -1;}

   t_bmp_header header;
   t_bmp_info_header iheader;
   memset(&header,0, sizeof(t_bmp_header));
   memset(&iheader,0, sizeof(t_bmp_info_header));
   header.bfType = 0x4d42;
   header.bfSize = sizeof(t_bmp_header)+sizeof(t_bmp_info_header)+b.w*b.h*b.ppb;
   header.bfOffBits = sizeof(t_bmp_header)+sizeof(t_bmp_info_header);

   iheader.biSize = sizeof(t_bmp_info_header);
   iheader.biWidth = b.w;
   iheader.biHeight = b.h;
   iheader.biPlanes = 1;
   iheader.bitCount = 8*b.ppb;
   fwrite(&header,sizeof(header),(size_t)1,f);
   fwrite(&iheader,sizeof(iheader),(size_t)1,f);

   fwrite((void *)b.bits,b.w*b.h*b.ppb,(size_t)1,f);

   fclose(f);

   return 0;
}

t_plane p[7];
t_sphere s[3];

t_rgbc buildRGB(const t_color& c) {
    t_rgbc a;
    a.r = (u8)(pow((c.r/(1.0+c.r)),1.0/2.2)  * 255.0);
    a.g = (u8)(pow((c.g/(1.0+c.g)),1.0/2.2)  * 255.0);
    a.b = (u8)(pow((c.b/(1.0+c.b)),1.0/2.2)  * 255.0);
    return a;
}
float inline random_m(){
    int a = 1;
return (float)(((float)rand()/(float)(RAND_MAX/a))*2.0-1.0);
}
vec3 inline brdfRay(const vec3& n,const vec3& in) {

  // MONTE CARLO
    vec3 rnd = {static_cast<float>(random_m()),static_cast<float>(random_m()),static_cast<float>(random_m())};

    if (dot(n, rnd) > 0.0)
        return normalize(rnd);
    else
        return normalize(-rnd);
    return normalize(rnd);
//REFLECT
  //vec3 out = addv( in , negv (   mulvs( n, 2.0 -dot(n, in))   ) );
//  return out;


}
#define RAY_DEPTH 5
float isLightVisible(const vec3& point, const vec3& l) {
        float r = INFI;
        float ri;
        vec3 ro = l;
        vec3 n ;
        vec3 rd =  normalize(point-l);

       ri = iPlane(p[0],ro,rd);
       if (ri < r) { r = ri; n = p[0].n;  }
       ri = iPlane(p[1],ro,rd);
       if ( ri < r) { r = ri; n = p[1].n;  }
       ri = iPlane(p[2],ro,rd);
       if ( ri < r) { r = ri; n = p[2].n;  }
       ri = iPlane(p[3],ro,rd);
       if ( ri < r) { r = ri; n = p[3].n;  }
       ri = iPlane(p[4],ro,rd);
       if ( ri < r) { r = ri; n = p[4].n;  }
       ri = iPlane(p[5],ro,rd);
       if ( ri < r) { r = ri; n = p[5].n;  }
       
       ri = iSphere(s[0],ro,rd);
       if ( ri < r) { r = ri; n = normalize(    ro + rd*ri  -s[0].p ) ;  }
        ri = iSphere(s[1],ro,rd);
       if ( ri < r) { r = ri; n = normalize(    ro + rd*ri  -s[1].p );  }

       vec3 res = ro + rd*r;
       
       return length( point - res );



}
t_rr rayCast(const vec3& ro_in, const vec3& rd_in) {

    vec3 ro = ro_in;
    vec3 rd = rd_in;
    float ri;

    t_rr res;
    vec3 n;
    float r = INFI;

    vec3 l = {0.6,0.6,0.0} ; /*light */


    t_color c = {0.0,0.0,0.0};

    for (int i = 0; i< RAY_DEPTH; i++) {

       float ray_decay =  1.0/(1.14*(float)i+1.0);


       r = INFI;

       ri = iPlane(p[0],ro,rd);
       if (ri < r) { r = ri; n = p[0].n;  }
       ri = iPlane(p[1],ro,rd);
       if ( ri < r) { r = ri; n = p[1].n;  }
       ri = iPlane(p[2],ro,rd);
       if ( ri < r) { r = ri; n = p[2].n;  }
       ri = iPlane(p[3],ro,rd);
       if ( ri < r) { r = ri; n = p[3].n;  }
       ri = iPlane(p[4],ro,rd);
       if ( ri < r) { r = ri; n = p[4].n;  }
       ri = iPlane(p[5],ro,rd);
       if ( ri < r) { r = ri; n = p[5].n;  }
       
       ri = iSphere(s[0],ro,rd);
       if ( ri < r) { r = ri; n = normalize(    ro + rd*ri  -s[0].p ) ;  }
        ri = iSphere(s[1],ro,rd);
       if ( ri < r) { r = ri; n = normalize(    ro + rd*ri  -s[1].p );  }


        
        ro = ro+(rd*r);
        rd = brdfRay(n,normalize( rd) );

        if (r == INFI)
          break; 

        if ( isLightVisible(ro,l) < EPS ) {
            vec3 light_dir = normalize( l-ro );
            //vec3  light_dir = normalize(l);
            float lamb = max(dot(n, light_dir ), 0.0)*ray_decay;
            c.r += lamb;
            c.g += lamb;
            c.b += lamb;

        } 
    }

    res.c = c;
    res.n = n;
    res.r = r;

    return res;

}
#define SAMPLES 80
int rayTrace(const t_rastr& b,u32 x0, u32 y0, u32 w, u32 h) {
   std::cout << "RAY thread id=" << std::this_thread::get_id() << "\n";
    vec3 proj_plane;
    vec3 ro = {0.0,0.0,-2.0};

    proj_plane.x = 0.0;
    proj_plane.y = 0.0;
    proj_plane.z = -1.0;


    for (u32 x = x0 ; x < w ; x++)
        for (u32 y = y0; y < h ; y++)
        {
            proj_plane.x = ((float)x / (float )b.w)* 2.0 - 1.0;
            proj_plane.y =  ((float)y / (float)b.h)* 2.0 - 1.0;
            t_rr rs;
            t_color ac = {0.0,0.0,0.0};
            vec3 rd = normalize(proj_plane-ro);

            for (int i = 0;i< SAMPLES; i++) {
                 rs = rayCast(ro, rd);
                 ac.r += rs.c.r;
                 ac.g += rs.c.g;
                 ac.b += rs.c.b;
            }

            ac.r /= (float)SAMPLES;
            ac.g /= (float)SAMPLES;
            ac.b /= (float)SAMPLES;

            b.bits[x*b.w+y] = buildRGB(ac);//buildRGB();// rayCast(t_ray ray);
        }

    return 0;
}

int initObj() {
    /*plane*/
    p[0]  = { {0.0,1.0,0.0} , 1.0 }; // |->
    p[1]  = { {1.0,0.0,0.0} , 1.0 }; // <-|
    p[2]  = { {-1.0,0.0,0.0} , 1.0 };
    p[3]  = { {0.0,0.0,-1.0} , 0.0 };

    p[4]  = { {0.0,-1.0,0.0} ,  1.0 };
    p[5]  = { {0.0,0.0, 1.0} , -1.0 };
    /*spheres*/
    s[0]  = { {0.0,0.0,0.0} , 0.25 };
    s[1]  = { {0.0,0.5,0.2} , 0.55 };
    return 0;
}

int main()
{
    auto start = chrono::steady_clock::now();
    std::cout << "Main thread id=" << std::this_thread::get_id() << "\n";
    std::cout << "Rnd Test " << "\n";
    srand(time(NULL)); /*configure RNG*/
    for (int i = 0 ; i < 120; i++)
    {
      float f = random_m();
      std::cout << f << "\n";
    }
    initObj();

    printf("RayTrace!\n");
    t_rastr b;
    b.h = 1000;
    b.w = 1000;
    b.ppb = 4;
    b.bits = (t_rgbc * )calloc(b.h*b.w*b.ppb,sizeof(char));

    auto r1 = std::async(std::launch::async, rayTrace ,b, 0     ,0     , b.w/2,b.h/2 );
    auto r2 = std::async(std::launch::async, rayTrace ,b, 0     ,b.h/2 , b.w/2,b.h   );
    auto r3 = std::async(std::launch::async, rayTrace ,b, b.w/2 ,0     , b.w  ,b.h/2 );
    auto r4 = std::async(std::launch::async, rayTrace ,b, b.w/2 ,b.h/2 , b.w  ,b.h   );

 /*   
    rayTrace(b, 0 ,0 , b.w/2,b.h/2);

    rayTrace(b, 0 ,b.h/2 , b.w/2,b.h);


    rayTrace(b, b.w/2 ,0, b.w,b.h/2);


    rayTrace(b, b.w/2 ,b.h/2 , b.w,b.h);
  */

    r4.get();
    r3.get();
    r2.get();
    r1.get();
    //r1.wait();
    writeBMP(b);

    auto end = chrono::steady_clock::now();

    auto diff = end- start;

    cout << chrono::duration <double, milli> (diff).count() << " ms" << endl;

    return 0;
}

