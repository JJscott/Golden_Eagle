
/*
* Spatial Scene graph
* Infulences from OpenSG, but it's focused toward static geometry
*
* @author Joshua Scott
*/

#pragma once

#include <array>
#include <chrono>
#include <memory>
#include <queue>
#include <stack>
#include <string>
#include <vector>
#include <functional>


#include "Window.hpp"
#include "Shader.hpp"
#include "Initial3D.hpp"
#include "Bound.hpp"


namespace ambition {
namespace scenegraph {

	// Forward declarations

	class SceneGraph;
	class SceneNode;
	class Camera;
	class Controller;

	class Core;
	class NullCore;
	class BoundCore;
	class GeometryCore;
	class LODCore;
	class SwitchCore;
	class TechniqueCore;
	class TransformCore;
	class TriggerCore;

	class DrawQueue;
	class Drawable;
	class Technique;



	class Drawable {
	public:
		//Renderable object
		virtual void draw() = 0;
	};

	class UnitCube : public Drawable {
	public:
		UnitCube();
		void draw();
	private:
		GLuint vaoID; // Our Vertex Array Object
		GLuint vboID; // Our Vertex Buffer Object
	};

	class DrawQueue {
	public:
		struct drawCall {
			Drawable * drawable;
			initial3d::mat4d transform;
			Technique * technique;
			drawCall(Drawable *d, initial3d::mat4d vm, Technique *t) : drawable(d), transform(vm), technique(t) {  };
		};

		DrawQueue(SceneGraph *);

		void addDrawable(Drawable *, initial3d::mat4d, Technique *);
		std::vector<drawCall> getDrawCalls();
		void execute(ShaderManager *);

	private:
		SceneGraph * m_scene;
		std::map<std::string, std::vector<Technique *>> m_techniques;
		std::map<Technique *, std::vector<drawCall>> m_drawcalls;
	};


	class Technique {
	public:
		virtual std::string programName() = 0;
		virtual void update(GLuint, initial3d::mat4d) = 0;
		virtual void engage(GLuint, SceneGraph *) = 0;
		virtual void disengage() = 0;
		virtual ~Technique() {  };
	};

	class DefaultTrianglesTechnique : public Technique {
	public:
		DefaultTrianglesTechnique();
		std::string programName();
		void update(GLuint, initial3d::mat4d);
		void engage(GLuint, SceneGraph *);
		void disengage();
	};



	class SceneGraph {
	public:
		SceneGraph();

		int getRootDepth();
		void setRootNode(SceneNode *);
		SceneNode * getRootNode();

		int getCameraDepth();
		SceneNode * getCameraNode();

		void setCamera(Camera *);
		Camera * getCamera();

	private:
		SceneNode *m_root;
		Camera *m_camera;
	};



	class SceneVisitor {
	public:
		//implementations of Core
		virtual void visit(const Core &) = 0;

		virtual void visit(const NullCore &);
		virtual void visit(const BoundCore &);
		virtual void visit(const GeometryCore &);
		virtual void visit(const LODCore &);
		virtual void visit(const SwitchCore &);
		virtual void visit(const TechniqueCore &);
		virtual void visit(const TransformCore &);

		//this method deals with setting up the traversal for the scenegraph
		//its default implemnetation is to start traversing EntryNode
		virtual void traverse(SceneGraph *);

	private:
		//this method deals with traversing the graph
		//it's implemntation is left to the subclass as not all nodes will need to be traversed
		virtual void traverse(SceneNode *);
	};

	class PrintVisitor : public SceneVisitor {
	public:
		void traverse(SceneGraph *);
		void visit(const Core &);
		void visit(const NullCore &);
		void visit(const BoundCore &);
		void visit(const GeometryCore &);
		void visit(const LODCore &);
		void visit(const SwitchCore &);
		void visit(const TechniqueCore &);
		void visit(const TransformCore &);
	private:
		void traverse(SceneNode *);
		int m_currentDepth;
	};

