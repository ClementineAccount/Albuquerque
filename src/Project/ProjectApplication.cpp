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

bool ProjectApplication::Load()
{

	SetWindowTitle("Plane Game");

	// Initialize SoLoud (automatic back-end selection)
	SoLoud::result init = soloud.init();
	SoLoud::result res = sample.load("data/sounds/start.wav"); // Load a wave file
	soloud.setGlobalVolume(soloud_volume);

	//Creating pipelines

	pipeline_flat = CreatePipeline();
	pipeline_lines = CreatePipelineLines();
	pipeline_textured = CreatePipelineTextured();

	LoadBuffers();

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

		if (IsKeyPressed(GLFW_KEY_SPACE))
		{
			aircraft_current_speed_scale = aircraft_speedup_scale;
			zoom_speed_level = 1.05f;
		}


		//Turning Left: Need to adjust both Roll and Velocity
		if (IsKeyPressed(GLFW_KEY_RIGHT))
		{
			aircraft_body.aircraft_angles_degrees.z += aircraft_angle_turning_degrees * dt_float;
		}

		if (IsKeyPressed(GLFW_KEY_LEFT))
		{
			aircraft_body.aircraft_angles_degrees.z -= aircraft_angle_turning_degrees * dt_float;
		}

		if (IsKeyPressed(GLFW_KEY_UP))
		{
			aircraft_body.aircraft_angles_degrees.x += aircraft_angle_turning_degrees * dt_float;
		}

		if (IsKeyPressed(GLFW_KEY_DOWN))
		{
			aircraft_body.aircraft_angles_degrees.x -= aircraft_angle_turning_degrees * dt_float;
		}



		//Update position based off the velocity
		aircraftPos += aircraft_body.direction_vector * aircraft_body.current_speed * aircraft_current_speed_scale * dt_float;


		//Debug Lines
		{
			ZoneScopedC(tracy::Color::Green);

			//for dispaly
			constexpr float line_length = 20.0f;
			
			glm::vec3 dir_vec_pt = aircraftPos + aircraft_body.direction_vector * line_length;
			AddDebugDrawLine(aircraftPos, dir_vec_pt, glm::vec3(1.0f, 0.0f, 0.0f));

			//glm::vec3 velpt = aircraftPos + glm::normalize(aircraft_body.aircraft_current_velocity) * line_length;
			//AddDebugDrawLine(aircraftPos, velpt, glm::vec3(1.0f, 0.0f, 0.0f));


			//Collider sync and draws
			//Collision::SyncAABB(aircraft_box_collider, aircraftPos);
			//Collision::SyncSphere(aircraft_sphere_collider, aircraftPos);

			////Profile only the drawing
			//
			//DrawLineAABB(aircraft_box_collider, glm::vec3(0.0f, 0.0f, 1.0f));
			//DrawLineSphere(aircraft_sphere_collider, glm::vec3(0.0f, 0.0, 1.0f));
		}
		
		{
			//aircraft uniform buffer changes
			ZoneScopedC(tracy::Color::Orange);
			glm::mat4 model(1.0f);
			model = glm::translate(model, aircraftPos);

			//To Do: Make this world coordinate system agnostic
			//Example: angleUp = magnitude of worldUp * aircraft_angle_degrees


			//Reset to the world forward as we apply the rotation in correct sequence
			aircraft_body.direction_vector = glm::vec3(0.0f, 0.0f, 1.0f);

			//This adjusts the yaw of the aircraft
			glm::mat4 rotModel(1.0f);

			rotModel = glm::rotate(rotModel, glm::radians(aircraft_body.aircraft_angles_degrees.y), worldUp);
			rotModel = glm::rotate(rotModel, glm::radians(aircraft_body.aircraft_angles_degrees.z), worldForward);
			rotModel = glm::rotate(rotModel, glm::radians(aircraft_body.aircraft_angles_degrees.x), worldRight);
			aircraft_body.direction_vector = glm::vec3(rotModel * glm::vec4(aircraft_body.direction_vector, 1.0f));

			model *= rotModel;

			ObjectUniforms a(model, glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));
			objectBufferaircraft.value().SubData(a, 0);
		}

		{
			//Camera logic stuff
			ZoneScopedC(tracy::Color::Blue);

			glm::vec3 camPos = aircraftPos - aircraftForward * cameraOffset.z + cameraOffsetTarget;
			glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
			glm::mat4 view = glm::lookAt(camPos, aircraftPos + cameraOffsetTarget, up);

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
		Fwog::Cmd::BindVertexBuffer(0, vertex_buffer_pos_line.value(), 0, 3 * sizeof(float));
		Fwog::Cmd::BindVertexBuffer(1, vertex_buffer_color_line.value(), 0, 3 * sizeof(float));
		Fwog::Cmd::Draw(num_points_world_axis, 1, 0, 0);

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