/*
* Terrain Generation Manager
* Asynchronously loads, caches and destroys terrain
*
* @author Joshua Scott
*/

#pragma once

#include <functional>
#include <limits>
#include <vector>

#include "Window.hpp"

#include "Bound.hpp"
#include "GPUCache.hpp"
#include "Initial3D.hpp"
#include "Perlin.hpp"
#include "SceneGraph.hpp"
#include "TerrainManager.hpp"

namespace ambition {

	class Planet;
	class TerrainGen;
	class TerrainChunk;
	class TerrainMesh;


	class CubeFace{
	public:
		const initial3d::vec3d face_up;
		const initial3d::vec3d face_tangent;
		const initial3d::mat4d planetUpRotationMat;

		static const CubeFace posX;
		static const CubeFace negX;
		static const CubeFace posY;
		static const CubeFace negY;
		static const CubeFace posZ;
		static const CubeFace negZ;

	private:
		CubeFace(initial3d::vec3d, initial3d::vec3d, initial3d::mat4d);
	};


	class HeightMap {
	public:
		//heights, size, samplesize, xstart index, zstart index
		HeightMap(std::vector<double>, int, int, int, int);
		double getHeight(double, double);
	private:
		int m_size;
		std::vector<double> m_heights;
		std::vector<initial3d::vec3d> m_normals;
	};


	class TerrainGen {
	public:

		double radius();
		double scale();
		int resolution();
		double minEdgeLength();
		double maxEdgeLength();

		double edgeFromDeviation(double, double);

		virtual bool isImpotent(initial3d::vec3d);
		virtual int getResolutionForUVW(initial3d::vec3d);
		virtual HeightMap getHeightMap(initial3d::vec3d, const CubeFace &) = 0;
	protected:
		double m_radius = 1;
		double m_scale = 1;
		int m_resolution = 2;
		double m_minLength = 0;
		double m_maxLength = std::numeric_limits<double>::infinity();
	};

	class FlatTerrainGen : public TerrainGen{
	public:
		//radius, res, minEdge, maxEdge
		FlatTerrainGen(double, double, double, double);
		HeightMap getHeightMap(initial3d::vec3d, const CubeFace &);
	};


	class PerlinTerrainGen : public TerrainGen {
	public:
		//radius, scale, res, minEdge, maxEdge
		PerlinTerrainGen(double, double, double, double, double);
		HeightMap getHeightMap(initial3d::vec3d, const CubeFace &);
	private:
		Perlin m_perlin;
	};


	class PlanetPerlinTerrainGen : public TerrainGen {
	public:
		//radius, scale, res, minEdge, maxEdge
		PlanetPerlinTerrainGen(double, double, double, double, double);
		HeightMap getHeightMap(initial3d::vec3d, const CubeFace &);
	private:
		Perlin m_perlin0;
		Perlin m_perlin1;
		Perlin m_perlin2;
	};


	class DefaultTerrainTechnique : public scenegraph::Technique {
	public:
		DefaultTerrainTechnique(Planet *);
		~DefaultTerrainTechnique();
		std::string programName();
		void update(GLuint, initial3d::mat4d);
		void engage(GLuint, scenegraph::SceneGraph *);
		void disengage();

		void accept(TerrainMesh *d);

	private:
		static GLuint m_diffuse_tex;
		static GLuint m_normal_tex;
		Planet *m_planet;
	};

	class ShadowTerrainTechnique : public scenegraph::Technique {
	public:
		ShadowTerrainTechnique(Planet *);
		std::string programName();
		void update(GLuint, initial3d::mat4d);
		void engage(GLuint, scenegraph::SceneGraph *);
		void disengage();
	
	private:
		Planet *m_planet;
	};


	class Planet {
	public:
		static Planet * create(TerrainGen *);
		double radius();
		double scale();
		scenegraph::SceneNode * planetRoot();
		TerrainGen * terrainGen();
	private:
		Planet(TerrainGen *);
		TerrainGen * m_terrainGen;

		std::vector<TerrainChunk *> m_rootChunks;

		scenegraph::SceneNode * m_planetRoot;
	};



	class TerrainChunk {
	public:
		TerrainChunk(Planet *, const CubeFace &);
		TerrainChunk(TerrainChunk *, initial3d::vec3d);
		virtual ~TerrainChunk();

		scenegraph::SceneNode * getSceneNode();

		void procreate();
		void checkChildren();
		void checkSiblings();
		bool isComatose();

		const initial3d::vec3d & uvw();

		static initial3d::vec3d normalFromUV(double, double);
		static initial3d::vec3d tangentFromUV(double, double);

	private:
		void buildMesh();//called on construction
		void buildSceneNode();

		Planet *m_planet;
		const CubeFace &m_cubeFace;
		TerrainChunk *m_parent;
		TerrainMesh *m_geometry;

		scenegraph::SceneNode *m_transNode;
		scenegraph::SceneNode *m_boundNode;
		scenegraph::SceneNode *m_LODNode;
		scenegraph::SceneNode *m_geoNode;
		scenegraph::SceneNode *m_childrenNode;

		initial3d::mat4d m_planetLocalMat;
		initial3d::mat4d m_planetParentMat;

		initial3d::vec3d m_uvw;
		initial3d::vec3d m_up;
		initial3d::vec3d m_up_tangent;

		std::vector<TerrainChunk *> m_children;
		bool m_isPregnant;
	};

	class TerrainMesh : public scenegraph::Drawable, public GPUCacheable {
	public:
		TerrainMesh(TerrainChunk *, const std::vector<initial3d::vec3d>&, const std::vector<initial3d::vec3d>&,
			const std::vector<initial3d::vec3d>&, const std::vector<initial3d::vec3d>&, int, Planet *);
		virtual ~TerrainMesh();
		void uploadMesh();

		void draw();
		bound::aabb getAABB();

		virtual size_t usage();
		virtual void unload();
		virtual GPUCacheManager::timestamp_t timestamp();

	private:
		TerrainChunk *m_chunk;
		bound::aabb m_aabb;
		int m_res;

		std::vector<GLuint> m_indices;
		std::vector<float> m_points;
		std::vector<float> m_normals;
		std::vector<float> m_tangents;
		std::vector<float> m_worldCoord;

		GLuint m_vao;
		std::vector<GLuint> m_vbo_array;

		size_t m_memCount;
		GPUCacheManager::timestamp_t m_timestamp;
	};

}