	class CameraToRootVisitor : public SceneVisitor {
	public:
		CameraToRootVisitor();

		void visit(const Core &);
		void visit(const TransformCore &);

		void traverse(SceneGraph *);

		initial3d::mat4d getModelViewMatrix();
	private:
		void traverse(SceneNode *);
		bool m_camBranch;
		initial3d::mat4d m_viewRootMatrix;
		initial3d::mat4d m_modelRootMatrix;
		initial3d::mat4d m_modelViewMatrix;
	};

	class NodeToNodeVisitor : public SceneVisitor {
	public:
		NodeToNodeVisitor(SceneNode *, SceneNode *);

		void visit(const Core &);
		void visit(const TransformCore &);

		void traverse(SceneGraph *);

		initial3d::mat4d getNodeToNodeMatrix();
	private:
		void traverse(SceneNode *);
		bool m_secondBranch;

		SceneNode *m_first;
		SceneNode *m_second;

		initial3d::mat4d m_secondRootMatrix;
		initial3d::mat4d m_firstRootMatrix;
		initial3d::mat4d m_firstSecondMatrix;
	};



	class SceneRenderer : public SceneVisitor {
	public:
		SceneRenderer(bool = false, initial3d::vec4d = initial3d::vec4d());

		void visit(const Core &);
		void visit(const NullCore &);
		void visit(const BoundCore &);
		void visit(const GeometryCore &);
		void visit(const LODCore &);
		void visit(const SwitchCore &);
		void visit(const TechniqueCore &);
		void visit(const TransformCore &);

		void traverse(SceneGraph *);
		DrawQueue & getDrawQueue();

	private:
		struct state {
			state getNewState();//helper method to make everything in it's right place

			Technique * technique;
			initial3d::mat4d modelViewMatrix;
			double cullState; //positive = visible, 0 = partial, negitive = invisible
			bool traverseAll;
			std::vector<SceneNode *> traverse; //-ve if all to be traversed

			inline state() : technique(nullptr), modelViewMatrix(), cullState(), traverse() {}
		};

		void traverse(SceneNode *);
		std::stack<state> m_stateStack;
		bound::convexhull m_frustrum;
		DrawQueue m_drawManager;
		bool m_useShadow;

		//HACKY HACKY
		initial3d::vec4d m_point;
	};




	/*

	*/
	class SceneNode {
	public:
		SceneNode();
		SceneNode(std::shared_ptr<Core>);

		void accept(SceneVisitor *);

		bool addChild(SceneNode *);
		bool removeChild(SceneNode *);

		SceneNode * getParent();
		SceneNode * nthParent(int);

		const std::vector<SceneNode *> & getChildren();
	private:
		std::shared_ptr<Core> m_core;
		SceneNode *m_parent;
		std::vector<SceneNode *> m_children;
	};

	//helper method
	SceneNode * nthParent(SceneNode *, int);

	/*
	All subclasses of Core should have private constructors
	and implement create, overloading if arguments are nessesary
	*/
	class Core {
	public:
		virtual void accept(SceneVisitor *) = 0;
		virtual int maxChildren();
	};

	class NullCore : public Core {
	public:
		static std::shared_ptr<Core> create();
		void accept(SceneVisitor *);
	private:
		NullCore();
	};

	class BoundCore : public Core {
	public:
		static std::shared_ptr<Core> create(bound::aabb);
		void accept(SceneVisitor *);
		virtual bound::aabb getBound() const;
	private:
		BoundCore(bound::aabb);
		bound::aabb m_bound;
	};

	//TODO
	class GeometryCore : public Core {
	public:
		static std::shared_ptr<Core> create(Drawable *);
		void accept(SceneVisitor *);
		Drawable * getDrawable() const;
	private:
		GeometryCore(Drawable *);
		Drawable *m_geometry;
	};

