
/*
* Bounding volume implementation
* Developed for specifically for frustum culling
*
* @author Joshua Scott
*/

#pragma once

#include <algorithm>
#include <cmath>
#include <map>
#include <utility>
#include <stdexcept>
#include <vector>

#include "Initial3D.hpp"


namespace ambition {
namespace bound {

	class aabb {
	public:
		inline static aabb fromMinMax(initial3d::vec3d min_, initial3d::vec3d max_) {
			initial3d::vec3d min = initial3d::vec3d::negative_extremes(min_, max_);
			initial3d::vec3d max = initial3d::vec3d::positive_extremes(min_, max_);
			initial3d::vec3d halfSize = (max - min) / 2;
			return aabb(min + halfSize, halfSize);
		}

		inline aabb() : m_center(initial3d::vec3d()), m_halfsize(initial3d::vec3d()) {  }
		inline aabb(initial3d::vec3d c, initial3d::vec3d hs) : m_center(c), m_halfsize(hs) {  }

		inline aabb transform(initial3d::mat4d mat) {
			initial3d::vec3d corners[8];
			corners[0] = mat * (m_center + initial3d::vec3d());
			corners[1] = mat * (m_center + initial3d::vec3d(m_halfsize.x(), 0, 0));
			corners[2] = mat * (m_center + initial3d::vec3d(0, m_halfsize.y(), 0));
			corners[3] = mat * (m_center + initial3d::vec3d(0, 0, m_halfsize.z()));
			corners[4] = mat * (m_center + initial3d::vec3d(m_halfsize.x(), m_halfsize.y(), 0));
			corners[5] = mat * (m_center + initial3d::vec3d(m_halfsize.x(), 0, m_halfsize.z()));
			corners[6] = mat * (m_center + initial3d::vec3d(0, m_halfsize.y(), m_halfsize.z()));
			corners[7] = mat * (m_center + initial3d::vec3d(m_center.x(), m_halfsize.y(), m_halfsize.z()));

			initial3d::vec3d min(corners[0]);
			initial3d::vec3d max(corners[0]);

			for (int i = 1; i < 8; i++) {
				min = initial3d::vec3d::negative_extremes(min, corners[i]);
				max = initial3d::vec3d::positive_extremes(max, corners[i]);
			}

			return aabb::fromMinMax(min, max);
		}

		inline bool contains(initial3d::vec3d p) {
			initial3d::vec3d dist = p.subtract(m_center);
			return (std::abs(dist.x()) <= m_halfsize.x()
				&& std::abs(dist.y()) <= m_halfsize.y()
				&& std::abs(dist.z()) <= m_halfsize.z());
		}

		inline initial3d::vec3d closestPoint(initial3d::vec3d p) { //TODO fix nd optimize
			initial3d::vec3d point = p.subtract(m_center);
			point.x() = (std::abs(point.x()) <= m_halfsize.x()) ? point.x() : m_halfsize.x();
			point.y() = (std::abs(point.y()) <= m_halfsize.y()) ? point.y() : m_halfsize.y();
			point.z() = (std::abs(point.z()) <= m_halfsize.z()) ? point.z() : m_halfsize.z();
			return point;
		}

		inline double distance(initial3d::vec3d p) { //TODO fix nd optimize
			initial3d::vec3d dist = p.subtract(m_center);
			dist.x() = (std::abs(dist.x()) <= m_halfsize.x()) ? 0 : std::abs(dist.x()) - m_halfsize.x();
			dist.y() = (std::abs(dist.y()) <= m_halfsize.y()) ? 0 : std::abs(dist.y()) - m_halfsize.y();
			dist.z() = (std::abs(dist.z()) <= m_halfsize.z()) ? 0 : std::abs(dist.z()) - m_halfsize.z();
			return +dist;
		}

		inline bool intersects(aabb bb) {
			initial3d::vec3d dist = m_center.subtract(bb.m_center);
			initial3d::vec3d size_sum = m_halfsize.add(bb.m_halfsize);
			return (std::abs(dist.x()) <= size_sum.x()
				&& std::abs(dist.y()) <= size_sum.y()
				&& std::abs(dist.z()) <= size_sum.z());
		}

		//TODO faster way to use center and half sizes
		//bool intersects(bsphere bs) {
		//	initial3d::vec3d sc = bs.getCenter();
		//	initial3d::vec3d d = initial3d::vec3d.zero();

