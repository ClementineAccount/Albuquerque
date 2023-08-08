#include <Fwog/Buffer.h>
#include <Fwog/Pipeline.h>

namespace Albuquerque
{
    namespace FwogHelpers
    {
        Fwog::GraphicsPipeline MakePipeline(std::string_view vertexShaderPath, std::string_view fragmentShaderPath);
    }
}