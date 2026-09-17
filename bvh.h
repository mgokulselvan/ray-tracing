#ifndef BVH_H
#define BVH_H

#include "aabb.h"
#include "rtweekend.h"
#include "hittable.h"
#include "hittable_list.h"

#include <algorithm>

class bvh_node : public hittable {
	public: 
		bvh_node(hittable_list list) : bvh_node(list.objects, 0, list.objects.size()){//calling another constructor
			//side note, here we are accepting hittable list as value and not reference, hence the local copy is created and calls the other constructor, and local copy is destroyed (not the original hittable list, just the local copy) but its ok, because we would have already created the bounding volume hierarcy
		}

		bvh_node(std::vector<shared_ptr<hittable>>& objects, size_t start, size_t end){//start of the list of nodes under this node as a tree and the end of it
			int axis = random_int(0,2);//randomly choose the axis to split along
			auto comparator = (axis == 0) ? box_x_compare//depending on the random axis, choose a comparator which will sort the bounding boxes accordingly
				: (axis == 1) ? box_y_compare
				: box_z_compare;

			size_t object_span = end-start;
			if(object_span == 1){//if only one bounding box remains, both left and right side of the node point to the same bounding box
				left = right = objects[start];
			}else if(object_span == 2){//if two bounding boxes remain, one to the left and another one to the right
				left = objects[start];
				right = objects[start+1];
			}else{//if more than 2 bounding boxes , half to left and half to right, and do this iteratively till you reach the leaf node
				std::sort(std::begin(objects) + start, std::begin(objects) + end, comparator);

				auto mid = start + object_span/2;
				left = make_shared<bvh_node>(objects, start, mid);
				right = make_shared<bvh_node>(objects, mid, end);
			}
			bbox = aabb(left->bounding_box(), right->bounding_box());
		}

		bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
			if(!bbox.hit(r, ray_t))
				return false;

			bool hit_left = left->hit(r, ray_t, rec);//if it hit the bounding box attached to the left of the node
			bool hit_right = right->hit(r, interval(ray_t.min, hit_left ? rec.t : ray_t.max), rec);//if it hit the bounding box attached to the right of the node

			return hit_left || hit_right;//if it even hit anything (either left or right) , here right and left are the bounding box covering the list of bounding boxes that are attached to the left and right side of this node, (also , all the objects on the left and right side combined make the bounding box that is represented by the current node)
		}

		aabb bounding_box() const override { return bbox; }

	private:
		shared_ptr<hittable> left;
		shared_ptr<hittable> right;
		aabb bbox;

		//comparator for each axis, and also ones with specific names that inturn just calls the generic function with proper parameters
		static bool box_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b, int axis_index) {
			auto a_axis_interval = a->bounding_box().axis_interval(axis_index);//a and b are hittable objects, and for them , we call bounding_box() which returns their bounding boxes, and we call axis_interval() on them, which returns the bounding box's interval in the axis according to axis_index, and since we are sorting according to one axis and splitting acording to it, this is sufficient
			auto b_axis_interval = b->bounding_box().axis_interval(axis_index);
			return a_axis_interval.min < b_axis_interval.min;
		}

		static bool box_x_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
			return box_compare(a, b, 0);
		}

		static bool box_y_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
			return box_compare(a, b, 1);
		}

		static bool box_z_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
			return box_compare(a, b, 2);
		}
};


#endif
