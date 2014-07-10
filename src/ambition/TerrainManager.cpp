/*
* Terrain Generation Manager
* Asynchronously loads, caches and destroys terrain
*
* @author Joshua Scott
*/

#include <cassert>
#include <functional>
#include <vector>

#include "Window.hpp"

#include "Concurrent.hpp"
#include "Bound.hpp"
#include "GPUCache.hpp"
#include "Image.hpp"
#include "Initial3D.hpp"
#include "Perlin.hpp"
#include "SceneGraph.hpp"
#include "TerrainManager.hpp"

// HACK reference to sun position, for shadow technique
// shadow techniques are kinda deprecated atm i guess
// extern initial3d::quatd sun_ori;

namespace ambition {

	using namespace bound;
	using namespace initial3d;
	using namespace scenegraph;
	using namespace std;




	CubeFace::CubeFace(vec3d face, vec3d tang, mat4d trans) : face_up(face), face_tangent(tang), planetUpRotationMat(trans) {  }

	//imagine the faces unroll like a cube map (cross on its side)
	const CubeFace CubeFace::posX(vec3d::i(), -vec3d::j(), mat4d::rotateZ(math::pi() / 2));
	const CubeFace CubeFace::negX(-vec3d::i(), vec3d::j(), mat4d::rotateZ(-math::pi() / 2));
	const CubeFace CubeFace::posY(vec3d::j(), vec3d::i(), mat4d());
	const CubeFace CubeFace::negY(-vec3d::j(), -vec3d::i(), mat4d::rotateZ(math::pi()));
	const CubeFace CubeFace::posZ(vec3d::k(), vec3d::i(), mat4d::rotateX(-math::pi() / 2));
	const CubeFace CubeFace::negZ(-vec3d::k(), vec3d::i(), mat4d::rotateX(math::pi() / 2));



	//heights size is the width of the heights array
	//smaple size is the number of samples to take from and including xstart and zstart
	HeightMap::HeightMap(vector<double> heights, int heights_size, int sample_size, int xstart = 0, int zstart = 0) : m_size(sample_size) {
		assert(int(heights.size()) == heights_size * heights_size);
		assert(int(heights.size()) >= (xstart + sample_size) * (zstart + sample_size));

		for (int z = zstart; z < zstart + m_size; z++) {
			for (int x = xstart; x < xstart + m_size; x++) {
				double index = x + z * heights_size;
				m_heights.push_back(heights[index]);
			}
		}
	}


	double HeightMap::getHeight(double x, double z) {
		double xRaw = (m_size-1) * x;
		double zRaw = (m_size-1) * z;

		// return m_heights[floor(xRaw) + m_size * floor(zRaw)];

		int xLow = floor(xRaw); int xHigh = ceil(xRaw);
		int zLow = floor(zRaw); int zHigh = ceil(zRaw);

		double result;

		if (xLow == xHigh && zLow == zHigh) {
			result = m_heights[int(xRaw) + m_size * int(zRaw)];
		} else {
			double xFrac = xRaw - xLow;
			double zFrac = zRaw - zLow;

			//bilinear interpolation of heights
			double top = m_heights[int(xLow) + m_size * int(zLow)] * xFrac + m_heights[int(xHigh) + m_size * int(zLow)] * (1-xFrac);
			double bottom = m_heights[int(xLow) + m_size * int(zHigh)] * xFrac + m_heights[int(xHigh) + m_size * int(zHigh)] * (1-xFrac);;

			result = top * zFrac + bottom * (1 - zFrac);
		}
		return result;
	}

	//vec3d HeightMap::getNormal(double x, double z) {
	//	double xRaw = (m_size - 1) * x;
	//	double zRaw = (m_size - 1) * z;
	//
	//	// return m_heights[floor(xRaw) + m_size * floor(zRaw)];
	//
	//	int xLow = floor(xRaw); int xHigh = ceil(xRaw);
	//	int zLow = floor(zRaw); int zHigh = ceil(zRaw);
	//
	//	vec3d result;
	//
	//	if (xLow == xHigh && zLow == zHigh) {
	//		result = m_normals[int(xRaw) + m_size * int(zRaw)];
	//	}
	//	else {
	//		double xFrac = xRaw - xLow;
	//		double zFrac = zRaw - zLow;
	//
	//		//bilinear interpolation of sphere normals
	//		vec3d top = m_normals[int(xLow) + m_size * int(zLow)] * xFrac + m_normals[int(xHigh) + m_size * int(zLow)] * (1 - xFrac);
	//		vec3d bottom = m_normals[int(xLow) + m_size * int(zHigh)] * xFrac + m_normals[int(xHigh) + m_size * int(zHigh)] * (1 - xFrac);;
	//
	//		result = top * zFrac + bottom * (1 - zFrac);
	//	}
	//	return result;
	//}


	double TerrainGen::radius() {
		return m_radius;
	}

	double TerrainGen::scale() {
		return m_scale;
	}

	int TerrainGen::resolution() {
		return m_resolution;
	}

	double TerrainGen::minEdgeLength() {
		return m_minLength;
	}

	double TerrainGen::maxEdgeLength() {
		return m_maxLength;
	}

	double edgeFromDeviation(double dev, double radius) {
		return 2 * math::sqrt(math::sq(radius) - math::sq(radius - dev));
	}

	bool TerrainGen::isImpotent(initial3d::vec3d uvw) {
		double chunkCirc = (m_radius * math::pi() / 2) * uvw.z(); //chunk circumfrence
		double maxFaceCount = chunkCirc / m_minLength;
		return m_resolution >= maxFaceCount;
	}

