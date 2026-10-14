#pragma once

#include "rengb_device.hpp"
#include "rengb_buffer.hpp"

#define GLM__FORCE_RADIANS
#define GLM_FOCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

// std
#include <vector>
#include <memory>
#include <unordered_map>

namespace rengb
{
    class RengbModel
    {
    public:
        struct Vertex 
        {
            glm::vec3 position{};
            glm::vec3 color{};
            glm::vec3 flashColor{};


            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

            bool operator==(const Vertex &other) const
            {

                return position == other.position && color == other.color;
            }
        };

        struct OrthogColors
        {
            glm::vec3 east{};
            glm::vec3 south{};
            glm::vec3 west{};
        };

        struct Builder
        {
            std::vector<Vertex> vertices{};
            std::vector<uint32_t> indices{};

            void loadModel (const std:: string &filepath);
            void loadModel (const std::string &filepath, const std::string &filepath2);
        };

        RengbModel(RengbDevice &device, const RengbModel::Builder &builder);
        ~RengbModel();

        RengbModel(const RengbModel &) = delete;
        RengbModel &operator=(const RengbModel &) = delete;

        static std::shared_ptr<RengbModel> createModelFromFile (RengbDevice &device, std::string const &filepath);
        static std::shared_ptr<RengbModel> createModelFromFile (RengbDevice &device, std::string const &filepath, std::string const &filepath2);
        static std::shared_ptr<RengbModel> createModelFromBuilder(RengbDevice &device, Builder &builder); 
        static std::shared_ptr<RengbModel> createModelFromVertices(RengbDevice &device, std::vector<Vertex>); 
        static Builder createBuilderFromFileUnindexed(const std::string &filepath);
        static Builder createBuilderFromFile(const std::string &filepath);

        static std::shared_ptr<RengbModel> getOrMakeModelFromFile (RengbDevice &device, std::string const &filepath);

        static std::vector<Vertex> loadVertices (const std::string &filepath);
        // Deprecated, done through the gui class
        static void processVerticesForFont(std::vector<RengbModel::Vertex> &vertices, float meshWidthPerChar, int vertexOffsets[], int const totalChars);
    
        static void clearModelCache();

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);
        void drawRange(VkCommandBuffer commandBuffer, size_t from, size_t to);

    private:
        void createVertexBuffers(const std::vector<Vertex> &vertices);
        void createIndexBuffers(const std::vector<uint32_t> &indices);

        RengbDevice &rengbDevice;

        std::unique_ptr<RengbBuffer> vertexBuffer;
        uint32_t vertexCount;

        bool hasIndexBuffer = false;
        std::unique_ptr<RengbBuffer> indexBuffer;
        uint32_t indexCount;


        static std::unordered_map<std::string, std::shared_ptr<RengbModel>> modelCache; // shared allows models to persist even after entity is rid of them, saves races.
    };
}

