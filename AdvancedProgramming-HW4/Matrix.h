#pragma once

#include "Group.h"
#include <functional>
#include <unordered_map>
#include <map>

/* Taken from example in lecture with some adjustments */
template<typename T, size_t DIMS>
struct MatrixCopier {
	static void copy(std::vector<T>& dest, size_t destFrom, size_t destSize, const size_t* destDims,
		const std::vector<T>& source, size_t sourceFrom, size_t sourceSize, const size_t* sourceDims)
	{
		auto destSize0 = destDims[0] ? destSize / destDims[0] : 0;
		auto sourceSize0 = sourceDims[0] ? sourceSize / sourceDims[0] : 0;
		for (size_t i = 0; i < sourceDims[0]; ++i)
		{
			MatrixCopier<T, DIMS - 1>::copy(dest, i * destSize0 + destFrom, destSize0, destDims + 1,
											source, i * sourceSize0 + sourceFrom, sourceSize0, sourceDims + 1);
		}
	}
};

/* Taken from example in lecture with some adjustments */
template<typename T>
struct MatrixCopier<T, 1> {
	static void copy(std::vector<T>& dest, size_t destFrom, size_t destSize, const size_t* destDims,
		const std::vector<T>& source, size_t sourceFrom, size_t sourceSize, const size_t* sourceDims)
	{
		for (size_t i = 0; i < sourceSize; ++i)
		{
			dest[i + destFrom] = source[i + sourceFrom];
		}
	}
};

template<class T, size_t DIMS>
class Matrix {
public:
	/* Taken from example in lecture with some adjustments */
	template<typename G = T>
	Matrix(const std::initializer_list<std::enable_if_t<DIMS == 1, G>>& values)
	{
		m_dimensions[0] = values.size();
		for (const auto& val : values)
			m_array.push_back(val);
	}
	
	/* Taken from example in lecture with some adjustments */
	template<typename G = T>
	Matrix(const std::initializer_list<Matrix<std::enable_if_t<(DIMS > 1), G>, DIMS - 1>>& values)
	{
		m_dimensions[0] = values.size();
		for (auto& mat : values)
		{
			for (size_t dim = 0; dim < DIMS - 1; ++dim)
			{
				if (mat.m_dimensions[dim] > m_dimensions[dim + 1])
				{
					m_dimensions[dim + 1] = mat.m_dimensions[dim];
				}
			}
		}

		size_t size = 1;
		for (size_t dim = 0; dim < DIMS; ++dim)
		{
			size *= m_dimensions[dim];
		}

		m_array = std::vector<T>(size); // "zero initialized" - T()
		size_t i = 0;
		size_t destSize = size / m_dimensions[0];
		for (auto& mat : values)
		{
			MatrixCopier<T, DIMS - 1>::copy(m_array, i * destSize, destSize, m_dimensions + 1,
											mat.m_array, 0, mat.m_array.size(), mat.m_dimensions);
			++i;
		}

		// Keeping num of elements for each dimension - to be used in groupValues()
		for (i = 0; i < DIMS; i++)
		{
			size /= m_dimensions[i];
			m_elementsPerDim[i] = size;
		}
	}

	template <typename FUNC>
	auto groupValues(FUNC lambda) const
	{
		using V = std::result_of_t<FUNC(T&)>;
		// Will keep for each Coordinate it's lambda expression result
		std::unordered_map<Coordinate<DIMS>, V, Hash> coordToGroupType;
		createMap(coordToGroupType, lambda); // Sets up coordToGroupType

		std::map<V, size_t> mapToGroup; // Maps group type value to index in allGroupsVec
		// allGroupsVec[i] = group of above mentioned value
		std::vector<std::set<std::set<Coordinate<DIMS>>>> allGroupsVec;
		
		while (!coordToGroupType.empty())
		{
			auto iter = coordToGroupType.begin();
			auto coor = iter->first;
			auto val = iter->second;
			std::set<Coordinate<DIMS>> innerGroup; // Group of coor
			innerGroup.insert(coor);
			coordToGroupType.erase(coor); // Erasing from map coordinate that has a group

			// Recursive function that finds all coordinates belonging to coor's group
			recGroupValues(innerGroup, coor, coordToGroupType, val);
			auto grpIter = mapToGroup.find(val);
			if (grpIter != mapToGroup.end())
			{
				auto index = grpIter->second;
				auto& grp = allGroupsVec[index];
				grp.insert(innerGroup);
			}
			else // If we first came across val
			{
				std::set<std::set<Coordinate<DIMS>>> grp;
				grp.insert(innerGroup);
				allGroupsVec.push_back(grp);
				mapToGroup[val] = allGroupsVec.size() - 1;
			}
		}
		std::set<Group<V, DIMS>> allGroups; // Return DS - sorted set of all groups
		for (const auto& elem : mapToGroup)
		{
			auto index = elem.second;
			allGroups.insert(Group<V, DIMS>(elem.first, allGroupsVec[index]));
		}

		return allGroups;
	}

	template <typename V>
	void recGroupValues(std::set<Coordinate<DIMS>>& grp, const Coordinate<DIMS>& coor,
		std::unordered_map<Coordinate<DIMS>, V, Hash>& coordsMap, const V& val) const
	{
		Coordinate<DIMS> tmpCoor(coor);
		for (size_t i = 0; i < DIMS; i++)
		{
			if (tmpCoor[i] > 0)
			{
				--tmpCoor[i];
				tmpCoor.generateKeyForHash();
				// If tmpCoor belongs to our group - insert to group and erase from map
				updateMapAndSet(grp, tmpCoor, coordsMap, val);
				++tmpCoor[i];
			}
			if (tmpCoor[i] < m_dimensions[i] - 1)
			{
				++tmpCoor[i];
				tmpCoor.generateKeyForHash();
				updateMapAndSet(grp, tmpCoor, coordsMap, val);
				--tmpCoor[i];
			}
		}
	}


private:
	std::vector<T> m_array;
	size_t m_dimensions[DIMS]		= {};
	size_t m_elementsPerDim[DIMS]	= {};
	
	friend class Matrix<T, DIMS + 1>;

	template <typename V>
	void updateMapAndSet(std::set<Coordinate<DIMS>>& grp, const Coordinate<DIMS>& coor,
		std::unordered_map<Coordinate<DIMS>, V, Hash>& coordsMap, const V& val) const
	{
		auto iter = coordsMap.find(coor);
		if (iter != coordsMap.end() && iter->second == val)
		{
			// Erasing from map coordinate that has a group
			coordsMap.erase(coor);
			grp.insert(coor);
			recGroupValues(grp, coor, coordsMap, val);
		}
	}

	template <typename V, typename FUNC>
	void createMap(std::unordered_map<Coordinate<DIMS>, V, Hash>& coordsMap, FUNC lambda) const
	{
		for (size_t i = 0; i < m_array.size(); i++)
		{
			Coordinate<DIMS> coor;
			V val = lambda(m_array[i]); // Calculating value
			auto index = i;

			// Mapping each global index to Coordinate
			for (size_t j = 0; j < DIMS; j++)
			{
				coor[j] = index / m_elementsPerDim[j];
				index %= m_elementsPerDim[j];
			}
			coor.generateKeyForHash();
			coordsMap[coor] = val;
		}
	}
};

template <typename T>
using Matrix2d = Matrix<T, 2>;

template <typename T>
using Matrix3d = Matrix<T, 3>;