	int TerrainGen::getResolutionForUVW(initial3d::vec3d uvw) {
		// math::log2(1 / tc->uvw().z()) //depth;

		double chunkCirc = (m_radius * math::pi() / 2) * uvw.z(); //chunk circumfrence
		double minFaceCount = chunkCirc / m_maxLength;
		double maxFaceCount = chunkCirc / m_minLength;

		//int estFaces = (m_resolution < minFaceCount) ? int(minFaceCount) : (m_resolution > maxFaceCount) ? int(maxFaceCount) : m_resolution;
		return initial3d::math::clamp<int>(m_resolution, minFaceCount, maxFaceCount);
	}


	FlatTerrainGen::FlatTerrainGen(double rad, double res, double min, double max) {
		m_radius = rad;
		m_scale = 1;
		m_resolution = res;
		m_minLength = min;
		m_maxLength = max;
	}

	HeightMap FlatTerrainGen::getHeightMap(initial3d::vec3d uvw, const CubeFace &cf) {
		int mapSize = getResolutionForUVW(uvw) + 1;
		vector<double> rawMap;

		for (int z = 0; z < mapSize; z++) {
			for (int x = 0; x < mapSize; x++) {
				rawMap.push_back(0);
			}
		}

		return HeightMap(rawMap, mapSize, mapSize, 0, 0);
	}



	PerlinTerrainGen::PerlinTerrainGen(double rad, double sca, double res, double min, double max) : m_perlin() {
		m_radius = rad;
		m_scale = sca;
		m_resolution = res;
		m_minLength = min;
		m_maxLength = max;
	}

	HeightMap PerlinTerrainGen::getHeightMap(initial3d::vec3d uvw, const CubeFace &cf) {

		mat4d rotate = cf.planetUpRotationMat.inverse();
		double size = uvw.z();

		vec3d topLeft = (rotate * vec4d(TerrainChunk::normalFromUV(uvw.x(), uvw.y()), 0)).xyz<double>();
		vec3d topRight = (rotate * vec4d(TerrainChunk::normalFromUV(uvw.x()+size, uvw.y()), 0)).xyz<double>();
		vec3d bottomLeft = (rotate * vec4d(TerrainChunk::normalFromUV(uvw.x(), uvw.y()+size), 0)).xyz<double>();
		vec3d bottomRight = (rotate * vec4d(TerrainChunk::normalFromUV(uvw.x()+size, uvw.y()+size), 0)).xyz<double>();

		int mapSize = getResolutionForUVW(uvw) + 1;
		vector<double> rawMap;

		for (int z = 0; z < mapSize; z++) {
			for (int x = 0; x < mapSize; x++) {
				//bilinear interpolation of sphere normals
				vec3d top = topLeft * (mapSize - 1 - x) + topRight * x;
				vec3d bottom = bottomLeft * (mapSize - 1 - x) + bottomRight * x;
				vec3d mid = ~(top * (mapSize - 1 - z) + bottom * z);

				double perlinPoint = m_perlin.getNoise(mid.x(), mid.y(), mid.z(), 3);
				rawMap.push_back(perlinPoint);
			}
		}

		return HeightMap(rawMap, mapSize, mapSize, 0, 0);
	}



	PlanetPerlinTerrainGen::PlanetPerlinTerrainGen(double rad, double sca, double res, double min, double max) : m_perlin0(), m_perlin1(), m_perlin2() {
		m_radius = rad;
		m_scale = sca;
		m_resolution = res;
		m_minLength = min;
		m_maxLength = max;
	}

	HeightMap PlanetPerlinTerrainGen::getHeightMap(initial3d::vec3d uvw, const CubeFace &cf) {

		mat4d rotate = cf.planetUpRotationMat.inverse();
		double size = uvw.z();

		vec3d topLeft = (rotate * vec4d(TerrainChunk::normalFromUV(uvw.x(), uvw.y()), 0)).xyz<double>();
		vec3d topRight = (rotate * vec4d(TerrainChunk::normalFromUV(uvw.x() + size, uvw.y()), 0)).xyz<double>();
		vec3d bottomLeft = (rotate * vec4d(TerrainChunk::normalFromUV(uvw.x(), uvw.y() + size), 0)).xyz<double>();
		vec3d bottomRight = (rotate * vec4d(TerrainChunk::normalFromUV(uvw.x() + size, uvw.y() + size), 0)).xyz<double>();


		int mapSize = getResolutionForUVW(uvw) + 1;
		vector<double> rawMap;

		for (int z = 0; z < mapSize; z++) {
			for (int x = 0; x < mapSize; x++) {
				//bilinear interpolation of sphere normals
				vec3d top = topLeft * (mapSize - 1 - x) + topRight * x;
				vec3d bottom = bottomLeft * (mapSize - 1 - x) + bottomRight * x;
				vec3d mid = ~(top * (mapSize - 1 - z) + bottom * z);

				// -0.5 to 0.5 is 1km, distance between humps is about 4km at 2048hz
				auto perlin = [&](const Perlin &p_, double fq_) -> double {
					return p_.getNoise(fq_ * mid.x(), fq_ * mid.y(), fq_ * mid.z());
				};

				double perlinPoint = 0;

				double regionalNoise0 = std::atan(100 * perlin(m_perlin0, 32.0)) / initial3d::math::pi() + 0.5; //regional
				double regionalNoise0small = std::atan(100 * (perlin(m_perlin0, 32.0)) - 0.25) / initial3d::math::pi() + 0.5;
				double regionalNoise1 = std::atan(100 * perlin(m_perlin1, 64.0)) / initial3d::math::pi() + 0.5;
				double regionalNoise2 = std::atan(100 * perlin(m_perlin2, 128.0)) / initial3d::math::pi() + 0.5;

				//perlinPoint += 2 * perlin(m_perlin0, 2048);

				double hillLine0 = std::exp(-5 * std::pow(perlin(m_perlin0, 9001), 2));
				double hillLine1 = std::exp(-10 * std::pow(perlin(m_perlin1, 4096), 2));
				double hillLine2 = std::exp(-5 * std::pow(perlin(m_perlin2, 2048), 2));

				perlinPoint += 0.5 * hillLine0 * (perlin(m_perlin1, 2048) + 0.5) * regionalNoise0;
				perlinPoint += 0.5 * hillLine1 * (perlin(m_perlin2, 2048) + 0.5) * regionalNoise0 * regionalNoise1;
				perlinPoint += 0.25 * hillLine2 * (perlin(m_perlin0, 2048) + 0.5) * regionalNoise0 * regionalNoise1 * regionalNoise2;

				//fine detail
				int octaves = 20;
				double amp = 0.1;
				double fq = 2048;
				for (int i = 0; i<octaves; i++, fq *= 1.8, amp *= 0.55) {
					perlinPoint += amp * perlin(m_perlin0, fq);
				}

				//rocky detail
				octaves = 20;
				amp = 0.3;
				fq = 2048;
				for (int i = 0; i<octaves; i++, fq *= 1.8, amp *= 0.55) {
					perlinPoint += amp * perlin(m_perlin1, fq) * regionalNoise0small;
				}

				rawMap.push_back(perlinPoint);
			}
		}

		return HeightMap(rawMap, mapSize, mapSize, 0, 0);
	}



