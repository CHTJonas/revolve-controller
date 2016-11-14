template <class T> constexpr const T& clamp(const T& v, const T& lo, const T& hi) {
	return max(min(v, hi), lo);
}

bool eStopsEngaged();
