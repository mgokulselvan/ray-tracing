#ifndef TRI_H
#define TRI_H

#include "../core/hittable.h"
#include "../core/hittable_list.h"
#include "../appearance/material.h"

class tri : public hittable {

public:
    tri(const point3 &a, const point3 &b, const point3 &c,
        shared_ptr<material> mat) {

        this->a = a;
        this->b = b;
        this->c = c;
        this->mat = mat;

        interval x(
                   std::fmin(a.x(), std::fmin(b.x(), c.x())),
                   std::fmax(a.x(), std::fmax(b.x(), c.x()))
                  );

        interval y(
                   std::fmin(a.y(), std::fmin(b.y(), c.y())),
                   std::fmax(a.y(), std::fmax(b.y(), c.y()))
                  );

        interval z(
                   std::fmin(a.z(), std::fmin(b.z(), c.z())),
                   std::fmax(a.z(), std::fmax(b.z(), c.z()))
                  );

        const double delta = 0.0001;

        if (x.size() < delta) x = x.expand(delta);
        if (y.size() < delta) y = y.expand(delta);
        if (z.size() < delta) z = z.expand(delta);

        bbox = aabb(x, y, z);
    }

    virtual bool hit(const ray &r, interval ray_t, hit_record &rec) const override {
        const double EPSILON = 1e-9;

        vec3 edge1 = b - a;
        vec3 edge2 = c - a;

        vec3 h = cross(r.direction(), edge2);

        double aa = dot(edge1, h);

        if (std::abs(aa) < EPSILON)
            return false;

        double f = 1.0 / aa;

        vec3 s = r.origin() - a;

        double u = f * dot(s, h);

        if (u < 0.0 || u > 1.0)
            return false;

        vec3 q = cross(s, edge1);

        double v = f * dot(r.direction(), q);

        if (v < 0.0 || u + v > 1.0)
            return false;

        double t = f * dot(edge2, q);

        if (!ray_t.surrounds(t))
            return false;

        rec.t = t;
        rec.p = r.at(t);
        rec.mat = mat;

        vec3 outward_normal = unit_vector(cross(edge1, edge2));

        rec.set_face_normal(r, outward_normal);

        return true;
    }

    aabb bounding_box() const override { return bbox; }

private:
    point3 a, b, c;
    shared_ptr<material> mat;
    aabb bbox;

};

#endif