		//	initial3d::vec3d min = this->getMin();
		//	initial3d::vec3d max = this->getMax();

		//	if (sc.x() < min.x()) { d.x() = sc.x() - min.x() }
		//	else if (sc.x() > max.x()) { d.x() = sc.x() - max.x() }
		//	if (sc.y() < min.y()) { d.y() = sc.y() - min.y() }
		//	else if (sc.y() > max.y()) { d.y() = sc.y() - max.y() }
		//	if (sc.z() < min.z()) { d.z() = sc.z() - min.z() }
		//	else if (sc.z() > max.z()) { d.z() = sc.z() - max.z() }

		//	return +d <= bs.radius();
		//}

		inline aabb getBoundingBox() {
			return *this;
		}

		inline initial3d::vec3d getCenter() {
			return m_center;
		}

		inline initial3d::vec3d getHalfSize() {
			return m_halfsize;
		}

		inline initial3d::vec3d getMax() {
			return m_center.add(m_halfsize);
		}

		inline initial3d::vec3d getMin() {
			return m_center.subtract(m_halfsize);
		}

	private:
		initial3d::vec3d m_center;
		initial3d::vec3d m_halfsize;
	};



	class plane {
	public:

		inline initial3d::vec3d abc() { return m_abc; }
		inline double d() { return m_d; }

		inline double evaluate(initial3d::vec3d p) {
			return p.dot(m_abc) - m_d;
		}

		inline double evaluate(aabb b) {
			initial3d::vec3d hsize = b.getHalfSize();
			initial3d::vec3d pTemp = initial3d::vec3d( //getting the positive most value
				(m_abc.x() > 0) ? hsize.x() : -hsize.x(),
				(m_abc.y() > 0) ? hsize.y() : -hsize.y(),
				(m_abc.z() > 0) ? hsize.z() : -hsize.z());

			initial3d::vec3d pPoint = b.getCenter() + pTemp;
			initial3d::vec3d nPoint = b.getCenter() - pTemp;

			double val = evaluate(nPoint);

			if (val <= 0) { //if the neg point is on the line or in the negative side
				val = evaluate(pPoint);
				if (val >= 0) { //if the positive point is on the line or in the positive side
					val = 0;
				}
			}
			return val;
		}

		inline plane transform(initial3d::mat4d t) {
			initial3d::vec3d pnorm = (t * initial3d::vec4d(m_abc, 0)).xyz<double>();
			initial3d::vec3d point = (t * initial3d::vec4d(m_abc * m_d, 1)).homogenise().xyz<double>();
			return plane(pnorm, point);
		}

		inline plane() : m_abc(initial3d::vec3d::zero()), m_d(0) {  }
		inline plane(double a, double b, double c, double d) : m_abc(~initial3d::vec3d(a, b, c)), m_d(d) {  }
		inline plane(initial3d::vec3d norm) : m_abc(~norm), m_d(0) {  }
		inline plane(initial3d::vec3d norm, double d) : m_abc(~norm), m_d(d) {  }
		inline plane(initial3d::vec3d norm, initial3d::vec3d point) : m_abc(~norm), m_d(m_abc.dot(point)) {  }
		inline plane(initial3d::vec3d p0, initial3d::vec3d p1, initial3d::vec3d p2)
			: m_abc(~((p1 - p0) ^ (p2 - p1))), m_d(m_abc.dot(p0))  {  }
	private:
		//where ax + by + cz = d
		initial3d::vec3d m_abc;
		double m_d;
	};



	class convexhull {
	public:
		using point_t = size_t;
		using halfedge_t = size_t;
		using face_t = size_t;

		class Point {
		public:
			initial3d::vec4d pos;
			inline bool isDirection() {
				return pos.w() == 0;
			}
			inline Point(initial3d::vec4d p) : pos(p) {  }
			inline Point(initial3d::vec3d p) : pos(initial3d::vec4d(p, 1.0)) {  }
		};



		class Face;

		class Halfedge {
		public:
			//point
			point_t head_i;

			//halfedges
			halfedge_t self_i;
			halfedge_t twin_i;
			halfedge_t next_i;
			halfedge_t prev_i;

			//face
			face_t face_i;


			Halfedge(halfedge_t idx, point_t p_idx) : head_i(p_idx), self_i(idx), twin_i(-1), next_i(-1), prev_i(-1), face_i(-1) {  }
		};

