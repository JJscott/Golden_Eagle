
/*
* Spatial Scene graph
* Infulences from OpenSG, but it's focused toward static geometry
*
* @author Joshua Scott
*/

#include <algorithm>
#include <array>
#include <chrono>
#include <functional>
#include <limits>
#include <memory>
#include <stack>
#include <string>
#include <vector>

#include "SceneGraph.hpp"

#include "Window.hpp"
#include "Shader.hpp"
#include "Initial3D.hpp"
#include "Bound.hpp"


namespace ambition {
namespace scenegraph {

	SceneGraph::SceneGraph() {
		m_root = nullptr;
		m_camera = nullptr;
	}

	int SceneGraph::getRootDepth() {
		if (m_root == nullptr) throw std::logic_error("SceneGraph root node is not set.");
		int rootDepth = -1;
		SceneNode * root = m_root;
		do {
			root = root->getParent();
			++rootDepth;
		} while (root != nullptr);
		return rootDepth;
	}

	void SceneGraph::setRootNode(SceneNode *root) {
		m_root = root;
	}

	SceneNode * SceneGraph::getRootNode() {
		if (m_root == nullptr) throw new std::logic_error("SceneGraph root node is not set.");
		return m_root;
	}

	int SceneGraph::getCameraDepth() {
		SceneNode * camNode = this->getCameraNode();
		int camDepth = -1;
		do {
			camNode = camNode->getParent();
			++camDepth;
		} while (camNode != nullptr);
		return camDepth;
	}

	SceneNode * SceneGraph::getCameraNode() {
		if (m_camera == nullptr) throw new std::logic_error("SceneGraph camera object is not set.");
		SceneNode *camNode = m_camera->getCameraNode();
		if (camNode == nullptr) throw new std::logic_error("SceneGraph camera node is not set.");
		return camNode;
	}

	void SceneGraph::setCamera(Camera *c) {
		m_camera = c;
	}

	Camera * SceneGraph::getCamera() {
		return m_camera;
	}



	void SceneVisitor::visit(const NullCore &c) { visit(static_cast<const Core&>(c)); }
	void SceneVisitor::visit(const BoundCore &c) { visit(static_cast<const Core&>(c)); }
	void SceneVisitor::visit(const GeometryCore &c) { visit(static_cast<const Core&>(c)); }
	void SceneVisitor::visit(const LODCore &c) { visit(static_cast<const Core&>(c)); }
	void SceneVisitor::visit(const SwitchCore &c) { visit(static_cast<const Core&>(c)); }
	void SceneVisitor::visit(const TechniqueCore &c) { visit(static_cast<const Core&>(c)); }
	void SceneVisitor::visit(const TransformCore &c) { visit(static_cast<const Core&>(c)); }

	//default implementation (traverse everything)
	void SceneVisitor::traverse(SceneGraph *sg) {
		if (sg->getRootNode() != nullptr)
			traverse(sg->getRootNode());
	}

	//default implementation (traverse everything)
	void SceneVisitor::traverse(SceneNode *sn) {
		sn->accept(this);
		std::vector<SceneNode *> v = sn->getChildren();
		for (SceneNode *c : v)
			traverse(c);
	}



	void PrintVisitor::traverse(SceneGraph *sg) {
		m_currentDepth = 0;
		traverse(sg->getRootNode());
	}

	void PrintVisitor::traverse(SceneNode *sn) {
		sn->accept(this);

		m_currentDepth++;
		std::vector<SceneNode *> v = sn->getChildren();
		for (SceneNode *c : v)
			traverse(c);
		m_currentDepth--;
	}

	void PrintVisitor::visit(const Core &) { std::cout << m_currentDepth << " : Core not implemented" << std::endl; }
	void PrintVisitor::visit(const NullCore &) { std::cout << m_currentDepth << " : NullCore" << std::endl; }
	void PrintVisitor::visit(const BoundCore &) { std::cout << m_currentDepth << " : BoundCore" << std::endl; }
	void PrintVisitor::visit(const GeometryCore &) { std::cout << m_currentDepth << " : GeometryCore" << std::endl; }
	void PrintVisitor::visit(const LODCore &) { std::cout << m_currentDepth << " : LODCore" << std::endl; }
	void PrintVisitor::visit(const SwitchCore &) { std::cout << m_currentDepth << " : SwitchCore" << std::endl; }
	void PrintVisitor::visit(const TechniqueCore &) { std::cout << m_currentDepth << " : TechniqueCore" << std::endl; }
	void PrintVisitor::visit(const TransformCore &c) { std::cout << m_currentDepth << " : TransformCore" << std::endl << c.getTransform() << std::endl; }



	CameraToRootVisitor::CameraToRootVisitor() {  }

	void CameraToRootVisitor::visit(const Core &c) {  }
	void CameraToRootVisitor::visit(const TransformCore &tc) {
		if (m_camBranch)
			m_viewRootMatrix = tc.getTransform() * m_viewRootMatrix; //matrix transform from second to root
		else
			m_modelRootMatrix = tc.getTransform() * m_modelRootMatrix;
	}

