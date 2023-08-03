#include <spdlog/spdlog.h>

#include <iostream>
#include <array>

#include <glm/vec3.hpp>
#include "header.hpp"

//Just trying to learn and understand how demongod's ECS thing works...
//https://old.reddit.com/r/cpp_questions/comments/r4rll0/comment/hmp300u/

using Entity = uint32_t;
using Position = glm::vec3;

using Tag = uint32_t;

namespace CompareTags
{
	constexpr Tag Position = 0x1;
}

constexpr size_t N_ENTS = 100;

namespace Data
{
	std::array<Tag, N_ENTS> tags;

	std::array<Position, N_ENTS> positions;
}

void AddComponent(Entity e, Tag tag)
{
	spdlog::debug("Add component to entity {0:d};", e);
	Data::tags[e] += tag;
}

void RemoveComponent(Entity e, Tag tag)
{
	Data::tags[e] -= tag;
}


//Add entity and then remove it lol
void test1()
{
	spdlog::info("test1()\n");

	//std::cout << "Add component of Position to entity 1\n";
	Entity e = 1;
	Entity e2 = 0;

	AddComponent(e, CompareTags::Position);

	auto hasPosition = [&](Entity e)
	{
		if (Data::tags[e] & CompareTags::Position)
		{
			spdlog::info("Entity: {0:d} contains the position tag", e);
			//std::cout << "Entity: " << e << " contains the position tag!";
			return true;
		}
		return false;
		//return true;
	};

	assert(hasPosition(e));
	assert(!hasPosition(e2));

	RemoveComponent(e, CompareTags::Position);
	assert(!hasPosition(e));
}

int main()
{
	spdlog::info("Hello World!\n");
	test1();
}