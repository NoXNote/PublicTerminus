#pragma once
#include "rengb_utils.hpp"
#include <glm/glm.hpp>
#include <unordered_set>
#include <rengb_model.hpp>
#include "transform.hpp"

namespace rengb
{
    struct TextItem
	{
		std::string text{"EmptyText"};
        // one colour per printable character
        std::string colours{"rrrrrrrrr"};
		int boxWidth = 0;
		float kerning = 6.f;
		float lineSpace = 10.f;
        float centering{0.f};        
		Transform specificTransform{}; //Apply transform per vertex in the builder funciton
        std::vector<std::string> extraModels = {};

        void centreText()
        {
            specificTransform.translation.x -= boxWidth * kerning * specificTransform.scale * centering;
            centering = 0.f;
        }

        TextItem(std::string t = "EmptyText", int bw = 0,  float k = 6.f, float ls = 10.f, float c = 0.f, Transform st = Transform{}, std::vector<std::string> em = {}) : text(t), boxWidth(bw), kerning(k),  lineSpace(ls), centering(c), specificTransform(st), extraModels(em)
        {
            if (boxWidth <= -1) boxWidth = text.length();
            if (centering != 0.f) centreText();
        }
	};

	struct TextCollection
	{
        float widthPerChar{16.f};
        int totalChars{94};
		std::vector<TextItem> textItems;
        std::vector<TextItem> dynamicTextItems;
        std::vector<RengbModel::Vertex> fontTemplate;
        std::shared_ptr<RengbModel> fontModel{};
        int vertexOffsets[94];

        std::shared_ptr<rengb::RengbModel> buildText(RengbDevice& device)
        {
            RengbModel::Builder builtText;
            for(TextItem item : textItems)
            {
                int wHeadx = 0, wHeady = 0;
                if (item.boxWidth <= -1) item.boxWidth = item.text.length();
                if (item.centering != 0.f) item.centreText();

                for(char c : item.text)
                {
                    if (c == '\n')
                    {
                        wHeadx = -1;
                        ++wHeady;
                    }
                    else if(c != ' ')
                    {
                        for(int i = vertexOffsets[c - 33]-1; i >= vertexOffsets[c - 33 + 1]; --i)
                        {
                            RengbModel::Vertex vertex = fontTemplate.at(i);
                            vertex.position.x += wHeadx * item.kerning;
                            vertex.position.z += wHeady * (item.lineSpace+1);

                            vertex.color = {1.f,1.f,1.f};
                            vertex.position = item.specificTransform.mat4() * glm::vec4(vertex.position, 0.f);
                            vertex.position += item.specificTransform.translation;
                            builtText.vertices.push_back(vertex);
                        }
                    }
                    if(item.boxWidth != 0 && wHeadx >= item.boxWidth)
                    {
                        wHeadx = 0;
                        ++wHeady;
                    } else ++wHeadx;
                }

                if(item.boxWidth!=0)
                { 
                    std::cout<<"hasBox"<<std::endl;
                    builtText.vertices.push_back(RengbModel::Vertex{
                        (item.specificTransform.mat4()*glm::vec4(0.f, 0.f, -item.lineSpace, 0.f)) + glm::vec4(item.specificTransform.translation, 0.f),
                        glm::vec3{0.f},
                        glm::vec3{0.f}});
                    builtText.vertices.push_back(RengbModel::Vertex{
                        (item.specificTransform.mat4()*glm::vec4{(item.boxWidth+1) * item.kerning, 0.f, -item.lineSpace, 0.f}) + glm::vec4(item.specificTransform.translation, 0.f),
                        glm::vec3{0.f},
                        glm::vec3{0.f}});
                    builtText.vertices.push_back(RengbModel::Vertex{
                        (item.specificTransform.mat4()*glm::vec4{0.f, 0.f, wHeady * (item.lineSpace+1), 0.f}) + glm::vec4(item.specificTransform.translation, 0.f),
                        glm::vec3{0.f},
                        glm::vec3{0.f}});

                    builtText.vertices.push_back(RengbModel::Vertex{
                        (item.specificTransform.mat4()*glm::vec4{(item.boxWidth+1) * item.kerning, 0.f, -item.lineSpace, 0.f}) + glm::vec4(item.specificTransform.translation, 0.f),
                        glm::vec3{0.f},
                        glm::vec3{0.f}});
                    builtText.vertices.push_back(RengbModel::Vertex{
                        (item.specificTransform.mat4()*glm::vec4{0.f, 0.f, wHeady * (item.lineSpace+1), 0.f}) + glm::vec4(item.specificTransform.translation, 0.f),
                        glm::vec3{0.f},
                        glm::vec3{0.f}});
                    builtText.vertices.push_back(RengbModel::Vertex{
                        (item.specificTransform.mat4()*glm::vec4{(item.boxWidth+1) * item.kerning, 0.f, wHeady * (item.lineSpace+1), 0.f} )+ glm::vec4(item.specificTransform.translation, 0.f),
                        glm::vec3{0.f},
                        glm::vec3{0.f}});
                }


                int offSetCount = 0;
                int const gridWidth = 3;

                for(std::string artefactName : item.extraModels)
                {
                    std::string path = "models/" + artefactName + ".obj";

                    for(RengbModel::Vertex vertex : RengbModel::loadVertices(path))
                    {
                    
                        glm::vec3 position = item.specificTransform.mat4() * glm::vec4(vertex.position.x + ((1.5f + (offSetCount%gridWidth)) * item.kerning * 2), vertex.position.y, vertex.position.z + ((offSetCount/gridWidth) * item.lineSpace * 3.f), 1.f);

                        builtText.vertices.push_back(RengbModel::Vertex{
                            position,
                            vertex.color,
                            vertex.flashColor
                        });
                    }
                    std::cout<<std::endl;
                    ++offSetCount;
                }
            }
            // Construct model from the vertex lists
            return RengbModel::createModelFromBuilder(device, builtText);
        };
        void makeTextObject()
        {
            int characterIndex = totalChars-1;
            int rollingVertexOffset = 0;
    
            for(RengbModel::Vertex &vertex : fontTemplate)
            {

                if(ceil(vertex.position.x) < widthPerChar * characterIndex) // Ceil because it acts undesirably at values near to whole
                {
                    vertexOffsets[characterIndex] = rollingVertexOffset;

                    characterIndex = characterIndex -1;
                    
                }
                ++rollingVertexOffset;
                vertex.position.x = vertex.position.x - widthPerChar * characterIndex; 
            }
    
            vertexOffsets[characterIndex] = fontTemplate.size();
        }
        void addItem(std::string textToAdd = "NO_TEXT", glm::vec3 translation = {}, float scale = 1.f, glm::vec3 rotation = {});

	};
}
