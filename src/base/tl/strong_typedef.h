#pragma once

// T is the type
// M is a unique struct
template<typename T, typename M>
class strong_typedef
{
	T instance;
public:
	explicit strong_typedef(const T other) : instance(other) {}
	strong_typedef() {}
	strong_typedef(const strong_typedef & other) : instance(other.instance) {}

	strong_typedef & operator=(const strong_typedef & other) { instance = other.instance; return *this; }
	//strong_typedef & operator=(const T & other) { instance = other; return *this; }
	operator const strong_typedef & () const { return instance; }
	operator T & () { return instance; }
	bool operator==(const strong_typedef &other) const { return instance == other.instance; }
	bool operator<(const strong_typedef &other) const { return instance < other.instance; }
};
