#ifndef H_DELAUNAY
#define H_DELAUNAY

#include "vector2.h"
#include "triangle.h"

#include <vector>

//typedef Vector2<float> Vec2f;

template <typename T>
class Delaunay
{
  public:
  const std::vector<Triangle<T> >& triangulate(std::vector<Vector2<T> > &vertices);
  const std::vector<Triangle<T> >& getTriangles() const { return _triangles; };
  const std::vector<Edge<T> >& getEdges() const { return _edges; };

	private:
		std::vector<Triangle<T> > _triangles;
		std::vector<Edge<T> > _edges;
};

#endif
