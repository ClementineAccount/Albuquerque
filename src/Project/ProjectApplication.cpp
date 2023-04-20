#include "stb_image.h"
#include "ProjectApplication.hpp"
#include "SceneLoader.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <spdlog/spdlog.h>

#include <Fwog/BasicTypes.h>
#include <Fwog/Buffer.h>
#include <Fwog/Pipeline.h>
#include <Fwog/Rendering.h>
#include <Fwog/Shader.h>

#include <unordered_map>
#include <filesystem>
#include <algorithm>
#include <iterator>
#include <fstream>
#include <vector>
#include <queue>
#include <set>
#include <array>
#include <iostream>
#include <functional>

#include <cstdarg>
#include <thread>
#include <soloud/soloud.h>
#include <soloud/soloud_wav.h>

#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>

static std::string Slurp(std::string_view path)
{
	std::ifstream file(path.data(), std::ios::ate);
	std::string result(file.tellg(), '\0');
	file.seekg(0);
	file.read((char*)result.data(), result.size());
	return result;
}

namespace fs = std::filesystem;


static constexpr char vert_shader_path[] = "data/shaders/FwogRacing/hello_car.vert.glsl";
static constexpr char frag_shader_path[] = "data/shaders/FwogRacing/hello_car.frag.glsl";
static constexpr char frag_texture_shader_path[] = "data/shaders/FwogRacing/hello_car_textured.frag.glsl";

static constexpr char vert_line_shader_path[] = "data/shaders/FwogRacing/lines.vert.glsl";
static constexpr char frag_line_shader_path[] = "data/shaders/FwogRacing/lines.frag.glsl";

static constexpr char vert_indexed_shader_path[] = "data/shaders/draw_indexed.vert.glsl";
static constexpr char frag_color_shader_path[] = "data/shaders/color.frag.glsl";
static constexpr char frag_phong_shader_path[] = "data/shaders/phongFog.frag.glsl";

static constexpr char vert_skybox_shader_path[] = "data/shaders/skybox.vert.glsl";
static constexpr char frag_skybox_shader_path[] = "data/shaders/skybox.frag.glsl";



static bool Collision::SphereAABBCollisionCheck(Sphere const& sphere, AABB const& aabb)
{
	using std::max;
	using std::min;

	glm::vec3 maxPoint = aabb.get_max_point();
	glm::vec3 minPoint = aabb.get_min_point();

	glm::vec3 nearestPointbox;
	nearestPointbox.x = max(minPoint.x, min(sphere.center.x, maxPoint.x));
	nearestPointbox.y = max(minPoint.y, min(sphere.center.y, maxPoint.y));
	nearestPointbox.z = max(minPoint.z, min(sphere.center.z, maxPoint.z));


	glm::vec3 center_to_point_box = nearestPointbox - sphere.center;
	return glm::dot(center_to_point_box, center_to_point_box)  < (sphere.radius * sphere.radius);
}