	void CameraToRootVisitor::traverse(SceneGraph *sg) {

		int camDepth = sg->getCameraDepth();
		int rootDepth = sg->getRootDepth();

		int currentDepth = std::min(camDepth, rootDepth);

		//get values for cam and root at the same depth
		SceneNode *cam = (camDepth < currentDepth) ? sg->getCameraNode() : sg->getCameraNode()->nthParent(camDepth - currentDepth);
		SceneNode *root = (rootDepth < currentDepth) ? sg->getRootNode() : sg->getRootNode()->nthParent(rootDepth - currentDepth);

		//either no cam/root is set or there is no common parent
		if (cam == nullptr || root == nullptr) {
			throw new std::logic_error("There is no common Scenegraph parent for the camera and root nodes");
		}

		//While we haven't found the common parent
		while (cam != root) {
			//Advance to next parents
			cam = cam->getParent();
			root = root->getParent();

			if (cam == nullptr || root == nullptr) //if one or both of cam and root are null, we have no common parent
				throw new std::logic_error("There is no common Scenegraph parent for the camera and root nodes");
		}

		//getting this far, cam and root have converged at common parent
		SceneNode *commonParent = cam; // or = root;
		SceneNode *stoppingPoint = commonParent->getParent();
		cam = sg->getCameraNode();
		root = sg->getRootNode();

		m_camBranch = true; //set traverse to use m_rootViewMatrix
		m_viewRootMatrix = initial3d::mat4d(1);
		while (cam != stoppingPoint) {
			traverse(cam);
			cam = cam->getParent();
		}

		m_camBranch = false; //set traverse to use m_modelRootMatrix
		m_modelRootMatrix = initial3d::mat4d(1);
		root = root->getParent(); //Do not process the root of the scenegraph
		while (root != stoppingPoint) {
			traverse(root);
			root = root->getParent();
		}

		m_modelViewMatrix = m_viewRootMatrix.inverse() * m_modelRootMatrix;
	}

	void CameraToRootVisitor::traverse(SceneNode *sn) {
		sn->accept(this);
	}

	initial3d::mat4d CameraToRootVisitor::getModelViewMatrix() {
		return m_modelViewMatrix;
	}



	NodeToNodeVisitor::NodeToNodeVisitor(SceneNode *sn1, SceneNode *sn2) : m_first(sn1), m_second(sn2) {  }

	void NodeToNodeVisitor::visit(const Core &c) {  }
	void NodeToNodeVisitor::visit(const TransformCore &tc) {
		if (m_secondBranch)
			m_secondRootMatrix = tc.getTransform() * m_secondRootMatrix; //matrix transform from vew to root
		else
			m_firstRootMatrix = tc.getTransform() * m_firstRootMatrix;
	}

	void NodeToNodeVisitor::traverse(SceneGraph *sg) {

		auto node_depth = [](SceneNode *n) -> int {
			int nodeDepth = -1;
			SceneNode * node = n;
			do {
				node = node->getParent();
				++nodeDepth;
			} while (node != nullptr);
			return nodeDepth;
		};

		int depth1 = node_depth(m_first);
		int depth2 = node_depth(m_second);

		int currentDepth = std::min(depth1, depth2);

		//get values for n2 and n1 at the same depth
		SceneNode *n1 = (depth1 < currentDepth) ? m_first : m_first->nthParent(depth1 - currentDepth);
		SceneNode *n2 = (depth2 < currentDepth) ? m_second : m_second->nthParent(depth2 - currentDepth);

		//either no n2/n1 is set or there is no common parent
		if (n2 == nullptr || n1 == nullptr) {
			throw new std::logic_error("There is no common Scenegraph parent for the n1 and n2 nodes");
		}

		//While we haven't found the common parent
		while (n2 != n1) {
			//Advance to next parents
			n2 = n2->getParent();
			n1 = n1->getParent();

			if (n2 == nullptr || n1 == nullptr) //if one or both of n2 and n1 are null, we have no common parent
				throw new std::logic_error("There is no common Scenegraph parent for the n1 and n2 nodes");
		}

		//getting this far, n2 and n1 have converged at common parent
		SceneNode *commonParent = n2; // or = n1;
		SceneNode *stoppingPoint = commonParent->getParent();
		n2 = sg->getCameraNode();
		n1 = sg->getRootNode();

		m_secondBranch = true; //set traverse to use m_secondRootMatrix
		m_secondRootMatrix = initial3d::mat4d(1);
		while (n2 != stoppingPoint) {
			traverse(n2);
			n2 = n2->getParent();
		}

		m_secondBranch = false; //set traverse to use m_firstRootMatrix
		m_firstRootMatrix = initial3d::mat4d(1);
		n1 = n1->getParent(); //Do not process the "first" node
		while (n1 != stoppingPoint) {
			traverse(n1);
			n1 = n1->getParent();
		}

		m_firstSecondMatrix = m_secondRootMatrix.inverse() * m_firstRootMatrix;
	}

	void NodeToNodeVisitor::traverse(SceneNode *sn) {
		sn->accept(this);
	}

	initial3d::mat4d NodeToNodeVisitor::getNodeToNodeMatrix() {
		return m_firstSecondMatrix;
	}



