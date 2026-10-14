#include "rengb_model.hpp"

#include "rengb_utils.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <cassert>
#include <cstring>
#include <unordered_map>
#include <iostream>

#ifndef ENGINE_DIR
#define ENGINE_DIR "../"
#endif



namespace std
{
    template <>
    struct hash<rengb::RengbModel::Vertex>
    {
        size_t operator()(rengb::RengbModel::Vertex const &vertex) const
        {
            size_t seed = 0;

            rengb::hashCombine(seed, vertex.position, vertex.color, vertex.flashColor);
            return seed;
        }
    };
}

namespace rengb
{
    std::unordered_map<std::string, std::shared_ptr<RengbModel>> RengbModel::modelCache;

    RengbModel::RengbModel(RengbDevice &device, const RengbModel::Builder &builder) : rengbDevice(device)
    {
        createVertexBuffers(builder.vertices);
        createIndexBuffers(builder.indices);
    }
    RengbModel::~RengbModel() {}


    void RengbModel::clearModelCache()
    {
        modelCache.clear();
    }


    std::shared_ptr<RengbModel> RengbModel::createModelFromFile(RengbDevice &device, const std::string &filepath)
    {
        
        Builder builder{};
        builder.loadModel(filepath);
        return std::make_shared<RengbModel>(device, builder);
    }

    std::shared_ptr<RengbModel> RengbModel::createModelFromFile(RengbDevice &device, const std::string &filepath, const std::string &filepath2)
    {
        Builder builder{};
        builder.loadModel(filepath, filepath2);
        return std::make_shared<RengbModel>(device, builder);
    }

    std::shared_ptr<RengbModel> RengbModel::createModelFromBuilder(RengbDevice &device, Builder &builder)
    {
        return std::make_shared<RengbModel>(device, builder);
    }

    RengbModel::Builder RengbModel::createBuilderFromFileUnindexed(const std::string &filepath)
    {
        Builder builder{};
        builder.vertices = RengbModel::loadVertices(filepath);
        return builder;
    }

    std::shared_ptr<RengbModel> RengbModel::createModelFromVertices(RengbDevice &device, std::vector<Vertex> vertices)
    {
        Builder builder{};
        builder.vertices = vertices;
        return std::make_shared<RengbModel>(device, builder);
    }


    RengbModel::Builder RengbModel::createBuilderFromFile(const std::string &filepath)
    {
        Builder builder{};
        builder.loadModel(filepath);
        return builder;
    }

    std::shared_ptr<RengbModel> RengbModel::getOrMakeModelFromFile(RengbDevice &device, std::string const &filepath)
    {
        auto it = modelCache.find(filepath);
        if (it != modelCache.end()) 
        {
            return it->second; // existing
        }
    
        // load and store in cache
        std::shared_ptr<RengbModel> newMesh = RengbModel::createModelFromFile(device, filepath);
        modelCache[filepath] = newMesh;
        return newMesh;
    }

    void RengbModel::processVerticesForFont(std::vector<RengbModel::Vertex> &vertices, float meshWidthPerChar, int vertexOffsets[], int const totalChars)
    {
        // note- vertex information is always from furthest face to closest!
		int characterIndex = totalChars-1;
        int rollingVertexOffset = 0;

		for(RengbModel::Vertex &vertex : vertices)
		{
			if(ceil(vertex.position.x) < meshWidthPerChar * characterIndex) // ceil because it bugs with values near to whole
			{
                vertexOffsets[characterIndex] = rollingVertexOffset;

                characterIndex = characterIndex -1;
                
			}
            ++rollingVertexOffset;
			vertex.position.x = vertex.position.x - meshWidthPerChar * characterIndex; 
            std::swap(vertex.position.y, vertex.position.z);
		}

        vertexOffsets[characterIndex] = vertices.size();
    }
    

    void RengbModel::createVertexBuffers(const std::vector<Vertex> &vertices)
    {
        vertexCount = static_cast<uint32_t>(vertices.size());
        assert(vertexCount >= 3 && "Vertex count must be at least 3");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
        uint32_t vertexSize = sizeof(vertices[0]);

        RengbBuffer stagingBuffer{
            rengbDevice,
            vertexSize,
            vertexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void *)vertices.data());

        vertexBuffer = std::make_unique<RengbBuffer>(
            rengbDevice,
            vertexSize,
            vertexCount,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        rengbDevice.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
    }

