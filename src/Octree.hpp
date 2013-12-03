
/*
 * Initial3D Octree Header
 *
 * octree<T> always holds pointers to T
 *
 * @author Ben Allen
 */

#ifndef INITIAL3D_OCTREE_H
#define INITIAL3D_OCTREE_H

#include <cassert>
#include <iostream>
#include <string>
#include <stdexcept>
#include <cstring>
#include <set>
#include <vector>
#include <algorithm>

#include "initial3d.h"

#define INITIAL3D_OCTREE_MAX_LEAF_ELEMENTS 16

namespace initial3d {

	template <typename T> class aabb;
	typedef aabb<float> aabbf;
	typedef aabb<double> aabbd;

	template <typename T> class octree;

	//
	// Example class, showing must-implement features for use with octree
	//
	class example_octree_element {
	public:
		// determine coordinate type of AABB
		typedef double coord_t;
		// get the AABB
		aabb<coord_t> getAABB() const;
	};

	//
	// Axis-Aligned Bounding Box
	//
	template <typename T>
	class aabb {
	private:
		vec3<T> m_min, m_max;
		
	public:
		typedef T coord_t;

		aabb(const vec3<T> &a, const vec3<T> &b) : m_min(vec3<T>::negative_extremes(a, b)), m_max(vec3<T>::positive_extremes(a, b)) { }
		
		const vec3<T> & min() const {
			return m_min;
		}

		const vec3<T> & max() const {
			return m_max;
		}

		vec3<T> centre() const {
			return 0.5 * (m_min + m_max);
		}

		bool contains(const vec3<T> &p) const {
			bool inside = true;
			inside = inside && (m_min.x() <= p.x() && p.x() <= m_max.x());
			inside = inside && (m_min.y() <= p.y() && p.y() <= m_max.y());
			inside = inside && (m_min.z() <= p.z() && p.z() <= m_max.z());
			return inside;
		}

		bool contains(const aabb<T> &a) const {
			bool inside = true;
			inside = inside && (m_min.x() <= a.m_min.x() && a.m_max.x() <= m_max.x());
			inside = inside && (m_min.y() <= a.m_min.y() && a.m_max.y() <= m_max.y());
			inside = inside && (m_min.z() <= a.m_min.z() && a.m_max.z() <= m_max.z());
			return inside;
		}

		bool intersects(const aabb<T> &a) const {
			bool hit = true;
			hit = hit && (m_min.x() <= a.m_max.x() && a.m_min.x() <= m_max.x());
			hit = hit && (m_min.y() <= a.m_max.y() && a.m_min.y() <= m_max.y());
			hit = hit && (m_min.z() <= a.m_max.z() && a.m_min.z() <= m_max.z());
			return hit;
		}

		inline friend std::ostream & operator<<(std::ostream &out, const aabb<T> &a) {
			out << "aabb[" << a.m_min << " <= x <= " << a.m_max << "]";
			return out;
		}

		inline static aabb<T> fromInnerSphere(const vec3<T> &pos, T radius) {
			vec3<T> vr = vec3<T>::one() * radius;
			return aabb<T>(pos - vr, pos + vr);
		}

		inline static aabb<T> fromOuterSphere(const vec3<T> &pos, T radius) {
			vec3<T> vr = vec3<T>::one() * radius / math::sqrt(3.0);
			return aabb<T>(pos - vr, pos + vr);
		}
	};

	//
	// Octree
	//
	template <typename T>
	class octree {
	public:
		typedef typename T::coord_t coord_t;

	protected:
		class out_of_bounds { };

		class Node {
		private:
			Node *m_parent;
			aabb<coord_t> m_aabb;
			bool m_isleaf;
			size_t m_count;
			std::set<T *> m_elements;
			Node *m_children[8];

		public:
			Node(Node *parent_, const aabb<coord_t> &aabb_) : 
				m_parent(parent_),
				m_aabb(aabb_),
				m_isleaf(true),
				m_count(0),
				m_elements() {
					// clear children pointers
					std::memset(m_children, 0, 8 * sizeof(Node *));
			}
			
			Node & operator=(const Node &other) {
				// copy assignment shouldnt really be needed for this class
				assert(false && "DONT CALL THIS!");
				return *this;
			}
			
			Node(const Node &other) :
				m_parent(other.m_parent),
				m_aabb(other.m_aabb),
				m_isleaf(other.isleaf),
				m_count(other.m_count),
				m_elements(other.m_elements) {
					// clear children pointers
					std::memset(m_children, 0, 8 * sizeof(Node *));
					// make new child nodes
					for (int i = 0; i < 8; i++) {
						if (other.m_children[i] != NULL) {
							m_children[i] = new Node(*(other.m_children[i]));
						}
					}
			}
			