std::string ProjectApplication::LoadFile(std::string_view path)
{
	std::ifstream file{ path.data() };
	return { std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
}

static Fwog::GraphicsPipeline CreatePipeline()
{
	// Specify our two vertex attributes: position and color.
	// Positions are 3x float, so we will use R32G32B32_FLOAT like we would in Vulkan.
	static constexpr auto sceneInputBindingDescs = std::array{
	  Fwog::VertexInputBindingDescription{
			// color
			.location = 0,
			.binding = 0,
			.format = Fwog::Format::R32G32B32_FLOAT,
			.offset = offsetof(Primitives::Vertex, position),
		  },
		  Fwog::VertexInputBindingDescription{
			// normal
			.location = 1,
			.binding = 0,
			.format = Fwog::Format::R32G32B32_FLOAT,
			.offset = offsetof(Primitives::Vertex, normal),
		  },
		  Fwog::VertexInputBindingDescription{
			// texcoord
			.location = 2,
			.binding = 0,
			.format = Fwog::Format::R32G32_FLOAT,
			.offset = offsetof(Primitives::Vertex, uv),
		  },
	};


	auto inputDescs = sceneInputBindingDescs;
	auto primDescs = Fwog::InputAssemblyState{ Fwog::PrimitiveTopology::TRIANGLE_LIST };


	auto vertexShader = Fwog::Shader(Fwog::PipelineStage::VERTEX_SHADER, ProjectApplication::LoadFile(vert_shader_path));
	auto fragmentShader = Fwog::Shader(Fwog::PipelineStage::FRAGMENT_SHADER, ProjectApplication::LoadFile(frag_phong_shader_path));

	return Fwog::GraphicsPipeline{ {
	  .vertexShader = &vertexShader,
	  .fragmentShader = &fragmentShader,
	  .inputAssemblyState = primDescs,
	  .vertexInputState = {inputDescs},
	  .depthState = {.depthTestEnable = true, .depthWriteEnable = true, .depthCompareOp = Fwog::CompareOp::LESS},
	} };
}


static Fwog::GraphicsPipeline CreatePipelineLines()
{
	auto descPos = Fwog::VertexInputBindingDescription{
	  .location = 0,
	  .binding = 0,
	  .format = Fwog::Format::R32G32B32_FLOAT,
	  .offset = 0,
	};

	auto descColor = Fwog::VertexInputBindingDescription{
	.location = 1,
	.binding = 1,
	.format = Fwog::Format::R32G32B32_FLOAT,
	.offset = 0,
	};

	auto inputDescs = { descPos, descColor };

	auto primDescs = Fwog::InputAssemblyState{ Fwog::PrimitiveTopology::LINE_LIST };
	auto depthDescs = Fwog::DepthState{ .depthTestEnable = false, .depthWriteEnable = false };
	auto vertexShader = Fwog::Shader(Fwog::PipelineStage::VERTEX_SHADER, ProjectApplication::LoadFile(vert_line_shader_path));
	auto fragmentShader = Fwog::Shader(Fwog::PipelineStage::FRAGMENT_SHADER, ProjectApplication::LoadFile(frag_line_shader_path));

	return Fwog::GraphicsPipeline{ {
	  .vertexShader = &vertexShader,
	  .fragmentShader = &fragmentShader,
	  .inputAssemblyState = primDescs,
	  .vertexInputState = {inputDescs},
	  .rasterizationState = {.cullMode = Fwog::CullMode::NONE},
	  .depthState = {.depthTestEnable = false, .depthWriteEnable = false},
	} };
}


static Fwog::GraphicsPipeline CreatePipelineTextured()
{
	// Specify our two vertex attributes: position and color.
	// Positions are 3x float, so we will use R32G32B32_FLOAT like we would in Vulkan.
	static constexpr auto sceneInputBindingDescs = std::array{
	  Fwog::VertexInputBindingDescription{
			// color
			.location = 0,
			.binding = 0,
			.format = Fwog::Format::R32G32B32_FLOAT,
			.offset = offsetof(Primitives::Vertex, position),
		  },
		  Fwog::VertexInputBindingDescription{
			// normal
			.location = 1,
			.binding = 0,
			.format = Fwog::Format::R32G32B32_FLOAT,
			.offset = offsetof(Primitives::Vertex, normal),
		  },
		  Fwog::VertexInputBindingDescription{
			// texcoord
			.location = 2,
			.binding = 0,
			.format = Fwog::Format::R32G32_FLOAT,
			.offset = offsetof(Primitives::Vertex, uv),
		  },
	};

	auto inputDescs = sceneInputBindingDescs;
	auto primDescs = Fwog::InputAssemblyState{ Fwog::PrimitiveTopology::TRIANGLE_LIST };

	auto vertexShader = Fwog::Shader(Fwog::PipelineStage::VERTEX_SHADER, ProjectApplication::LoadFile(vert_shader_path));
	auto fragmentShader = Fwog::Shader(Fwog::PipelineStage::FRAGMENT_SHADER, ProjectApplication::LoadFile(frag_texture_shader_path));

	return Fwog::GraphicsPipeline{ {
	  .vertexShader = &vertexShader,
	  .fragmentShader = &fragmentShader,
	  .inputAssemblyState = primDescs,
	  .vertexInputState = {inputDescs},
	  .depthState = {.depthTestEnable = true, .depthWriteEnable = true, .depthCompareOp = Fwog::CompareOp::LESS},
	} };
}


static Fwog::GraphicsPipeline CreatePipelineColoredIndex()
{
	//To be honest since this is the same as the others I might as well just pass in the shader paths instead

	static constexpr auto sceneInputBindingDescs = std::array{
		Fwog::VertexInputBindingDescription{
			// position
			.location = 0,
			.binding = 0,
			.format = Fwog::Format::R32G32B32_FLOAT,
			.offset = offsetof(Primitives::Vertex, position),
	},
	Fwog::VertexInputBindingDescription{
			// normal
			.location = 1,
			.binding = 0,
			.format = Fwog::Format::R32G32B32_FLOAT,
			.offset = offsetof(Primitives::Vertex, normal),
	},
	Fwog::VertexInputBindingDescription{
			// texcoord
			.location = 2,
			.binding = 0,
			.format = Fwog::Format::R32G32_FLOAT,
			.offset = offsetof(Primitives::Vertex, uv),
	},
	};

	auto inputDescs = sceneInputBindingDescs;
	auto primDescs = Fwog::InputAssemblyState{ Fwog::PrimitiveTopology::TRIANGLE_LIST };

	auto vertexShader = Fwog::Shader(Fwog::PipelineStage::VERTEX_SHADER, ProjectApplication::LoadFile(vert_indexed_shader_path));
	auto fragmentShader = Fwog::Shader(Fwog::PipelineStage::FRAGMENT_SHADER, ProjectApplication::LoadFile(frag_phong_shader_path));

	return Fwog::GraphicsPipeline{ {
			.vertexShader = &vertexShader,
			.fragmentShader = &fragmentShader,
			.inputAssemblyState = primDescs,
			.vertexInputState = {inputDescs},
			.depthState = {.depthTestEnable = true, .depthWriteEnable = true, .depthCompareOp = Fwog::CompareOp::LESS},
		} };

}

Fwog::GraphicsPipeline ProjectApplication::CreatePipelineSkybox()
{
	static constexpr auto sceneInputBindingDescs = std::array{
		Fwog::VertexInputBindingDescription{
			// position
			.location = 0,
			.binding = 0,
			.format = Fwog::Format::R32G32B32_FLOAT,
			.offset = 0
	}};

	auto inputDescs = sceneInputBindingDescs;
	auto primDescs = Fwog::InputAssemblyState{ Fwog::PrimitiveTopology::TRIANGLE_LIST };

	auto vertexShader = Fwog::Shader(Fwog::PipelineStage::VERTEX_SHADER, ProjectApplication::LoadFile(vert_skybox_shader_path));
	auto fragmentShader = Fwog::Shader(Fwog::PipelineStage::FRAGMENT_SHADER, ProjectApplication::LoadFile(frag_skybox_shader_path));

	return Fwog::GraphicsPipeline{ {
			.vertexShader = &vertexShader,
			.fragmentShader = &fragmentShader,
			.inputAssemblyState = primDescs,
			.vertexInputState = {inputDescs},
			.depthState = {.depthTestEnable = true, .depthWriteEnable = true, .depthCompareOp = Fwog::CompareOp::LESS_OR_EQUAL},
		} };

}


void ProjectApplication::AddDebugDrawLine(glm::vec3 ptA, glm::vec3 ptB, glm::vec3 color) {

	
	std::array<glm::vec3, 2> linePos{ptA, ptB};
	std::array<glm::vec3, 2> colorPos{color, color};
  vertex_buffer_draw_lines.value().SubData(linePos, sizeof(glm::vec3) * curr_num_draw_points);
  vertex_buffer_draw_colors.value().SubData(colorPos, sizeof(glm::vec3) * curr_num_draw_points);

  curr_num_draw_points += 2;
}



void ProjectApplication::ClearLines()
{
	curr_num_draw_points = 0;
}


void ProjectApplication::DrawLineAABB(Collision::AABB const& aabb, glm::vec3 boxColor)
{
	//It is ok to recalculate face points from the extents despite performance cost because this draw function is optional

	//back : negative z-axis
	//forward : positive z-axis 
	glm::vec3 backface_down_left = glm::vec3(aabb.center.x - aabb.halfExtents.x, aabb.center.y - aabb.halfExtents.y, aabb.center.z - aabb.halfExtents.z);
	glm::vec3 backface_down_right = glm::vec3(aabb.center.x + aabb.halfExtents.x, aabb.center.y - aabb.halfExtents.y, aabb.center.z - aabb.halfExtents.z);
	glm::vec3 backface_up_left = glm::vec3(aabb.center.x - aabb.halfExtents.x, aabb.center.y + aabb.halfExtents.y, aabb.center.z - aabb.halfExtents.z);
	glm::vec3 backface_up_right = glm::vec3(aabb.center.x + aabb.halfExtents.x, aabb.center.y + aabb.halfExtents.y, aabb.center.z - aabb.halfExtents.z);

	glm::vec3 frontface_down_left = glm::vec3(aabb.center.x - aabb.halfExtents.x, aabb.center.y - aabb.halfExtents.y, aabb.center.z + aabb.halfExtents.z);
	glm::vec3 frontface_down_right = glm::vec3(aabb.center.x + aabb.halfExtents.x, aabb.center.y - aabb.halfExtents.y, aabb.center.z + aabb.halfExtents.z);
	glm::vec3 frontface_up_left = glm::vec3(aabb.center.x - aabb.halfExtents.x, aabb.center.y + aabb.halfExtents.y, aabb.center.z + aabb.halfExtents.z);
	glm::vec3 frontface_up_right = glm::vec3(aabb.center.x + aabb.halfExtents.x, aabb.center.y + aabb.halfExtents.y, aabb.center.z + aabb.halfExtents.z);


	//Back face
	AddDebugDrawLine(backface_down_left, backface_down_right, boxColor);
	AddDebugDrawLine(backface_down_left, backface_up_left, boxColor);
	AddDebugDrawLine(backface_up_left, backface_up_right, boxColor);
	AddDebugDrawLine(backface_up_right, backface_down_right, boxColor);

	//Front Face
	AddDebugDrawLine(frontface_down_left, frontface_down_right, boxColor);
	AddDebugDrawLine(frontface_down_left, frontface_up_left, boxColor);
	AddDebugDrawLine(frontface_up_left, frontface_up_right, boxColor);
	AddDebugDrawLine(frontface_up_right, frontface_down_right, boxColor);

	//Left Face
	AddDebugDrawLine(backface_down_left, frontface_down_left, boxColor);
	AddDebugDrawLine(backface_up_left, frontface_up_left, boxColor);


	//Right Face
	AddDebugDrawLine(backface_down_right, frontface_down_right, boxColor);
	AddDebugDrawLine(backface_up_right, frontface_up_right, boxColor);


}


void ProjectApplication::DrawLineSphere(Collision::Sphere const& sphere, glm::vec3 sphereColor)
{

	//http://www.songho.ca/opengl/gl_sphere.html

	constexpr uint32_t num_stacks = 16;
	constexpr uint32_t num_slices = 16;

	static glm::vec3 temp_horizontal{ 0.f };
	static glm::vec3 temp_vert{ 0.f };
	static glm::vec3 normal{ 0.f };


	glm::vec3 position_horizontal = sphere.center;
	glm::vec3 position_vert = sphere.center;
	glm::vec3 local_origin = sphere.center;

	float radius = sphere.radius;


	for (uint32_t curr_stack{0}; curr_stack <= num_stacks; ++curr_stack)
	{
		float theta = static_cast<float>(curr_stack * PI / num_stacks);
		float sin_theta = sin(theta);
		float cos_theta = cos(theta);

		for (uint32_t curr_slice{0}; curr_slice <= num_slices; ++curr_slice)
		{
			float phi = static_cast<float>(curr_slice * 2 * PI / num_slices);
			float sin_phi = sin(phi);
			float cos_phi = cos(phi);

			normal.x = cos_phi * sin_theta;
			normal.y = cos_theta;
			normal.z = sin_phi * sin_theta;

			temp_horizontal = position_horizontal;
			temp_vert = position_vert;

			position_horizontal.x = radius * normal.x;
			position_horizontal.y = radius * normal.y;
			position_horizontal.z = radius * normal.z;

			position_horizontal = local_origin + position_horizontal;

			position_vert.y = radius * normal.x;
			position_vert.x = radius * normal.y;
			position_vert.z = radius * normal.z;

			position_vert = local_origin + position_vert;

			AddDebugDrawLine(position_horizontal, temp_horizontal, sphereColor);
			AddDebugDrawLine(position_vert, temp_vert, sphereColor);
		}
	}
}

void ProjectApplication::AfterCreatedUiContext()
{

}

void ProjectApplication::BeforeDestroyUiContext()
{

}



void ProjectApplication::CreateSkybox()
{
	//So I don't forgor: https://github.com/fendevel/Guide-to-Modern-OpenGL-Functions#uploading-cube-maps


	using namespace Fwog;

	int32_t textureWidth, textureHeight, textureChannels;
	constexpr int32_t expected_num_channels = 4;
	

	unsigned char* textureData_skybox_front = stbi_load("data/skybox/front.png", &textureWidth, &textureHeight, &textureChannels, expected_num_channels);
	assert(textureData_skybox_front);

	unsigned char* textureData_skybox_back = stbi_load("data/skybox/back.png", &textureWidth, &textureHeight, &textureChannels, expected_num_channels);
	assert(textureData_skybox_back);

	unsigned char* textureData_skybox_left = stbi_load("data/skybox/left.png", &textureWidth, &textureHeight, &textureChannels, expected_num_channels);
	assert(textureData_skybox_left);

	unsigned char* textureData_skybox_right = stbi_load("data/skybox/right.png", &textureWidth, &textureHeight, &textureChannels, expected_num_channels);
	assert(textureData_skybox_right);


	unsigned char* textureData_skybox_up = stbi_load("data/skybox/up.png", &textureWidth, &textureHeight, &textureChannels, expected_num_channels);
	assert(textureData_skybox_up);

	unsigned char* textureData_skybox_down = stbi_load("data/skybox/down.png", &textureWidth, &textureHeight, &textureChannels, expected_num_channels);
	assert(textureData_skybox_down);

	//https://www.khronos.org/opengl/wiki/Cubemap_Texture
	const uint32_t right_id = 0;
	const uint32_t left_id = 1;
	const uint32_t up_id = 2;
	const uint32_t down_id = 3;

	//front instead of forwrad to match the cubemap naming
	const uint32_t front_id = 4;
	const uint32_t back_id = 5;

	uint32_t num_cube_faces = 6;

	Fwog::TextureCreateInfo createInfo{
		.imageType = ImageType::TEX_CUBEMAP,
		.format = Fwog::Format::R8G8B8A8_SRGB,
		.extent = { static_cast<uint32_t>(textureWidth), static_cast<uint32_t>(textureHeight)},
		.mipLevels =  uint32_t(1 + floor(log2(glm::max(textureWidth, textureHeight)))),
		.arrayLayers = 1,
		.sampleCount = SampleCount::SAMPLES_1,
	};

	skybox_texture = Fwog::Texture(createInfo);

	//groundAlbedo = Fwog::CreateTexture2DMip({ Fwog::ImageType::TEX_CUBEMAP, static_cast<uint32_t>(textureWidth), static_cast<uint32_t>(textureHeight) }, Fwog::Format::R8G8B8A8_SRGB, uint32_t(1 + floor(log2(glm::max(textureWidth, textureHeight)))));

	auto upload_face = [&](uint32_t curr_face, unsigned char* texture_pixel_data)
	{
		Fwog::TextureUpdateInfo updateInfo{ .dimension = Fwog::UploadDimension::THREE,
			.level = 0,
			.offset = {.depth = curr_face},
			.size = {static_cast<uint32_t>(textureWidth), static_cast<uint32_t>(textureHeight), 1},
			.format = Fwog::UploadFormat::RGBA,
			.type = Fwog::UploadType::UBYTE,
			.pixels = texture_pixel_data };
		skybox_texture.value().SubImage(updateInfo);

		stbi_image_free(texture_pixel_data);
	};


	upload_face(right_id, textureData_skybox_right);
	upload_face(left_id, textureData_skybox_left);
	upload_face(up_id, textureData_skybox_up);
	upload_face(down_id, textureData_skybox_down);
	upload_face(front_id, textureData_skybox_front);
	upload_face(back_id, textureData_skybox_back);

	skybox_texture.value().GenMipmaps();
	pipeline_skybox = CreatePipelineSkybox();
	vertex_buffer_skybox.emplace(Primitives::skybox_vertices);
}

void ProjectApplication::LoadGroundPlane()
{
	//to do: better texture loading systems. this can break so easily and its jank as hell
	int32_t textureWidth, textureHeight, textureChannels;
	constexpr int32_t expected_num_channels = 4;
	unsigned char* textureData = stbi_load("data/textures/GroundForest003_Flat.png", &textureWidth, &textureHeight, &textureChannels, expected_num_channels);
	assert(textureData);
	groundAlbedo = Fwog::CreateTexture2DMip({ static_cast<uint32_t>(textureWidth), static_cast<uint32_t>(textureHeight) }, Fwog::Format::R8G8B8A8_SRGB,uint32_t(1 + floor(log2(glm::max(textureWidth, textureHeight)))));
	Fwog::TextureUpdateInfo updateInfo{ .dimension = Fwog::UploadDimension::TWO,
		.level = 0,
		.offset = {},
		.size = {static_cast<uint32_t>(textureWidth), static_cast<uint32_t>(textureHeight), 1},
		.format = Fwog::UploadFormat::RGBA,
		.type = Fwog::UploadType::UBYTE,
		.pixels = textureData };
	groundAlbedo.value().SubImage(updateInfo);
	groundAlbedo.value().GenMipmaps();
	stbi_image_free(textureData);

	glm::mat4 modelPlane = glm::mat4(1.0f);
	modelPlane = glm::scale(modelPlane, planeScale);
	ObjectUniforms planeUniform;
	planeUniform.model = modelPlane;
	objectBufferPlane = Fwog::TypedBuffer<ObjectUniforms>(Fwog::BufferStorageFlag::DYNAMIC_STORAGE);
	objectBufferPlane.value().SubData(planeUniform, 0);

	vertex_buffer_plane.emplace(Primitives::plane_vertices);
	index_buffer_plane.emplace(Primitives::plane_indices);

}

void ProjectApplication::LoadBuffers()
{
	//Creating world axis stuff
	{
		glm::vec3 worldUpFinal = worldOrigin + (worldUp)*axisScale;
		glm::vec3 worldForwardFinal = worldOrigin + (worldForward)*axisScale;
		glm::vec3 worldRightFinal = worldOrigin + (worldRight)*axisScale;

		std::array<glm::vec3, num_points_world_axis> axisPos{ worldOrigin, worldUpFinal, worldOrigin, worldForwardFinal, worldOrigin, worldRightFinal };
		std::array<glm::vec3, num_points_world_axis> axisColors{ worldUpColor,
			worldUpColor,
			worldForwardColor,
			worldForwardColor,
			worldRightcolor,
			worldRightcolor };

		vertex_buffer_pos_line = Fwog::TypedBuffer<glm::vec3>(axisPos);
		vertex_buffer_color_line = Fwog::TypedBuffer<glm::vec3>(axisColors);
	}

	//Create collision line buffer
	{
		//Doesn't matter what the default initalization is since we only draw the valid points
		std::array<glm::vec3, max_num_draw_points> linePts{};

		vertex_buffer_draw_lines = Fwog::TypedBuffer<glm::vec3>(max_num_draw_points, Fwog::BufferStorageFlag::DYNAMIC_STORAGE);
		vertex_buffer_draw_colors = Fwog::TypedBuffer<glm::vec3>(max_num_draw_points, Fwog::BufferStorageFlag::DYNAMIC_STORAGE);
		vertex_buffer_draw_lines.value().SubData(linePts, 0);
		vertex_buffer_draw_colors.value().SubData(linePts, 0 );
	}



	//Camera Settings
	{
		static glm::vec3 camPos = glm::vec3(3.0f, 3.0f, 3.0f);
		static glm::vec3 origin = glm::vec3(0.0f, 0.0f, 0.0f);
		static glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
		static glm::mat4 view = glm::lookAt(camPos, origin, up);
		static glm::mat4 proj = glm::perspective(PI / 2.0f, 1.6f, nearPlane, farPlane);
		static glm::mat4 viewProj = proj * view;

		globalStruct.viewProj = viewProj;
		globalStruct.eyePos = camPos;

		globalUniformsBuffer = Fwog::TypedBuffer<GlobalUniforms>(Fwog::BufferStorageFlag::DYNAMIC_STORAGE);
		globalUniformsBuffer_skybox = Fwog::TypedBuffer<GlobalUniforms>(Fwog::BufferStorageFlag::DYNAMIC_STORAGE);
		globalUniformsBuffer.value().SubData(globalStruct, 0);
		globalUniformsBuffer_skybox.value().SubData(globalStruct, 0);
	}


	//Creating ground plane
	LoadGroundPlane();

	//Creating the aircraft
	{
		Utility::LoadModelFromFile(scene_aircraft, "data/models/AircraftPropeller.glb", glm::mat4{ 1.0f }, true);
		ObjectUniforms aircraftUniform;
		aircraftUniform.model = glm::mat4(1.0f);
		aircraftUniform.model = glm::translate(aircraftUniform.model, aircraftPos);
		aircraftUniform.model = glm::scale(aircraftUniform.model, aircraftScale);

		aircraft_box_collider.center = aircraftPos;
		aircraft_box_collider.halfExtents = aircraftScale * aircraftCollisionScale;

		aircraft_sphere_collider.radius = aircraft_sphere_radius;
		aircraft_sphere_collider.center = aircraftPos;

		aircraftUniform.color = aircraftColor;
		objectBufferaircraft = Fwog::TypedBuffer<ObjectUniforms>(Fwog::BufferStorageFlag::DYNAMIC_STORAGE);
		object_buffer_propeller = Fwog::TypedBuffer<ObjectUniforms>(Fwog::BufferStorageFlag::DYNAMIC_STORAGE);
		objectBufferaircraft.value().SubData(aircraftUniform, 0);

		aircraftUniform.color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

		object_buffer_propeller.value().SubData(aircraftUniform, 0);
	}

	//Load the actual scene vertices here
	Utility::LoadModelFromFile(scene_collectable, "data/models/collectableSphere.glb", glm::mat4{ 1.0f }, true);
	collectableObjectBuffers  = Fwog::TypedBuffer<ObjectUniforms>(max_num_collectables, Fwog::BufferStorageFlag::DYNAMIC_STORAGE);

	building_vertex_buffer.emplace(Primitives::cube_vertices);
	building_index_buffer.emplace(Primitives::cube_indices);

}

void ProjectApplication::CreateGroundChunks()
{
	//Makes the chunks surrounding relative to a center
	constexpr glm::vec3 center_position{0.0f, 0.0f, 0.0f};
	constexpr glm::vec3 center_chunk_scale{planeScale};

	//length of the world unit vectors (can also be dot product) allows not having to worry about what direction is forward/front
	glm::vec3 forward_chunk_offset{center_position.x, center_position.y, center_position.z + glm::length(worldForward) * center_chunk_scale.z};
	glm::vec3 back_chunk_offset{center_position.x, center_position.y, center_position.z - glm::length(worldForward) * center_chunk_scale.z};
	glm::vec3 right_chunk_offset{center_position.x + glm::length(worldRight) * center_chunk_scale.x, center_position.y, center_position.z};
	glm::vec3 left_chunk_offset{center_position.x - glm::length(worldRight) * center_chunk_scale.x, center_position.y, center_position.z};

	glm::vec3 forward_right_chunk_offset{center_position.x + glm::length(worldRight)  * center_chunk_scale.x, center_position.y, center_position.z + glm::length(worldForward) * center_chunk_scale.z};
	glm::vec3 forward_left_chunk_offset{center_position.x - glm::length(worldRight)  * center_chunk_scale.x, center_position.y, center_position.z + glm::length(worldForward) * center_chunk_scale.z};

	glm::vec3 back_right_chunk_offset{center_position.x + glm::length(worldRight) * center_chunk_scale.x, center_position.y, center_position.z - glm::length(worldForward) * center_chunk_scale.z};
	glm::vec3 back_left_chunk_offset{center_position.x - glm::length(worldRight) * center_chunk_scale.x, center_position.y, center_position.z - glm::length(worldForward) * center_chunk_scale.z};



	//I heard rumors of these things called 'constructors'...
	//Not as clumsy or random as a lambda; an elegant weapon for a more civilized age...
	auto create_chunk = [&](glm::vec3 chunk_center) {

		grond_chunk_list.emplace_back();
		ground_chunk& chunk = grond_chunk_list.back();
		chunk.ground_center = chunk_center;
		glm::mat4 model(1.0f);
		model = glm::translate(model, chunk_center);
		model = glm::scale(model, planeScale);
		chunk.ground_uniform.model = model;
		chunk.object_buffer =  Fwog::TypedBuffer<ObjectUniforms>(Fwog::BufferStorageFlag::DYNAMIC_STORAGE);
		chunk.object_buffer.value().SubData(chunk.ground_uniform, 0);
	};

	create_chunk(forward_chunk_offset);
	create_chunk(back_chunk_offset);
	create_chunk(right_chunk_offset);
	create_chunk(left_chunk_offset);
	create_chunk(forward_right_chunk_offset);
	create_chunk(forward_left_chunk_offset);
	create_chunk(back_right_chunk_offset);
	create_chunk(back_left_chunk_offset);
}

void ProjectApplication::AddCollectable(glm::vec3 position, glm::vec3 scale, glm::vec3 color)
{
	ObjectUniforms collectableUniform;
	collectableUniform.model = glm::mat4(1.0f);
	collectableUniform.model = glm::translate(collectableUniform.model, position);
	collectableUniform.model = glm::scale(collectableUniform.model, scale);
	collectableUniform.color = glm::vec4(color, 1.0f);

	collectableObjectBuffers.value().SubData(collectableUniform, sizeof(collectableUniform) * collectableList.size());
	collectableList.emplace_back(position, scale, false);
}


void ProjectApplication::LoadCollectables()
{

	//Add placeholder collectables
	constexpr float start_forward_pos = 120.0f;
	constexpr float start_up_pos = 100.0f;
	constexpr float forward_distance_offset = 30.0f;
	constexpr float up_distance_offset = 25.0f;

	constexpr size_t num_collectables = 10;
	//uniform scaling so...
	constexpr float collectable_scale = 4.0f;
	for (size_t i = 0; i < num_collectables; ++i)
	{
		AddCollectable(glm::vec3(0.0f, start_up_pos + up_distance_offset * i, start_forward_pos + i * forward_distance_offset), glm::vec3(collectable_scale, collectable_scale, collectable_scale));
	}

}

void ProjectApplication::LoadBuildings()
{
	constexpr size_t num_buildings = 2;
	constexpr float distance_offset = 30.0f;
	constexpr float y_scale_offset = 250.0f;

	glm::vec3 curr_center{50.0f, 0.0f, 100.0f};
	glm::vec3 curr_scale{10.0f, 100.0f, 10.0f};

	for (size_t i = 0; i < num_buildings; ++i)
	{


		buildingObjectList.emplace_back(curr_center + worldRight * (distance_offset * i) + worldForward * (distance_offset * i), glm::vec3(curr_scale.x, curr_scale.y + y_scale_offset * i,  curr_scale.z));


		//Just the starting building idea
		glm::mat4 model(1.0f);
		model = glm::translate(model, buildingObjectList[i].building_center);
		model = glm::scale(model, buildingObjectList[i].building_scale);

		ObjectUniforms temp;
		temp.model = model;
		temp.color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

		buildingObjectList[i].object_buffer = Fwog::TypedBuffer<ObjectUniforms>(Fwog::BufferStorageFlag::DYNAMIC_STORAGE);
		buildingObjectList[i].object_buffer.value().SubData(temp, 0);
	}
}



bool ProjectApplication::Load()
{

	SetWindowTitle("Plane Game");

	// Initialize SoLoud (automatic back-end selection)

	//To Do: Like actually use the res properly
	SoLoud::result init = soloud.init();
	SoLoud::result res = sample.load("data/sounds/start.wav"); // Load a wave file
	sample.setVolume(0.75);
	res = plane_flying_sfx.load("data/sounds/planeFlying.wav");
	plane_speedup_sfx.load("data/sounds/planeFlying.wav");

	background_music.load("data/sounds/backgroundMusic.wav");
	background_music.setVolume(0.30);

	level_editor_music.load("data/sounds/levelEditorMusic.wav");
	level_editor_music.setVolume(0.70);

	collectable_pickup_sfx.load("data/sounds/collectablePlaceholderSound.wav");

	soloud.setGlobalVolume(soloud_volume);

	//Creating pipelines

	pipeline_flat = CreatePipeline();
	pipeline_lines = CreatePipelineLines();
	pipeline_textured = CreatePipelineTextured();
	pipeline_colored_indexed = CreatePipelineColoredIndex();

	LoadBuffers();
	CreateGroundChunks();
	CreateSkybox();

	//LoadCollectables();
	//LoadBuildings();

	//Play sfx
	plane_flying_sfx.setLooping(true);
	plane_flying_sfx.setVolume(0.40);


	soloud.play(background_music);

	StartLevel();


	return true;
}


void ProjectApplication::ResetLevel()
{
	collectableList.clear();
	buildingObjectList.clear();
	StartLevel();
}

void ProjectApplication::StartLevel()
{
	sample.stop();
	plane_flying_sfx_handle = soloud.play(plane_flying_sfx);

	LoadBuildings();
	LoadCollectables();

	render_plane = true;
	aircraftPos = aircarftStartPos;
	aircraft_body = PhysicsBody{aircraft_starting_speed, aircraft_starting_direction_vector};

	ObjectUniforms aircraftUniform;
	aircraftUniform.model = glm::mat4(1.0f);
	aircraftUniform.model = glm::translate(aircraftUniform.model, aircraftPos);
	aircraftUniform.model = glm::scale(aircraftUniform.model, aircraftScale);

	aircraft_box_collider.center = aircraftPos;
	aircraft_box_collider.halfExtents = aircraftScale * aircraftCollisionScale;

	aircraft_sphere_collider.radius = aircraft_sphere_radius;
	aircraft_sphere_collider.center = aircraftPos;

	aircraftUniform.color = aircraftColor;
	objectBufferaircraft.value().SubData(aircraftUniform, 0);
}


//Free roam camera controls
void ProjectApplication::UpdateEditorCamera(double dt)
{
	//First person camera controls
	static bool first_person_camera_mode = true;
	static bool first_person_button_down = false;


	static constexpr float editor_camera_speed_scale = 30.0f;

	if (!first_person_button_down && IsKeyPressed(GLFW_KEY_GRAVE_ACCENT))
	{
		first_person_camera_mode = !first_person_camera_mode;
		first_person_button_down = true;

		editorCamera.forward = glm::normalize(editorCamera.target - editorCamera.position);
		editorCamera.right = glm::cross(editorCamera.forward, worldUp);
		editorCamera.up = glm::cross(editorCamera.right, editorCamera.forward);

	}
	else if (first_person_button_down && IsKeyRelease(GLFW_KEY_GRAVE_ACCENT))
	{
		first_person_button_down = false;
	}


	if (first_person_camera_mode)
	{
		if (IsKeyPressed(GLFW_KEY_W))
		{
			editorCamera.position = editorCamera.position + editorCamera.forward * static_cast<float>(dt) * editor_camera_speed_scale;
			editorCamera.target = editorCamera.target + editorCamera.forward * static_cast<float>(dt) * editor_camera_speed_scale;
		}

		if (IsKeyPressed(GLFW_KEY_S))
		{
		
			editorCamera.position = editorCamera.position - editorCamera.forward * static_cast<float>(dt) * editor_camera_speed_scale;
			editorCamera.target = editorCamera.target - editorCamera.forward * static_cast<float>(dt) * editor_camera_speed_scale;
		}

		if (IsKeyPressed(GLFW_KEY_A))
		{
			editorCamera.position = editorCamera.position - editorCamera.right * static_cast<float>(dt) * editor_camera_speed_scale;
			editorCamera.target = editorCamera.target - editorCamera.right * static_cast<float>(dt) * editor_camera_speed_scale;
		}

		if (IsKeyPressed(GLFW_KEY_D))
		{
			editorCamera.position = editorCamera.position + editorCamera.right * static_cast<float>(dt) * editor_camera_speed_scale;
			editorCamera.target = editorCamera.target + editorCamera.right * static_cast<float>(dt) * editor_camera_speed_scale;
		}

		if (IsKeyPressed(GLFW_KEY_Q))
		{
			editorCamera.position = editorCamera.position + editorCamera.up * static_cast<float>(dt) * editor_camera_speed_scale;
			editorCamera.target = editorCamera.target + editorCamera.up * static_cast<float>(dt) * editor_camera_speed_scale;
		}

		if (IsKeyPressed(GLFW_KEY_E))
		{
			editorCamera.position = editorCamera.position - editorCamera.up * static_cast<float>(dt) * editor_camera_speed_scale;
			editorCamera.target = editorCamera.target - editorCamera.up * static_cast<float>(dt) * editor_camera_speed_scale;
		}

		if (IsMouseKeyPressed(GLFW_MOUSE_BUTTON_1))
		{
			MouseRaycast(editorCamera);
		}


		//To Do: Make this cneter of the screen
		static double mouseX = windowWidth / 2;
		static double mouseY = windowHeight / 2;
		static double lastX = windowWidth / 2;
		static double lastY =  windowHeight / 2;
		GetMousePosition(mouseX, mouseY);

		static float yawDegrees = 90.0f;
		static float pitchDegrees = 0.0f;

		static bool firstMouseRotate = true;

		if (!(mouseX == std::numeric_limits<double>::infinity() ||
			mouseX == -std::numeric_limits<double>::infinity() ||
			mouseY == std::numeric_limits<double>::infinity() ||
			mouseY == -std::numeric_limits<double>::infinity()))
		{
			if (IsMouseKeyPressed(GLFW_MOUSE_BUTTON_2)) //Right click
 			{
				//Prototype from https://learnopengl.com/code_viewer_gh.php?code=src/1.getting_started/7.3.camera_mouse_zoom/camera_mouse_zoom.cpp first

				//Relative to entire window for now

				if (firstMouseRotate)
				{
					lastX = mouseX;
					lastY = mouseY;
					firstMouseRotate = false;
				}


				float xoffset = mouseX - lastX;
				float yoffset = mouseY - lastY;
				lastX = mouseX;
				lastY = mouseY;


				float sensitivity = 1.0f;
				xoffset *= sensitivity;
				yoffset *= sensitivity;


				yawDegrees += xoffset;
				pitchDegrees += yoffset;

				// make sure that when pitch is out of bounds, screen doesn't get flipped
				if (pitchDegrees > 89.0f)
					pitchDegrees = 89.0f;
				if (pitchDegrees < -89.0f)
					pitchDegrees = -89.0f;

				editorCamera.forward.x = cos(glm::radians(yawDegrees)) * cos(glm::radians(pitchDegrees));
				editorCamera.forward.y = sin(glm::radians(pitchDegrees));
				editorCamera.forward.z = sin(glm::radians(yawDegrees)) * cos(glm::radians(pitchDegrees));
				editorCamera.forward = glm::normalize(editorCamera.forward);

				editorCamera.right = glm::cross(editorCamera.forward, worldUp);
				editorCamera.up = glm::cross(editorCamera.right, editorCamera.forward);

				editorCamera.target = editorCamera.position + editorCamera.forward;
			}
			else
			{
				firstMouseRotate = true;
			}
		}
	}

}

float ProjectApplication::lerp(float start, float end, float t)
{
	return start + (end - start) * t;

}

void ProjectApplication::Update(double dt)
{
	static bool gameover_button_down = false;
	if (!gameover_button_down && IsKeyPressed(GLFW_KEY_F))
	{
		gameover_button_down = true;
		if (curr_game_state == game_states::playing)
			curr_game_state = game_states::game_over;
		else if (curr_game_state == game_states::game_over)
			curr_game_state = game_states::playing;
	}
	else if (gameover_button_down && IsKeyRelease(GLFW_KEY_F))
	{
		gameover_button_down = false;
	}


	//Change of state
	if (prev_game_state != curr_game_state)
	{
		if (curr_game_state == game_states::game_over)
		{
			soloud.play(sample); 
			plane_flying_sfx.stop();
			render_plane = false;
		}
		else if (curr_game_state == game_states::playing)
		{
			if (prev_game_state == game_states::game_over)
			{
				ResetLevel();
			}
		}
		else if (curr_game_state == game_states::level_editor)
		{
			plane_flying_sfx.stop();
			background_music.stop();
			soloud.play(level_editor_music);

			editorCamera = gameplayCamera;

			editorCamera.forward = glm::normalize(editorCamera.target - editorCamera.position);
			editorCamera.right = glm::cross(editorCamera.forward, worldUp);
			editorCamera.up = glm::cross(editorCamera.right, editorCamera.forward);
		}

		prev_game_state = curr_game_state;
	}

	if (IsKeyPressed(GLFW_KEY_ESCAPE))
	{
		Close();
	}


	static bool wasKeyPressed_Editor = false;
	if (!wasKeyPressed_Editor && IsKeyPressed(GLFW_KEY_1))
	{
		wasKeyPressed_Editor = true;
		curr_game_state = game_states::level_editor;
	}
	else if (wasKeyPressed_Editor && IsKeyRelease(GLFW_KEY_1))
	{
		wasKeyPressed_Editor = false;
	}


	if (curr_game_state == game_states::level_editor)
	{
		static bool wasKeyPressed_Gameplay = false;
		if (!wasKeyPressed_Editor && IsKeyPressed(GLFW_KEY_2))
		{
			wasKeyPressed_Editor = true;
			curr_game_state = game_states::playing;
			level_editor_music.stop();
			soloud.play(background_music);
			plane_flying_sfx_handle = soloud.play(plane_flying_sfx);
		}
		else if (wasKeyPressed_Editor && IsKeyRelease(GLFW_KEY_2))
		{
			wasKeyPressed_Editor = false;
		}

		UpdateEditorCamera(dt);


		//Camera logic stuff
		ZoneScopedC(tracy::Color::Blue);

		glm::mat4 view = glm::lookAt(editorCamera.position, editorCamera.target, editorCamera.up);
		glm::mat4 proj = glm::perspective((PI / 2.0f), 1.6f, nearPlane, farPlane);
		glm::mat4 viewProj = proj * view;

		globalStruct.viewProj = viewProj;
		globalStruct.eyePos = editorCamera.position;
		
		globalUniformsBuffer.value().SubData(globalStruct, 0);
		
		glm::mat4 view_rot_only = glm::mat4(glm::mat3(view));
		globalStruct.viewProj = proj * view_rot_only;
		globalUniformsBuffer_skybox.value().SubData(globalStruct, 0);

	}


	if (curr_game_state == game_states::playing)
	{
		{
			//aircraft Inputs
			float dt_float = static_cast<float>(dt);
			float zoom_speed_level = 1.0f;

			aircraft_current_speed_scale = 1.0f;
			{
				ZoneScopedC(tracy::Color::Green);


				aircraft_body.propeller_angle_degrees = lerp(0.0f, 360.0f, elasped_propeller_t);
				elasped_propeller_t += propeller_revolutions_per_second * dt;
				if (aircraft_body.propeller_angle_degrees > 360.0f)
				{
					aircraft_body.propeller_angle_degrees = fmod(aircraft_body.propeller_angle_degrees, 360.0f);
					elasped_propeller_t -= 1.0f;
				}

				//aircraft_body.propeller_angle_degrees += propeller_angle_turning_degrees * dt;


				//To Do: Reset if after 360?


				aircraft_body.forward_vector = worldForward;
				aircraft_body.right_vector = worldRight;
				aircraft_body.up_vector = worldUp;

				aircraft_body.forward_vector = glm::vec3(aircraft_body.rotMatrix * glm::vec4(aircraft_body.forward_vector, 1.0f));
				aircraft_body.right_vector = glm::vec3(aircraft_body.rotMatrix * glm::vec4(aircraft_body.right_vector, 1.0f));
				aircraft_body.up_vector = glm::vec3(aircraft_body.rotMatrix * glm::vec4(aircraft_body.up_vector, 1.0f));

				soloud.setVolume(plane_flying_sfx_handle, 0.40);

				if (IsKeyPressed(GLFW_KEY_SPACE))
				{
					aircraft_current_speed_scale = aircraft_speedup_scale;
					zoom_speed_level = 1.02f;

					soloud.setVolume(plane_flying_sfx_handle, 0.80);
				}

				//Turning Left: Need to adjust both Roll and Velocity
				if (IsKeyPressed(GLFW_KEY_RIGHT))
				{

					//aircraft_body.rotMatrix = glm::rotate(aircraft_body.rotMatrix, aircraft_angle_turning_degrees * dt_float, aircraft_body.forward_vector);

					//aircraft_body.aircraft_angles_degrees.z += aircraft_angle_turning_degrees * dt_float;

					//aircraft_body.rotMatrix = glm::rotate(aircraft_body.rotMatrix, glm::radians(aircraft_angle_turning_degrees) * dt_float, aircraft_body.forward_vector);
					aircraft_body.rotMatrix = glm::rotate(aircraft_body.rotMatrix, glm::radians(aircraft_angle_turning_degrees) * dt_float, worldForward);
				}

				if (IsKeyPressed(GLFW_KEY_LEFT))
				{
					//aircraft_body.rotMatrix = glm::rotate(aircraft_body.rotMatrix, glm::radians(-aircraft_angle_turning_degrees) * dt_float, aircraft_body.forward_vector);
					aircraft_body.rotMatrix = glm::rotate(aircraft_body.rotMatrix, glm::radians(-aircraft_angle_turning_degrees) * dt_float, worldForward);
				}

				if (IsKeyPressed(GLFW_KEY_UP))
				{
					//aircraft_body.rotMatrix = glm::rotate(aircraft_body.rotMatrix, glm::radians(aircraft_angle_turning_degrees) * dt_float, aircraft_body.right_vector);
					aircraft_body.rotMatrix = glm::rotate(aircraft_body.rotMatrix, glm::radians(aircraft_angle_turning_degrees) * dt_float, worldRight);
					//aircraft_body.aircraft_angles_degrees.x += aircraft_angle_turning_degrees * dt_float;
				}

				if (IsKeyPressed(GLFW_KEY_DOWN))
				{
					//aircraft_body.rotMatrix = glm::rotate(aircraft_body.rotMatrix, glm::radians(-aircraft_angle_turning_degrees) * dt_float, aircraft_body.right_vector);
					aircraft_body.rotMatrix = glm::rotate(aircraft_body.rotMatrix, glm::radians(-aircraft_angle_turning_degrees) * dt_float, worldRight);

					//aircraft_body.aircraft_angles_degrees.x -= aircraft_angle_turning_degrees * dt_float;
				}

				//Rudders (The yaw)
				if (IsKeyPressed(GLFW_KEY_Q))
				{
					aircraft_body.rotMatrix = glm::rotate(aircraft_body.rotMatrix, glm::radians(aircraft_angle_turning_degrees) * dt_float, worldUp);
				}

				if (IsKeyPressed(GLFW_KEY_E))
				{
					aircraft_body.rotMatrix = glm::rotate(aircraft_body.rotMatrix, glm::radians(-aircraft_angle_turning_degrees) * dt_float, worldUp);

				}



				//Update position based off the velocity

				//Experimenting with another approach
				aircraftPos += aircraft_body.forward_vector * aircraft_body.current_speed * aircraft_current_speed_scale * dt_float;
				Collision::SyncSphere(aircraft_sphere_collider, aircraftPos);

				if (draw_player_colliders)
				{
					DrawLineSphere(aircraft_sphere_collider, glm::vec3(1.0f, 0.0, 0.0f));
				}
			}

			{
				//aircraft uniform buffer changes
				ZoneScopedC(tracy::Color::Orange);
				glm::mat4 model(1.0f);
				glm::mat4 propModel(1.0f);

				model = glm::translate(model, aircraftPos);
				propModel = glm::translate(propModel, aircraftPos);
				propModel = glm::rotate(propModel, glm::radians(aircraft_body.propeller_angle_degrees), aircraft_body.forward_vector);

				propModel *= aircraft_body.rotMatrix;
				model *= aircraft_body.rotMatrix;

				ObjectUniforms a(model, glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));
				objectBufferaircraft.value().SubData(a, 0);

				a.model = propModel;
				a.color = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);

				object_buffer_propeller.value().SubData(a, 0);
			}

			{
				//Camera logic stuff
				ZoneScopedC(tracy::Color::Blue);

				gameplayCamera.position = (aircraftPos - aircraft_body.forward_vector * 25.0f) + aircraft_body.up_vector * 10.0f;
				gameplayCamera.target =  aircraftPos + (aircraft_body.up_vector * 10.0f);
				gameplayCamera.up =  aircraft_body.up_vector;

				glm::mat4 view = glm::lookAt(gameplayCamera.position, gameplayCamera.target, gameplayCamera.up);
				glm::mat4 view_rot_only = glm::mat4(glm::mat3(view));

				//we dont actually have to recalculate this every frame yet but we might wanna adjust fov i guess
				glm::mat4 proj = glm::perspective((PI / 2.0f) * zoom_speed_level, 1.6f, nearPlane, farPlane);
				glm::mat4 viewProj = proj * view;

				globalStruct.viewProj = proj * view_rot_only;
				globalUniformsBuffer_skybox.value().SubData(globalStruct, 0);

				globalStruct.viewProj = viewProj;
				globalStruct.eyePos = gameplayCamera.position;

				globalUniformsBuffer.value().SubData(globalStruct, 0);
			}
		}




		//Collision Checks with collectable
		for (size_t i = 0; i < collectableList.size(); ++i)
		{
			auto& collectable = collectableList[i];

			if (collectable.isCollected)
			{
				continue;
			}

			if (draw_collectable_colliders)
			{
				DrawLineSphere(collectable.collider, glm::vec3(1.0f, 0.0, 0.0f));
			}


			if (Collision::sphereCollisionCheck(aircraft_sphere_collider, collectable.collider))
			{
				soloud.play(collectable_pickup_sfx);
				collectable.isCollected = true;

				//Because we use instancing. Decided to simply change the scale to set it to not render. Maybe there is a better way?
				ObjectUniforms temp;
				temp.model = glm::scale(temp.model, glm::vec3(0.0f, 0.0f, 0.0f));
				collectableObjectBuffers.value().SubData(temp, sizeof(ObjectUniforms) * i);
			}
		}

		//Collision checks with buildings
		for (auto const& building : buildingObjectList)
		{
			if (Collision::SphereAABBCollisionCheck(aircraft_sphere_collider, building.building_collider))
			{
				curr_game_state = game_states::game_over;
				break;
			}
		}
	}
}


