#include "math-ext.h"
#include "vec3.h"
#include "bmp.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <random>

#include <future>
#include <chrono>
#include <iostream>
#include <functional>

using namespace std;

static const int RAY_DEPTH  = 7;
static const int SAMPLES = 10;


class HDRColor {
    public:
        HDRColor() 
            :r(0.0),g(0.0),b(0.0) 
            {}
        HDRColor(double _r,double _g, double _b)
            :r(_r),g(_g),b(_b) 
            {}

    double r;
    double g;
    double b;
};


HDRColor inline operator * (const HDRColor& v1 , double s){
    return  HDRColor(v1.r*s , v1.g*s , v1.b*s);
}
HDRColor inline operator * (double s, const HDRColor& v1 ){
    return  v1*s;
}
HDRColor inline operator + (const HDRColor& v1 , const HDRColor& v2){
    return  HDRColor(v1.r+v2.r , v1.g+v2.g , v1.b+v2.b);
}
HDRColor inline operator * (const HDRColor& v1 , const HDRColor& v2){
    return  HDRColor(v1.r*v2.r , v1.g*v2.g , v1.b*v2.b);
}
/*ray result*/
struct RayResult {
    double r;
    vec3 n;
    HDRColor c;
};

class Ray {
    public:
    Ray()
    {}
    Ray(const vec3& rayOrigin,const vec3& rayDirection)
    :ro(rayOrigin),rd(rayDirection) 
    {}
    vec3 ro;
    vec3 rd;
};

class Material {
    public:
    Material() 
        :baseColor(0.0,0.0,0.0) 
    {}
    Material(const HDRColor& _baseColor) 
        :baseColor(_baseColor)
    {    
    }

    HDRColor baseColor;
    

};

struct AnalyticalSphere {
    public:
    AnalyticalSphere(const vec3& _center, double _radius,const Material& _material) 
    :p(_center) , material(_material) , r(_radius)
    {}
    double const Intersect(const Ray& ray) 
    {
        const vec3& ro = ray.ro;
        const vec3& rd = ray.rd;
        const vec3 oc = ro-p;
        double b = dot(rd,oc);
        double c = dot(oc,oc) - r*r;
        double t = b*b - c;
        if (t > 0.0)
            t = -b - sqrt(t);
        if (t < 0)
            return INFI;
        return t;
    }
    /*data*/
    vec3 p;
    double r;
    Material material;
};

class AnalyticalPlane
{
  public:
    AnalyticalPlane(const vec3& normal, double shift, const Material& _material)
        : n(normalize(normal)), d(shift), material(_material)
    {
    }
    double Intersect(const Ray &ray)
    {
        vec3 ro = ray.ro;
        vec3 rd = ray.rd;
        double s = (-(dot(n, ro) + d)) / (dot(n, rd) + EPS);

        if (s < (EPS * 2.0) || isnan(s))
            return INFI;

        return s;
    }
    vec3 n;
    double d;
    Material material;
};




std::vector<AnalyticalPlane> p;
std::vector<AnalyticalSphere> s;


std::mt19937 gen; 
std::uniform_real_distribution<> rnd(-1.0, 1.0);


double lin2srgb(double x) {
 if (x <= 0.0031308)
    return x * 12.92;
  else
    return 1.055 * pow(x, 1.0 / 2.4) - 0.055;

}


t_rgbc buildRGB(const HDRColor& c) {
    t_rgbc a;
    //TODO BetterHDR
    /* HDR Operator */
    a.r = (u8)(lin2srgb((c.r/(1.0+c.r)))  * 255.0); //check me
    a.g = (u8)(lin2srgb((c.g/(1.0+c.g)))  * 255.0);
    a.b = (u8)(lin2srgb((c.b/(1.0+c.b)))  * 255.0);
    return a;
}
double inline random_m(){
return rnd(gen);
}
vec3 randomVector() {
    return vec3(random_m(),random_m(),random_m());
}


vec3 inline SampleRay(const vec3& n,const vec3& in) {

  // MONTE CARLO
   vec3 rnd = randomVector();

    if (dot(n, rnd) > 0.0)
        return normalize(rnd);
     return normalize(-rnd);
}

bool isLightVisible(const vec3& point, const vec3& l) {
        double r = INFI;
        double min_l = EPS*100;
        double ri;
        vec3 n ;
        vec3 rd =  normalize(point-l);
        Ray ray(l,rd);
        for (auto a: p) {
            ri = a.Intersect(ray);
            if (ri < r && ri > min_l) { 
                r = ri; 
            }
        }
        for (auto a: s)
        {
            ri = a.Intersect(ray);
            if ( ri < r&& ri > min_l) {
                 r = ri; 
            }
        }

       vec3 res = l + rd*r;

       return (length(point - res ) < 100*EPS);
}