	SceneRenderer::SceneRenderer(bool useShadow, initial3d::vec4d point) : m_drawManager(nullptr), m_useShadow(useShadow), m_point(point) {  }

	void SceneRenderer::visit(const Core &) {
		throw new std::logic_error("Core function not implemented");
	}

	void SceneRenderer::visit(const NullCore &c) {  }

	void SceneRenderer::visit(const BoundCore &c) {
		initial3d::mat4d invModelView = m_stateStack.top().modelViewMatrix.inverse();
		if (m_stateStack.top().cullState == 0) {
			std::vector<bound::convexhull::Face> faces = m_frustrum.getFaces();
			for (auto it = faces.begin(); m_stateStack.top().cullState == 0 && it != faces.end(); ++it) {
				m_stateStack.top().cullState = std::min(m_stateStack.top().cullState, (*it).p.transform(invModelView).evaluate(c.getBound()));
			}
		}
	}

	void SceneRenderer::visit(const GeometryCore &c) {
		if (m_stateStack.top().technique)
			m_drawManager.addDrawable(c.getDrawable(), m_stateStack.top().modelViewMatrix, m_stateStack.top().technique);
	}

	void SceneRenderer::visit(const LODCore &c) {
		m_stateStack.top().traverseAll = false;

		initial3d::vec3d cameraPosition = m_stateStack.top().modelViewMatrix.inverse() * initial3d::vec3d::zero();
		m_stateStack.top().traverse = c.getLOD(cameraPosition);
	}

	void SceneRenderer::visit(const SwitchCore &c) {  }

	void SceneRenderer::visit(const TechniqueCore &c) {
		m_stateStack.top().technique = m_useShadow ? c.getShadowTechnique() : c.getMainTechnique();
	}

	void SceneRenderer::visit(const TransformCore &c) {
		m_stateStack.top().modelViewMatrix *= c.getTransform();
	}

	void SceneRenderer::traverse(SceneGraph *sg) {
		CameraToRootVisitor ctr;
		ctr.traverse(sg);

		m_drawManager = DrawQueue(sg);

		while (!m_stateStack.empty()) m_stateStack.pop();
		m_stateStack.push(state());
		m_stateStack.top().modelViewMatrix = ctr.getModelViewMatrix();
		m_stateStack.top().cullState = 0;

		m_frustrum = sg->getCamera()->getViewFrustrum();
		if (m_useShadow) {
			m_frustrum.addPoint(m_point);
		}

		SceneNode * root = sg->getRootNode();
		traverse(root);
	}

	void SceneRenderer::traverse(SceneNode *sn) {
		m_stateStack.push(m_stateStack.top().getNewState()); //add state ontop of stack for this scenenode

		sn->accept(this);

		//traverse All children if they are partially or fully visible
		//traverse none if they are not visible
		if (m_stateStack.top().cullState >= 0) {
			std::vector<SceneNode *> v;

			if (m_stateStack.top().traverseAll) { //check that all children are to be traversed
				v = sn->getChildren();
			} else { //only traverse the given scenenodes
				v = m_stateStack.top().traverse;
			}
			for (std::vector<SceneNode *>::iterator it = v.begin(); it != v.end(); ++it)
					traverse(*it);
		}

		m_stateStack.pop(); //remove the state that was on the stack for this node
	}

	DrawQueue & SceneRenderer::getDrawQueue() {
		return m_drawManager;
	}



	SceneRenderer::state SceneRenderer::state::getNewState() {
		state s;
		s.technique = technique;
		s.modelViewMatrix = modelViewMatrix;
		s.cullState = cullState;
		s.traverseAll = true;
		return s;
	}



	SceneNode::SceneNode() : m_core(NullCore::create()), m_parent(nullptr) {  }

	SceneNode::SceneNode(std::shared_ptr<Core> c) : m_core(c), m_parent(nullptr) {  }

	void SceneNode::accept(SceneVisitor *sv) {
		m_core->accept(sv);
	}

	bool SceneNode::addChild(SceneNode *s) {
		if (s->m_parent != nullptr)
			s->m_parent->removeChild(s);
		m_children.push_back(s);
		s->m_parent = this;
		return true;
	}

	bool SceneNode::removeChild(SceneNode *s) {
		for (auto it = m_children.begin(); it != m_children.end(); ++it) {
			if (*it == s) {
				s->m_parent = nullptr;
				m_children.erase(it);
				return true;
			}
		}
		return false;
	}

	SceneNode * SceneNode::getParent() {
		return m_parent;
	}

	const std::vector<SceneNode *> & SceneNode::getChildren() {
		return m_children;
	}



	SceneNode * SceneNode::nthParent(int i) {
		SceneNode * node = this;
		while (i-- > 0 && node != nullptr) {
			node = node->getParent();
		}
		return node;
	}



	int Core::maxChildren() { return std::numeric_limits<int>::max(); }



	std::shared_ptr<Core> NullCore::create() {
		auto pNullCore = std::shared_ptr<NullCore>(new NullCore());
		std::shared_ptr<Core> pCore = pNullCore;
		return pCore;
	}
	void NullCore::accept(SceneVisitor *v) { v->visit(*this); }
	NullCore::NullCore() {  }



