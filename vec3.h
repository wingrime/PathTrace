#pragma once
#include "math-ext.h"
/* micro vector library*/
class vec3 {
public:
    vec3() 
    :x(0.0),y(0.0),z(0.0)
    {}
    vec3(double _x,double _y,double _z)
    :x(_x),y(_y),z(_z)
    {}
    double x;
    double y;
    double z;

    vec3 inline operator + (const vec3& v2) const {
      vec3 rv = vec3(x+v2.x ,y+v2.y , z+v2.z);
    return rv;
    };
    vec3 inline operator - (const vec3& v2) const {
      vec3 rv = vec3(x-v2.x ,y-v2.y , z-v2.z);
    return rv;
    };
    vec3 inline operator - () const {
      return vec3(-x ,-y ,-z);
    };

};

vec3 inline operator * (const vec3& v1 , double s){

    return vec3(v1.x*s , v1.y*s , v1.z*s);
}
vec3 inline operator * (double s,const vec3& v1 ){
    return v1*s;
}
double inline dot(const vec3& v1 ,const  vec3& v2) {

    return (v1.x*v2.x +  v1.y*v2.y + v1.z*v2.z);
}
double inline length(const vec3& v1) {

    return sqrt(dot(v1,v1));
}

vec3 inline normalize(const  vec3& v1) {

    double len = 1.0/sqrt(dot(v1,v1));
    return len*v1;

}