		class Face {
		public:
			face_t self_i;
			halfedge_t edge_i;
			bool flag_inf;
			bool flag_delete;
			plane p;

			inline double evaluate(initial3d::vec3d pos) {
				if (flag_inf) { return initial3d::math::inf<double>(); }
				return p.evaluate(pos);
			}
			inline double evaluate(const Point &point) {
				if (flag_inf) { return initial3d::math::inf<double>(); }
				return ~(point.pos.xyz<double>()) * ~(p.abc());
			}

			inline Face(face_t idx, Halfedge &e0, Halfedge &e1, Halfedge &e2) : self_i(idx), flag_delete(false) {
				edge_i = e0.self_i;
				convexhull::edgePairSeq(e0, e1);
				convexhull::edgePairSeq(e1, e2);
				convexhull::edgePairSeq(e2, e0);
				e0.face_i = self_i;
				e1.face_i = self_i;
				e2.face_i = self_i;
			}


			inline Face(face_t idx, Halfedge &e0, Halfedge &e1, Halfedge &e2, Halfedge &e3) : self_i(idx), flag_delete(false) {
				edge_i = e0.self_i;
				convexhull::edgePairSeq(e0, e1);
				convexhull::edgePairSeq(e1, e2);
				convexhull::edgePairSeq(e2, e3);
				convexhull::edgePairSeq(e3, e0);
				e0.face_i = self_i;
				e1.face_i = self_i;
				e2.face_i = self_i;
				e3.face_i = self_i;
			}

			inline Face(face_t idx, std::vector<Halfedge> &v_edges, std::vector<halfedge_t> &v_edges_i) : self_i(idx), flag_delete(false) {
				edge_i = v_edges_i[0];
				for (int i = 0; i < int(v_edges_i.size() - 1); i++) {
					convexhull::edgePairSeq(v_edges[v_edges_i[i]], v_edges[v_edges_i[i + 1]]);
					v_edges[v_edges_i[i]].face_i = self_i;
				}
				convexhull::edgePairSeq(v_edges[v_edges_i[v_edges_i.size() - 1]], v_edges[v_edges_i[0]]);
				v_edges[v_edges_i[v_edges_i.size() - 1]].face_i = self_i;
			}
		};

		inline convexhull() {  }

		inline convexhull(const convexhull &other) : m_points(other.m_points), m_edges(other.m_edges), m_faces(other.m_faces) {  }

		inline convexhull & operator=(const convexhull &other) {
			m_points = other.m_points;
			m_edges = other.m_edges;
			m_faces = other.m_faces;
			return *this;
		}

		//convex hull all the panes point inwards
		inline convexhull(const std::vector<Face> &f_, const std::vector<Halfedge> &e_, const std::vector<Point> &p_)
			: m_points(p_), m_edges(e_), m_faces(f_) {  }

		inline ~convexhull() {  }

		//returns positive if the point is inside the convex hull
		inline double evaluate(initial3d::vec3d p) {
			double val = initial3d::math::inf<double>();
			for (Face f : m_faces)
				val = std::min(f.evaluate(p), val);
			return val;
		}

		//returns positive if the point is inside the convex hull
		inline double evaluate(const Point &p) {
			double val = initial3d::math::inf<double>();
			for (Face f : m_faces)
				val = std::min(f.evaluate(p), val);
			return val;
		}

		inline void addDirection(initial3d::vec3d d) {
			addPoint(convexhull::createPoint(m_points, initial3d::vec4d(d, 0.0)));
		}

		inline void addPoint(initial3d::vec3d p) {
			addPoint(convexhull::createPoint(m_points, initial3d::vec4d(p, 1.0)));
		}

		inline void addPoint(initial3d::vec4d p) {
			addPoint(convexhull::createPoint(m_points, p));
		}

