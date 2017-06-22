#pragma once

#include "Coordinate.h"
#include <set>


template <typename T, size_t DIMS>
struct Group
{
	using GroupsOfType = std::set<std::set<Coordinate<DIMS>>>;
	
	T first;
	GroupsOfType second;

	bool operator<(const Group<T, DIMS>& other) const
	{
		return first < other.first;
	}

	Group(T _first, GroupsOfType grpOfType) : first(_first), second(grpOfType) {}
	Group() {}
};
