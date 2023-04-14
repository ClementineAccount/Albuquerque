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



void ProjectApplication::AddCollisionDrawLine(glm::vec3 ptA, glm::vec3 ptB, glm::vec3 color) {

	
	std::array<glm::vec3, 2> linePos{ptA, ptB};
	std::array<glm::vec3, 2> colorPos{color, color};
  vertex_buffer_collision_lines.value().SubData(linePos, sizeof(glm::vec3) * curr_num_collision_points);
  vertex_buffer_collision_colors.value().SubData(colorPos, sizeof(glm::vec3) * curr_num_collision_points);

  curr_num_collision_points += 2;
}



void ProjectApplication::ClearLines()
{
	curr_num_collision_points = 0;
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
	AddCollisionDrawLine(backface_down_left, backface_down_right, boxColor);
	AddCollisionDrawLine(backface_down_left, backface_up_left, boxColor);
	AddCollisionDrawLine(backface_up_left, backface_up_right, boxColor);
	AddCollisionDrawLine(backface_up_right, backface_down_right, boxColor);

	//Front Face
	AddCollisionDrawLine(frontface_down_left, frontface_down_right, boxColor);
	AddCollisionDrawLine(frontface_down_left, frontface_up_left, boxColor);
	AddCollisionDrawLine(frontface_up_left, frontface_up_right, boxColor);
	AddCollisionDrawLine(frontface_up_right, frontface_down_right, boxColor);

	//Left Face
	AddCollisionDrawLine(backface_down_left, frontface_down_left, boxColor);
	AddCollisionDrawLine(backface_up_left, frontface_up_left, boxColor);


	//Right Face
	AddCollisionDrawLine(backface_down_right, frontface_down_right, boxColor);
	AddCollisionDrawLine(backface_up_right, frontface_up_right, boxColor);


}


void ProjectApplication::DrawLineSphere(Collision::Sphere const& sphere, glm::vec3 sphereColor)
{
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

			AddCollisionDrawLine(position_horizontal, temp_horizontal, sphereColor);
			AddCollisionDrawLine(position_vert, temp_vert, sphereColor);
		}
	}
}

void ProjectApplication::AfterCreatedUiContext()
{

}

void ProjectApplication::BeforeDestroyUiContext()
{

}

