#include <iostream>
#include <cmath>

#include "rengb_traversal_system.hpp"
#include "components/renderable.hpp"
#include "components/transform.hpp"
#include "components/world.hpp"
#include "components/collisionshape.hpp"
#include "core/rengb_coordinator.hpp"
#include "rengb_model.hpp"

namespace rengb
{
	extern Coordinator gCoordinator;


	struct SimplePushConstantData
	{
		glm::mat4 modelMatrix{1.f};
		glm::vec4 darken{0.f};
		glm::vec4 widthHeightTime{0.f};
	};

	TraversalSystem::TraversalSystem(RengbDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout): rengbDevice{device}
	{
		createPipelineLayout(globalSetLayout);
		createPipeline(renderPass);
	}

	void TraversalSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		if (vkCreatePipelineLayout(rengbDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
			VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void TraversalSystem::createPipeline(VkRenderPass renderPass)
	{
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		RengbPipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		rengbPipeline = std::make_unique<RengbPipeline>(
			rengbDevice,

			"shaders/world.vert.spv",
			"shaders/boil.frag.spv",
			pipelineConfig);
	}

	void TraversalSystem::renderGameObjects(FrameInfo &frameInfo, float time, float width, float height, glm::vec4 &cumFlash)
	{
		time = time > glm::two_pi<float>() ? 0.f : time + (frameInfo.frameTime); 

		rengbPipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0,
			1,
			&frameInfo.globalDescriptorSet,
			0,
			nullptr);

		SimplePushConstantData push{};

		push.widthHeightTime.x = width;
		push.widthHeightTime.y = height;
		push.widthHeightTime.z = time;

		push.darken = cumFlash;

		for (auto const &entity : mEntities)
		{
			auto &terrain = gCoordinator.GetComponent<Terrain>(entity);
			auto &world = gCoordinator.GetComponent<World>(entity);
			auto &renderable = gCoordinator.GetComponent<Renderable>(entity);


			if(glm::abs(renderable.loading) > glm::epsilon<float>())
			{

				push.modelMatrix = glm::mat4{
					{1, 0.0f, 0.0f, 0.0f},
					{0.0f, 1.f, 0.0f, 0.0f},
					{0.0f, 0.0f, 1.f, 0.0f},
					{0.0f, renderable.loading, 0.0f, 1.0f}};
					renderable.loading -= renderable.loading*8*frameInfo.frameTime;
			} else
			{
				push.modelMatrix = glm::mat4{
					{1, 0.0f, 0.0f, 0.0f},
					{0.0f, 1.f, 0.0f, 0.0f},
					{0.0f, 0.0f, 1.f, 0.0f},
					{0.0f, 0.0f, 0.0f, 1.0f}};
			}


			vkCmdPushConstants(
				frameInfo.commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantData),
				&push);
			terrain.model->bind(frameInfo.commandBuffer);
			terrain.model->draw(frameInfo.commandBuffer);

		}
	}

	void TraversalSystem::Update(glm::vec3 playerCurrentPos)
	{
		for (auto const &entity : mEntities)
		{
			setGoalVertex(entity, aabbClosestVertex(entity, playerCurrentPos));
		}
	}



	std::vector<RengbModel::Vertex> TraversalSystem::convertMeshToVertexArray(Triangle_mesh& mesh) 
	{
		std::vector<RengbModel::Vertex> vertices;
		vertices.reserve(mesh.number_of_vertices()); // Reserve space for efficiency
	
		for (auto face : mesh.faces()) {
			// Get the 3 vertices of the triangle
			for (auto v : CGAL::vertices_around_face(mesh.halfedge(face), mesh)) {
				Point_3 p = mesh.point(v);
	
				// Create a vertex and add it directly (no indexing, just raw data)
				
				if(glm::epsilonEqual(static_cast<float>(p.y()), 0.f, 0.1f))
				{
					vertices.push_back({
						glm::vec3(static_cast<float>(p.x()), static_cast<float>(p.y())+5.f, static_cast<float>(p.z())),
						glm::vec3(0.0f, 0.0f, 0.0f),  // White color
						glm::vec3(0.2f, 0.1f, 0.0f)   // Red flashColor
					});
				} else
				{
					vertices.push_back({
						glm::vec3(static_cast<float>(p.x()), static_cast<float>(p.y()), static_cast<float>(p.z())),
						glm::vec3(0.3f, 0.1f, 0.0f),  // White color
						glm::vec3(1.0f, 0.6f, 0.0f)   // Red flashColor
					});			
				}

			}
		}
	
		return vertices;
	}

