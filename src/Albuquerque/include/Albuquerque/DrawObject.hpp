#pragma once

#include <Fwog/Buffer.h>
#include <glm/mat4x4.hpp>
#include <optional>

#include <unordered_map>
#include <string>

namespace Albuquerque
{
    namespace FwogHelpers
    {
        class MeshBuffer
        {
        public:
            MeshBuffer() = delete;

            template <typename T1, typename T2>
            static MeshBuffer Init(T1 const& vertexList, T2 const& indexList, size_t indexCount)
            {
                MeshBuffer buffer;
                buffer.vertexBuffer.emplace(vertexList);
                buffer.indexBuffer.emplace(indexList);
                
                //Fwog takes in uint32_t for the indexCount but .size() on a container returns size_t. I'll just cast it here and hope its fine.
                buffer.indexCount = static_cast<uint32_t>(indexCount);

                return buffer;
            }
        private:
            Fwog::Buffer vertexBuffer;
            Fwog::Buffer indexBuffer;
            uint32_t indexCount;
        };

        //To Do: Get this to set key to be some hash ID rather than string comparsion
        std::unordered_map<std::string, MeshBuffer> meshBufferMap;




        //I wonder if I can even decouple this from the renderer?
        struct DrawObject
        {
            //T1 and T2 can be different container types. std::array or std::vector.
            //Didn't want this to be a constructor because the actual DrawObject struct does not need to be templated.
            template <typename T1, typename T2>
            static DrawObject Init(T1 const& vertexList, T2 const& indexList, size_t indexCount)
            {
                DrawObject object;
                object.vertexBuffer.emplace(vertexList);
                object.indexBuffer.emplace(indexList);
                object.modelUniformBuffer =  Fwog::TypedBuffer<DrawObject::ObjectUniform>(Fwog::BufferStorageFlag::DYNAMIC_STORAGE);
                object.modelUniformBuffer.value().UpdateData(object.objectStruct, 0);

                //Fwog takes in uint32_t for the indexCount but .size() on a container returns size_t. I'll just cast it here and hope its fine.
                object.indexCount = static_cast<uint32_t>(indexCount);

                return object;
            };

            //To Do: Set this to be a reference rather than value. Retrieve it from a mesh database
            //std::optional<Fwog::Buffer> vertexBuffer;
            //std::optional<Fwog::Buffer> indexBuffer;

            //uint32_t indexCount;

            //Non owning pointer to meshBuffer to be retrieved from meshBufferMap
            MeshBuffer* meshBufferRef = nullptr;

            struct ObjectUniform
            {
                glm::mat4 modelTransform = glm::mat4(1.0f);
            };
            ObjectUniform objectStruct;

            std::optional<Fwog::TypedBuffer<ObjectUniform>> modelUniformBuffer;
        };
    }
}