void ProjectApplication::MouseRaycast(camera const& cam)
{
	ZoneScopedC(tracy::Color::Purple2);

	glm::mat4 proj = glm::perspective((PI / 2.0f), 1.6f, nearPlane, farPlane);
	glm::mat4 view = glm::lookAt(cam.position, cam.target, cam.up);
	//https://antongerdelan.net/opengl/raycasting.html

	//Viewport Coordinate
	static double mouseX = windowWidth / 2;
	static double mouseY = windowHeight / 2;
	GetMousePosition(mouseX, mouseY);

	//NDC Coordinate
	float x = (2.0f * mouseX) / windowWidth - 1.0f;
	float y = 1.0f - (2.0f * mouseY) / windowHeight;
	float z = 1.0f;
	glm::vec3 ray_nds = glm::vec3(x, y, z);

	//Homogenous Clip Coordinates
	glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);

	//View Space Coordinates
	glm::vec4 ray_eye = inverse(proj) * ray_clip;

	ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

	//World Coordinates
	glm::vec4 ray_wor = (inverse(view) * ray_eye);
	glm::vec3 ray_world_vec3 = glm::vec3(ray_wor.x, ray_wor.y, ray_wor.z);
	ray_world_vec3 = glm::normalize(ray_world_vec3);

	constexpr float debug_mouse_click_length = 5000.0f;


	std::cout << "Ray: " <<  (ray_world_vec3).x << ", " << (ray_world_vec3).y << ", " << (ray_world_vec3).z << "\n";

	std::vector<buildingObject*> accepted_colliders;

	for (auto& buillding : buildingObjectList)
	{
		if (Collision::RaycastCheck(cam.position, ray_world_vec3, buillding.building_collider))
		{
			accepted_colliders.push_back(&buillding);
		}
	}

	if (accepted_colliders.empty())
		return;

	std::sort<buildingObject*>(accepted_colliders.front(), accepted_colliders.back(), [&](auto const& lhs, auto const& rhs)
	{
			glm::vec3 lhs_vec = (lhs.building_center - cam.position);
			glm::vec3 rhs_vec = (rhs.building_center - cam.position);
			float dist_lhs = glm::dot(lhs_vec, lhs_vec);
			float dist_rhs = glm::dot(rhs_vec, rhs_vec);
			return dist_lhs < dist_rhs;
	});

	buildingObject& building = *accepted_colliders.front();

	//Yea we should create a function for this
	ObjectUniforms temp;
	glm::mat4 model(1.0f);
	model = glm::translate(model, building.building_center);
	model = glm::scale(model, building.building_scale);
	temp.color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	temp.model = model;
	building.object_buffer.value().SubData(temp, 0);

	//AddDebugDrawLine(cam.position, cam.position + ray_world_vec3 * debug_mouse_click_length, glm::vec3(0.0f, 0.0f, 1.0f));
}

