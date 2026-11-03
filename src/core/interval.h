#ifndef INTERVAL_H
#define INTERVAL_H

class interval {//used to define t values range between which the ray can travel , as you know position of the ray is a function of t, so this affects the number of positions that the ray can have
	public:
		double min, max;

		interval() : min(+infinity), max(-infinity) {} //default interval is empty
													   //notice how min is in +ve and max is in -ve , thats why its empty

		interval(double min, double max): min(min), max(max) {}

		interval(const interval& a, const interval& b){
			//create interval which closely encloses the intervals a and b
			//required to make big bounding box consisting of smaller bounding boxes, done by combining the intervals of each axis
			min = a.min <= b.min ? a.min : b.min;
			max = a.max >= b.max ? a.max : b.max;
		}

		double size() const {
			return max - min;
		}

		bool contains(double x) const {
			return min <= x && x <=max;
		}

		bool surrounds(double x) const {
			return min < x && x < max;
		}

		double clamp(double x) const {
			if (x < min) return min;
			if (x > max) return max;
			return x;
		}

		interval expand(double delta) const {
			auto padding = delta/2;
			return interval(min - padding, max + padding);
		}


		static const interval empty, universe;
};

const interval interval::empty = interval(+infinity,-infinity);
const interval interval::universe = interval(-infinity,+infinity);

interval operator+(const interval& ival, double displacement) {
	return interval(ival.min + displacement , ival.max + displacement);
}

interval operator+(double displacement, const interval& ival) {
	return ival + displacement;
}

//classname::memberName
#endif