	std::shared_ptr<Core> BoundCore::create(bound::aabb b) {
		auto pNullCore = std::shared_ptr<BoundCore>(new BoundCore(b));
		std::shared_ptr<Core> pCore = pNullCore;
		return pCore;
	}
	void BoundCore::accept(SceneVisitor *v) { v->visit(*this); }
	bound::aabb BoundCore::getBound() const { return m_bound; }
	BoundCore::BoundCore(bound::aabb b) : m_bound(b) {  }



	std::shared_ptr<Core> GeometryCore::create(Drawable *d) {
		auto pNullCore = std::shared_ptr<GeometryCore>(new GeometryCore(d));
		std::shared_ptr<Core> pCore = pNullCore;
		return pCore;
	}
	void GeometryCore::accept(SceneVisitor *v) { v->visit(*this); }
	GeometryCore::GeometryCore(Drawable *d) : m_geometry(d) {  }
	Drawable * GeometryCore::getDrawable() const { return m_geometry; }



	std::shared_ptr<Core> LODFunctionCore::create(lod_func_t f) {
		auto pLODFunctionCore = std::shared_ptr<LODFunctionCore>(new LODFunctionCore(f));
		std::shared_ptr<Core> pCore = pLODFunctionCore;
		return pCore;
	}
	void LODFunctionCore::accept(SceneVisitor *v) { v->visit(*this); }
	std::vector<SceneNode *> LODFunctionCore::getLOD(initial3d::vec3d dis) const { return m_lodFunc(dis); }
	LODFunctionCore::LODFunctionCore(lod_func_t f) : m_lodFunc(f) {  }



	//TODO
	std::shared_ptr<Core> SwitchCore::create() {
		return NullCore::create();
	}
	void SwitchCore::accept(SceneVisitor *v) { v->visit(*this); }
	SwitchCore::SwitchCore() {  }



	std::shared_ptr<Core> TechniqueCore::create(Technique * m, Technique * s = nullptr) {
		auto pTechniqueCore = std::shared_ptr<TechniqueCore>(new TechniqueCore(m, s));
		std::shared_ptr<Core> pCore = pTechniqueCore;
		return pCore;
	}
	Technique * TechniqueCore::getMainTechnique() const { return m_mainTechnique; }
	Technique * TechniqueCore::getShadowTechnique() const { return m_shadowTechnique; }
	void TechniqueCore::accept(SceneVisitor *v) { v->visit(*this); }

	TechniqueCore::TechniqueCore(Technique * m, Technique * s) : m_mainTechnique(m), m_shadowTechnique(s) {  }



	std::shared_ptr<Core> StaticTransformCore::create(initial3d::mat4d m) {
		auto pTransCore = std::shared_ptr<StaticTransformCore>(new StaticTransformCore(m));
		std::shared_ptr<Core> pCore = pTransCore;
		return pCore;
	}
	void StaticTransformCore::accept(SceneVisitor *v) { v->visit(*this); }
	initial3d::mat4d StaticTransformCore::getTransform() const { return m_transform; }
	StaticTransformCore::StaticTransformCore(initial3d::mat4d m) : m_transform(m) {  }



	std::shared_ptr<Core> ControllerTransformCore::create(Controller *c) {
		auto pControllerTransformCore = std::shared_ptr<ControllerTransformCore>(new ControllerTransformCore(c));
		std::shared_ptr<Core> pCore = pControllerTransformCore;
		return pCore;
	}
	void ControllerTransformCore::accept(SceneVisitor *v) { v->visit(*this); }
	initial3d::mat4d ControllerTransformCore::getTransform() const { return m_controller->getTransform(); }
	ControllerTransformCore::ControllerTransformCore(Controller *c) : m_controller(c) {  }





	Camera::Camera() : m_projectionTransform(initial3d::mat4d(0)), m_cameraNode(nullptr) {}

	void Camera::setPerspectiveProjection(double fovy, double aspect, double zNear, double zFar) {
		double f = initial3d::math::cot(fovy / 2);

		m_projectionTransform = initial3d::mat4d(0);
		m_projectionTransform(0, 0) = f / aspect;
		m_projectionTransform(1, 1) = f;
		m_projectionTransform(2, 2) = (zFar + zNear) / (zNear - zFar);
		m_projectionTransform(2, 3) = (2 * zFar * zNear) / (zNear - zFar);
		m_projectionTransform(3, 2) = -1;

		m_zfar = zFar;

		resetViewFrustrum();
	}

	void Camera::setOrthographicProjection(double left, double right, double bottom, double top, double nearVal, double farVal) {
		m_projectionTransform = initial3d::mat4d(0);
		m_projectionTransform(0, 0) = 2 / (right - left);
		m_projectionTransform(0, 3) = (right + left) / (right - left);
		m_projectionTransform(1, 1) = 2 / (top - bottom);
		m_projectionTransform(1, 3) = (top + bottom) / (top - bottom);
		m_projectionTransform(2, 2) = -2 / (farVal - nearVal);
		m_projectionTransform(2, 3) = (farVal + nearVal) / (farVal - nearVal);
		m_projectionTransform(3, 3) = 1;

		m_zfar = farVal;

		resetViewFrustrum();
	}


