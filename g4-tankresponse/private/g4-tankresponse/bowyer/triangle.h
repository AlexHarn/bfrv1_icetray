#ifndef H_TRIANGLE
#define H_TRIANGLE

#include "vector2.h"
#include "edge.h"

//typedef Vector2<float> Vec2f;

template<typename T>
class Triangle
{
	public:
		Triangle(const Vector2<T> &_p1, const Vector2<T> &_p2, const Vector2<T> &_p3);
	
		bool containsVertex(const Vector2<T> &v);
		bool circumCircleContains(const Vector2<T> &v);
	
		Vector2<T> p1;
		Vector2<T> p2;
		Vector2<T> p3;
		Edge<T> e1;				
		Edge<T> e2;
		Edge<T> e3;
};

template<typename T>
inline std::ostream &operator << (std::ostream &str, const Triangle<T> & t)
{
	return str << "Triangle:" << std::endl << "\t" << t.p1 << std::endl << "\t" << t.p2 << std::endl << "\t" << t.p3 << std::endl << "\t" << t.e1 << std::endl << "\t" << t.e2 << std::endl << "\t" << t.e3 << std::endl;
		
}

template<typename T>
inline bool operator == (const Triangle<T> &t1, const Triangle<T> &t2)
{
	return	(t1.p1 == t2.p1 || t1.p1 == t2.p2 || t1.p1 == t2.p3) &&
			(t1.p2 == t2.p1 || t1.p2 == t2.p2 || t1.p2 == t2.p3) && 
			(t1.p3 == t2.p1 || t1.p3 == t2.p2 || t1.p3 == t2.p3);
}



#endif