			Node * parent() {
				return m_parent;
			}

			aabb<coord_t> getAABB() {
				return m_aabb;
			}

			size_t count() {
				return m_count;
			}

			size_t countRecursively() {
				size_t c = m_elements.size();
				for (Node **pn = m_children + 8; pn --> m_children; ) {
					if (*pn != NULL) c += (*pn)->countRecursively();
				}
				return c;
			}
			
		private:
			void dump(std::set<T *> &elements) {
				elements.insert(m_elements.begin(), m_elements.end());
				for (int i = 0; i < 8; i++) {
					if (m_children[i] != NULL) m_children[i]->dump(elements);
				}
			}

			unsigned int childID(const vec3<coord_t> &p) {
				const vec3<coord_t> centre = m_aabb.centre();
				unsigned int cid = 0;
				cid |= int(p.x() >= centre.x()) << 0;
				cid |= int(p.y() >= centre.y()) << 1;
				cid |= int(p.z() >= centre.z()) << 2;
				assert(cid < 8);
				return cid;
			}
			
			void unleafify();

			void leafify();

		public:
			bool add(T *t) {
				aabb<coord_t> a = t->getAABB();
				if (!m_aabb.contains(a)) throw out_of_bounds();
				if (m_isleaf && m_count < INITIAL3D_OCTREE_MAX_LEAF_ELEMENTS) {
					if (!m_elements.insert(t).second) return false;
				} else {
					// not a leaf or should not be
					unleafify();
					unsigned int cid_min = childID(a.min());
					unsigned int cid_max = childID(a.max());
					if (cid_min != cid_max) {
						// element spans multiple child nodes, add to this one
						if (!m_elements.insert(t).second) return false;
					} else {
						// element contained in one child node - create if necessary then add
						Node *child = m_children[cid_min];
						if (child == NULL) {
							const vec3<coord_t> centre = m_aabb.centre();
							// vector to a corner of the current node's aabb
							vec3<coord_t> vr = m_aabb.max() - centre;
							if (!(cid_min & 0x1)) vr.x() = -vr.x();
							if (!(cid_min & 0x2)) vr.y() = -vr.y();
							if (!(cid_min & 0x4)) vr.z() = -vr.z();
							child = new Node(this, aabb<coord_t>(centre, centre + vr));
							m_children[cid_min] = child;
						}
						try {
							if (!child->add(t)) return false;
						} catch (out_of_bounds &e) {
							// child doesn't want to accept it
							if (!m_elements.insert(t).second) return false;
						}
					}
				}
				m_count++;
				return true;
			}

			void putChild(Node *child) {
				child->m_parent = this;
				int cid = childID(child->getAABB().centre());
				m_children[cid] = child;
				m_count += child->count();
				unleafify();
			}

			bool remove(T *t) {
				aabb<coord_t> a = t->getAABB();
				if (!m_aabb.contains(a)) return false;
				unsigned int cid_min = childID(a.min());
				unsigned int cid_max = childID(a.max());
				if (m_elements.erase(t)) {
					// remove from this node succeeded
				} else {
					if (m_isleaf) return false;
					// split across children => should be in this node
					if (cid_min != cid_max) return false;
					// remove from child
					Node *child = m_children[cid_min];
					if (child == NULL) return false;
					if (!child->remove(t)) return false;
					if (child->count() == 0) {
						// child is now empty, delete it
						m_children[cid_min] = NULL;
						delete child;
					}
				}
				m_count--;
				if (m_count <= INITIAL3D_OCTREE_MAX_LEAF_ELEMENTS) leafify();
				return true;
			}

			bool contains(T *t) {
				aabb<coord_t> a = t->getAABB();
				if (!m_aabb.contains(a)) return false;
				if (m_elements.find(t) != m_elements.end()) return true;
				if (m_isleaf) return false;
				unsigned int cid_min = childID(a.min());
				unsigned int cid_max = childID(a.max());
				// if split across children, should be in elements
				if (cid_min != cid_max) return false;
				Node *child = m_children[cid_min];
				if (child == NULL) return false;
				return child->contains(t);
			}

			void find(std::vector<T *> &found, const aabb<coord_t> &a) {
				if (m_aabb.intersects(a)) {
					for (typename std::set<T *>::const_iterator it = m_elements.begin(); it != m_elements.end(); it++) {
						if (a.intersects((*it)->getAABB())) found.push_back(*it);
					}
					if (m_isleaf) return;
					unsigned int cid_min = childID(a.min());
					unsigned int cid_max = childID(a.max());
					if (cid_min == cid_max) {
						// a intersects only one child
						Node *child = m_children[cid_min];
						if (child != NULL) child->find(found, a);
					} else {
						for (int i = 0; i < 8; i++) {
							if (m_children[i] != NULL) m_children[i]->find(found, a);
						}
					}
				}
			}