		inline void addPoint(point_t newP) {

			if (evaluate(m_points[newP]) < 0) {
				//find all the faces to be removed
				std::vector<face_t> delFace;
				for (auto f = m_faces.begin(); f != m_faces.end(); ++f) {
					double eval = (*f).evaluate(m_points[newP]);
					if (eval < 0) {
						delFace.push_back((*f).self_i);
						(*f).flag_delete = true;
					}
				}

				//find an edge that has a twin with a non flagged face
				halfedge_t base = -1;
				for (face_t f_i : delFace) {
					for (halfedge_t he_i : faceGetEdges(m_faces, m_edges, f_i)) {
						if (!m_faces[m_edges[m_edges[he_i].twin_i].face_i].flag_delete) {
							base = m_edges[he_i].twin_i; // halfdge on the non-flagged side
							break;
						}
					}
					if (base > 0) break;
				}

				//create the first face of the new set

				//first create the new twin to the base
				halfedge_t baseTwin = createHalfEdge(m_edges, m_edges[m_edges[base].prev_i].head_i);
				convexhull::edgePairTwin(m_edges, base, baseTwin);

				//this is the edge we connect up last
				halfedge_t startEdge = convexhull::createHalfEdge(m_edges, newP);

				//this is the edge we'll connect to the next face
				halfedge_t connectEdge = convexhull::createHalfEdge(m_edges, m_edges[base].head_i);

				convexhull::createFace(m_faces, m_edges, m_points, baseTwin, startEdge, connectEdge);

				halfedge_t endCase = m_edges[base].twin_i;
				do { //cycle around the base of the base to work out the end case base edge
					endCase = m_edges[m_edges[endCase].twin_i].prev_i;
				} while (!m_faces[m_edges[m_edges[endCase].twin_i].face_i].flag_delete);


				do {
					base = m_edges[base].twin_i;
					do { //cycle around the head of the base to work out the next base edge
						base = m_edges[m_edges[base].twin_i].next_i;
					} while (!m_faces[m_edges[m_edges[base].twin_i].face_i].flag_delete);

					halfedge_t connectEdgeTwin = convexhull::createHalfEdge(m_edges, m_edges[m_edges[connectEdge].prev_i].head_i);
					convexhull::edgePairTwin(m_edges, connectEdge, connectEdgeTwin);

					baseTwin = convexhull::createHalfEdge(m_edges, m_edges[m_edges[base].prev_i].head_i);
					convexhull::edgePairTwin(m_edges, base, baseTwin);

					halfedge_t lastEdge = convexhull::createHalfEdge(m_edges, m_edges[base].head_i);

					convexhull::createFace(m_faces, m_edges, m_points, baseTwin, connectEdgeTwin, lastEdge);

					//cycle for next iteration
					connectEdge = lastEdge;
				} while (base != endCase);

				convexhull::edgePairTwin(m_edges, connectEdge, startEdge);

				//get rid of the faces to be deleted
				removeFaces(m_faces, m_edges, m_points, delFace);
			}
		}

		inline const std::vector<Face>& getFaces() {
			return m_faces;
		};


		inline static point_t createPoint(std::vector<Point> &v_points, initial3d::vec4d p) {
			point_t idx = v_points.size();
			v_points.push_back(Point(p));
			return idx;
		}


		inline static halfedge_t createHalfEdge(std::vector<Halfedge> &v_edges, int p_i) {
			halfedge_t idx = v_edges.size();
			v_edges.push_back(Halfedge(idx, p_i));
			return idx;
		}


		inline static face_t createFace(std::vector<Face> &v_faces, std::vector<Halfedge> &v_edges,
			std::vector<Point> &v_points, halfedge_t e0, halfedge_t e1, halfedge_t e2) {

			face_t idx = v_faces.size();
			v_faces.push_back(Face(idx, v_edges[e0], v_edges[e1], v_edges[e2]));
			convexhull::faceConstructPlane(v_faces, v_edges, v_points, idx);
			return idx;
		}

		inline static face_t createFace(std::vector<Face> &v_faces, std::vector<Halfedge> &v_edges,
			std::vector<Point> &v_points, halfedge_t e0, halfedge_t e1, halfedge_t e2, halfedge_t e3) {

			face_t idx = v_faces.size();
			v_faces.push_back(Face(idx, v_edges[e0], v_edges[e1], v_edges[e2], v_edges[e3]));
			convexhull::faceConstructPlane(v_faces, v_edges, v_points, idx);
			return idx;
		}

		inline static face_t createFace(std::vector<Face> &v_faces, std::vector<Halfedge> &v_edges,
			std::vector<Point> &v_points, std::vector<halfedge_t> &v_input) {

			face_t idx = v_faces.size();
			v_faces.push_back(Face(idx, v_edges, v_input));
			convexhull::faceConstructPlane(v_faces, v_edges, v_points, idx);
			return idx;
		}


