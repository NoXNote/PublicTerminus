#pragma once

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
#include <rengb_model.hpp>
#include <iostream>
#include <fstream>
#include <glm/gtx/quaternion.hpp>
#include "unordered_map"

typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_3 Point_3;
typedef CGAL::Surface_mesh<Point_3> Triangle_mesh;
typedef Triangle_mesh::Vertex_index Vertex_index;
typedef CGAL::Heat_method_3::Surface_mesh_geodesic_distances_3<Triangle_mesh> HeatMethod;
typedef CGAL::Surface_mesh<Point_3>::Property_map<Vertex_index, double> DistanceMap;

typedef CGAL::AABB_face_graph_triangle_primitive<Triangle_mesh> Primitive;
typedef CGAL::AABB_traits_3<Kernel, Primitive> AABBTraits;
typedef CGAL::AABB_tree<AABBTraits> AABBTree;

namespace rengb
{
    struct Terrain
    {
        Triangle_mesh mesh;
        Vertex_index goal;
        DistanceMap distance_map;
    	std::shared_ptr<AABBTree> tree;
        bool justToDoor{false};
        std::shared_ptr<RengbModel> model{};
        std::unordered_map<Vertex_index, size_t> indicesOfColliders;

        glm::vec3 randomPoint()
        {
            std::vector<Triangle_mesh::Vertex_index> interiorVertices;
            
            for (auto v : mesh.vertices()) {
                size_t neighborCount = 0;
                
                // store interior vertecies
                for (auto neighbor : CGAL::vertices_around_target(mesh.halfedge(v), mesh)) {
                    ++neighborCount;
                }
        
                if (neighborCount >= 4) {
                    interiorVertices.push_back(v);
                }
            }

            size_t partIndex = std::rand() % (interiorVertices.size() - 1);
            Triangle_mesh::Vertex_index chosenVertex = interiorVertices[partIndex];
            Point_3 p = mesh.point(chosenVertex);

            
            return glm::vec3(static_cast<float>(p.x()), static_cast<float>(p.y()), static_cast<float>(p.z()));
        };

        glm::vec3 randomPoint(glm::vec3 playerPosition)
        {

            std::vector<Triangle_mesh::Vertex_index> interiorVertices;
            
            for (auto v : mesh.vertices()) {
                size_t neighborCount = 0;
                
                // store interior vertecies
                for (auto neighbor : CGAL::vertices_around_target(mesh.halfedge(v), mesh)) {
                    ++neighborCount;
                }
        
                if (neighborCount >= 4) {
                    interiorVertices.push_back(v);
                }
            }

            glm::vec3 randomPosition;
            // Try to find a position that is not too close to player
            do
            {
                size_t partIndex = std::rand() % (interiorVertices.size() - 1);
                Triangle_mesh::Vertex_index chosenVertex = interiorVertices[partIndex];
                Point_3 p = mesh.point(chosenVertex);
                randomPosition = glm::vec3(static_cast<float>(p.x()), static_cast<float>(p.y()), static_cast<float>(p.z()));
            } while (glm::dot((randomPosition - playerPosition), (randomPosition - playerPosition)) < 100.f);
            std::cout<<std::endl;
            return randomPosition;
        };
    };
}