			void print(std::string indent) {
				std::cout << indent << m_aabb << std::endl;
				for (typename std::set<T *>::const_iterator it = m_elements.begin(); it != m_elements.end(); it++) {
					std::cout << indent << "    " << **it << std::endl;
				}
				for (int i = 0; i < 8; i++) {
					if (m_children[i] != NULL) m_children[i]->print(indent + "    ");
				}
			}

			size_t height() {
				size_t h = 0;
				for (int i = 0; i < 8; i++) {
					if (m_children[i] != NULL) {
						size_t hn = m_children[i]->height();
						h = initial3d::math::max(h, hn);
					}
				}
				return h + 1;
			}

			double heightAvg() {
				double h = 0;
				int c = 0;
				for (int i = 0; i < 8; i++) {
					if (m_children[i] != NULL) {
						h += m_children[i]->heightAvg();
						c++;
					}
				}
				return 1 + (c > 0 ? h / c : 0);
			}

			// iterators...
			
			~Node() {
				// delete child nodes
				for (int i = 0; i < 8; i++) {
					if (m_children[i] != NULL) delete m_children[i];
				}
			}
		};

	private:
		Node *m_root;

	public:
		octree() {
			m_root = new Node(NULL, aabb<coord_t>(vec3<coord_t>::zero(), vec3<coord_t>::one()));
		}

		octree(const vec3<coord_t> &root_p0, const vec3<coord_t> &root_p1) {
			m_root = new Node(NULL, aabb<coord_t>(root_p0, root_p1));
		}
		
		octree(const octree<T> &other) {
			m_root = new Node(other.m_root);
		}

		octree & operator=(const octree<T> &other) {
			delete m_root;
			m_root = new Node(other.m_root);
			return *this;
		}
		
		bool add(T *t) {
			try {
				return m_root->add(t);
			} catch (out_of_bounds &e) {
				// make new root
				Node * const oldroot = m_root;
				const aabb<coord_t> a = m_root->getAABB();
				// vector from centre to max of new root
				const vec3<coord_t> vr = a.max() - a.min();
				// vector from centre of current root to centre of new element
				const vec3<coord_t> vct = t->getAABB().centre() - a.centre();
				// vector from current root to corner nearest centre of new element
				vec3<coord_t> corner = vr * 0.5;
				if (vct.x() < 0) corner.x() = -corner.x();
				if (vct.y() < 0) corner.y() = -corner.y();
				if (vct.z() < 0) corner.z() = -corner.z();
				// centre of new root
				const vec3<coord_t> newcentre = a.centre() + corner;
				m_root = new Node(NULL, aabb<coord_t>(newcentre - vr, newcentre + vr));
				if (oldroot->count() > 0) {
					// only preserve old root if it had elements
					m_root->putChild(oldroot);
				} else {
					delete oldroot;
				}
				// re-attempt to add
				return add(t);
			}
		}

		bool remove(T *t) {
			// TODO root reduction
			return m_root->remove(t);
		}

		bool contains(T *t) const {
			return m_root->contains(t);
		}

		void find(std::vector<T *> &found, const aabb<coord_t> &a) {
			m_root->find(found, a);
		}

		void print() const {
			m_root->print("");
		}

		size_t height() const {
			return m_root->height();
		}

		double heightAvg() const {
			return m_root->heightAvg();
		}

		size_t count() const {
			return m_root->count();
		}

		size_t countRecursively() const {
			return m_root->countRecursively();
		}
		
		~octree() {
			delete m_root;
		}
	};

	template <typename T>
	void octree<T>::Node::unleafify() {
		if (m_isleaf) {
			m_isleaf = false;
			// copy current elements then clear
			std::vector<T *> temp(m_elements.begin(), m_elements.end());
			m_elements.clear();
			// decrement the count because re-adding will increment it again
			m_count -= temp.size();
			for (int i = 0; i < temp.size(); i++) {
				if (temp[i] != NULL) {
					add(temp[i]);
				}
			}
		}
	}

	template <typename T>
	void octree<T>::Node::leafify() {
		if (!m_isleaf) {
			m_isleaf = true;
			for (int i = 0; i < 8; i++) {
				if (m_children[i] != NULL) {
					m_children[i]->dump(m_elements);
					delete m_children[i];
					m_children[i] = NULL;
				}
			}
		}
	}

}

#endif