		inline static initial3d::vec3d edgeDirection(std::vector<Halfedge> &v_edges, std::vector<Point> &v_points, halfedge_t index) {
			Point head = v_points[v_edges[index].head_i];
			if (head.isDirection())
				return head.pos.xyz<double>();
			Point base = v_points[v_edges[v_edges[index].prev_i].head_i];
			if (base.isDirection())
				return -(base.pos.xyz<double>());

			return ~(head.pos.xyz<double>() - base.pos.xyz<double>());
		}

		inline static void edgePairTwin(std::vector<Halfedge> &v_edges, halfedge_t e0, halfedge_t e1) {
			v_edges[e0].twin_i = e1;
			v_edges[e1].twin_i = e0;
		}

		inline static void edgePairTwin(Halfedge &e0, Halfedge &e1) {
			e0.twin_i = e1.self_i;
			e1.twin_i = e0.self_i;
		}

		//from to
		inline static void edgePairSeq(std::vector<Halfedge> &v_edges, halfedge_t e0, halfedge_t e1) {
			v_edges[e1].prev_i = e0;
			v_edges[e0].next_i = e1;
		}

		inline static void edgePairSeq(Halfedge &e0, Halfedge &e1) {
			e1.prev_i = e0.self_i;
			e0.next_i = e1.self_i;
		}

		inline static std::vector<halfedge_t> faceGetEdges(std::vector<Face> &v_faces, std::vector<Halfedge> &v_edges, face_t f) {
			std::vector<halfedge_t> edges;
			halfedge_t start = v_faces[f].edge_i;
			edges.push_back(start);
			halfedge_t next = v_edges[start].next_i;
			while (next != start) {
				edges.push_back(next);
				next = v_edges[next].next_i;
			}
			return edges;
		}

		//helper method constructtes and assigns a plane to a face after creation
		inline static void faceConstructPlane(std::vector<Face> &v_faces, std::vector<Halfedge> &v_edges,
			std::vector<Point> &v_points, face_t f_i) { //this could be extended to actually give a plane for inf_faces TODO
			halfedge_t left = -1;
			for (halfedge_t he_i : faceGetEdges(v_faces, v_edges, f_i)) {
				if (!v_points[v_edges[he_i].head_i].isDirection()) {
					left = he_i;
					break;
				}
			}

			if (!(v_faces[f_i].flag_inf = (left == halfedge_t(-1)))) {
				halfedge_t right = v_edges[left].next_i;
				initial3d::vec3d norm = (-edgeDirection(v_edges, v_points, left)) ^ edgeDirection(v_edges, v_points, right);
				initial3d::vec3d point = v_points[v_edges[left].head_i].pos;
				v_faces[f_i].p = plane(norm, point);
			}
		}

		inline static void removeFaces(std::vector<Face> &faces, std::vector<Halfedge> &edges, std::vector<Point> &points,
			std::vector<face_t> &faces_rem) {

			std::vector<Face> newFaces;
			std::vector<Halfedge> newEdges;

			std::map<halfedge_t, halfedge_t> halfedgeToOrigin;
			std::map<halfedge_t, halfedge_t> halfedgeToNew;

			for (Face f : faces) {
				auto pos = std::find(faces_rem.begin(), faces_rem.end(), f.self_i);
				if (pos == faces_rem.end()) {


					std::vector<halfedge_t> f_edges = faceGetEdges(faces, edges, f.self_i);
					std::vector<halfedge_t> newEdges_i;

					for (halfedge_t he_i : f_edges) {
						halfedge_t new_i = createHalfEdge(newEdges, edges[he_i].head_i);
						newEdges_i.push_back(new_i);
						halfedgeToOrigin[new_i] = he_i;
						halfedgeToNew[he_i] = new_i;
					}

					convexhull::createFace(newFaces, newEdges, points, newEdges_i);
				}
			}

			//link all of the edges
			for (Halfedge he : newEdges) {
				convexhull::edgePairTwin(newEdges, he.self_i, halfedgeToNew[edges[halfedgeToOrigin[he.self_i]].twin_i]);
			}

			faces = newFaces;
			edges = newEdges;
		}

	private:
		std::vector<Point> m_points;
		std::vector<Halfedge> m_edges;
		std::vector<Face> m_faces;
	};





