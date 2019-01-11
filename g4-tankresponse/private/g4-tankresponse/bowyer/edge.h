#ifndef H_EDGE
#define H_EDGE

#include "vector2.h"

//typedef Vector2<float> Vec2f;

template<typename T>
class Edge
{
	public:
		Edge(const Vector2<T> &p1, const Vector2<T> &p2) : p1(p1), p2(p2) {};
		Edge(const Edge &e) : p1(e.p1), p2(e.p2) {};

		Vector2<T> p1;
		Vector2<T> p2;
};

template<typename T>
inline std::ostream &operator << (std::ostream &str, Edge<T> const &e)
{
	return str << "Edge " << e.p1 << ", " << e.p2;
}

template<typename T>
inline bool operator == (const Edge<T> & e1, const Edge<T> & e2)
{
	return 	(e1.p1 == e2.p1 && e1.p2 == e2.p2) ||
			(e1.p1 == e2.p2 && e1.p2 == e2.p1);
}

#endif 

