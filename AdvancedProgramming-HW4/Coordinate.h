#pragma once

#include <vector>
#include <string>

template <size_t DIMS>
class Coordinate
{
public:
	Coordinate() : coords(DIMS) {}

	Coordinate(const Coordinate<DIMS>& other) : coords(other.coords), _key(other._key) {}

	std::vector<size_t>::iterator begin()
	{
		return coords.begin();
	}

	std::vector<size_t>::iterator end()
	{
		return coords.end();
	}

	std::vector<size_t>::const_iterator begin() const
	{
		return coords.begin();
	}

	std::vector<size_t>::const_iterator end() const
	{
		return coords.end();
	}

	bool operator<(const Coordinate<DIMS>& other) const
	{
		return coords < other.coords;
	}

	void generateKeyForHash()
	{
		_key = "";
		for (size_t i = 0; i < DIMS - 1; i++)
		{
			_key += std::to_string(coords[i]) + ":";
		}
		_key += std::to_string(coords[DIMS - 1]);
	}

	const std::string& key() const
	{
		return _key;
	}

	bool operator==(const Coordinate<DIMS>& other) const
	{
		return coords == other.coords;
	}

	size_t& operator[](size_t index)
	{
		return coords[index];
	}

private:
	std::vector<size_t>	coords;
	std::string			_key;
};

struct Hash
{
	template <size_t DIMS>
	size_t operator()(const Coordinate<DIMS>& coor) const
	{
		return std::hash<std::string>()(coor.key());
	}
};