    void RengbModel::createIndexBuffers(const std::vector<uint32_t> &indices)
    {
        indexCount = static_cast<uint32_t>(indices.size());
        hasIndexBuffer = indexCount > 0;

        if (!hasIndexBuffer)
            return;

        VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
        uint32_t indexSize = sizeof(indices[0]);

        RengbBuffer stagingBuffer{
            rengbDevice,
            indexSize,
            indexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void *)indices.data());

        indexBuffer = std::make_unique<RengbBuffer>(
            rengbDevice,
            indexSize,
            indexCount,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        rengbDevice.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
    }

    void RengbModel::draw(VkCommandBuffer commandBuffer)
    {
        if (hasIndexBuffer)
        {
            vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
        }
        else
        {
            vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
        }
    }

    void RengbModel::drawRange(VkCommandBuffer commandBuffer, size_t from, size_t to)
    {
        if (hasIndexBuffer)
        {
            std::cout << "Shouldn't be indexed!";

        }
        else
        {
            vkCmdDraw(commandBuffer, to - from, 1, from, 0);
        }
    }

    void RengbModel::bind(VkCommandBuffer commandBuffer)
    {
        VkBuffer buffers[] = {vertexBuffer->getBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

        if (hasIndexBuffer)
        {
            vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        }
    }

    std::vector<VkVertexInputBindingDescription> RengbModel::Vertex::getBindingDescriptions()
    {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> RengbModel::Vertex::getAttributeDescriptions() 
    {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
        attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
        attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, flashColor)});

        return attributeDescriptions;
    }

    void RengbModel::Builder::loadModel(const std::string &filepath)
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        std::string enginePath = ENGINE_DIR + filepath;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, enginePath.c_str()))
        {
            throw std::runtime_error(warn + err);
        }

        vertices.clear();
        indices.clear();

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        
            
        for (const auto &shape : shapes)
        {
            for (const auto &index : shape.mesh.indices)
            {
                Vertex vertex{};

                if (index.vertex_index >= 0)
                {
                    vertex.position = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2],
                    };

                    vertex.color = {
                        attrib.colors[3 * index.vertex_index + 0],
                        attrib.colors[3 * index.vertex_index + 1],
                        attrib.colors[3 * index.vertex_index + 2],
                    };
                }

                if (uniqueVertices.count(vertex) == 0)
                {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }
                indices.push_back(uniqueVertices[vertex]);
            }
        }
    }

    void RengbModel::Builder::loadModel(const std::string &filepath, const std::string &filepath2)
    {
        tinyobj::attrib_t attrib, attrib2;
        std::vector<tinyobj::shape_t> shapes, shapes2;
        std::vector<tinyobj::material_t> materials, materials2;
        std::string warn, err;

        std::string enginePath = ENGINE_DIR + filepath;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, enginePath.c_str()))
        {
            throw std::runtime_error(warn + err);
        }
        enginePath = ENGINE_DIR + filepath2;
        if (!tinyobj::LoadObj(&attrib2, &shapes2, &materials2, &warn, &err, enginePath.c_str()))
        {
            throw std::runtime_error(warn + err);
        }

        vertices.clear();
        indices.clear();

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        
        for (const auto &shape : shapes)
        {

            for (const auto &index : shape.mesh.indices)
            {
                Vertex vertex{};

                if (index.vertex_index >= 0)
                {
                    vertex.position = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2],
                    };

                    vertex.color = {
                        attrib.colors[3 * index.vertex_index + 0],
                        attrib.colors[3 * index.vertex_index + 1],
                        attrib.colors[3 * index.vertex_index + 2],
                    };

                    vertex.flashColor = {
                        attrib2.colors[3 * index.vertex_index + 0],
                        attrib2.colors[3 * index.vertex_index + 1],
                        attrib2.colors[3 * index.vertex_index + 2],
                    };
                }

                if (uniqueVertices.count(vertex) == 0)
                {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }
                indices.push_back(uniqueVertices[vertex]);
            }
        }
    }


    std::vector<RengbModel::Vertex> RengbModel::loadVertices(const std::string &filepath)
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        std::string enginePath = ENGINE_DIR + filepath;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, enginePath.c_str()))
        {
            throw std::runtime_error(warn + err);
        }

        std::vector<Vertex> vertices;


        for (const auto &shape : shapes)
        {
            for (const auto &index : shape.mesh.indices)
            {
                Vertex vertex{};

                vertex.position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2],
                };

                vertex.color = {
                    attrib.colors[3 * index.vertex_index + 0],
                    attrib.colors[3 * index.vertex_index + 1],
                    attrib.colors[3 * index.vertex_index + 2],
                };


                vertices.push_back(vertex);
            }
        }
        return vertices;
    }
}