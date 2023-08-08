#include "include/Albuquerque/FwogHelpers.hpp"


#include <string_view>
#include <vector>
#include <memory>
#include <array>
#include <optional>

namespace Albuquerque
{
    namespace FwogHelpers
    {
        Fwog::GraphicsPipeline MakePipeline(std::string_view vertexShaderPath, std::string_view fragmentShaderPath)
        {
        auto LoadFile = [](std::string_view path)
        {
            std::ifstream file{ path.data() };
            std::string returnString { std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
            return returnString;
        };

        auto vertexShader = Fwog::Shader(Fwog::PipelineStage::VERTEX_SHADER, LoadFile(vertexShaderPath));
        auto fragmentShader = Fwog::Shader(Fwog::PipelineStage::FRAGMENT_SHADER, LoadFile(fragmentShaderPath));

        //Ensures this matches the shader and your vertex buffer data type

        auto sceneInputBindingDescs = std::array{
            Fwog::VertexInputBindingDescription{
                // position
                .location = 0,
                    .binding = 0,
                    .format = Fwog::Format::R32G32B32_FLOAT,
                    .offset = offsetof(Albuquerque::Primitives::Vertex, position),
            },
                Fwog::VertexInputBindingDescription{
                // normal
                .location = 1,
                    .binding = 0,
                    .format = Fwog::Format::R32G32B32_FLOAT,
                    .offset = offsetof(Albuquerque::Primitives::Vertex, normal),
            },
                Fwog::VertexInputBindingDescription{
                // texcoord
                .location = 2,
                    .binding = 0,
                    .format = Fwog::Format::R32G32_FLOAT,
                    .offset = offsetof(Albuquerque::Primitives::Vertex, uv),
            },
        };

        auto inputDescs = sceneInputBindingDescs;
        auto primDescs =
            Fwog::InputAssemblyState{ Fwog::PrimitiveTopology::TRIANGLE_LIST };

        return Fwog::GraphicsPipeline{{
                .vertexShader = &vertexShader,
                    .fragmentShader = &fragmentShader,
                    .inputAssemblyState = primDescs,
                    .vertexInputState = { inputDescs },
                    .depthState = { .depthTestEnable = true,
                    .depthWriteEnable = true,
                    .depthCompareOp = Fwog::CompareOp::LESS },
            }};
        }
    }
}

