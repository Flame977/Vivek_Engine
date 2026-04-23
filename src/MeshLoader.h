#pragma once
#include <string>
#include <stdexcept>
#include <iostream>
#include "MeshData.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"


class  MeshLoader
{
public:
	static MeshData Load(const std::string& path);
};