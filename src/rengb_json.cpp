#include "rengb_json.hpp"

#include "rengb_utils.hpp"
#include "components/transform.hpp"
#include "components/world.hpp"
#include "components/rigidbody.hpp"
#include "components/renderable.hpp"
#include "components/collisionshape.hpp"
#include "components/enemypart.hpp"
#include "components/enemyparent.hpp"
#include "components/door.hpp"
#include "components/gun_object.hpp"
#include "components/player.hpp"
#include "components/held_object.hpp"
#include "components/room.hpp"
#include "components/sleep.hpp"
#include "components/projectile.hpp"
#include <glm/gtc/epsilon.hpp>
// maybe make not a system
namespace rengb
{
	extern Coordinator gCoordinator;

	Entity RengbJson::generateEnemy(std::string name, RengbDevice &device, glm::vec3 intialPostion, Entity parentRoom = 0, bool active = false)
	{
		std::ifstream f("../data/" + name + ".json");
		json data = json::parse(f);

		Entity parent = gCoordinator.CreateEntity();

		gCoordinator.AddComponent(
			parent,
			EnemyParent{
				.hitpoints = data["hitpoints"],
				.type = data["type"]});

		gCoordinator.AddComponent(
			parent,
			Transform{
				.translation = {intialPostion.x, intialPostion.y -3.1f, intialPostion.z}});

		gCoordinator.AddComponent(
			parent,
			RigidBody{
				.acceleration = glm::vec3(0, 20.f,0), .mass = 10.f, .friction = 10.f});

		gCoordinator.AddComponent(
			parent,
			CollisionShape{
				.colliderCollection{AABB{glm::vec3(-0.5, -3, -0.5), glm::vec3(0.5, 0, 0.5)}}});
				//.collider = SphereCollider{glm::vec3(0.f), 2.f}});

		gCoordinator.AddComponent(parent, Projectile{});


		gCoordinator.AddComponent(parent, Renderable{.model = RengbModel::getOrMakeModelFromFile(device, "models/marker.obj")});
		
		if(active)
		{
			gCoordinator.AddComponent(
				parent,
				World{});			
		} else 
		{
			gCoordinator.AddComponent(
				parent,
				Sleep{parentRoom});		
		}

		auto &enemyParent = gCoordinator.GetComponent<EnemyParent>(parent);

		if (data.contains("Torso") && data["Torso"].is_array())
		{
			size_t partIndex = std::rand() % (data["Torso"].size());

			{
				std::string filepath{"models/torso_" + (std::string)(data["Torso"][partIndex]["partname"]) + ".obj"};

				std::shared_ptr<RengbModel> rengbModel;

				rengbModel = RengbModel::getOrMakeModelFromFile(device, filepath);

				Entity entity = gCoordinator.CreateEntity();

				gCoordinator.AddComponent(
					entity,
					Renderable{
						.model = rengbModel,
						.life = 1.f,
						.partType = 0});

				gCoordinator.AddComponent(
					entity,
					EnemyPart{
						parent,
						glm::vec3{0, 0, 0},
						10,
						"body"
					});

				gCoordinator.AddComponent(
					entity,
					Transform{
						.translation = glm::vec3{0, 0, 0}});

				enemyParent.children.emplace(std::move(entity));
			}

			{
				std::string filepath{"models/hip_" + (std::string)(data["Torso"][partIndex]["partname"]) + ".obj"};

				std::shared_ptr<RengbModel> rengbModel;

				rengbModel = RengbModel::getOrMakeModelFromFile(device, filepath);

				Entity entity = gCoordinator.CreateEntity();

				gCoordinator.AddComponent(
					entity,
					Renderable{
						.model = rengbModel,
						.life = 1.f,
						.partType = 4});

				gCoordinator.AddComponent(
					entity,
					EnemyPart{
						parent,
						glm::vec3{0, 0, 0},
						10,
						"hip"
					});

				gCoordinator.AddComponent(
					entity,
					Transform{
						.translation = glm::vec3{0, 0, 0}});

				enemyParent.children.emplace(std::move(entity));
			}
		}
		if (data.contains("Head") && data["Head"].is_array())
		{
			size_t partIndex = std::rand() % (data["Head"].size());

			std::string filepath{"models/head_" + (std::string)(data["Head"][partIndex]["partname"]) + ".obj"};

			std::shared_ptr<RengbModel> rengbModel;

			rengbModel = RengbModel::getOrMakeModelFromFile(device, filepath);

			Entity entity = gCoordinator.CreateEntity();

			gCoordinator.AddComponent(
				entity,
				Renderable{
					.model = rengbModel,
					.life = 1.f,
					.partType = 0});

			gCoordinator.AddComponent(
				entity,
				EnemyPart{
					parent,
					glm::vec3{0, 0, 0},
					10,
					"head"
				});

			gCoordinator.AddComponent(
				entity,
				Transform{
					.translation = glm::vec3{0, 0, 0}});

			enemyParent.children.emplace(std::move(entity));
		}
		if (data.contains("Leg") && data["Leg"].is_array())
		{
			size_t partIndex = std::rand() % (data["Leg"].size());
			std::string const pathPrefix[] = {"thigh", "shin", "foot"};

			for (int i = 0; i < 3; ++i)
			{
				std::string filepath{"models/" + pathPrefix[i] + "_" + (std::string)(data["Leg"][partIndex]["partname"]) + ".obj"};

				std::shared_ptr<RengbModel> rengbModel;

				rengbModel = RengbModel::getOrMakeModelFromFile(device, filepath);

				Entity entity = gCoordinator.CreateEntity();

				gCoordinator.AddComponent(
					entity,
					Renderable{
						.model = rengbModel,
						.life = 1.f,
						.partType = i + 1,
						.mirrorable = true});

				gCoordinator.AddComponent(
					entity,
					EnemyPart{
						parent,
						glm::vec3{0, 0, 0},
						10,
						pathPrefix[i]
					});

				gCoordinator.AddComponent(
					entity,
					Transform{
						.translation = glm::vec3{0, 0, 0}});

				enemyParent.children.emplace(std::move(entity));
			}
		}

		if (data.contains("Arm") && data["Arm"].is_array())
		{
			size_t partIndex = std::rand() % (data["Arm"].size());
			std::string const pathPrefix[] = {"three", "fore", "hand"};

			for (int i = 0; i < 3; ++i)
			{
				std::string filepath{"models/" + pathPrefix[i] + "_" + (std::string)(data["Arm"][partIndex]["partname"]) + ".obj"};

				std::shared_ptr<RengbModel> rengbModel;

				rengbModel = RengbModel::getOrMakeModelFromFile(device, filepath);

				Entity entity = gCoordinator.CreateEntity();

				gCoordinator.AddComponent(
					entity,
					Renderable{
						.model = rengbModel,
						.life = 1.f,
						.partType = i + 5,
						.mirrorable = true});

				gCoordinator.AddComponent(
					entity,
					EnemyPart{
						parent,
						glm::vec3{0, 0, 0},
						10,
						pathPrefix[i]
					});

				gCoordinator.AddComponent(
					entity,
					Transform{
						.translation = glm::vec3{0, 0, 0}});

				enemyParent.children.emplace(std::move(entity));
			}
		}

		if (data.contains("Gun") && data["Gun"].is_array())
		{
			std::ifstream fp("../data/" + name + "_positions.json");
			json dataPositions = json::parse(fp);

			size_t partIndex = std::rand() % (data["Gun"].size());



			std::string filepath{"models/" + (std::string)data["Gun"][partIndex]["partname"] + ".obj"};

			std::shared_ptr<RengbModel> rengbModel;

			rengbModel = RengbModel::getOrMakeModelFromFile(device, filepath);

			Entity entity = gCoordinator.CreateEntity();

			gCoordinator.AddComponent(
				entity,
				Renderable{
					.model = rengbModel,
					.life = 1.f,
					.partType = 5,
					.mirrorable = false});


			gCoordinator.AddComponent(
				entity,
				EnemyPart{
					parent,
					glm::vec3{0, 0, 0},
					10,
					"fore"
				});

			gCoordinator.AddComponent(
				entity,
				Transform{
					.translation = glm::vec3(0.f)
				});
			
			enemyParent.children.emplace(std::move(entity));
		}

		return parent;
	}