	class LODCore : public Core {
	public:
		virtual std::vector<SceneNode *> getLOD(initial3d::vec3d) const = 0;
	private:
	};

	class LODFunctionCore : public LODCore {
	public:
		using lod_func_t = std::function<std::vector<SceneNode *>(initial3d::vec3d)>;
		static std::shared_ptr<Core> create(lod_func_t);
		void accept(SceneVisitor *);
		std::vector<SceneNode *> getLOD(initial3d::vec3d) const;
	private:
		LODFunctionCore(lod_func_t);
		lod_func_t m_lodFunc;
	};

	//TODO
	// class StaticLODCore : public LODCore {
	// public:
	// 	static std::shared_ptr<Core> create(std::vector<double>, std::vector<double>);
	// 	void accept(SceneVisitor *);
	// 	std::vector<SceneNode *> getLOD(double) const;
	// private:
	// 	LODCore(std::vector<double>);
	// 	std::vector<double> m_distance;
	// };

	//TODO
	class SwitchCore : public Core {
	public:
		static std::shared_ptr<Core> create(/*somthing SWITCH thingy*/);
		void accept(SceneVisitor *);
	private:
		SwitchCore();
	};

	class TechniqueCore : public Core {
	public:
		static std::shared_ptr<Core> create(Technique *, Technique *);
		void accept(SceneVisitor *);
		virtual Technique * getMainTechnique() const;
		virtual Technique * getShadowTechnique() const;
	private:
		TechniqueCore(Technique *, Technique *);
		Technique *m_mainTechnique;
		Technique *m_shadowTechnique;
	};

	class TransformCore : public Core {
	public:
		virtual initial3d::mat4d getTransform() const = 0;
	private:

	};

	class StaticTransformCore : public TransformCore {
	public:
		static std::shared_ptr<Core> create(initial3d::mat4d);
		void accept(SceneVisitor *);
		initial3d::mat4d getTransform() const;
	private:
		StaticTransformCore(initial3d::mat4d);
		initial3d::mat4d m_transform;
	};

	class ControllerTransformCore : public TransformCore {
	public:
		static std::shared_ptr<Core> create(Controller *);
		void accept(SceneVisitor *);
		initial3d::mat4d getTransform() const;
	private:
		ControllerTransformCore(Controller *);
		Controller *m_controller;
	};



	class Camera {
	public:
		Camera();

		void setPerspectiveProjection(double, double, double, double);
		void setOrthographicProjection(double, double, double, double, double, double);
		initial3d::mat4d getProjectionTransform();
		bound::convexhull getViewFrustrum();
		double getFarPlane();

		void setCameraNode(SceneNode *);
		SceneNode * getCameraNode();
	private:
		void resetViewFrustrum();
		bound::convexhull m_frustrum;
		double m_zfar;

		initial3d::mat4d m_projectionTransform;
		SceneNode *m_cameraNode;
	};



	class Controller {
	public:
		virtual initial3d::mat4d getTransform() = 0;
		virtual void update() = 0;
	};


	class FPSCamera : public Controller {
	public:
		FPSCamera(Window *, const initial3d::vec3d &, double, double);
		virtual initial3d::mat4d getTransform();
		void update();
	private:
		Window * m_window;
		initial3d::vec3d m_pos;
		initial3d::quatd m_ori;
		double m_rot_v;
		bool m_mouse_captured;
		std::chrono::steady_clock::time_point m_time_last;
		double m_speed;
	};


	class OribitalFPSCamera : public Controller {
	public:
		OribitalFPSCamera(Window *, const initial3d::vec3d &, double, double);
		initial3d::mat4d getTransform();
		void update();
	private:
		Window * m_window;
		initial3d::vec3d m_pos;
		initial3d::quatd m_ori;
		double m_rot_v;
		bool m_mouse_captured;
		std::chrono::steady_clock::time_point m_time_last;
		double m_speed;
	};

} // scenegraph
} // ambition