	void TraversalSystem::loadTerrainMesh(Entity ter, std::string terrainName, float rotInDeg, glm::vec3 translation, RengbDevice &device)
	{

		auto areAnglesEqual = [](float angle1, float angle2, float epsilon = 1e-6f) 
		{
			float diff = glm::mod(angle1 - angle2 + glm::pi<float>(), glm::two_pi<float>()) - glm::pi<float>();
			return glm::epsilonEqual(diff, 0.0f, epsilon);
		};

		auto &mesh = gCoordinator.GetComponent<Terrain>(ter).mesh;
		auto &model = gCoordinator.GetComponent<Terrain>(ter).model;
		std::vector<Point_3> points;
		std::vector<std::vector<std::size_t>> faces;

		if (!CGAL::IO::read_OBJ("../" + terrainName, points, faces))
		{
			std::cerr << "Error reading OBJ file!" << std::endl;
		}

		if(areAnglesEqual(rotInDeg, glm::half_pi<float>()))
		{ // swap x and z, set new z to negative
		    for (auto &point : points)
    		{
				point = Point_3(point.z() + translation.x, point.y() + translation.y, -point.x() + translation.z);
			}
		} else if(areAnglesEqual(rotInDeg, glm::pi<float>()))
		{ // set x and z to negative
		    for (auto &point : points)
    		{
				point = Point_3(-point.x() + translation.x, point.y() + translation.y, -point.z() + translation.z);
			}
		} else if(areAnglesEqual(rotInDeg, glm::three_over_two_pi<float>()))
		{ // swap x and z set new x to negaitve
		    for (auto &point : points)
    		{
				point = Point_3(-point.z() + translation.x, point.y() + translation.y, point.x() + translation.z);
			}
		}

		CGAL::Polygon_mesh_processing::polygon_soup_to_polygon_mesh(points, faces, mesh);
		CGAL::Polygon_mesh_processing::triangulate_faces(mesh);

		model = RengbModel::createModelFromVertices(device, std::move(convertMeshToVertexArray(mesh)));
	}
	void TraversalSystem::buildDistanceMap(Entity ter)
	{
		auto &terrain = gCoordinator.GetComponent<Terrain>(ter);
		auto &mesh = terrain.mesh;
		auto &distance_map = terrain.distance_map;
		auto &goal = terrain.goal;

		// Define a property map to store geodesic distances
		distance_map = mesh.add_property_map<Vertex_index, double>("v:dist", 0.0).first;

		// select the goal position (target vertex)
		Vertex_index goal_vertex = *std::next(mesh.vertices_begin(), goal); // 10th vertex

		// compute the geodesic distance map from the goal
		HeatMethod heat_method(mesh);
		heat_method.add_source(goal_vertex);
		heat_method.estimate_geodesic_distances(distance_map);

	}
	std::vector<Vertex_index> TraversalSystem::computerPath(Entity ter, Vertex_index start) // Vertex_index
	{
		auto &terrain = gCoordinator.GetComponent<Terrain>(ter);
		auto &mesh = terrain.mesh;
		auto &goal = terrain.goal;
		auto &distance_map = terrain.distance_map;
		std::vector<Vertex_index> path;
		Vertex_index current = start;
		path.push_back(current);

		while (current != goal)
		{
			Vertex_index next_vertex;
			double min_distance = std::numeric_limits<double>::infinity();

			for (auto neighbor : CGAL::vertices_around_target(mesh.halfedge(current), mesh))
			{

				double neighbor_distance = distance_map[neighbor];

				if (neighbor_distance < min_distance)
				{
					min_distance = neighbor_distance;
					next_vertex = neighbor;
				}
			}


			if (min_distance == std::numeric_limits<double>::infinity())
			{
				std::cerr << "No valid path found!" << std::endl;
				return {};
			}

			current = next_vertex;
			path.push_back(current);

		}

		return path;
	}