bool ProjectApplication::Load()
{

	// Initialize SoLoud (automatic back-end selection)
	SoLoud::result init = soloud.init();
	SoLoud::result res = sample.load("data/sounds/start.wav"); // Load a wave file
	soloud.setGlobalVolume(soloud_volume);

	//Creating pipelines

	pipeline_flat = CreatePipeline();
	pipeline_lines = CreatePipelineLines();
	pipeline_textured = CreatePipelineTextured();

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
		std::array<glm::vec3, max_num_collision_points> linePts{};

        vertex_buffer_collision_lines = Fwog::TypedBuffer<glm::vec3>(max_num_collision_points, Fwog::BufferStorageFlag::DYNAMIC_STORAGE);
        vertex_buffer_collision_colors = Fwog::TypedBuffer<glm::vec3>(max_num_collision_points, Fwog::BufferStorageFlag::DYNAMIC_STORAGE);
		vertex_buffer_collision_lines.value().SubData(linePts, 0);
		vertex_buffer_collision_colors.value().SubData(linePts, 0 );
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

	//Creating the car
	{
		Utility::LoadModelFromFile(scene_car, "data/models/Car_BodyOnly.glb", glm::mat4{ 1.0f }, true);
		ObjectUniforms carUniform;
		carUniform.model = glm::mat4(1.0f);
		
		carUniform.model = glm::translate(carUniform.model, carPos);
		carUniform.model = glm::scale(carUniform.model, carScale);

		car_box_collider.center = carPos;
		car_box_collider.halfExtents = carScale * carCollisionScale;

		car_sphere_collider.radius = car_sphere_radius;
		car_sphere_collider.center = carPos;

		carUniform.color = carColor;
		objectBufferCar = Fwog::TypedBuffer<ObjectUniforms>(Fwog::BufferStorageFlag::DYNAMIC_STORAGE);
		objectBufferCar.value().SubData(carUniform, 0);

		Utility::LoadModelFromFile(scene_wheels, "data/models/Car_WheelsOnly.glb", glm::mat4{ 1.0f }, true);
		ObjectUniforms wheelUniform;
		wheelUniform.model = glm::mat4(1.0f);
		wheelUniform.color = wheelColor;

		objectBufferWheels = Fwog::TypedBuffer<ObjectUniforms>(Fwog::BufferStorageFlag::DYNAMIC_STORAGE);
		objectBufferWheels.value().SubData(wheelUniform, 0);

	}



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

	if (IsKeyPressed(GLFW_KEY_SPACE))
	{
		soloud.play(sample); 
	}

	
	{
		//Car Inputs

		float dt_float = static_cast<float>(dt);
		float zoom_speed_level = 1.0f;

		if (IsKeyPressed(GLFW_KEY_UP))
		{
			zoom_speed_level = 1.05f;
			if (IsKeyPressed(GLFW_KEY_LEFT))
			{
				car_angle_degrees += car_angle_turning_degrees * dt_float;
				carForward = glm::vec3(glm::sin(glm::radians(car_angle_degrees)), 0.0f, glm::cos(glm::radians(car_angle_degrees)));
			}

			if (IsKeyPressed(GLFW_KEY_RIGHT))
			{
				car_angle_degrees += -car_angle_turning_degrees * dt_float;
				carForward = glm::vec3(glm::sin(glm::radians(car_angle_degrees)), 0.0f, glm::cos(glm::radians(car_angle_degrees)));
			}
			carPos += carForward * car_speed_scale * dt_float;
		}
		if (IsKeyPressed(GLFW_KEY_DOWN))
		{
			if (IsKeyPressed(GLFW_KEY_RIGHT) == GLFW_PRESS)
			{
				car_angle_degrees += car_angle_turning_degrees * dt_float;
				carForward = glm::vec3(glm::sin(glm::radians(car_angle_degrees)), 0.0f, glm::cos(glm::radians(car_angle_degrees)));
			}

			if (IsKeyPressed(GLFW_KEY_LEFT) == GLFW_PRESS)
			{
				car_angle_degrees += -car_angle_turning_degrees * dt_float;
				carForward = glm::vec3(glm::sin(glm::radians(car_angle_degrees)), 0.0f, glm::cos(glm::radians(car_angle_degrees)));
			}

			carPos -= carForward * car_speed_scale_reverse * dt_float;
		}

		{
			//Collider sync and draws
			Collision::SyncAABB(car_box_collider, carPos);
			Collision::SyncSphere(car_sphere_collider, carPos);

			//Profile only the drawing
			ZoneScopedC(tracy::Color::Green);
			DrawLineAABB(car_box_collider, glm::vec3(0.0f, 0.0f, 1.0f));
			DrawLineSphere(car_sphere_collider, glm::vec3(0.0f, 0.0, 1.0f));
		}
		
		
		{
			//Car uniform buffer changes
			ZoneScopedC(tracy::Color::Orange);
			glm::mat4 model(1.0f);
			model = glm::translate(model, carPos);
			model = glm::rotate(model, glm::radians(car_angle_degrees), worldUp);

			ObjectUniforms a(model, glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));
			ObjectUniforms b(model, glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));

			objectBufferWheels.value().SubData(b, 0);
			objectBufferCar.value().SubData(a, 0);
		}

		{
			//Camera logic stuff
			ZoneScopedC(tracy::Color::Blue);
			glm::vec3 camPos = carPos - carForward * 15.0f + cameraOffsetTarget;
			glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
			glm::mat4 view = glm::lookAt(camPos, carPos + cameraOffsetTarget, up);

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

	//Drawing a car + wheels
	{
		Fwog::Cmd::BindGraphicsPipeline(pipeline_flat.value());
		Fwog::Cmd::BindUniformBuffer(0, globalUniformsBuffer.value());
		Fwog::Cmd::BindUniformBuffer(1, objectBufferCar.value());
		Fwog::Cmd::BindVertexBuffer(0, scene_car.meshes[0].vertexBuffer, 0, sizeof(Utility::Vertex));
		Fwog::Cmd::BindIndexBuffer(scene_car.meshes[0].indexBuffer, Fwog::IndexType::UNSIGNED_INT);
		Fwog::Cmd::DrawIndexed(static_cast<uint32_t>(scene_car.meshes[0].indexBuffer.Size()) / sizeof(uint32_t), 1, 0, 0, 0);

		Fwog::Cmd::BindUniformBuffer(1, objectBufferWheels.value());
		Fwog::Cmd::BindVertexBuffer(0, scene_wheels.meshes[0].vertexBuffer, 0, sizeof(Utility::Vertex));
		Fwog::Cmd::BindIndexBuffer(scene_wheels.meshes[0].indexBuffer, Fwog::IndexType::UNSIGNED_INT);
		Fwog::Cmd::DrawIndexed(static_cast<uint32_t>(scene_wheels.meshes[0].indexBuffer.Size()) / sizeof(uint32_t), 1, 0, 0, 0);
	}

	//Drawing axis lines
	{
		Fwog::Cmd::BindGraphicsPipeline(pipeline_lines.value());
		Fwog::Cmd::BindUniformBuffer(0, globalUniformsBuffer.value());
		Fwog::Cmd::BindVertexBuffer(0, vertex_buffer_pos_line.value(), 0, 3 * sizeof(float));
		Fwog::Cmd::BindVertexBuffer(1, vertex_buffer_color_line.value(), 0, 3 * sizeof(float));
		Fwog::Cmd::Draw(num_points_world_axis, 1, 0, 0);

		// Drawing collision lines
        if (curr_num_collision_points != 0 && curr_num_collision_points < max_num_collision_points) 
		{
			Fwog::Cmd::BindVertexBuffer(0, vertex_buffer_collision_lines.value(), 0, 3 * sizeof(float));
            Fwog::Cmd::BindVertexBuffer(1, vertex_buffer_collision_colors.value(), 0, 3 * sizeof(float));
			Fwog::Cmd::Draw(curr_num_collision_points, 1, 0, 0);

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