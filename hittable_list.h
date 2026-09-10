#ifndef HITTABLE_LIST_H
#define HITTABLE_LIST_H

#include "rtweekend.h"
#include <vector>

//using std::make_shared;//pointer to an allocated type , has reference counting, increases counter each time some variable refers to it, safely deletes the object when all the pointers to this object go out of scope
//using std::shared_ptr;//this is the return type of make_shared, pointers refering to it are of this type

class hittable_list : public hittable {//used to define a world of objects of which each are hittable i.e. ray can interact with them
	public:
		std::vector<shared_ptr<hittable>> objects;//the objects of this array aka world? aka environment?

		hittable_list() {}
		hittable_list(shared_ptr<hittable> object) { add(object); }

		void clear() { objects.clear(); }

		void add(shared_ptr<hittable> object) {//add new hittable object to the existing world/environment/whatever you wanna call it
			objects.push_back(object);
		}

		bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
			/*story time
			  so in one world, there might be lot of you know... overlapping objects, one in front of another and all that shenanigans, but on our image, only the one which is closest to the camera is visible,(objects are opaque, atleast for now?) , so we have a closest_so far in this hit functoin, this function is called for 1 ray and the world, so we gotta get the closest point the the camera along that direction, so we call the hit function of respective object with range being from minimum specified in the world / main, basically minimum t from which we have to render stuff to the max, but then , each time we get a hit, we decrease max with the hit location, why? , now it means, we only search from min that was defined to that object, hence only those objects that are in front of that object, which makes sense, so yea, we keep doing it and then at the end whichever is nearest to the camera automatically gets sent to the function call, we also send a boolean which tells if the ray even hit anything in the world i.e. list of hittable objects, if it didnt its a false, if it did, its a true, and automatically details will be recorded in the object given by the function caller
			 */
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
