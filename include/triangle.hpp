#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>
#include <iostream>
using namespace std;

class Triangle: public Object3D {

public:
	Triangle() = delete;

    // a b c are three vertex positions of the triangle
	Triangle( const Vector3f& a, const Vector3f& b, const Vector3f& c,  
	const Vector2f& auv, const Vector2f& buv, const Vector2f& cuv, Material* m) : Object3D(m) {
		_vertices[0] = a;
		_vertices[1] = b;
		_vertices[2] = c;
		_texCoords[0] = auv;
		_texCoords[1] = buv;
		_texCoords[2] = cuv;
		normal = Vector3f::cross(b - a, c - a);
		normal.normalize();
		_normals[0] = Vector3f::ZERO;
		_normals[1] = Vector3f::ZERO;
		_normals[2] = Vector3f::ZERO;
	}

	~Triangle() override = default;

	friend ostream& operator<<(ostream& os, const Triangle& t){
		os << "Triangle: " << t._vertices[0] << " " << t._vertices[1] << " " << t._vertices[2] << " " << t.normal << endl;
		return os;
	}

	void setNormals(const Vector3f& na, const Vector3f& nb, const Vector3f& nc){
		_normals[0] = na;
		_normals[1] = nb;
		_normals[2] = nc;
	}

	bool intersect( const Ray& ray,  Hit& hit , float tmin) override {
		Vector3f vertices[3] = {_vertices[0], _vertices[1], _vertices[2]};
		//bump map before intersect
		if(material->hasTexture()){
			float bumpMultiplier = 0, bump = 0;
			EmpiricalMaterial*m = dynamic_cast<EmpiricalMaterial*>(material);
			if(m == nullptr){
				std::cout << "material is not EmpiricalMaterial" << endl;
				exit(0);
			}
			std::make_pair(bumpMultiplier, bump) = m->getBump(_texCoords[0]);
			vertices[0] += bump * bumpMultiplier * _normals[0];
			std::make_pair(bumpMultiplier, bump) = m->getBump(_texCoords[1]);
			vertices[1] += bump * bumpMultiplier * _normals[1];
			std::make_pair(bumpMultiplier, bump) = m->getBump(_texCoords[2]);
			vertices[2] += bump * bumpMultiplier * _normals[2];
			
		}


		//a faster algorithm
		//source: luuyiran
		Vector3f origin = ray.getOrigin();
		Vector3f direction = ray.getDirection();
		Vector3f E1 = vertices[1] - vertices[0];
		Vector3f E2 = vertices[2] - vertices[0];
		Vector3f P = Vector3f::cross(direction, E2);
		float det = Vector3f::dot(E1, P);
		if(det < tmin ) return false;
		float invdet = 1 / det;

		Vector3f T = origin - _vertices[0];
		float u = Vector3f::dot(T, P) * invdet;
		if(u < 0 || u > 1) return false;

		Vector3f Q = Vector3f::cross(T, E1);
		float v = Vector3f::dot(direction, Q) * invdet;
		if(v < 0 || u + v > 1) return false;

		float t = Vector3f::dot(E2, Q) * invdet;
		if(t < tmin || t >= hit.getT()) return false;

		//set texcoord according to barycentric coordinates
		Vector2f texCoord = Vector2f::ZERO;
		//if(material->hasTexture())
			texCoord = (1 - u - v) * _texCoords[0] + u * _texCoords[1] + v * _texCoords[2];
		//std::cout << "texCoord in triangle intersect: " << texCoord << endl;

		//normal interpolation
		Vector3f hitNormal = normal;
		if(!material->hasTexture()){
			if(_normals[0] == Vector3f::ZERO && _normals[1] == Vector3f::ZERO && _normals[2] == Vector3f::ZERO){
				if(hitNormal == Vector3f::ZERO){
					hitNormal = Vector3f::cross(E1, E2);
					hitNormal.normalize();
				}
			}else{
				hitNormal = (1 - u - v) * _normals[0] + u * _normals[1] + v * _normals[2];
				hitNormal.normalize();
			}
		}else{
			EmpiricalMaterial*m = dynamic_cast<EmpiricalMaterial*>(material);
			if(m == nullptr){
				std::cout << "material is not EmpiricalMaterial" << endl;
				exit(0);
			}
			//TODO: normal map
			//hitNormal = m->getNormal(texCoord);
			hitNormal = (1 - u - v) * _normals[0] + u * _normals[1] + v * _normals[2];
		}

		hit.set(t, material, hitNormal, texCoord);
		return true;
	}
	Vector3f normal;
	Vector3f _vertices[3];
	Vector2f _texCoords[3];
	Vector3f _normals[3];

protected:
};

#endif //TRIANGLE_H