	void RengbJson::generateLevel(std::string name, RengbDevice &device, int limit, Entity originatingDoorID, int depth) // this is big brain, make it recrusive. 1. Read the door position radius orientation and originiating room from the JSON, and create & write to a new door json. Call the generate level function again, passing the ID of the newly created door entity. At the begining of the function, if ID isnt -1 (default case) write to the passed door entity the id of the newly created room.
	{
		auto areAnglesEqual = [](float a, float b, float epsilon = 1e-5f) 
		{
			auto normaliseAngle = [](float angle) -> float 
			{
				constexpr double twoPi = glm::two_pi<double>();
				double dAngle = static_cast<double>(angle);
				dAngle = std::fmod(dAngle, twoPi);
				if (dAngle < 0.0) dAngle += twoPi;
				return static_cast<float>(dAngle);
			};
		
			float normA = normaliseAngle(a);
			float normB = normaliseAngle(b);
			return glm::epsilonEqual(normA, normB, epsilon);
		};

		std::ifstream f("../data/" + name + ".json");
		json data = json::parse(f);

		Entity room = gCoordinator.CreateEntity();

		std::shared_ptr<RengbModel> rengbModel;
		std::vector<Entity> doorsForRoom; // include all doors in room, creates new entities, except for one which joins the last room door to this room. Selected randomly

		// the angle in radians the new room will have to rotate to connect with the old room
		float toRotateByToJoin{0.f};
		glm::vec3 toTranslateByToJoin{0.f}; //the translation to apply to the new room to join the new room's door with the old room's door

		// later iterate over the vector to generate offshoot rooms.
		if (data.contains("doors") && data["doors"].is_array())
		{
			auto addDoorComponents = [&](Entity doorEntity, glm::vec3 translation, float rotation, std::shared_ptr<RengbModel> model) 
			{
				gCoordinator.AddComponent(doorEntity, Transform{.translation = translation,
				.rotation = glm::vec3{0.f, rotation, 0.f}});

				gCoordinator.AddComponent(doorEntity, Door{.roomThatIn = room});
				gCoordinator.AddComponent(doorEntity, Renderable{.model = model});
			};


			if (depth > 0)
			{
				std::default_random_engine generator; 

				std::uniform_int_distribution randomDoor(0, (int)(data["doors"].size())); // is a non root room

				std::string filepath{"models/lDoor.obj"};
				std::shared_ptr<RengbModel> model = RengbModel::getOrMakeModelFromFile(device, filepath);

				int indexOfDoorEnterance = randomDoor(generator); // start by getting the index of the door that the old room will be joining to in the new room
				int currentIndex = 0;		  // tracks the index of the door that is itereated over in the json


				for (auto door : data["doors"]) // iterate over ever door in the json
				{
					glm::vec3 newDoorPosition = glm::vec3{door["posx"], door["posy"], door["posz"]};
					float newDoorRotationYRad = glm::radians((float)(door["rotxDeg"]));

					if (currentIndex++ != indexOfDoorEnterance)
					{
						// prepare all the doors that are not the door that is joining to later be transformed, and then be used as doors coming out
						// this creates a brand new door
						Entity doorEntity = gCoordinator.CreateEntity();

						// translation is prior to correcting to stitch to previous room as we do not know which door it will be connected to yet :)

						if (depth == limit)
						{
							gCoordinator.AddComponent(doorEntity, Transform{.translation = newDoorPosition,
								.rotation = glm::vec3{0.f, newDoorRotationYRad, 0.f}});
							gCoordinator.AddComponent(doorEntity, Door{.roomThatIn = room});

							gCoordinator.AddComponent(doorEntity, Renderable{.model = model}); // Add Special model for closed Door
							// add logic to add collider for door to the room AABB

						}
						else
						{
							addDoorComponents(doorEntity, newDoorPosition, newDoorRotationYRad, model);
						}
						

						doorsForRoom.push_back(doorEntity);
					}
					else // dont create another door entity to avoid duplicate door, just read info to know how much to rotate
					{
						glm::vec3 correctedDoorPosition = newDoorPosition;

						auto &origDoorTransform = gCoordinator.GetComponent<Transform>(originatingDoorID);
						gCoordinator.GetComponent<Door>(originatingDoorID).roomToGoTo = room;

						// for the rotation, get previousrot - newRot + 180
						toRotateByToJoin = origDoorTransform.rotation.y - newDoorRotationYRad + glm::pi<float>();

						if (static_cast<int>((toRotateByToJoin/glm::half_pi<float>()) + 0.5f) % 4 == 0) // if (areAnglesEqual(toRotateByToJoin, 0.f))
						{
							correctedDoorPosition = glm::vec3(newDoorPosition.x, newDoorPosition.y, newDoorPosition.z);
						}
						else if (static_cast<int>((toRotateByToJoin/glm::half_pi<float>()) + 0.5f) % 4 == 1) // else if (areAnglesEqual(toRotateByToJoin, glm::half_pi<float>()))
						{ // swap x and z, set new z to negative
							correctedDoorPosition = glm::vec3(newDoorPosition.z, newDoorPosition.y, -newDoorPosition.x);
						}
						else if (static_cast<int>((toRotateByToJoin/glm::half_pi<float>()) + 0.5f) % 4 == 2) // else if (areAnglesEqual(toRotateByToJoin, glm::pi<float>()))
						{ // set x and z to negative
							correctedDoorPosition = glm::vec3(-newDoorPosition.x, newDoorPosition.y, -newDoorPosition.z);
						}
						else if (static_cast<int>((toRotateByToJoin/glm::half_pi<float>()) + 0.5f) % 4 == 3) // else if (areAnglesEqual(toRotateByToJoin, glm::three_over_two_pi<float>()))
						{ // swap x and z set new x to negaitve
							correctedDoorPosition = glm::vec3(-newDoorPosition.z, newDoorPosition.y, newDoorPosition.x);
						}
						else std::cout<<"FAILED for toRotaateByToJoin = " << toRotateByToJoin << " with caluclation of = " << static_cast<int>((toRotateByToJoin/glm::half_pi<float>()) + 0.5f) % 4 << std::endl;

						toTranslateByToJoin = origDoorTransform.translation - correctedDoorPosition;

						gCoordinator.AddComponent(room, Transform{.translation = toTranslateByToJoin, .rotation = glm::vec3(0.f, toRotateByToJoin, 0.f)});
					}
				}
				// now rotate all the doors based on how much the new door needs to rotate to connect with the old door by translating
				for (auto doorEntity : doorsForRoom)
				{
					auto& doorPosition = gCoordinator.GetComponent<Transform>(doorEntity);
					auto& doorDoor = gCoordinator.GetComponent<Door>(doorEntity);
					doorPosition.rotation.y += toRotateByToJoin;
					

					glm::vec3 correctedDoorPosition = doorPosition.translation;

					if (static_cast<int>((toRotateByToJoin/glm::half_pi<float>()) + 0.5f) % 4 == 0) // if (areAnglesEqual(toRotateByToJoin, 0.f))
					{
						correctedDoorPosition = glm::vec3(doorPosition.translation.x, doorPosition.translation.y, doorPosition.translation.z);
					}
					else if (static_cast<int>((toRotateByToJoin/glm::half_pi<float>()) + 0.5f) % 4 == 1) // else if (areAnglesEqual(toRotateByToJoin, glm::half_pi<float>()))
					{ // swap x and z, set new z to negative
						
						correctedDoorPosition = glm::vec3(doorPosition.translation.z, doorPosition.translation.y, -doorPosition.translation.x);
					}
					else if (static_cast<int>((toRotateByToJoin/glm::half_pi<float>()) + 0.5f) % 4 == 2) // else if (areAnglesEqual(toRotateByToJoin, glm::pi<float>()))
					{ // set x and z to negative
						
						correctedDoorPosition = glm::vec3(-doorPosition.translation.x, doorPosition.translation.y, -doorPosition.translation.z);
					}
					else if (static_cast<int>((toRotateByToJoin/glm::half_pi<float>()) + 0.5f) % 4 == 3) // else if (areAnglesEqual(toRotateByToJoin, glm::three_over_two_pi<float>()))
					{ // swap x and z set new x to negaitve
						
						correctedDoorPosition = glm::vec3(-doorPosition.translation.z, doorPosition.translation.y, doorPosition.translation.x);
					}
					else std::cout<<"FAILED for toRotaateByToJoin = " << toRotateByToJoin << " with caluclation of = " << static_cast<int>((toRotateByToJoin/glm::half_pi<float>()) + 0.5f) % 4 << std::endl;

					doorPosition.translation = correctedDoorPosition + toTranslateByToJoin;
				}
				gCoordinator.AddComponent(room, Room{doorsForRoom});
				gCoordinator.GetComponent<Room>(room).doors.push_back(originatingDoorID);
			}
			else // Root of room
			{
				gCoordinator.AddComponent(room, World{});

				std::string filepath{"models/lDoor.obj"};
				std::shared_ptr<RengbModel> model = RengbModel::getOrMakeModelFromFile(device, filepath);

				int iter = 0;
				// is a root room
				for (auto door : data["doors"])
				{
					Entity doorEntity = gCoordinator.CreateEntity();
					addDoorComponents(doorEntity, glm::vec3{(float)door["posx"], (float)door["posy"], (float)door["posz"]}, glm::radians((float)door["rotxDeg"]), model);
					gCoordinator.AddComponent(doorEntity, World{});
					doorsForRoom.push_back(doorEntity);
				}
				gCoordinator.AddComponent(room, Transform{});
				gCoordinator.AddComponent(room, Room{doorsForRoom});
			}
		}
		else if (depth != 0)
		{
			std::cerr << "Impossible, no root room to connect to!" << std::endl;
		}

		if (data.contains("stagemesh"))
		{
			std::string filepath{"models/" + (std::string)(data["stagemesh"]) + ".obj"};

			rengbModel = RengbModel::getOrMakeModelFromFile(device, filepath);

			gCoordinator.AddComponent(room, Renderable{.model = rengbModel});
		}


		if (data.contains("aabbs") && data["aabbs"].is_array())
		{
			gCoordinator.AddComponent(room, CollisionShape{.colliderCollection{}});
			auto &roomCollisions = gCoordinator.GetComponent<CollisionShape>(room);

			glm::vec3 minVertex;
			glm::vec3 maxVertex;
			std::cout<<toRotateByToJoin/glm::pi<float>() << " AABBS equal " << std::endl;
			if (static_cast<int>((toRotateByToJoin/glm::half_pi<float>()) + 0.5f) % 4 == 0)
			{	
				std::cout<<"orientation 0"<<std::endl;
				for (auto aabb : data["aabbs"]) // load and correct AABBs
				{
					minVertex = glm::vec3{aabb["minx"], aabb["miny"], aabb["minz"]};
					maxVertex = glm::vec3{aabb["maxx"], aabb["maxy"], aabb["maxz"]};
					roomCollisions.colliderCollection.push_back(AABB{minVertex, maxVertex});
				}
			}
			else if (static_cast<int>((toRotateByToJoin/glm::half_pi<float>()) + 0.5f) % 4 == 1) 
			{	// swap x and z, set new z to negative
				std::cout<<"orientation 1"<<std::endl;
				for (auto aabb : data["aabbs"]) // load and correct AABBs
				{
					minVertex = glm::vec3{aabb["minz"], aabb["miny"], -(float)(aabb["maxx"])};
					maxVertex = glm::vec3{aabb["maxz"], aabb["maxy"], -(float)(aabb["minx"])};
					roomCollisions.colliderCollection.push_back(AABB{minVertex, maxVertex});
				}
			}
			else if (static_cast<int>((toRotateByToJoin/glm::half_pi<float>()) + 0.5f) % 4 == 2) 
			{	// set x and z to negative
				std::cout<<"orientation 2"<<std::endl;
				for (auto aabb : data["aabbs"]) // load and correct AABBs
				{
					minVertex = glm::vec3{-(float)(aabb["maxx"]), aabb["miny"], -(float)(aabb["maxz"])};
					maxVertex = glm::vec3{-(float)(aabb["minx"]), aabb["maxy"], -(float)(aabb["minz"])};
					roomCollisions.colliderCollection.push_back(AABB{minVertex, maxVertex});
				}
			}
			else if (static_cast<int>((toRotateByToJoin/glm::half_pi<float>()) + 0.5f) % 4 == 3)
			{	// swap x and z set new x to negaitve
				std::cout<<"orientation 3"<<std::endl;
				for (auto aabb : data["aabbs"]) // load and correct AABBs
				{
					minVertex = glm::vec3{-(float)(aabb["maxz"]), aabb["miny"], aabb["minx"]};
					maxVertex = glm::vec3{-(float)(aabb["minz"]), aabb["maxy"], aabb["maxx"]};
					roomCollisions.colliderCollection.push_back(AABB{minVertex, maxVertex});
				}
			}
			else std::cout<<"FAILED for toRotaateByToJoin = " << toRotateByToJoin << " with caluclation of = " << static_cast<int>((toRotateByToJoin/glm::half_pi<float>()) + 0.5f) % 4 << std::endl;
			std::cout<<std::endl;

			if (depth == limit)
			{
				// do something special, terminating room
			}
		}

		if (data.contains("navmesh"))
		{
			gCoordinator.AddComponent(room, Terrain{});

			std::string filepath{"models/" + (std::string)(data["navmesh"]) + ".obj"};


			TraversalSystem::loadTerrainMesh(room, filepath, toRotateByToJoin, toTranslateByToJoin, device);
			TraversalSystem::buildAABBTree(room);

			if (data.contains("enemydifficulty"))
			{
				auto &roomNavMesh = gCoordinator.GetComponent<Terrain>(room);
				std::string filepath{"../data/difficulty_" + to_string(data["enemydifficulty"]) + ".json"};
				std::ifstream enemyf(filepath);
				json enemydata = json::parse(enemyf);

				if (enemydata.contains("banks") && enemydata["banks"].is_array())
				{
					auto& enemiesForRoom = gCoordinator.GetComponent<Room>(room).enemiesIn;
					enemiesForRoom.reserve(enemydata["banks"].size()); // Reserve space for efficiency

					size_t partIndex = std::rand() % (enemydata["banks"].size() - 1);

					for(std::string enemy : enemydata["banks"][partIndex])
					{
						enemiesForRoom.insert(RengbJson::generateEnemy((std::string)enemy, device, roomNavMesh.randomPoint() + glm::vec3{0.f,0.f,0.f}, room, (depth == 0)));
					}
				}
			}
		}




		if (depth != limit)
		{
			if (data.contains("possiblestages") && data["possiblestages"].is_array())
			{
				if (data.contains("doors") && data["doors"].is_array())
				{
					for (Entity doorEntity : doorsForRoom)
					{
						//auto &transf = gCoordinator.GetComponent<Transform>(doorEntity);
						// rengbModel = RengbModel::createModelFromFile(device, "models/face4.obj");
						// Entity ball = gCoordinator.CreateEntity();
						// gCoordinator.AddComponent(ball, Transform{.translation = transf.translation});
						// gCoordinator.AddComponent(ball, World{});
						// gCoordinator.AddComponent(ball, Renderable{.model = rengbModel});

						//std::cout << "x " << transf.translation.x << "  y " << transf.translation.y << "  z " << transf.translation.z << std::endl;
						//std::cout << "rotation is " << transf.rotation.y << std::endl;
						// Drunken walk, using first index for now
						generateLevel(data["possiblestages"][0], device, limit, doorEntity, depth + 1); // make so that each room only stores possible cooridoors coming off, and the coridoors store the possible rooms nex.
					}
				}
			}
		}
	}