	initial3d::mat4d Camera::getProjectionTransform() {
		return m_projectionTransform;
	}

	void Camera::setCameraNode(SceneNode *n) {
		m_cameraNode = n;
	}

	SceneNode * Camera::getCameraNode() {
		return m_cameraNode;
	}

	void Camera::resetViewFrustrum() {
		using namespace initial3d;
		using namespace bound;
		// m_frustrum.clear();

		// initial3d::mat4d invProjMat = m_projectionTransform.inverse();

		// initial3d::vec3d frontTopRight = invProjMat * initial3d::vec3d(1, 1, 1);
		// initial3d::vec3d frontTopLeft = invProjMat * initial3d::vec3d(-1, 1, 1);
		// initial3d::vec3d frontBottomRight = invProjMat * initial3d::vec3d(1, -1, 1);
		// initial3d::vec3d frontBottomLeft = invProjMat * initial3d::vec3d(-1, -1, 1);
		// initial3d::vec3d backTopRight = invProjMat * initial3d::vec3d(1, 1, -1);
		// initial3d::vec3d backTopLeft = invProjMat * initial3d::vec3d(-1, 1, -1);
		// initial3d::vec3d backBottomRight = invProjMat * initial3d::vec3d(1, -1, -1);
		// initial3d::vec3d backBottomLeft = invProjMat * initial3d::vec3d(-1, -1, -1);

		// m_frustrum.push_back(bound::plane(backTopRight, frontTopRight, frontBottomRight));//right
		// m_frustrum.push_back(bound::plane(backTopLeft, frontTopLeft, frontTopRight));//top
		// m_frustrum.push_back(bound::plane(backBottomLeft, frontBottomLeft, frontTopLeft));//left
		// m_frustrum.push_back(bound::plane(backBottomRight, frontBottomRight, frontBottomLeft));//bottom
		// m_frustrum.push_back(bound::plane(backTopRight, backBottomRight, backBottomLeft));//front
		// m_frustrum.push_back(bound::plane(frontTopRight, frontTopLeft, frontBottomLeft));//back

		std::vector<convexhull::Face> faces;
		std::vector<convexhull::Halfedge> edges;
		std::vector<convexhull::Point> points;

		initial3d::mat4d invProjMat = m_projectionTransform.inverse();

		convexhull::point_t frontTopRight = convexhull::createPoint(points, vec4d(invProjMat * initial3d::vec3d(1, 1, 1)).homogenise());
		convexhull::point_t frontTopLeft = convexhull::createPoint(points, vec4d(invProjMat * initial3d::vec3d(-1, 1, 1)).homogenise());
		convexhull::point_t frontBottomRight = convexhull::createPoint(points, vec4d(invProjMat * initial3d::vec3d(1, -1, 1)).homogenise());
		convexhull::point_t frontBottomLeft = convexhull::createPoint(points, vec4d(invProjMat * initial3d::vec3d(-1, -1, 1)).homogenise());
		convexhull::point_t backTopRight = convexhull::createPoint(points, vec4d(invProjMat * initial3d::vec3d(1, 1, -1)).homogenise());
		convexhull::point_t backTopLeft = convexhull::createPoint(points, vec4d(invProjMat * initial3d::vec3d(-1, 1, -1)).homogenise());
		convexhull::point_t backBottomRight = convexhull::createPoint(points, vec4d(invProjMat * initial3d::vec3d(1, -1, -1)).homogenise());
		convexhull::point_t backBottomLeft = convexhull::createPoint(points, vec4d(invProjMat * initial3d::vec3d(-1, -1, -1)).homogenise());


		//all combinations
		convexhull::halfedge_t frontTop_left = convexhull::createHalfEdge(edges, frontTopLeft);
		convexhull::halfedge_t frontTop_right = convexhull::createHalfEdge(edges, frontTopRight);
		convexhull::halfedge_t backTop_left = convexhull::createHalfEdge(edges, backTopLeft);
		convexhull::halfedge_t backTop_right = convexhull::createHalfEdge(edges, backTopRight);
		convexhull::halfedge_t leftTop_front = convexhull::createHalfEdge(edges, frontTopLeft);
		convexhull::halfedge_t leftTop_back = convexhull::createHalfEdge(edges, backTopLeft);
		convexhull::halfedge_t rightTop_front = convexhull::createHalfEdge(edges, frontTopRight);
		convexhull::halfedge_t rightTop_back = convexhull::createHalfEdge(edges, backTopRight);
		convexhull::edgePairTwin(edges, frontTop_left, frontTop_right);
		convexhull::edgePairTwin(edges, backTop_left, backTop_right);
		convexhull::edgePairTwin(edges, leftTop_front, leftTop_back);
		convexhull::edgePairTwin(edges, rightTop_front, rightTop_back);

		convexhull::halfedge_t frontBottom_left = convexhull::createHalfEdge(edges, frontBottomLeft);
		convexhull::halfedge_t frontBottom_right = convexhull::createHalfEdge(edges, frontBottomRight);
		convexhull::halfedge_t backBottom_left = convexhull::createHalfEdge(edges, backBottomLeft);
		convexhull::halfedge_t backBottom_right = convexhull::createHalfEdge(edges, backBottomRight);
		convexhull::halfedge_t leftBottom_front = convexhull::createHalfEdge(edges, frontBottomLeft);
		convexhull::halfedge_t leftBottom_back = convexhull::createHalfEdge(edges, backBottomLeft);
		convexhull::halfedge_t rightBottom_front = convexhull::createHalfEdge(edges, frontBottomRight);
		convexhull::halfedge_t rightBottom_back = convexhull::createHalfEdge(edges, backBottomRight);
		convexhull::edgePairTwin(edges, frontBottom_left, frontBottom_right);
		convexhull::edgePairTwin(edges, backBottom_left, backBottom_right);
		convexhull::edgePairTwin(edges, leftBottom_front, leftBottom_back);
		convexhull::edgePairTwin(edges, rightBottom_front, rightBottom_back);

		convexhull::halfedge_t frontLeft_top = convexhull::createHalfEdge(edges, frontTopLeft);
		convexhull::halfedge_t frontLeft_bottom = convexhull::createHalfEdge(edges, frontBottomLeft);
		convexhull::halfedge_t frontRight_top = convexhull::createHalfEdge(edges, frontTopRight);
		convexhull::halfedge_t frontRight_bottom = convexhull::createHalfEdge(edges, frontBottomRight);
		convexhull::halfedge_t backLeft_top = convexhull::createHalfEdge(edges, backTopLeft);
		convexhull::halfedge_t backLeft_bottom = convexhull::createHalfEdge(edges, backBottomLeft);
		convexhull::halfedge_t backRight_top = convexhull::createHalfEdge(edges, backTopRight);
		convexhull::halfedge_t backRight_bottom = convexhull::createHalfEdge(edges, backBottomRight);
		convexhull::edgePairTwin(edges, frontLeft_top, frontLeft_bottom);
		convexhull::edgePairTwin(edges, frontRight_top, frontRight_bottom);
		convexhull::edgePairTwin(edges, backLeft_top, backLeft_bottom);
		convexhull::edgePairTwin(edges, backRight_top, backRight_bottom);


		//create and add each face
		convexhull::createFace(faces, edges, points, rightTop_back, backRight_bottom, rightBottom_front, frontRight_top);//right
		convexhull::createFace(faces, edges, points, frontTop_left, leftTop_back, backTop_right, rightTop_front);//top
		convexhull::createFace(faces, edges, points, frontLeft_top, frontTop_right, frontRight_bottom, frontBottom_left);//front
		convexhull::createFace(faces, edges, points, leftTop_front, frontLeft_bottom, leftBottom_back, backLeft_top);//left
		convexhull::createFace(faces, edges, points, backBottom_left, leftBottom_front, frontBottom_right, rightBottom_back);//bottom
		convexhull::createFace(faces, edges, points, backTop_left, backLeft_bottom, backBottom_right, backRight_top);//back

		m_frustrum = convexhull(faces, edges, points);
	}

