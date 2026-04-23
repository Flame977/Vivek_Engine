#include "MeshLoader.h"

MeshData MeshLoader::Load(const std::string& path)
{
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(
		path,
		aiProcess_Triangulate |
		aiProcess_GenNormals |
		aiProcess_JoinIdenticalVertices
		//aiProcess_FlipUVs   // this caused the uvs to look wierd...
	);

	if (!scene || !scene->HasMeshes())
	{
		throw std::runtime_error(importer.GetErrorString());
	}

	MeshData data{};
	aiMesh* mesh = scene->mMeshes[0];


	// Detect valid UV channel
	int uvChannel = -1;
	for (unsigned int i = 0; i < mesh->GetNumUVChannels(); i++)
	{
		if (mesh->HasTextureCoords(i))
		{
			uvChannel = i;
			break;
		}
	}

	// Vertices 
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex v{};

		// Position
		v.position = {
			mesh->mVertices[i].x,
			mesh->mVertices[i].y,
			mesh->mVertices[i].z
		};

		// Normal
		if (mesh->HasNormals())
		{
			v.normal = {
				mesh->mNormals[i].x,
				mesh->mNormals[i].y,
				mesh->mNormals[i].z
			};
		}

		// UVs
		if (mesh->HasTextureCoords(0))
		{
			v.uv = {
				mesh->mTextureCoords[0][i].x,
				mesh->mTextureCoords[0][i].y
			};
		}

		/*
		if (mesh->HasTangentsAndBitangents())
		{
			const aiVector3D& t = mesh->mTangents[i];
			const aiVector3D& b = mesh->mBitangents[i];
			const aiVector3D& n = mesh->mNormals[i];

			glm::vec3 tangent = glm::vec3(t.x, t.y, t.z);
			glm::vec3 bitangent = glm::vec3(b.x, b.y, b.z);
			glm::vec3 normal = glm::vec3(n.x, n.y, n.z);

			float handedness = (glm::dot(glm::cross(normal, tangent), bitangent) < 0.0f) ? -1.0f : 1.0f;

			v.tangent = glm::vec4(tangent, handedness);
		}
		else
		{
			v.tangent = glm::vec4(0, 0, 0, 1);
		}
		*/

		data.vertices.push_back(v);
	}

	// Indices 
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		const aiFace& face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
		{
			data.indices.push_back(face.mIndices[j]);
		}
	}

	std::cout << "Loaded Model Successfully...!" << std::endl;
	return data;
}