	class boundt {
	public:
		static void test() {
			std::cout << "Doing tests" << std::endl;

			using namespace initial3d;
			aabb unit(vec3d::zero(), vec3d(.5, .5, .5));

			//test points
			//inside
			if (!unit.contains(vec3d())) throw std::runtime_error("got borked here broz");

			//outside
			if (unit.contains(vec3d::one())) throw std::runtime_error("got borked here broz");


			//test aabb intersection
			//inside
			if (!unit.intersects(aabb(vec3d(0, 0, 0), vec3d(.2, .2, .2)))) throw std::runtime_error("got borked here broz");
			if (!unit.intersects(aabb(vec3d(1, 1, 1), vec3d(.6, .6, .6)))) throw std::runtime_error("got borked here broz");
			if (!unit.intersects(aabb(vec3d(-1, -1, -1), vec3d(.6, .6, .6)))) throw std::runtime_error("got borked here broz");
			if (!unit.intersects(aabb(vec3d(1, 0, 0), vec3d(.6, .1, .1)))) throw std::runtime_error("got borked here broz");


			//outside
			if (unit.intersects(aabb(vec3d(1, 0, 0), vec3d(.2, .2, .2)))) throw std::runtime_error("got borked here broz");
			if (unit.intersects(aabb(vec3d(1, 1, 1), vec3d(.2, .2, .2)))) throw std::runtime_error("got borked here broz");
			if (unit.intersects(aabb(vec3d(-1, 0, 0), vec3d(.2, .2, .2)))) throw std::runtime_error("got borked here broz");
			if (unit.intersects(aabb(vec3d(-1, -1, -1), vec3d(.2, .2, .2)))) throw std::runtime_error("got borked here broz");

			//plane construction and intersection tests on one one planes
			testPlaneConstruct(plane(1, 1, 1, +vec3d::one()));
			testPlaneConstruct(plane(vec3d::one(), +vec3d::one()));
			testPlaneConstruct(plane(vec3d::one(), vec3d::one()));

			plane tplane(vec3d::i());
			if (tplane.evaluate(aabb(vec3d::i(), vec3d::one()*0.1)) <= 0) throw std::runtime_error("got borked here broz");

			//plane transformation tests
			testPlaneConstruct(plane(vec3d::one()).transform(mat4d::translate(vec3d::one())));

			tplane = plane(vec3d::i()).transform(mat4d::rotateY(3.1415)); // rotate to point in opisite directions
			std::cout << tplane.abc() << " : " << tplane.d() << std::endl;
			std::cout << tplane.evaluate(aabb(vec3d::i(), vec3d::one()*0.1)) << std::endl;
			if (tplane.evaluate(aabb(vec3d::i(), vec3d::one()*0.1)) > 0) throw std::runtime_error("got borked here broz");

			tplane = tplane.transform(mat4d::translate(vec3d(2, 0, 0))); // move along pos-x
			std::cout << tplane.abc() << " : " << tplane.d() << std::endl;
			std::cout << tplane.evaluate(aabb(vec3d::i(), vec3d::one()*0.1)) << std::endl;
			if (tplane.evaluate(aabb(vec3d::i(), vec3d::one()*0.1)) <= 0) throw std::runtime_error("got borked here broz");


		}

		static void testPlaneConstruct(plane tplane ) {
			using namespace initial3d;

			//plane point tests
			if (tplane.evaluate(vec3d::one()*1.1) <= 0) throw std::runtime_error("got borked here broz");
			if (std::abs(tplane.evaluate(vec3d::one())) > 0.0005) throw std::runtime_error("got borked here broz");
			if (tplane.evaluate(vec3d::one()*0.9) >= 0) throw std::runtime_error("got borked here broz");

			//plane aabb intersection tests
			if (tplane.evaluate(aabb(vec3d::one()*2, vec3d(.2, .2, .2))) <= 0) throw std::runtime_error("got borked here broz");
			if (tplane.evaluate(aabb(vec3d::one()*0.9, vec3d(.2, .2, .2))) != 0) throw std::runtime_error("got borked here broz");
			if (tplane.evaluate(aabb(vec3d::one(), vec3d(.2, .2, .2))) != 0) throw std::runtime_error("got borked here broz");
			if (tplane.evaluate(aabb(vec3d::one()*1.1, vec3d(.2, .2, .2))) != 0) throw std::runtime_error("got borked here broz");

			if (tplane.evaluate(aabb(vec3d::zero(), vec3d(.2, .2, .2))) >= 0) throw std::runtime_error("got borked here broz");
		}
	};

} // bound
} // ambition