	bound::convexhull Camera::getViewFrustrum() { return m_frustrum; }

	double Camera::getFarPlane() {
		return m_zfar;
	}


	FPSCamera::FPSCamera(Window *win, const initial3d::vec3d &pos, double rot_h = 0, double rot_v = 0) :
		m_window(win), m_pos(pos), m_ori(), m_rot_v(rot_v), m_mouse_captured(false) {
		m_time_last = std::chrono::steady_clock::now();
		m_speed = 2.5;
	}


	//initial3d::quatd FPSCamera::getOrientation() {
	//	return m_ori * initial3d::quatd::axisangle(initial3d::vec3d::i(), m_rot_v);
	//}

	void FPSCamera::update() {

		using namespace initial3d;
		using namespace std;

		// time since last update
		std::chrono::steady_clock::time_point time_now = std::chrono::steady_clock::now();
		double dt = std::chrono::duration_cast<std::chrono::duration<double>>(time_now - m_time_last).count();
		m_time_last = time_now;

		// pixels per 2*pi
		double rot_speed = 600;

		vec3d up = vec3d::j(); // up is world up
		vec3d forward = -~(m_ori * vec3d::k()).reject(up);
		vec3d side = ~(forward ^ up);

		if (m_mouse_captured) {
			int h = m_window->height();
			int w = m_window->width();
			double x, y;
			glfwGetCursorPos(m_window->handle(), &x, &y);
			x -= w * 0.5;
			y -= h * 0.5;
			double rot_h = -x / rot_speed;
			m_ori = quatd::axisangle(up, rot_h) * m_ori;
			m_rot_v += -y / rot_speed;
			m_rot_v = math::clamp(m_rot_v, -0.499 * math::pi(), 0.499 * math::pi());
			glfwSetCursorPos(m_window->handle(), w * 0.5, h * 0.5);
		}

		vec3d move = vec3d::zero();

		if (m_window->getKey(GLFW_KEY_W)) move += forward;
		if (m_window->getKey(GLFW_KEY_S)) move -= forward;
		if (m_window->getKey(GLFW_KEY_A)) move -= side;
		if (m_window->getKey(GLFW_KEY_D)) move += side;
		if (m_window->getKey(GLFW_KEY_LEFT_SHIFT)) move -= up;
		if (m_window->getKey(GLFW_KEY_SPACE)) move += up;

		//TODO change this
		if (m_window->pollKey(GLFW_KEY_GRAVE_ACCENT) == GLFW_PRESS) {
			m_mouse_captured = !m_mouse_captured;
			glfwSetCursorPos(m_window->handle(), m_window->width() * 0.5, m_window->height() * 0.5);
		}

		try {
			vec3d dpos = ~move * m_speed * dt;
			m_pos = m_pos + dpos;

			//cout << m_pos << endl;
		}
		catch (nan_error &e) {
			// no movement, do nothing
		}
	}