	std::vector<glm::vec3> TraversalSystem::extractPathPositions(Entity ter, const std::vector<Vertex_index> &path) // Point_3
	{
		auto &terrain = gCoordinator.GetComponent<Terrain>(ter);
		auto &mesh = terrain.mesh;

		std::vector<glm::vec3> path_positions;
		for (auto v : path)
		{
			glm::vec3 cartesianPos{mesh.point(v).x(), mesh.point(v).y(), mesh.point(v).z()};

			path_positions.push_back(cartesianPos);
		}
		return path_positions;
	}

	Vertex_index TraversalSystem::getStartVertex(Entity ter)
	{
		auto &terrain = gCoordinator.GetComponent<Terrain>(ter);
		auto &mesh = terrain.mesh;

		return *std::next(mesh.vertices_begin(), 0); 
	}

	void TraversalSystem::setGoalVertex(Entity ter, Vertex_index goal)
	{
		auto &terrain = gCoordinator.GetComponent<Terrain>(ter);
		auto &mesh = terrain.mesh;

		terrain.goal = *std::next(mesh.vertices_begin(), goal); 

		buildDistanceMap(ter);
	}

	std::vector<glm::vec3> TraversalSystem::getPathPositions(Entity ter, glm::vec3 realPosition)
	{
		return extractPathPositions(ter, computerPath(ter, aabbClosestVertex(ter, realPosition)));
	}

	void TraversalSystem::buildAABBTree(Entity ter)
	{
		auto &terrain = gCoordinator.GetComponent<Terrain>(ter);
		auto &mesh = terrain.mesh;
		auto &tree = terrain.tree;

		tree = std::make_unique<AABBTree>(faces(mesh).first, faces(mesh).second, mesh);
		tree->accelerate_distance_queries();
	}

	void TraversalSystem::modifyTerrain(Entity ter, glm::vec3 realPosition, RengbDevice &device)
	{
		auto &terrain = gCoordinator.GetComponent<Terrain>(ter);
		auto &collision = gCoordinator.GetComponent<CollisionShape>(ter);
		Vertex_index closestVertex = aabbClosestVertex(ter, realPosition);

		Point_3 original_point = terrain.mesh.point(closestVertex);

		float newY = original_point.y();
		if(original_point.y() == 0.f)
		{
			newY = -0.8f;
			collision.colliderCollection.push_back(SphereCollider{{original_point.x(), original_point.y()+0.7f, original_point.z()}, 1.f});
		} else
		{
			newY -= (3.f+newY)*0.1f;			
		}

		Point_3 new_point = Point_3(original_point.x(), newY, original_point.z() );

		terrain.mesh.point(closestVertex) = new_point;
		buildAABBTree(ter);
		terrain.model = RengbModel::createModelFromVertices(device, std::move(convertMeshToVertexArray(terrain.mesh)));
	}

	Vertex_index TraversalSystem::aabbClosestVertex(Entity ter, glm::vec3 realPosition, float skew)
	{
		auto &terrain = gCoordinator.GetComponent<Terrain>(ter);
		auto &tree = terrain.tree;
		auto &mesh = terrain.mesh;
		glm::vec3 randomSkew = glm::vec3((float)(rand()) / (float)(RAND_MAX) - 0.5,(float)(rand()) / (float)(RAND_MAX) - 0.5,(float)(rand()) / (float)(RAND_MAX) - 0.5)*skew;
		const Point_3 querey_point = Point_3(realPosition.x + randomSkew.x, realPosition.y + randomSkew.y, realPosition.z + randomSkew.z);

		std::pair<Point_3, Triangle_mesh::Face_index> closest = tree->closest_point_and_primitive(querey_point);

		Point_3 closest_point = closest.first;

		Vertex_index closest_vertex;
		double min_distance = std::numeric_limits<double>::infinity();

		for (auto v : CGAL::vertices_around_face(halfedge(closest.second, mesh), mesh))
		{
			double distance = CGAL::squared_distance(mesh.point(v), closest_point);
			if (distance < min_distance)
			{
				min_distance = distance;
				closest_vertex = v;
			}
		}

		return closest_vertex;
	}

	void TraversalSystem::destroySystem()
	{
		vkDestroyPipelineLayout(rengbDevice.device(), pipelineLayout, nullptr);
		rengbPipeline.reset();
	}

	TraversalSystem::~TraversalSystem()
	{
		destroySystem();
	}

}
