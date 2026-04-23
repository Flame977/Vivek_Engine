#pragma once
#include <vector>
#include "Vertex.h"


struct MeshData
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	//Utility helpers
	bool IsValid() const
	{
		return !vertices.empty() && !indices.empty();
	}

	void Clear()
	{
		vertices.clear();
		indices.clear();
	}

	size_t VertexCount() const
	{
		return vertices.size();
	}

	size_t IndexCount() const
	{
		return indices.size();
	}
};