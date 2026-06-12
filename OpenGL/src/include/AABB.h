#pragma once

#include <glm/glm.hpp>

struct AABB{
	glm::vec3 min;
	glm::vec3 max;
};

inline bool pointInsideAabb(	const glm::vec3& p, 
				const glm::vec3& min,
				const glm::vec3& max){

	return  p.x >= min.x && p.x <= max.x &&
		p.y >= min.y && p.y <= max.y &&
		p.z >= min.z && p.z <= max.z;
}

inline bool pointInsideAabb(const glm::vec3& p, const AABB& box){
	return pointInsideAabb(p, box.min, box.max);
}

						
