#ifndef HITTABLE_LIST_H
#define HITTABLE_LIST_H

#include "rtweekend.h"
#include <vector>

//using std::make_shared;//pointer to an allocated type , has reference counting, increases counter each time some variable refers to it, safely deletes the object when all the pointers to this object go out of scope
//using std::shared_ptr;//this is the return type of make_shared, pointers refering to it are of this type

class hittable_list : public hittable {
	public:
		std::vector<shared_ptr<hittable>> objects;

		hittable_list() {}
		hittable_list(shared_ptr<hittable> object) { add(object); }

		void clear() { objects.clear(); }

		void add(shared_ptr<hittable> object) {
			objects.push_back(object);
		}

		bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
			hit_record temp_rec;
			bool hit_anything = false;
			auto closest_so_far = ray_t.max;

			for(const auto& object : objects) {
				if (object->hit(r, interval(ray_t.min,closest_so_far), temp_rec)) {
					hit_anything = true;
					closest_so_far = temp_rec.t;
					rec = temp_rec;
				}
			}
			return hit_anything;
		}
};
#endif
