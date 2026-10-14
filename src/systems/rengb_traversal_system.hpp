#pragma once
#include "rengb_system.hpp"

#include "rengb_camera.hpp"
#include "rengb_device.hpp"
#include "rengb_frame_info.hpp"
//#include "rengb_game_object.hpp"
#include "rengb_pipeline.hpp"
#include <iostream>
#include <fstream>
#include <glm/glm.hpp>

#include "components/terrain.hpp"

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Surface_mesh.h>
// GETTING PATH
#include <CGAL/Heat_method_3/Surface_mesh_geodesic_distances_3.h>
// GETTING THE MODEL READ FINE EVEN WITH QUADS
#include <CGAL/Polygon_mesh_processing/IO/polygon_mesh_io.h>
#include <CGAL/Polygon_mesh_processing/polygon_soup_to_polygon_mesh.h>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
// GETTING THE MODEL READ FINE EVEN WITH QUADS
#include <CGAL/Polygon_mesh_processing/triangulate_faces.h>

#include <CGAL/boost/graph/helpers.h>
// GETTING THE MODEL READ FINE EVEN WITH QUADS
#include <CGAL/IO/polygon_soup_io.h>
#include <CGAL/boost/graph/property_maps.h>
// FINDING CLOSEST POINT FROM PLAYER AND ENEMY TO MESH
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits_3.h>
#include <CGAL/AABB_face_graph_triangle_primitive.h>

typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_3 Point_3;
typedef CGAL::Surface_mesh<Point_3> Triangle_mesh;
typedef Triangle_mesh::Vertex_index Vertex_index;
typedef CGAL::Heat_method_3::Surface_mesh_geodesic_distances_3<Triangle_mesh> HeatMethod;
typedef CGAL::Surface_mesh<Point_3>::Property_map<Vertex_index, double> DistanceMap;

typedef CGAL::AABB_face_graph_triangle_primitive<Triangle_mesh> Primitive;
typedef CGAL::AABB_traits_3<Kernel, Primitive> AABBTraits;
typedef CGAL::AABB_tree<AABBTraits> AABBTree;

using Transform = CGAL::Aff_transformation_3<Kernel>;


namespace rengb{
    class TraversalSystem : public System
    {
	public:
		TraversalSystem(
			RengbDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~TraversalSystem();

        void Init();

        void Update(glm::vec3 playerCurrentPos);

        static void loadTerrainMesh(Entity ter, std::string terrainName, float rotInRad, glm::vec3 translation, RengbDevice &device);
        static void buildDistanceMap(Entity ter);
        static std::vector<Vertex_index> computerPath(Entity ter, Vertex_index start);
        static std::vector<glm::vec3> extractPathPositions(Entity ter, const std::vector<Vertex_index>& path); // point
        static Vertex_index getStartVertex(Entity ter);
        static void setGoalVertex(Entity ter, Vertex_index goal);
        static std::vector<glm::vec3> getPathPositions(Entity ter, glm::vec3 realPosition);
		static void modifyTerrain(Entity ter, glm::vec3 realPosition, RengbDevice &device);

        static void buildAABBTree(Entity ter);
        static Vertex_index aabbClosestVertex(Entity ter, glm::vec3 realPosition, float skew = 8.f);

		static std::vector<RengbModel::Vertex> convertMeshToVertexArray(Triangle_mesh&  mesh);

        float updateDistance;
        glm::vec3 lastUpdatedAt;


		TraversalSystem(const TraversalSystem &) = delete;
		TraversalSystem &operator=(const TraversalSystem &) = delete;



		void renderGameObjects(FrameInfo &frameInfo, float time, float width, float height, glm::vec4 &cumFlash);


		void destroySystem();

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		RengbDevice &rengbDevice;

		std::unique_ptr<RengbPipeline> rengbPipeline;
		VkPipelineLayout pipelineLayout;
		float time{0.f};
	};
}
