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
	auto fragmentShader = Fwog::Shader(Fwog::PipelineStage::FRAGMENT_SHADER, ProjectApplication::LoadFile(frag_shader_path));

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

	auto vertexShader = Fwog::Shader(Fwog::PipelineStage::VERTEX_SHADER, ProjectApplication::LoadFile(vert_indexed_shader_path));
	auto fragmentShader = Fwog::Shader(Fwog::PipelineStage::FRAGMENT_SHADER, ProjectApplication::LoadFile(frag_color_shader_path));

	return Fwog::GraphicsPipeline{ {
			.vertexShader = &vertexShader,
			.fragmentShader = &fragmentShader,
			.inputAssemblyState = primDescs,
			.vertexInputState = {inputDescs},
			.depthState = {.depthTestEnable = true, .depthWriteEnable = true, .depthCompareOp = Fwog::CompareOp::LESS},
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

	constexpr uint32_t num_stacks = 8;
	constexpr uint32_t num_slices = 8;

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
		globalUniformsBuffer = Fwog::TypedBuffer<GlobalUniforms>(Fwog::BufferStorageFlag::DYNAMIC_STORAGE);
		globalUniformsBuffer.value().SubData(viewProj, 0);
	}


	//Creating ground plane
	{
		//to do: better texture loading systems. this can break so easily and its jank as hell
		int32_t textureWidth, textureHeight, textureChannels;
		constexpr int32_t expected_num_channels = 4;
		unsigned char* textureData = stbi_load("data/textures/GroundForest003_Flat.png", &textureWidth, &textureHeight, &textureChannels, expected_num_channels);
		assert(textureData);
		groundAlbedo = Fwog::CreateTexture2D({ static_cast<uint32_t>(textureWidth), static_cast<uint32_t>(textureHeight) }, Fwog::Format::R8G8B8A8_SRGB);
		Fwog::TextureUpdateInfo updateInfo{ .dimension = Fwog::UploadDimension::TWO,
			.level = 0,
			.offset = {},
			.size = {static_cast<uint32_t>(textureWidth), static_cast<uint32_t>(textureHeight), 1},
			.format = Fwog::UploadFormat::RGBA,
			.type = Fwog::UploadType::UBYTE,
			.pixels = textureData };
		groundAlbedo.value().SubImage(updateInfo);
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

	//Creating the aircraft
	{
		Utility::LoadModelFromFile(scene_aircraft, "data/models/aircraftPlaceholder.glb", glm::mat4{ 1.0f }, true);
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
		objectBufferaircraft.value().SubData(aircraftUniform, 0);
	}
}



void ProjectApplication::AddCollectable(glm::vec3 position, glm::vec3 scale, glm::vec3 color)
{
	collectableList.emplace_back(position, scale, false);
	ObjectUniforms collectableUniform;
	collectableUniform.model = glm::mat4(1.0f);

	collectableUniform.model = glm::translate(collectableUniform.model, position);
	collectableUniform.model = glm::scale(collectableUniform.model, scale);
	collectableUniform.color = glm::vec4(color, 1.0f);

	collectableObjectBuffers.value().SubData(collectableUniform, sizeof(collectableUniform) * num_active_collectables);
	num_active_collectables += 1;
}


void ProjectApplication::LoadCollectables()
{
	//Load the actual scene vertices here
	Utility::LoadModelFromFile(scene_collectable, "data/models/collectableSphere.glb", glm::mat4{ 1.0f }, true);
	collectableObjectBuffers  = Fwog::TypedBuffer<ObjectUniforms>(max_num_collectables, Fwog::BufferStorageFlag::DYNAMIC_STORAGE);

	//Add placeholder collectables

	constexpr float forward_distance_offset = 30.0f;
	constexpr float up_distance_offset = 25.0f;
	for (size_t i = 0; i < 20; ++i)
	{
		AddCollectable(glm::vec3(0.0f, 50.0f + up_distance_offset * i, 100.0f + i * forward_distance_offset), glm::vec3(2.0f, 2.0f, 2.0f));
	}

}



bool ProjectApplication::Load()
{

	SetWindowTitle("Plane Game");

	// Initialize SoLoud (automatic back-end selection)

	//To Do: Like actually use the res properly
	SoLoud::result init = soloud.init();
	SoLoud::result res = sample.load("data/sounds/start.wav"); // Load a wave file
	sample.setVolume(0.1);
	res = plane_flying_sfx.load("data/sounds/planeFlying.wav");

	background_music.load("data/sounds/backgroundMusic.wav");
	background_music.setVolume(0.30);

	soloud.setGlobalVolume(soloud_volume);

	//Creating pipelines

	pipeline_flat = CreatePipeline();
	pipeline_lines = CreatePipelineLines();
	pipeline_textured = CreatePipelineTextured();
	pipeline_colored_indexed = CreatePipelineColoredIndex();

	LoadBuffers();
	LoadCollectables();

	//Play sfx

	plane_flying_sfx.setLooping(true);
	plane_flying_sfx.setVolume(0.40);
	soloud.play(plane_flying_sfx);

	soloud.play(background_music);


	return true;
}

//void DrawVAO(uint32_t shader_id, uint32_t vao)
//{
//    glUseProgram(shader_id);
//    glBindVertexArray(vao);
//    glDrawArrays(GL_TRIANGLES, 0, 3);
//}



void ProjectApplication::Update(double dt)
{

	if (IsKeyPressed(GLFW_KEY_ESCAPE))
	{
		Close();
	}

	if (IsKeyPressed(GLFW_KEY_Q))
	{
		soloud.play(sample); 
	}

	{

		//aircraft Inputs
		float dt_float = static_cast<float>(dt);
		float zoom_speed_level = 1.0f;

		aircraft_current_speed_scale = 1.0f;

		{
			ZoneScopedC(tracy::Color::Green);


			aircraft_body.forward_vector = worldForward;
			aircraft_body.right_vector = worldRight;
			aircraft_body.up_vector = worldUp;

			aircraft_body.forward_vector = glm::vec3(aircraft_body.rotMatrix * glm::vec4(aircraft_body.forward_vector, 1.0f));
			aircraft_body.right_vector = glm::vec3(aircraft_body.rotMatrix * glm::vec4(aircraft_body.right_vector, 1.0f));
			aircraft_body.up_vector = glm::vec3(aircraft_body.rotMatrix * glm::vec4(aircraft_body.up_vector, 1.0f));

			if (IsKeyPressed(GLFW_KEY_SPACE))
			{
				aircraft_current_speed_scale = aircraft_speedup_scale;
				zoom_speed_level = 1.02f;
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



			//Update position based off the velocity

			//Experimenting with another approach
			aircraftPos += aircraft_body.forward_vector * aircraft_body.current_speed * aircraft_current_speed_scale * dt_float;

		}




		//Debug Lines
		{
			//

			////for dispaly
			//constexpr float line_length = 20.0f;
			//
			////glm::vec3 dir_vec_pt = aircraftPos + aircraft_body.forward_vector * line_length;
			////AddDebugDrawLine(aircraftPos, dir_vec_pt, glm::vec3(0.0f, 0.0f, 1.0f));

			////glm::vec3 velpt = aircraftPos + glm::normalize(aircraft_body.aircraft_current_velocity) * line_length;
			////AddDebugDrawLine(aircraftPos, velpt, glm::vec3(1.0f, 0.0f, 0.0f));


			////Collider sync and draws
			////Collision::SyncAABB(aircraft_box_collider, aircraftPos);
			////Collision::SyncSphere(aircraft_sphere_collider, aircraftPos);

			//////Profile only the drawing
			////
			////DrawLineAABB(aircraft_box_collider, glm::vec3(0.0f, 0.0f, 1.0f));
			////DrawLineSphere(aircraft_sphere_collider, glm::vec3(0.0f, 0.0, 1.0f));
		}
		
		{
			//aircraft uniform buffer changes
			ZoneScopedC(tracy::Color::Orange);
			glm::mat4 model(1.0f);
			model = glm::translate(model, aircraftPos);
			model *= aircraft_body.rotMatrix;

			ObjectUniforms a(model, glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));
			objectBufferaircraft.value().SubData(a, 0);
		}

		{
			//Camera logic stuff
			ZoneScopedC(tracy::Color::Blue);


			glm::vec3 camPos = (aircraftPos - aircraft_body.forward_vector * 25.0f) + aircraft_body.up_vector * 10.0f;
			glm::mat4 view = glm::lookAt(camPos, aircraftPos + (aircraft_body.up_vector * 10.0f), aircraft_body.up_vector);

			//we dont actually have to recalculate this every frame yet but we might wanna adjust fov i guess
			glm::mat4 proj = glm::perspective((PI / 2.0f) * zoom_speed_level, 1.6f, nearPlane, farPlane);
			glm::mat4 viewProj = proj * view;
			globalUniformsBuffer.value().SubData(viewProj, 0);
		}
	}
}



void ProjectApplication::RenderScene()
{
	ZoneScopedC(tracy::Color::Red);

	Fwog::BeginSwapchainRendering(Fwog::SwapchainRenderInfo{
	.viewport =
	  Fwog::Viewport{
		.drawRect{.offset = {0, 0}, .extent = {windowWidth, windowHeight}},
		.minDepth = 0.0f, .maxDepth = 1.0f
	  },
	.colorLoadOp = Fwog::AttachmentLoadOp::CLEAR,
	.clearColorValue = {skyColor.r, skyColor.g, skyColor.b, 1.0f},
	 .depthLoadOp = Fwog::AttachmentLoadOp::CLEAR,
	  .clearDepthValue = 1.0f
		});


	//Drawing a plane
	{
		Fwog::SamplerState ss;
		ss.minFilter = Fwog::Filter::LINEAR;
		ss.magFilter = Fwog::Filter::LINEAR;
		ss.addressModeU = Fwog::AddressMode::REPEAT;
		ss.addressModeV = Fwog::AddressMode::REPEAT;
		auto nearestSampler = Fwog::Sampler(ss);

		Fwog::Cmd::BindGraphicsPipeline(pipeline_textured.value());
		Fwog::Cmd::BindUniformBuffer(0, globalUniformsBuffer.value());
		Fwog::Cmd::BindUniformBuffer(1, objectBufferPlane.value());
		Fwog::Cmd::BindSampledImage(0, groundAlbedo.value(), nearestSampler);
		Fwog::Cmd::BindVertexBuffer(0, vertex_buffer_plane.value(), 0, sizeof(Primitives::Vertex));
		Fwog::Cmd::BindIndexBuffer(index_buffer_plane.value(), Fwog::IndexType::UNSIGNED_SHORT);
		Fwog::Cmd::DrawIndexed(static_cast<uint32_t>(Primitives::plane_indices.size()), 1, 0, 0, 0);
	}

	//Drawing the collectables
	{
		if (num_active_collectables > 0)
		{
			Fwog::Cmd::BindGraphicsPipeline(pipeline_colored_indexed.value());
			Fwog::Cmd::BindUniformBuffer(0, globalUniformsBuffer.value());
			Fwog::Cmd::BindStorageBuffer(1, collectableObjectBuffers.value());
			Fwog::Cmd::BindVertexBuffer(0, scene_collectable.meshes[0].vertexBuffer, 0, sizeof(Utility::Vertex));
			Fwog::Cmd::BindIndexBuffer(scene_collectable.meshes[0].indexBuffer,  Fwog::IndexType::UNSIGNED_INT);
			Fwog::Cmd::DrawIndexed(static_cast<uint32_t>(scene_collectable.meshes[0].indexBuffer.Size()) / sizeof(uint32_t), num_active_collectables, 0, 0, 0);
		}
	}

	//Drawing a aircraft + wheels
	{
		Fwog::Cmd::BindGraphicsPipeline(pipeline_flat.value());
		Fwog::Cmd::BindUniformBuffer(0, globalUniformsBuffer.value());
		Fwog::Cmd::BindUniformBuffer(1, objectBufferaircraft.value());
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