	initial3d::mat4d FPSCamera::getTransform() {
		initial3d::quatd rot = (m_ori * initial3d::quatd::axisangle(initial3d::vec3d::i(), m_rot_v));
		return initial3d::mat4d::translate(m_pos) * initial3d::mat4d::rotate(rot); //TODO check / recently switched this
	}



	OribitalFPSCamera::OribitalFPSCamera(Window *win, const initial3d::vec3d &pos, double rot_h = 0, double rot_v = 0) :
		m_window(win), m_pos(pos), m_ori(), m_rot_v(rot_v), m_mouse_captured(false) {
		m_time_last = std::chrono::steady_clock::now();
		m_speed = 10;
	}

	void OribitalFPSCamera::update() {

		using namespace initial3d;
		using namespace std;

		// time since last update
		std::chrono::steady_clock::time_point time_now = std::chrono::steady_clock::now();
		double dt = std::chrono::duration_cast<std::chrono::duration<double>>(time_now - m_time_last).count();
		m_time_last = time_now;

		// pixels per 2*pi
		double rot_speed = 600;

		vec3d up = ~m_pos; // we won't be going to the centre of the planet, right?
		vec3d forward = -~(m_ori * vec3d::k()).reject(up);
		vec3d side = ~(forward ^ up);

		if (m_mouse_captured) {
			int h = m_window->height();
			int w = m_window->width();
			double x, y;
			glfwGetCursorPos(m_window->handle(), &x, &y);
			x -= w * 0.5;
			y -= h * 0.5;
			double rot_h = -x / rot_speed;
			m_ori = quatd::axisangle(up, rot_h) * m_ori;
			m_rot_v += -y / rot_speed;
			m_rot_v = math::clamp(m_rot_v, -0.499 * math::pi(), 0.499 * math::pi());
			glfwSetCursorPos(m_window->handle(), w * 0.5, h * 0.5);
		}

		vec3d move = vec3d::zero();

		if (m_window->getKey(GLFW_KEY_W) == GLFW_PRESS) move += forward;
		if (m_window->getKey(GLFW_KEY_S) == GLFW_PRESS) move -= forward;
		if (m_window->getKey(GLFW_KEY_A) == GLFW_PRESS) move -= side;
		if (m_window->getKey(GLFW_KEY_D) == GLFW_PRESS) move += side;
		if (m_window->getKey(GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) move -= up;
		if (m_window->getKey(GLFW_KEY_SPACE) == GLFW_PRESS) move += up;

		if (m_window->pollKey(GLFW_KEY_GRAVE_ACCENT) == GLFW_PRESS) {
			m_mouse_captured = !m_mouse_captured;
			glfwSetCursorPos(m_window->handle(), m_window->width() * 0.5, m_window->height() * 0.5);
		}

		if (m_window->pollKey(GLFW_KEY_LEFT_BRACKET)) m_speed *= 0.5;
		if (m_window->pollKey(GLFW_KEY_RIGHT_BRACKET)) m_speed *= 2.0;

		try {
			double pos_mag = +m_pos;
			vec3d dpos = ~move * m_speed * dt;
			vec3d pos2 = m_pos + dpos;
			try {
				quatd rot = quatd::axisangle(~m_pos ^ ~pos2, m_pos.angle(pos2));
				m_ori = rot * m_ori;
			}
			catch (nan_error &e) {
				// no rotation, do nothing
			}
			m_pos = pos2;
			+m_pos = pos_mag + dpos * up;

			cout << m_pos << endl;

		}
		catch (nan_error &e) {
			// no movement, do nothing
		}
	}

	initial3d::mat4d OribitalFPSCamera::getTransform() {
		initial3d::quatd rot = (m_ori * initial3d::quatd::axisangle(initial3d::vec3d::i(), m_rot_v));
		return initial3d::mat4d::translate(m_pos) * initial3d::mat4d::rotate(rot);
	}

/*
	void LookAtCamera::lookAt(double eyeX, double eyeY, double eyeZ, double centerX, double centerY, double centerZ, double upX, double upY, double upZ) {
		initial3d::vec3d f = ~(initial3d::vec3d(centerX - eyeX, centerY - eyeY, centerZ - eyeZ));
		initial3d::vec3d up = ~(initial3d::vec3d(upX, upY, upZ));
		initial3d::vec3d s = ~(f ^ up);
		initial3d::vec3d u = ~(s ^ f);

		m_viewTransform = initial3d::mat4d();
		m_viewTransform(0, 0) = s.x();
		m_viewTransform(0, 1) = s.y();
		m_viewTransform(0, 2) = s.z();
		m_viewTransform(1, 0) = u.x();
		m_viewTransform(1, 1) = u.y();
		m_viewTransform(1, 2) = u.z();
		m_viewTransform(2, 0) = -f.x();
		m_viewTransform(2, 1) = -f.y();
		m_viewTransform(2, 2) = -f.z();
		m_viewTransform(3, 3) = 1.0;

		initial3d::mat4d trans = initial3d::mat4d().translate(-eyeX, -eyeY, -eyeZ);

		m_viewTransform *= trans;
	}
*/

	UnitCube::UnitCube() {
		glGenVertexArrays(1, &vaoID);
		glBindVertexArray(vaoID);
		glGenBuffers(1, &vboID);

		// vertex positions
		float pos[] = {
			-1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f, 1.0f,
			-1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,
			-1.0f, 1.0f, -1.0f,
			1.0f, -1.0f, 1.0f,
			-1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
			1.0f, 1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,
			-1.0f, 1.0f, 1.0f,
			-1.0f, 1.0f, -1.0f,
			1.0f, -1.0f, 1.0f,
			-1.0f, -1.0f, 1.0f,
			-1.0f, -1.0f, -1.0f,
			-1.0f, 1.0f, 1.0f,
			-1.0f, -1.0f, 1.0f,
			1.0f, -1.0f, 1.0f,
			1.0f, 1.0f, 1.0f,
			1.0f, -1.0f, -1.0f,
			1.0f, 1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
			1.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 1.0f,
			1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, -1.0f,
			-1.0f, 1.0f, -1.0f,
			1.0f, 1.0f, 1.0f,
			-1.0f, 1.0f, -1.0f,
			-1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f,
			-1.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 1.0f
		};
		glBindBuffer(GL_ARRAY_BUFFER, vboID);
		glBufferData(GL_ARRAY_BUFFER, 3 * 36 * sizeof(float), pos, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void UnitCube::draw() {
		glBindVertexArray(vaoID); // Bind our Vertex Array Object
		glDrawArrays(GL_TRIANGLES, 0, 36); // Draw our square
		glBindVertexArray(0); // Unbind our Vertex Array Object
	}



	DefaultTrianglesTechnique::DefaultTrianglesTechnique() {  }

	std::string DefaultTrianglesTechnique::programName() {
		return "scene_default_triangles.glsl";
	}

	void DefaultTrianglesTechnique::update(GLuint prog, initial3d::mat4d mv) {
		glUniformMatrix4fv(glGetUniformLocation(prog, "modelViewMatrix"), 1, true, initial3d::mat4f(mv));
	}

	void DefaultTrianglesTechnique::engage(GLuint prog, SceneGraph *sg) {
		glUniform1f(glGetUniformLocation(prog, "zfar"), float(sg->getCamera()->getFarPlane()));
		glUniformMatrix4fv(glGetUniformLocation(prog, "projectionMatrix"), 1, true, initial3d::mat4f(sg->getCamera()->getProjectionTransform()));
	}

	void DefaultTrianglesTechnique::disengage() {  }



	DrawQueue::DrawQueue(SceneGraph *sg) : m_scene(sg) {  }

	void DrawQueue::addDrawable(Drawable *d, initial3d::mat4d mv, Technique *t ) {
		std::string progName = t->programName();
		auto f_p = m_techniques.find(progName);

		if (f_p == m_techniques.end()) { //if this program hasn't been recorded
			m_techniques[progName] = std::vector<Technique *>();
			m_techniques[progName].push_back(t);
			m_drawcalls[t] = std::vector<drawCall>();

		} else {
			auto f_t = std::find(m_techniques[progName].begin(), m_techniques[progName].end(), t);

			if (f_t == m_techniques[progName].end()) { //if this technique hasn't been recorded
				m_techniques[progName].push_back(t);
				m_drawcalls[t] = std::vector<drawCall>();
			}
		}

		//finally add the draw calls
		m_drawcalls[t].push_back(drawCall(d, mv, t));

	}

	std::vector<DrawQueue::drawCall> DrawQueue::getDrawCalls() {
		std::vector<DrawQueue::drawCall> drawCalls;

		for (std::pair<std::string, std::vector<Technique *>> p : m_techniques) {
			for (Technique *t : p.second) {
				for (drawCall dc : m_drawcalls[t]) {
					drawCalls.push_back(dc);
				}
			}
		}

		return drawCalls;
	}

	void DrawQueue::execute(ShaderManager *shaderman) {
		std::string prog = "";
		Technique * tech = nullptr;
		GLuint prog_id = 0;

		for (DrawQueue::drawCall d : getDrawCalls()) {
			if (d.technique != tech) {
				if (tech) tech->disengage();
				tech = d.technique;
				if (d.technique->programName() != prog) {
					prog = d.technique->programName();
					prog_id = shaderman->getProgram(prog);
					glUseProgram(prog_id);
				}
				tech->engage(prog_id, m_scene);
			}
			tech->update(prog_id, d.transform);
			d.drawable->draw();
		}
		if (tech) tech->disengage();
	}


} // scenegraph
} // ambition