	GLuint DefaultTerrainTechnique::m_diffuse_tex = 0;
	GLuint DefaultTerrainTechnique::m_normal_tex = 0;

	DefaultTerrainTechnique::DefaultTerrainTechnique(Planet *p) : m_planet(p) {
		if (m_diffuse_tex == 0) { //lazely initilise this shit
			image diffuse_image(image::type_png(), "./res/texture/grass.png");
			//image diffuse_image(image::type_png(), "./res/textures/cobblestone_d.png");
			glGenTextures(1, &m_diffuse_tex);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_diffuse_tex);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			if (glfwExtensionSupported("GL_EXT_texture_filter_anisotropic"))
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, diffuse_image.width(), diffuse_image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, diffuse_image.data());
			glGenerateMipmap(GL_TEXTURE_2D);

			image normal_image(image::type_png(), "./res/texture/cobblestone_n.png");
			glGenTextures(1, &m_normal_tex);
			glBindTexture(GL_TEXTURE_2D, m_normal_tex);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			if (glfwExtensionSupported("GL_EXT_texture_filter_anisotropic"))
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, normal_image.width(), normal_image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, normal_image.data());
			glGenerateMipmap(GL_TEXTURE_2D);

			glBindTexture(GL_TEXTURE_2D, 0); //for the lulz
		}
	}

	DefaultTerrainTechnique::~DefaultTerrainTechnique() {
		glDeleteTextures(1, &m_diffuse_tex);
		glDeleteTextures(1, &m_normal_tex);
	}

	std::string DefaultTerrainTechnique::programName() {
		return "scene_terrain_chunk.glsl";
	}

	void DefaultTerrainTechnique::update(GLuint prog, initial3d::mat4d mv) {
		glUniformMatrix4fv(glGetUniformLocation(prog, "modelViewMatrix"), 1, true, initial3d::mat4f(mv));
	}

	void DefaultTerrainTechnique::engage(GLuint prog, SceneGraph *sg) {
		glUniform1f(glGetUniformLocation(prog, "zfar"), float(sg->getCamera()->getFarPlane()));
		glUniformMatrix4fv(glGetUniformLocation(prog, "projectionMatrix"), 1, true, initial3d::mat4f(sg->getCamera()->getProjectionTransform()));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_diffuse_tex);
		glUniform1i(glGetUniformLocation(prog, "sampler_diffuse"), 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_normal_tex);
		glUniform1i(glGetUniformLocation(prog, "sampler_normal"), 1);



		//NODE TO NODE SCENE TRAVERSAL
		//planet to view matrix
		NodeToNodeVisitor pv(m_planet->planetRoot(), sg->getCamera()->getCameraNode());
		pv.traverse(sg);
		mat4d planetViewMat = pv.getNodeToNodeMatrix();
		glUniformMatrix4fv(glGetUniformLocation(prog, "viewPlanetMatrix"), 1, true, initial3d::mat4f(planetViewMat.inverse()));
	}

	void DefaultTerrainTechnique::disengage() {
		// glBindTexture(GL_TEXTURE_2D, 0); //not for the lulz
	}

	ShadowTerrainTechnique::ShadowTerrainTechnique(Planet *p) : m_planet(p) {
	
	}

	std::string ShadowTerrainTechnique::programName() {
		return "shadow_triangles_adjacency.glsl";
	}

	void ShadowTerrainTechnique::update(GLuint prog, initial3d::mat4d mv) {
		glUniformMatrix4fv(glGetUniformLocation(prog, "modelViewMatrix"), 1, true, initial3d::mat4f(mv));
	}

	void ShadowTerrainTechnique::engage(GLuint prog, scenegraph::SceneGraph *sg) {
		glUniform1f(glGetUniformLocation(prog, "zfar"), float(sg->getCamera()->getFarPlane()));
		glUniformMatrix4fv(glGetUniformLocation(prog, "projectionMatrix"), 1, true, initial3d::mat4f(sg->getCamera()->getProjectionTransform()));
		NodeToNodeVisitor p2v(m_planet->planetRoot(), sg->getCamera()->getCameraNode());
		p2v.traverse(sg);
		// HACK WARNING (sun orientation)
		//glUniform3fv(glGetUniformLocation(prog, "light_norm_v"), 1, (p2v.getNodeToNodeMatrix() * vec4d(sun_ori * vec3d::k(-1), 0)).xyz<float>());
	
	}

	void ShadowTerrainTechnique::disengage() {
		// nothing
	}

	Planet * Planet::create(TerrainGen *tg) {
		Planet *planet = new Planet(tg);

		planet->m_planetRoot = new SceneNode(TechniqueCore::create(
			new DefaultTerrainTechnique(planet),
			new ShadowTerrainTechnique(planet)
		));

		//create the root terrain chunks
		planet->m_rootChunks.push_back(new TerrainChunk(planet, CubeFace::posX));
		planet->m_rootChunks.push_back(new TerrainChunk(planet, CubeFace::negX));
		planet->m_rootChunks.push_back(new TerrainChunk(planet, CubeFace::posY));
		planet->m_rootChunks.push_back(new TerrainChunk(planet, CubeFace::negY));
		planet->m_rootChunks.push_back(new TerrainChunk(planet, CubeFace::posZ));
		planet->m_rootChunks.push_back(new TerrainChunk(planet, CubeFace::negZ));

		//build the root scene node
		for (TerrainChunk *tc : planet->m_rootChunks) {
			planet->m_planetRoot->addChild(tc->getSceneNode());
		}

		// planet->m_planetRoot->addChild(planet->m_rootChunks[0]->getSceneNode());

		//planet->m_rootChunks[0]->generateChildren();

		return planet;
	}

	Planet::Planet(TerrainGen *tg) : m_terrainGen(tg) {  }

	double Planet::radius() {
		return m_terrainGen->radius();
	}

	double Planet::scale() {
		return m_terrainGen->scale();
	}

	SceneNode * Planet::planetRoot() {
		return m_planetRoot;
	}

	TerrainGen * Planet::terrainGen() {
		return m_terrainGen;
	}



	TerrainChunk::TerrainChunk(Planet *p, const CubeFace &cf)
		: m_planet(p), m_cubeFace(cf), m_parent(nullptr), m_uvw(vec3d::k()), m_isPregnant(false) {
		//create up and tangent
		m_up = cf.face_up;
		m_up_tangent = cf.face_tangent;

		m_planetLocalMat = mat4d::translate(0, -m_planet->radius(), 0) * cf.planetUpRotationMat;
		m_planetParentMat = m_planetLocalMat.inverse();//special case local space to planet space

		buildMesh();
		buildSceneNode();
	}

	TerrainChunk::TerrainChunk(TerrainChunk *par, vec3d uvw)
		: m_planet(par->m_planet), m_cubeFace(par->m_cubeFace), m_parent(par), m_uvw(uvw), m_isPregnant(false) {

		//create up and tangent
		double hs = m_uvw.z() / 2;
		m_up = (m_cubeFace.planetUpRotationMat.inverse() * vec4d(normalFromUV(m_uvw.x() + hs, m_uvw.y() + hs), 0)).xyz<double>();
		m_up_tangent = (m_cubeFace.planetUpRotationMat.inverse() * vec4d(tangentFromUV(m_uvw.x() + hs, m_uvw.y() + hs), 0)).xyz<double>();

		//build transform to main cube face
		mat4d rotateToUp = mat4d::rotate(quatd::axisangle(m_up ^ m_cubeFace.face_up, m_up.angle(m_cubeFace.face_up)));
		vec3d newTangent = (rotateToUp * vec4d(m_up_tangent, 0)).xyz<double>(); //transform tangent into space
		mat4d rotateToTangent = mat4d::rotate(quatd::axisangle(newTangent ^ m_cubeFace.face_tangent, newTangent.angle(m_cubeFace.face_tangent)));

		m_planetLocalMat = mat4d::translate(0, -m_planet->radius(), 0) * m_cubeFace.planetUpRotationMat * rotateToTangent * rotateToUp;
		m_planetParentMat = m_parent->m_planetLocalMat * m_planetLocalMat.inverse();

		buildMesh();
		buildSceneNode();
	}

	TerrainChunk::~TerrainChunk() {
		if (!m_children.empty()) {
			for (TerrainChunk *tc : m_children) {
				delete tc;
			}
		}

		cout << "Deleting chunk : " << m_uvw << endl;

		SceneNode *parentNode = m_transNode->getParent();
		if (parentNode) parentNode->removeChild(m_transNode);

		delete m_transNode;
		delete m_boundNode;
		delete m_LODNode;
		delete m_geoNode;
		delete m_childrenNode;

		//unload
		GPUCacheManager::remove(m_geometry);
		delete m_geometry;

	}

	SceneNode * TerrainChunk::getSceneNode() {
		return m_transNode;
	}

	void TerrainChunk::procreate() {
		if (m_children.empty() && !m_isPregnant) {

			m_isPregnant = true;

			double hs = m_uvw.z() / 2;
			vec3d topLeft(m_uvw.x(), m_uvw.y(), hs);
			vec3d topRight(m_uvw.x() + hs, m_uvw.y(), hs);
			vec3d bottomLeft(m_uvw.x(), m_uvw.y() + hs, hs);
			vec3d bottomRight(m_uvw.x() + hs, m_uvw.y() + hs, hs);

			AsyncExecutor::enqueueSlow([=](){
				TerrainChunk *topLeftC = new TerrainChunk(this, topLeft);
				TerrainChunk *topRightC = new TerrainChunk(this, topRight);
				TerrainChunk *bottomLeftC = new TerrainChunk(this, bottomLeft);
				TerrainChunk *bottomRightC = new TerrainChunk(this, bottomRight);

				AsyncExecutor::enqueueMain([=](){
					GPUCacheManager::add(topLeftC->m_geometry);
					GPUCacheManager::add(topRightC->m_geometry);
					GPUCacheManager::add(bottomLeftC->m_geometry);
					GPUCacheManager::add(bottomRightC->m_geometry);

					topLeftC->m_geometry->uploadMesh();
					topRightC->m_geometry->uploadMesh();
					bottomLeftC->m_geometry->uploadMesh();
					bottomRightC->m_geometry->uploadMesh();

					m_children.push_back(topLeftC);
					m_children.push_back(topRightC);
					m_children.push_back(bottomLeftC);
					m_children.push_back(bottomRightC);

					for (TerrainChunk *tc : m_children) {
						m_childrenNode->addChild(tc->getSceneNode());
					}

					m_isPregnant = false;
				});
			});
		}
	}

	void TerrainChunk::checkChildren() {
		if (!m_children.empty()) {

			for (TerrainChunk *tc : m_children) {
				if (!tc->isComatose())
					return;
			}

			for (TerrainChunk *tc : m_children) {
				delete tc;
			}

			m_children.clear();

			checkSiblings();
		}
	}

	void TerrainChunk::checkSiblings() {
		if (m_parent != nullptr) {
			m_parent->checkChildren();
		}
	}

	bool TerrainChunk::isComatose() {
		return m_children.empty() && !m_isPregnant && m_geometry->usage() == 0;//TODO
	}



	void TerrainChunk::buildMesh() {
		int squares = m_planet->terrainGen()->getResolutionForUVW(m_uvw);

		double size = m_uvw.z();
		mat4d upm = m_cubeFace.planetUpRotationMat.inverse(); //transform normals to planet space
		vec3d topLeft = ~(upm * vec4d(normalFromUV(m_uvw.x(), m_uvw.y()), 0)).xyz<double>();
		vec3d topRight = ~(upm * vec4d(normalFromUV(m_uvw.x() + size, m_uvw.y()), 0)).xyz<double>();
		vec3d bottomLeft = ~(upm * vec4d(normalFromUV(m_uvw.x(), m_uvw.y() + size), 0)).xyz<double>();
		vec3d bottomRight = ~(upm * vec4d(normalFromUV(m_uvw.x() + size, m_uvw.y() + size), 0)).xyz<double>();

		vec3d topLeft_tangent = ~(upm * vec4d(tangentFromUV(m_uvw.x(), m_uvw.y()), 0)).xyz<double>();
		vec3d topRight_tangent = ~(upm * vec4d(tangentFromUV(m_uvw.x() + size, m_uvw.y()), 0)).xyz<double>();
		vec3d bottomLeft_tangent = ~(upm * vec4d(tangentFromUV(m_uvw.x(), m_uvw.y() + size), 0)).xyz<double>();
		vec3d bottomRight_tangent = ~(upm * vec4d(tangentFromUV(m_uvw.x() + size, m_uvw.y() + size), 0)).xyz<double>();

		//get heightmap
		HeightMap hm = m_planet->terrainGen()->getHeightMap(m_uvw, m_cubeFace);

		//transform from world coord to pseudo world coord for tiles
		double tSize = 1.0; //tileSizeLCM
		vec3d center = (m_planetLocalMat.inverse() * vec3d::zero()) / tSize; //TODO use the LCM (tSize) of all texure sizes (world size) in future
		vec3d trans(floor(center.x()) * tSize, floor(center.y()) * tSize, floor(center.z()) * tSize);

		//build mesh in planet space
		vector<vec3d> meshPoints;
		vector<vec3d> meshNormals;
		vector<vec3d> meshTangents;
		vector<vec3d> meshWorldCoord; //triplaner texture coords

		for (int z = 0; z <= squares; z++) {
			for (int x = 0; x <= squares; x++) {
				//bilinear interpolation of sphere normals and tangents
				vec3d top = topLeft * (squares - x) + topRight * x;
				vec3d bottom = bottomLeft * (squares - x) + bottomRight * x;
				vec3d mid = ~(top * (squares - z) + bottom * z);

				vec3d top_t = topLeft_tangent * (squares - x) + topRight_tangent * x;
				vec3d bottom_t = bottomLeft_tangent * (squares - x) + bottomRight_tangent * x;
				vec3d mid_t = ~(top_t * (squares - z) + bottom_t * z);

				//induction of point into planet space
				vec3d planetPoint = (mid * (m_planet->radius() + (hm.getHeight(x/double(squares), z/double(squares)) * m_planet->scale())));
				//transform to local space
				vec3d localPoint = m_planetLocalMat * planetPoint;
				meshPoints.push_back(localPoint);

				meshNormals.push_back(mid);
				meshTangents.push_back(mid_t);

				meshWorldCoord.push_back(planetPoint - trans);
			}
		}

		auto get_index = [&](int x, int z) -> int {
			// clamp to edge of heightmap
			if (z < 0) z = 0;
			if (z > squares) z = squares;
			if (x < 0) x = 0;
			if (x > squares) x = squares;
			return x + z * (squares + 1);
		};

		for (int z = 0; z <= squares; z++) {
			for (int x = 0; x <= squares; x++) {
				//quick non-magic hack
				vec3d cen = meshPoints[get_index(x, z)];
				vec3d top = meshPoints[get_index(x, z-1)] - cen;
				vec3d left = meshPoints[get_index(x-1, z)] - cen;
				vec3d bottom = meshPoints[get_index(x, z+1)] - cen;
				vec3d right = meshPoints[get_index(x+1, z)] - cen;
				vec3d norm = vec3d::zero();
				if (x > 0) {
					if (z > 0) //topleft
						norm += ~(top ^ left);
					if (z < squares) //bottomleft
						norm += ~(left ^ bottom);
				}
				if (x < squares) {
					if (z > 0) //topright
						norm += ~(right ^ top);
					if (z < squares) //bottomright
						norm += ~(bottom ^ right);
				}
				try {
					vec3d bitang = meshTangents[get_index(x, z)] ^ meshNormals[get_index(x, z)];
					meshNormals[get_index(x, z)] = (~norm); //TODO tangents
					meshTangents[get_index(x, z)] = ~(meshNormals[get_index(x, z)] ^ bitang);
				}
				catch (nan_error &e) {
					// just leave the normal like it was
				}
			}
		}

		m_geometry = new TerrainMesh(this, meshPoints, meshNormals, meshTangents, meshWorldCoord, squares + 1, m_planet);
	}

	void TerrainChunk::buildSceneNode() {

		auto lod_func = [=](vec3d camPos) -> vector<SceneNode *> {
			vector<SceneNode *> ans;
			if (m_geometry->getAABB().distance(camPos) < m_planet->radius() * m_uvw.z()) {
				if (!m_children.empty()) {
					ans.push_back(m_childrenNode);
				} else {
					procreate();
					ans.push_back(m_geoNode);
				}
			} else {
				ans.push_back(m_geoNode);
			}
			return ans;
		};

		//transform to space
		m_transNode = new SceneNode(StaticTransformCore::create(m_planetParentMat));
		//bounding volume
		m_boundNode = new SceneNode(BoundCore::create(m_geometry->getAABB()));
		m_transNode->addChild(m_boundNode);

		if (!m_planet->terrainGen()->isImpotent(m_uvw)) //lodCore
			m_LODNode = new SceneNode(LODFunctionCore::create(lod_func));
		else //nullCore
			m_LODNode = new SceneNode(NullCore::create());

		m_boundNode->addChild(m_LODNode);

		//geometry
		m_geoNode = new SceneNode(GeometryCore::create(m_geometry));
		m_LODNode->addChild(m_geoNode);
		//children
		m_childrenNode = new SceneNode(NullCore::create());
		m_LODNode->addChild(m_childrenNode);
	}

	const vec3d & TerrainChunk::uvw() { return m_uvw; }

	vec3d TerrainChunk::normalFromUV(double u, double v) {
		return ~(vec3d(u - 0.5, 0.5, v - 0.5));
	}

	vec3d TerrainChunk::tangentFromUV(double u, double v) {
		return ~(vec3d(0.5, 0.5 - u, 0));
	}

	TerrainMesh::TerrainMesh(TerrainChunk *tc, const vector<vec3d>& points_, const vector<vec3d>& normals_,
		const vector<vec3d>& tangents_, const vector<vec3d>& worldCoord_, int res, Planet *p_) : m_chunk(tc), m_res(res), m_vao(0) {

		// dummy point at 0 index
		m_points.push_back(0);
		m_points.push_back(0);
		m_points.push_back(0);

		// dummy point at 0 index
		m_normals.push_back(0);
		m_normals.push_back(0);
		m_normals.push_back(0);

		// dummy point at 0 index
		m_tangents.push_back(0);
		m_tangents.push_back(0);
		m_tangents.push_back(0);

		// dummy point at 0 index
		m_worldCoord.push_back(0);
		m_worldCoord.push_back(0);
		m_worldCoord.push_back(0);

		double skirtStart = -1; //index of start of the skirt -1
		double skirtEnd = res + 1; //index of end of the skirt -1

		// generate actual points and find aabb
		vec3d min = points_[0];
		vec3d max = points_[0];
		for (int i = skirtStart; i < skirtEnd; i++) {
			for (int j = skirtStart; j < skirtEnd; j++) {
				if (j == skirtStart || j == skirtEnd - 1 || i == skirtStart || i == skirtEnd - 1) { //point is either a skirt of corner
					if ((j == skirtStart && (i == skirtStart || i == skirtEnd - 1))
						|| (j == skirtEnd - 1 && (i == skirtStart || i == skirtEnd - 1))) { // dummy point at corner indexes, pointless but helps building ya know?
						m_points.push_back(0);
						m_points.push_back(0);
						m_points.push_back(0);

						m_normals.push_back(0);
						m_normals.push_back(0);
						m_normals.push_back(0);

						m_tangents.push_back(0);
						m_tangents.push_back(0);
						m_tangents.push_back(0);

						m_worldCoord.push_back(0);
						m_worldCoord.push_back(0);
						m_worldCoord.push_back(0);

					} else { //this point is a skirt so work out where it should go

						//skirts need to be fixed, god help me TODO

						int modj = std::max(0, std::min(j, res - 1));
						int modi = std::max(0, std::min(i, res - 1));
						vec3d p = points_[modj + modi*res];
						vec3d diff = (~(vec3d(0, -p_->radius(), 0) - p) * p_->scale() / (math::log2(1 / tc->uvw().z()) + 1)); //extend point toward the center TODO dynamic (moar smart)
						vec3d sp = p + diff;

						min = vec3d::negative_extremes(min, sp);
						max = vec3d::positive_extremes(max, sp);
						m_points.push_back(sp.x());
						m_points.push_back(sp.y());
						m_points.push_back(sp.z());

						vec3d n = normals_[modj + modi*res];
						m_normals.push_back(n.x());
						m_normals.push_back(n.y());
						m_normals.push_back(n.z());

						vec3d t = tangents_[modj + modi*res];
						m_tangents.push_back(t.x());
						m_tangents.push_back(t.y());
						m_tangents.push_back(t.z());

						vec3d extension;
						vec3d w = worldCoord_[modj + modi*res];
						if (j == skirtStart) { // extends left
							extension = w - worldCoord_[modj+1 + modi*res];
						} else if (j == skirtEnd - 1) { // extends right
							extension = w - worldCoord_[modj-1 + modi*res];
						} else if (i == skirtStart) { // extends up
							extension = w - worldCoord_[modj + (modi+1)*res];
						} else if (i == skirtEnd - 1) { // extends down
							extension = w - worldCoord_[modj + (modi-1)*res];
						}
						+extension = +diff;
						//special handeling of the world texture pos of skirts
						m_worldCoord.push_back(w.x() + extension.x());
						m_worldCoord.push_back(w.y() + extension.y());
						m_worldCoord.push_back(w.z() + extension.z());
					}
				} else { //point is not a skirt or corner
					vec3d p = points_[j + i*res];
					min = vec3d::negative_extremes(min, p);
					max = vec3d::positive_extremes(max, p);
					m_points.push_back(p.x());
					m_points.push_back(p.y());
					m_points.push_back(p.z());

					vec3d n = normals_[j + i*res];
					m_normals.push_back(n.x());
					m_normals.push_back(n.y());
					m_normals.push_back(n.z());

					vec3d t = tangents_[j + i*res];
					m_tangents.push_back(t.x());
					m_tangents.push_back(t.y());
					m_tangents.push_back(t.z());

					vec3d w = worldCoord_[j + i*res];
					m_worldCoord.push_back(w.x());
					m_worldCoord.push_back(w.y());
					m_worldCoord.push_back(w.z());
				}
			}
		}

		m_aabb = aabb::fromMinMax(min, max);


		// triangle vertex indices (m_indices)
		// heightmap cell triangulation
		// true -> link (+x,-z) diagonal
		vector<bool> triangulation;

		//trianglation does not extend towards the skirts
		auto get_triangulation = [&](int z, int x) -> bool {
			if (z < 0 || z >= res - 1) return false;
			if (x < 0 || x >= res - 1) return false;
			return triangulation[(res - 1) * z + x];
		};

		//get point clamping to non-skirt indexes
		auto get_index = [&](int z, int x) -> unsigned {
			// reserve index 0 for 'not a vertex'
			if (z < 0 || z >= res) return 0;
			if (x < 0 || x >= res) return 0;
			return 1u + unsigned(res + 2) * unsigned(z + 1) + unsigned(x + 1);
		};

		auto get_point = [&](int z, int x) -> vec3f {
			// clamp to edge of heightmap
			if (z < 0) z = 0;
			if (z >= res) z = res - 1;
			if (x < 0) x = 0;
			if (x >= res) x = res - 1;
			unsigned i = get_index(z, x);
			return vec3f(m_points[3 * i], m_points[3 * i + 1], m_points[3 * i + 2]);
		};

		// auto push_index = [&](int z, int x) {
		// 	m_indices.push_back(get_index(z, x));
		// };

		//get point clamping to skirt indexes
		auto get_index_with_skirt = [&](int z, int x) -> unsigned {
			// reserve index 0 for 'not a vertex'
			if (z < skirtStart || z >= skirtEnd) return 0;
			if (x < skirtStart || x >= skirtEnd) return 0;
			if (z == skirtStart && (x == skirtStart || x == skirtEnd - 1)) return 0;
			if (z == skirtEnd - 1 && (x == skirtStart || x == skirtEnd - 1)) return 0;
			return 1u + unsigned(res + 2) * unsigned(z + 1) + unsigned(x + 1);
		};

		auto push_index_with_skirt = [&](int z, int x) {
			m_indices.push_back(get_index_with_skirt(z, x));
		};

		// calculate triangulation
		for (int z = 0; z < res - 1; z++) {
			for (int x = 0; x < res - 1; x++) {
				// curvature on (+x,-z) diagonal
				float c0 = get_point(z + 2, x - 1).y() - get_point(z + 1, x).y() - get_point(z, x + 1).y() + get_point(z - 1, x + 2).y();
				// curvature on (+x,+z) diagonal
				float c1 = get_point(z - 1, x - 1).y() - get_point(z, x).y() - get_point(z + 1, x + 1).y() + get_point(z + 2, x + 2).y();
				// local curvature on (+x,+z) diagonal using midpoint from (+x,-z) diagonal
				float c1a = 2.0f * get_point(z, x).y() - get_point(z + 1, x).y() - get_point(z, x + 1).y() + 2.0f * get_point(z + 1, x + 1).y();
				// local curvature on (+x,-z) diagonal using midpoint from (+x,+z) diagonal
				float c0b = 2.0f * get_point(z + 1, x).y() - get_point(z, x).y() - get_point(z + 1, x + 1).y() + 2.0f * get_point(z, x + 1).y();

				// calculate errors for each diagonal based on how well its midpoint matches the curvature
				float err_a = math::abs(c0) + math::abs(c1a - c1);
				float err_b = math::abs(c1) + math::abs(c0b - c0);

				triangulation.push_back(err_a < err_b);
			}
		}

		// emit indices
		for (int z = skirtStart; z < skirtEnd - 1; z++) {
			for (int x = skirtStart; x < skirtEnd - 1; x++) {
				if ((z == skirtStart && (x == skirtStart || x == skirtEnd - 2))
					|| (z == skirtEnd - 2 && (x == skirtStart || x == skirtEnd - 2))) {
					//if at the "corners" of the skirt, do NOTHING!!!
				} else {
					if (get_triangulation(z, x)) {
						// link (+x,-z) diagonal
						// first tri (:), p1 = (x,z)
						//         6 //
						//       / | //
						//     1---5 //
						//   / |:/ | //
						// 2---3---4 //
						push_index_with_skirt(z, x);
						push_index_with_skirt(z + get_triangulation(z, x - 1), x - 1);
						push_index_with_skirt(z + 1, x);
						push_index_with_skirt(z + 1, x + 1);
						push_index_with_skirt(z, x + 1);
						push_index_with_skirt(z - 1, x + get_triangulation(z - 1, x));
						// second tri (:), p6 = (x,z)
						// 6---5---4 //
						// | /:| /   //
						// 1---3     //
						// | /       //
						// 2         //
						push_index_with_skirt(z + 1, x);
						push_index_with_skirt(z + 2, x + !get_triangulation(z + 1, x));
						push_index_with_skirt(z + 1, x + 1);
						push_index_with_skirt(z + !get_triangulation(z, x + 1), x + 2);
						push_index_with_skirt(z, x + 1);
						push_index_with_skirt(z, x);

					}
					else {
						// link (+x,+z) diagonal
						// first tri (:), p1 = (x,z)
						// 6         //
						// | \       //
						// 1---5     //
						// | \:| \   //
						// 2---3---4 //
						push_index_with_skirt(z, x);
						push_index_with_skirt(z + 1, x);
						push_index_with_skirt(z + 1, x + 1);
						push_index_with_skirt(z + !get_triangulation(z, x + 1), x + 2);
						push_index_with_skirt(z, x + 1);
						push_index_with_skirt(z - 1, x + get_triangulation(z - 1, x));

						// second tri (:), p1 = (x,z)
						// 2---1---6 //
						//   \ |:\ | //
						//     3---5 //
						//       \ | //
						//         4 //
						push_index_with_skirt(z, x);
						push_index_with_skirt(z + get_triangulation(z, x - 1), x - 1);
						push_index_with_skirt(z + 1, x);
						push_index_with_skirt(z + 2, x + !get_triangulation(z + 1, x));
						push_index_with_skirt(z + 1, x + 1);
						push_index_with_skirt(z, x + 1);
					}
				}
			}
		}

		//This should be modified when changing this method laater one TODO
		m_memCount = 0;
		m_memCount += m_indices.size() * sizeof(GLuint);
		m_memCount += m_points.size() * sizeof(float);
		m_memCount += m_normals.size() * sizeof(float);
		m_memCount += m_tangents.size() * sizeof(float);
		m_memCount += m_worldCoord.size() * sizeof(float);
	}



	TerrainMesh::~TerrainMesh() {
		// calling unload() here could result in multiple deletion of this chunk
		assert(m_vao == 0 && "attempt to delete loaded TerrainMesh");
	}

	void TerrainMesh::uploadMesh() {

		// update timestamp here instead of draw() so it gets set on birth
		m_timestamp = GPUCacheManager::now();

		if (m_vao == 0) {

			GPUCacheManager::alloc(m_memCount);

			glGenVertexArrays(1, &m_vao);
			glBindVertexArray(m_vao);
			GLuint vbo_p, vbo_n, vbo_t, vbo_w, ibo;
			glGenBuffers(1, &vbo_p);
			glGenBuffers(1, &vbo_n);
			glGenBuffers(1, &vbo_t);
			glGenBuffers(1, &vbo_w);
			glGenBuffers(1, &ibo);

			// record the vbo that need to be deleted after
			m_vbo_array.push_back(vbo_p);
			m_vbo_array.push_back(vbo_n);
			m_vbo_array.push_back(vbo_t);
			m_vbo_array.push_back(vbo_w);
			m_vbo_array.push_back(ibo);

			// upload indices
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo); // this sticks to the vao
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(GLuint), &m_indices[0], GL_STATIC_DRAW);

			// upload positions
			glBindBuffer(GL_ARRAY_BUFFER, vbo_p);
			glBufferData(GL_ARRAY_BUFFER, m_points.size() * sizeof(float), &m_points[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

			// upload normals
			glBindBuffer(GL_ARRAY_BUFFER, vbo_n);
			glBufferData(GL_ARRAY_BUFFER, m_normals.size() * sizeof(float), &m_normals[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

			// upload tangents
			glBindBuffer(GL_ARRAY_BUFFER, vbo_t);
			glBufferData(GL_ARRAY_BUFFER, m_tangents.size() * sizeof(float), &m_tangents[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

			// upload psuedo world coords
			glBindBuffer(GL_ARRAY_BUFFER, vbo_w);
			glBufferData(GL_ARRAY_BUFFER, m_worldCoord.size() * sizeof(float), &m_worldCoord[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(3);
			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

			// cleanup
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
	}

	void TerrainMesh::draw() {
		uploadMesh();
		glBindVertexArray(m_vao);
		glDrawElements(GL_TRIANGLES_ADJACENCY, ( (m_res + 1) * (m_res + 1) + 4) * 12, GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);
	}

	bound::aabb TerrainMesh::getAABB() {
		return m_aabb;
	}


	size_t TerrainMesh::usage() {
		if (m_vao == 0)
			return 0;
		return m_memCount;
	}

	void TerrainMesh::unload() {
		if (m_vao != 0) {
			glDeleteBuffers(m_vbo_array.size(), &m_vbo_array[0]);
			m_vbo_array.clear();
			glDeleteVertexArrays(1, &m_vao);
			m_vao = 0;

			GPUCacheManager::free(m_memCount);

			m_chunk->checkSiblings();

			double t = chrono::duration_cast<chrono::duration<double>>(GPUCacheManager::now() - m_timestamp).count();
			cout << "unloading TerrainMesh, last access: " << t << endl;
		}
	}

	GPUCacheManager::timestamp_t TerrainMesh::timestamp() {
		return m_timestamp;
	}

}