void ProjectApplication::RenderScene()
{
	//RenderMousePick();

	ZoneScopedC(tracy::Color::Red);

	Fwog::BeginSwapchainRendering(Fwog::SwapchainRenderInfo{
	.viewport =
	  Fwog::Viewport{
		.drawRect{.offset = {0, 0}, .extent = {windowWidth, windowHeight}},
		.minDepth = 0.0f, .maxDepth = 1.0f
	  },
	.colorLoadOp = Fwog::AttachmentLoadOp::CLEAR,
	.clearColorValue = {skyColorFoggy.r, skyColorFoggy.g, skyColorFoggy.b, 1.0f},
	 .depthLoadOp = Fwog::AttachmentLoadOp::CLEAR,
	  .clearDepthValue = 1.0f
		});





	//Drawing a ground plane
	{
		Fwog::SamplerState ss;
		ss.minFilter = Fwog::Filter::LINEAR;
		ss.magFilter = Fwog::Filter::LINEAR;
		ss.mipmapFilter = Fwog::Filter::LINEAR;
		ss.addressModeU = Fwog::AddressMode::REPEAT;
		ss.addressModeV = Fwog::AddressMode::REPEAT;
		ss.anisotropy = Fwog::SampleCount::SAMPLES_16;
		auto nearestSampler = Fwog::Sampler(ss);

		Fwog::Cmd::BindGraphicsPipeline(pipeline_textured.value());
		Fwog::Cmd::BindUniformBuffer(0, globalUniformsBuffer.value());
		Fwog::Cmd::BindUniformBuffer(1, objectBufferPlane.value());
		Fwog::Cmd::BindSampledImage(0, groundAlbedo.value(), nearestSampler);
		Fwog::Cmd::BindVertexBuffer(0, vertex_buffer_plane.value(), 0, sizeof(Primitives::Vertex));
		Fwog::Cmd::BindIndexBuffer(index_buffer_plane.value(), Fwog::IndexType::UNSIGNED_SHORT);
		Fwog::Cmd::DrawIndexed(static_cast<uint32_t>(Primitives::plane_indices.size()), 1, 0, 0, 0);

		//Drawing the other ground planes
		for (auto const& ground : grond_chunk_list)
		{
			Fwog::Cmd::BindUniformBuffer(1, ground.object_buffer.value());
			Fwog::Cmd::BindSampledImage(0, groundAlbedo.value(), nearestSampler);
			Fwog::Cmd::BindVertexBuffer(0, vertex_buffer_plane.value(), 0, sizeof(Primitives::Vertex));
			Fwog::Cmd::BindIndexBuffer(index_buffer_plane.value(), Fwog::IndexType::UNSIGNED_SHORT);
			Fwog::Cmd::DrawIndexed(static_cast<uint32_t>(Primitives::plane_indices.size()), 1, 0, 0, 0);
		}
	}



	//Drawing buildings
	{
		for (auto const& building : buildingObjectList)
		{
			Fwog::Cmd::BindGraphicsPipeline(pipeline_flat.value());
			Fwog::Cmd::BindUniformBuffer(0, globalUniformsBuffer.value());
			Fwog::Cmd::BindUniformBuffer(1, building.object_buffer.value());
			Fwog::Cmd::BindVertexBuffer(0, building_vertex_buffer.value(), 0, sizeof(Primitives::Vertex));
			Fwog::Cmd::BindIndexBuffer(building_index_buffer.value(), Fwog::IndexType::UNSIGNED_SHORT);
			Fwog::Cmd::DrawIndexed(static_cast<uint32_t>(building_index_buffer.value().Size()), 1, 0, 0, 0);
		}
	}

	//Drawing the collectables
	{
		if (!collectableList.empty())
		{
			Fwog::Cmd::BindGraphicsPipeline(pipeline_colored_indexed.value());
			Fwog::Cmd::BindUniformBuffer(0, globalUniformsBuffer.value());
			Fwog::Cmd::BindStorageBuffer(1, collectableObjectBuffers.value());
			Fwog::Cmd::BindVertexBuffer(0, scene_collectable.meshes[0].vertexBuffer, 0, sizeof(Utility::Vertex));
			Fwog::Cmd::BindIndexBuffer(scene_collectable.meshes[0].indexBuffer,  Fwog::IndexType::UNSIGNED_INT);
			Fwog::Cmd::DrawIndexed(static_cast<uint32_t>(scene_collectable.meshes[0].indexBuffer.Size()) / sizeof(uint32_t), collectableList.size(), 0, 0, 0);
		}
	}

	//Drawing a aircraft
	if (render_plane)
	{
		Fwog::Cmd::BindGraphicsPipeline(pipeline_flat.value());
		Fwog::Cmd::BindUniformBuffer(0, globalUniformsBuffer.value());
		Fwog::Cmd::BindUniformBuffer(1, objectBufferaircraft.value());
		Fwog::Cmd::BindVertexBuffer(0, scene_aircraft.meshes[1].vertexBuffer, 0, sizeof(Utility::Vertex));
		Fwog::Cmd::BindIndexBuffer(scene_aircraft.meshes[1].indexBuffer, Fwog::IndexType::UNSIGNED_INT);
		Fwog::Cmd::DrawIndexed(static_cast<uint32_t>(scene_aircraft.meshes[1].indexBuffer.Size()) / sizeof(uint32_t), 1, 0, 0, 0);

		Fwog::Cmd::BindUniformBuffer(1, object_buffer_propeller.value());
		Fwog::Cmd::BindVertexBuffer(0, scene_aircraft.meshes[0].vertexBuffer, 0, sizeof(Utility::Vertex));
		Fwog::Cmd::BindIndexBuffer(scene_aircraft.meshes[0].indexBuffer, Fwog::IndexType::UNSIGNED_INT);
		Fwog::Cmd::DrawIndexed(static_cast<uint32_t>(scene_aircraft.meshes[0].indexBuffer.Size()) / sizeof(uint32_t), 1, 0, 0, 0);
	}

	//Drawing axis lines
	{
		Fwog::Cmd::BindGraphicsPipeline(pipeline_lines.value());
		Fwog::Cmd::BindUniformBuffer(0, globalUniformsBuffer.value());
		if (renderAxis)
		{
			Fwog::Cmd::BindVertexBuffer(0, vertex_buffer_pos_line.value(), 0, 3 * sizeof(float));
			Fwog::Cmd::BindVertexBuffer(1, vertex_buffer_color_line.value(), 0, 3 * sizeof(float));
			Fwog::Cmd::Draw(num_points_world_axis, 1, 0, 0);
		}

		// Drawing collision lines
        if (curr_num_draw_points != 0 && curr_num_draw_points < max_num_draw_points) 
		{
			Fwog::Cmd::BindVertexBuffer(0, vertex_buffer_draw_lines.value(), 0, 3 * sizeof(float));
            Fwog::Cmd::BindVertexBuffer(1, vertex_buffer_draw_colors.value(), 0, 3 * sizeof(float));
			Fwog::Cmd::Draw(curr_num_draw_points, 1, 0, 0);

			//Allows DrawLine to be called every frame without creating buffers. Should make new buffer  if want presistent lines ofc 
			ClearLines();
        }
	}


	//Drawing skybox last depth buffer
	{

		Fwog::SamplerState ss;
		ss.minFilter = Fwog::Filter::LINEAR;
		ss.magFilter = Fwog::Filter::LINEAR;
		ss.mipmapFilter = Fwog::Filter::LINEAR;
		ss.addressModeU = Fwog::AddressMode::REPEAT;
		ss.addressModeV = Fwog::AddressMode::REPEAT;
		ss.anisotropy = Fwog::SampleCount::SAMPLES_16;
		auto nearestSampler = Fwog::Sampler(ss);

		Fwog::Cmd::BindGraphicsPipeline(pipeline_skybox.value());
		Fwog::Cmd::BindUniformBuffer(0, globalUniformsBuffer_skybox.value());
		Fwog::Cmd::BindSampledImage(0, skybox_texture.value(), nearestSampler);
		Fwog::Cmd::BindVertexBuffer(0, vertex_buffer_skybox.value(), 0, 3 * sizeof(float));
		Fwog::Cmd::Draw(Primitives::skybox_vertices.size() / 3, 1, 0, 0);
	}




	Fwog::EndRendering();
}

void ProjectApplication::RenderUI(double dt)
{
	//This is needed or else there's a crash
	glClearColor(0.0f, 0.0f, 1.0f, 1.0f);

	ImGui::Begin("Window");
	{
		ImGui::Text("Framerate: %.0f Hertz", 1 / dt);
		ImGui::End();
	}
}

ProjectApplication::~ProjectApplication()
{
	soloud.stopAll();
	soloud.deinit();
}