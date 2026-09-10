#ifndef VEC3_H //header guard- if this header file havent been defined in the file that imports this, THEN, define it
#define VEC3_H//this is like associating this variable with a file, so if it tries importing this class again, the above line checks if this variable that we have defined is in that file, if it is, it skips it, if not, then it defines this variable so that this scenario doesnt happen , and proceeds to define the class

#include "rtweekend.h"


class vec3{
	public:
		double e[3];

		vec3(): e{0,0,0}{/*function body, nothing to do here, because the object to be initialized has been initialized already using the member initializer list:- ( : e{0,0,0} ) */}
		vec3(double e0, double e1, double e2) : e{e0,e1,e2}{}


		double x() const { return e[0]; } //'const' here tells that it promises not to modify the object
		double y() const { return e[1]; }
		double z() const { return e[2]; }

		vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); } //tells what to do when unary minus is used on an object of vec3 , lets say v , this tells what -v means ,or what it returns
		double operator[](int i) const { return e[i]; }
		double& operator[](int i) { return e[i]; }//c++ is intelligent, it returns reference to e[i](you told the return type to be double& ) but you dont write &e[i] , it will do it 

		vec3& operator+=(const vec3& v){
			e[0] +=v.e[0];
			e[1] +=v.e[1];
			e[2] +=v.e[2];
			return *this;//return this object itself, thats what += means
		}

		vec3& operator*=(double t){
			e[0] *=t;
			e[1] *=t;
			e[2] *=t;
			return *this;	
		}

		vec3& operator/=(double t){
			return *this *= (1/t);
		}

		double length() const {
			return std:: sqrt(length_squared());
		}

		double length_squared() const {
			return e[0]*e[0] + e[1]*e[1] + e[2]*e[2];
		}


		static vec3 random() {
			return vec3(random_double(),random_double(),random_double());
		}

		static vec3 random(double min, double max) {
			return vec3(random_double(min,max), random_double(min,max),random_double(min,max));
		}


};//all functions within the class modify the left hand vec3 object
  //all functions outside dont modify left hand vec3 object, and maybe even modify right hand vec3 object too, or create new object

		using point3 = vec3; //point3 is an alias for vec3

		//utility functions
		inline std::ostream& operator<<(std::ostream& out, const vec3& v){
			return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
		}

		inline vec3 operator+(const vec3& u, const vec3& v){
			return vec3(u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2]);
		}

		inline vec3 operator-(const vec3& u, const vec3& v){
			return vec3(u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2]);
		}

		inline vec3 operator*(const vec3& u, const vec3& v){
			return vec3(u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2]);
		}

		inline vec3 operator*( double t, const vec3& v){
			return vec3(t*v.e[0],t*v.e[1],t*v.e[2]);
		}

		inline vec3 operator*(const vec3& v, double t){
			return t * v;
		}

		inline vec3 operator/(const vec3& v,double t){
			return (1/t) * v;
		}

		inline double dot(const vec3& u,const vec3& v){
			return u.e[0] * v.e[0]
				+ u.e[1] * v.e[1]
				+ u.e[2] * v.e[2];
		}

		inline vec3 cross(const vec3& u, const vec3& v){
			return vec3(u.e[1] * v.e[2] - u.e[2] * v.e[1],
					u.e[2] * v.e[0] - u.e[0] * v.e[2],
					u.e[0] * v.e[1] - u.e[1] * v.e[0]);
		}

		inline vec3 unit_vector(const vec3& v){
			return v / v.length();
		}

		inline vec3 random_unit_vector() {
			while(true) {
				auto p = vec3::random(-1,1);//generate a vector randomly, we could have generated it so that it stays within a unit sphere, but we do this , because, distribution of random from -1 to 1 , ensures that the probability of getting all direction is equal which is needed for diffuse(matte) materials
				auto lensq = p.length_squared();
				if(1e-160 <= lensq && lensq <= 1)//the vector cant be lesser than some value, because if done so , the sqrt of its length becomes infinite, because it so close to 0, and its length <=1 , because it should be within unit sphere
					return p / sqrt(lensq);//return it , but normalized, basically unit vector
			}
		}
		
		inline vec3 random_on_hemisphere(const vec3& normal) {
			vec3 on_unit_sphere = random_unit_vector();
			if(dot(on_unit_sphere, normal) > 0.0)//In the same hemisphere as the normal, so that we get a vector which is in general direction as the normal
				return on_unit_sphere;//if so , just return it
			else
			 return -on_unit_sphere;//if its not, then flip it, which will make it in general direction of the normal
		}

#endif