RayResult intersect(const Ray &ray)
{
    double min_l = EPS*100;
    double ri = INFI;
    double r = INFI;
    RayResult res;
    Material material;
    vec3 n;

    // intersect
    for (auto a : p)
    {
        ri = a.Intersect(ray);
        if (ri < r && ri > min_l)
        {
            r = ri;
            n = a.n;
            material = a.material;
        }
    }

    for (auto a : s)
    {
        ri = a.Intersect(ray);
        if (ri < r && ri > min_l)
        {
            r = ri;
            n = normalize((ray.ro + ray.rd * ri) - a.p);
            material = a.material;
        }
    }
    if (r == INFI)
    {
        res.r = 0.0;
        return res;
    }

    res.n = n;
    res.r = r;
    //res.material = material; TODO
    res.c = material.baseColor;
    return res;
}

HDRColor gatherLi(const Ray& ray,const vec3& lightPoint,int iter) {

    RayResult i = intersect(ray);

    HDRColor res(0.0,0.0,0.0);
    if (i.r == 0.0)
        return res; //eh nothing
    if (iter <= 0)
        return res; //recursion limit 

    vec3 iSecPoint = ray.ro+ray.rd*i.r;

    vec3 lightDir = normalize( lightPoint-iSecPoint);        
    //lambert
    // light incedence


    HDRColor Li(0.0,0.0,0.0);
    if (isLightVisible(iSecPoint,lightPoint))
        Li = i.c*fmax(dot(i.n, lightDir ), 0.0);//*HDRColor(1.0,1.0,1.0);//light color
    
    vec3 sampleRay = SampleRay(i.n, ray.rd) ;

    Ray newRay( iSecPoint,sampleRay);

    double Lo = fmax(dot(i.n,newRay.rd),0.0);

    Li = Li+ gatherLi(newRay,lightPoint,iter-1)*Lo;
    
    return Li;

}


HDRColor rayCast(const Ray& ray,const vec3& lightSRC) {

    return gatherLi(ray,lightSRC,RAY_DEPTH);

}

int rayTrace(const BMPData& b,u32 x0, u32 y0, u32 w, u32 h) {
    vec3 proj_plane = vec3(0.0,0.0,-1.0);
    vec3 ro = vec3(0.0,0.0,-2.0);


    for (u32 x = x0 ; x < w ; x++)
        for (u32 y = y0; y < h ; y++)
        {

            proj_plane.x = ((double)x / (double )b.w)* 2.0 - 1.0;
            proj_plane.y =  ((double)y / (double)b.h)* 2.0 - 1.0;
            RayResult rs;
            HDRColor ac = HDRColor(0.0,0.0,0.0);
            vec3 rd = normalize(proj_plane-ro);
            Ray ray(ro,rd);
            for (int i = 0;i< SAMPLES; i++) {
                 ac = ac + rayCast(ray,vec3(0.95+0.01*random_m(),0.01*random_m(),0.01*random_m()));
            }

            ac.r /= (double)SAMPLES;
            ac.g /= (double)SAMPLES;
            ac.b /= (double)SAMPLES;

            b.bits[x*b.w+y] = buildRGB(ac);//buildRGB();// rayCast(t_ray ray);
        }

    return 0;
}

int initObj() {

    Material red( HDRColor(1.0,0.0,0.0) );
    Material blue( HDRColor(0.0,0.0,1.0) );
    Material white( HDRColor(1.0,1.0,1.0) );
    
    /*plane*/
    p.push_back(AnalyticalPlane( vec3(0.0,1.0,0.0) , 1.0 ,white)); // |->
    p.push_back(AnalyticalPlane( vec3(1.0,0.0,0.0) , 1.0 ,blue)); // <-|
    p.push_back(AnalyticalPlane( vec3(-1.0,0.0,0.0) , 1.0,blue));
    p.push_back(AnalyticalPlane( vec3(0.0,0.0,-1.0) , 1.0,red));

    p.push_back(AnalyticalPlane( vec3(0.0,-1.0,0.0) ,  1.0,blue));
    p.push_back(AnalyticalPlane( vec3(0.0,0.0, 1.0) , -1.5,white));
    /*spheres*/
    
    vec3 center(0.0,0.1,0.0);

    //s.push_back(AnalyticalSphere(center,0.2, white));
    s.push_back(AnalyticalSphere(vec3(0.0,0.3,0.0),0.35, red));
    s.push_back(AnalyticalSphere(vec3(0.0,-0.3,0.0),0.35, blue));


    return 0;
}

int main()
{
    auto start = chrono::steady_clock::now();
    initObj();

    BMPData b = allocBMP(500,500);

    auto r1 = std::async(std::launch::async, rayTrace ,b, 0     ,0     , b.w/2,b.h/2 );
    auto r2 = std::async(std::launch::async, rayTrace ,b, 0     ,b.h/2 , b.w/2,b.h   );
    auto r3 = std::async(std::launch::async, rayTrace ,b, b.w/2 ,0     , b.w  ,b.h/2 );
    auto r4 = std::async(std::launch::async, rayTrace ,b, b.w/2 ,b.h/2 , b.w  ,b.h   );


    r4.get();
    r3.get();
    r2.get();
    r1.get();
    writeBMP("out2.bmp",b);

    auto end = chrono::steady_clock::now();

    auto diff = end- start;

    cout << chrono::duration <double, milli> (diff).count() << " ms" << endl;

    return 0;
}