	Entity RengbJson::generatePlayer(std::string player, RengbDevice &device)
	{
		std::ifstream f("../data/" + player + ".json");
		json data = json::parse(f);

		Entity playerEntity = gCoordinator.CreateEntity();

		gCoordinator.AddComponent(
			playerEntity,
			CollisionShape{
				.colliderCollection{AABB{glm::vec3(-0.25, 0, -0.25), glm::vec3(0.25, 1.5, 0.25)}, SphereCollider{glm::vec3(0,2.f,0), 0.5f}}});
				//.collider = SphereCollider{glm::vec3(0.f), 2.f}});

		gCoordinator.AddComponent(
			playerEntity,
			Transform{
				.translation = {0,-20.f,0}});

		gCoordinator.AddComponent(
			playerEntity,
			RigidBody{
				.acceleration = glm::vec3(0, 20.f,0), .mass = 100.f, .friction = 0.f});
			
		gCoordinator.AddComponent(
			playerEntity,
			Player{});

		gCoordinator.AddComponent(
			playerEntity,
			World{});

		if (data.contains("weapon"))
		{
			Entity gun = gCoordinator.CreateEntity();
			//gCoordinator.GetComponent<Player>(parent).gun = gun;

			std::string filepath{"models/" + (std::string)(data["weapon"]) + ".obj"};
			// std::cout << filepath << std::endl;
			std::shared_ptr<RengbModel> rengbModel = RengbModel::getOrMakeModelFromFile(device, filepath);

			gCoordinator.AddComponent(gun, Transform{.translation = glm::vec3{.7f, 0.4f, .6f}, .scale = 0.8f, .rotation = glm::vec3(0.f,0.f,0.f)});
			gCoordinator.AddComponent(gun, HeldObject{.restPosition = glm::vec3{.7f, 0.4f, .6f},});
			gCoordinator.AddComponent(gun, Renderable{.model = rengbModel});
			gCoordinator.AddComponent(gun, GunObjectComponent{});

			std::string weaponFilePath = "../data/" + (std::string)data["weapon"] + ".json"; 
			std::ifstream weaponf(weaponFilePath);
			json weapondata = json::parse(weaponf);
			auto& gunComp = gCoordinator.GetComponent<GunObjectComponent>(gun);

			for(auto barrel : weapondata["barrels"])
			{
				// std::cout<<barrel["posx"] << "x " << barrel["posy"] << "y " << barrel["posz"] << "z " <<std::endl;
				gunComp.muzzlePositions.push_back(std::make_tuple(glm::vec3{barrel["posx"], barrel["posy"], barrel["posz"]}, 0.f));
			}

			gunComp.recoil = weapondata["recoil"];
			gunComp.rateDelay = weapondata["rateDelay"];
			gunComp.ID = weapondata["id"];
			gunComp.type = weapondata["type"];
			gunComp.damage = weapondata["damage"];
			gunComp.flashSize = weapondata["flashSize"];
			gunComp.maxAmmo = weapondata["maxAmmo"];
			gunComp.ammo = weapondata["maxAmmo"];
			gunComp.reloadDuration = weapondata["reloadDuration"];
			gunComp.bulletLook = weapondata["bulletLook"];
			gunComp.segmentCount = weapondata["segmentCount"];

			// filepath = "models/" + (std::string)(data["weapon"]) + ".obj";
			gunComp.flashParts.push_back(RengbModel::getOrMakeModelFromFile(device, "models/playerFlash.obj")); 
			gunComp.flashParts.push_back(RengbModel::getOrMakeModelFromFile(device, "models/NEWFLASH5.obj")); 


			gCoordinator.GetComponent<Player>(playerEntity).gun = gun;
		}

		if (data.contains("off_arm"))
		{
			Entity off_arm = gCoordinator.CreateEntity();


			std::string filepath{"models/" + (std::string)(data["off_arm"]) + ".obj"};
			// std::cout << filepath << std::endl;
			std::shared_ptr<RengbModel> rengbModel = RengbModel::getOrMakeModelFromFile(device, filepath);
			glm::vec3 position{-0.4f, 0.3f, -0.1f};
			gCoordinator.AddComponent(off_arm, Transform{.translation = position, .scale = .8f});
			gCoordinator.AddComponent(off_arm, HeldObject{.restPosition = position});
			gCoordinator.AddComponent(off_arm, Renderable{.model = rengbModel});

			gCoordinator.GetComponent<Player>(playerEntity).offarm = off_arm;
		}

		if (data.contains("arm"))
		{
			Entity arm = gCoordinator.CreateEntity();
			//gCoordinator.GetComponent<Player>(parent).gun = gun;

			std::string filepath{"models/" + (std::string)(data["arm"]) + ".obj"};
			// std::cout << filepath << std::endl;
			std::shared_ptr<RengbModel> rengbModel = RengbModel::getOrMakeModelFromFile(device, filepath);

			gCoordinator.AddComponent(arm, Transform{.translation = glm::vec3{.7f, 0.4f, .6f}, .scale = 0.8f, .rotation = glm::vec3(0.f,0.f,0.f)});
			gCoordinator.AddComponent(arm, HeldObject{.restPosition = glm::vec3{.7f, 0.4f, .6f},});
			gCoordinator.AddComponent(arm, Renderable{.model = rengbModel});

			gCoordinator.GetComponent<Player>(playerEntity).arm = arm;
		}

		return playerEntity;
	}
}