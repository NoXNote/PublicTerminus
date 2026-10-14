#include <iostream>

#include "rengb_traversal_system.hpp"

#include "components/rigidbody.hpp"
#include "components/transform.hpp"
#include "components/collisionshape.hpp"

#include "core/rengb_coordinator.hpp"

#include <utility>
#include <glm/gtc/epsilon.hpp>

namespace rengb
{
	extern Coordinator gCoordinator;

	void TraversalSystem::Init()
	{
	}

	void TraversalSystem::Update(glm::vec3 playerCurrentPos, Entity roomPlayerIsIn)
	{
		// use OpenMP to do all heat calculations at once
		// pass entity of room currently in nav system iterates over all navmaps.
		// if the room it is in is prox 0, calculate source vertex being player position in room.
		// if the room is prox 1, caluclate source vertex being door leading into the room
		// (read doors in room, and find vertex closest to door that holds entity of prox 0 room)
	}

	void TraversalSystem::loadTerrainMesh(Entity ter, std::string terrainName, float rotInDeg)
	{

		auto areAnglesEqual = [](float angle1, float angle2, float epsilon = 1e-6f) 
		{
			float diff = glm::mod(angle1 - angle2 + glm::pi<float>(), glm::two_pi<float>()) - glm::pi<float>();
			return glm::epsilonEqual(diff, 0.0f, epsilon);
		};

		auto &mesh = gCoordinator.GetComponent<Terrain>(ter).mesh;
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
				point = Point_3(point.z(), point.y(), -point.x());
			}
		} else if(areAnglesEqual(rotInDeg, glm::pi<float>()))
		{ // set x and z to negative
		    for (auto &point : points)
    		{
				point = Point_3(-point.x(), point.y(), -point.z());
			}
		} else if(areAnglesEqual(rotInDeg, glm::three_over_two_pi<float>()))
		{ // swap x and z set new x to negaitve
		    for (auto &point : points)
    		{
				point = Point_3(-point.z(), point.y(), point.x());
			}
		}

		CGAL::Polygon_mesh_processing::polygon_soup_to_polygon_mesh(points, faces, mesh);

		CGAL::Polygon_mesh_processing::triangulate_faces(mesh);
	}
	void TraversalSystem::buildDistanceMap(Entity ter)
	{
		auto &terrain = gCoordinator.GetComponent<Terrain>(ter);
		auto &mesh = terrain.mesh;
		auto &distance_map = terrain.distance_map;
		auto &goal = terrain.goal;

		// Define a property map to store geodesic distances
		// auto vpm = mesh.points(); // Vertex point map
		distance_map = mesh.add_property_map<Vertex_index, double>("v:dist", 0.0).first;

		// select the goal position (target vertex)
		Vertex_index goal_vertex = *std::next(mesh.vertices_begin(), goal); // 10th vertex

		// compute the geodesic distance map from the goal
		HeatMethod heat_method(mesh);
		heat_method.add_source(goal_vertex);
		heat_method.estimate_geodesic_distances(distance_map);

		// print computed geodesic distances for debugging
		// for (auto v : mesh.vertices()) {
		//     std::cout << "Vertex " << v << " Distance: " << distance_map[v] << std::endl;
		// }
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

		// std::cout<<"START VERTEX IS: " <<(int)start.idx()<<std::endl<<std::endl;
		// std::cout<<"GOAL VERTEX IS: " <<(int)goal.idx()<<std::endl<<std::endl;
		while (current != goal)
		{
			// std::cout<<"a";
			Vertex_index next_vertex;
			double min_distance = std::numeric_limits<double>::infinity();

			for (auto neighbor : CGAL::vertices_around_target(mesh.halfedge(current), mesh))
			{

				double neighbor_distance = distance_map[neighbor];
				// std::cout<<"Neighbor is number: "<< neighbor.idx() << " at position: " << mesh.point(neighbor) << " at distance: " << neighbor_distance<<std::endl;
				if (neighbor_distance < min_distance)
				{
					min_distance = neighbor_distance;
					next_vertex = neighbor;
				}
			}

			// iterate over neighbors to find the next step
			// for (auto halfedge : CGAL::halfedges_around_target(mesh.halfedge(current), mesh))
			// {
			//     //std::cout<<"b";
			//     std::cout << "half edge is number " << halfedge.idx() << std::endl;
			//     Vertex_index neighbor = target(halfedge, mesh);
			//     std::cout << "Neighbor is number " << neighbor.idx() << std::endl;
			//     double neighbor_distance = distance_map[neighbor];
			//     std::cout<<"Neightbour Distance of "<< current.idx() << " is "<<neighbor_distance<<std::endl;

			//     if (neighbor_distance < min_distance) {
			//         std::cout<<"YES! next is "<< neighbor.idx() << std::endl;
			//         min_distance = neighbor_distance;
			//         next_vertex = neighbor;
			//     }
			// }
			// std::cout << "checked all half edges for the vertex number " << current.idx()<<std::endl<<std::endl;
			if (min_distance == std::numeric_limits<double>::infinity())
			{
				std::cerr << "No valid path found!" << std::endl;
				return {};
			}

			current = next_vertex;
			path.push_back(current);
			// std::cout<<current.idx()<< " at position " << mesh.point(current) << std::endl;
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
			// std::cout<<cartesianPos.x<<"x "<<cartesianPos.y<<"y "<<cartesianPos.z<<"z "<<std::endl;
			path_positions.push_back(cartesianPos);
		}
		return path_positions;
	}

	Vertex_index TraversalSystem::getStartVertex(Entity ter)
	{
		auto &terrain = gCoordinator.GetComponent<Terrain>(ter);
		auto &mesh = terrain.mesh;

		return *std::next(mesh.vertices_begin(), 487); // 1st vertex TEMP
	}

	void TraversalSystem::setGoalVertex(Entity ter)
	{
		auto &terrain = gCoordinator.GetComponent<Terrain>(ter);
		auto &mesh = terrain.mesh;

		terrain.goal = *std::next(mesh.vertices_begin(), 20); // 20th TEMP vertex

		buildDistanceMap(ter);
	}

	std::vector<glm::vec3> TraversalSystem::getPathPositions(Entity ter, glm::vec3 realPosition)
	{
		return extractPathPositions(ter, computerPath(ter, aabbCloseestVertex(ter, realPosition)));
	}

	void TraversalSystem::buildAABBTree(Entity ter)
	{
		auto &terrain = gCoordinator.GetComponent<Terrain>(ter);
		auto &mesh = terrain.mesh;
		auto &tree = terrain.tree;

		tree = std::make_unique<AABBTree>(faces(mesh).first, faces(mesh).second, mesh);
		tree->accelerate_distance_queries();
	}

	Vertex_index TraversalSystem::aabbCloseestVertex(Entity ter, glm::vec3 realPosition)
	{
		auto &terrain = gCoordinator.GetComponent<Terrain>(ter);
		auto &tree = terrain.tree;
		auto &mesh = terrain.mesh;

		const Point_3 querey_point = Point_3(realPosition.x, realPosition.y, realPosition.z);

		std::pair<Point_3, Triangle_mesh::Face_index> closest = tree->closest_point_and_primitive(querey_point);
		// auto closest = tree.closest_point_and_primitive(querey_point);
		//  Point_3 closest_point = closest.first; // closest point on the surface
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

}