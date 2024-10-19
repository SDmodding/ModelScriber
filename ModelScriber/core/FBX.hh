//===================================================================================
// 
//	Description:
//		This file is helper & wrapper for FBXSDK related stuff.
// 
//===================================================================================
#pragma once

namespace core
{
	FbxManager* gFbxMgr = nullptr;

	void InitializeFBX()
	{
		gFbxMgr = FbxManager::Create();
		gFbxMgr->SetIOSettings(FbxIOSettings::Create(gFbxMgr, IOSROOT));
	}

	FbxScene* ImportScene(const char* filename)
	{
		auto importer = FbxImporter::Create(gFbxMgr, "FbxImporter");
		if (!importer->Initialize(filename, -1, gFbxMgr->GetIOSettings()))
		{
			qAssertF(false, "ERROR: Call to FbxImporter::Initialize failed '%s'\n", importer->GetStatus().GetErrorString());
			return nullptr;
		}

		auto scene = FbxScene::Create(gFbxMgr, "FbxScene");
		if (!importer->Import(scene))
		{
			qAssertF(false, "ERROR: Call to FbxImporter::Import failed '%s'\n", importer->GetStatus().GetErrorString());
			return nullptr;
		}

		importer->Destroy();

		FbxGeometryConverter geometryConverter(gFbxMgr);
		geometryConverter.Triangulate(scene, true);

		return scene;
	}

	FbxMesh* GetMesh(FbxScene* scene, int index)
	{
		int numNodes = scene->GetNodeCount();
		int mesh_index = 0;

		for (int i = 0; numNodes > i; ++i)
		{
			auto node = scene->GetNode(i);
			if (!node) {
				continue;
			}

			auto mesh = node->GetMesh();
			if (!mesh) {
				continue;
			}

			if (mesh_index++ == index) {
				return mesh;
			}
		}

		return nullptr;
	}

	int GetNumMeshes(FbxScene* scene)
	{
		int numNodes = scene->GetNodeCount();
		int numMeshes = 0;

		for (int i = 0; numNodes > i; ++i)
		{
			auto node = scene->GetNode(i);
			if (node && node->GetMesh()) {
				++numMeshes;
			}
		}

		return numMeshes;
	}

	FbxSurfaceMaterial* GetMeshMaterial(FbxMesh* mesh)
	{
		return mesh->GetNode()->GetMaterial(0);
	}

	FbxSkin* GetMeshSkin(FbxMesh* mesh, int index = 0)
	{
		int numDeformers = mesh->GetDeformerCount();
		int skin_index = 0;

		for (int i = 0; numDeformers > i; ++i)
		{
			auto skin = static_cast<FbxSkin*>(mesh->GetDeformer(i, FbxDeformer::eSkin));
			if (!skin) {
				continue;
			}

			if (skin_index == index) {
				return skin;
			}

			++skin_index;
		}

		return nullptr;
	}

	template <typename T>
	T* GetMaterialObject(FbxSurfaceMaterial* material, const char* name)
	{
		auto materialProperty = material->FindProperty(name);
		if (!materialProperty.IsValid()) {
			return nullptr;
		}

		if (materialProperty.GetSrcObjectCount() <= 0) {
			return nullptr;
		}

		return materialProperty.GetSrcObject<T>(0);
	}

	struct BoneWeightCP
	{
		struct BonePair
		{
			const char* mName;
			double mWeight;
		};
		FbxArray<BonePair> mList;
	};

	FbxArray<BoneWeightCP> GetCPBoneWeights(FbxMesh* mesh)
	{
		FbxArray<BoneWeightCP> boneWeights;

		auto skin = GetMeshSkin(mesh);
		if (skin) 
		{
			int numClusters = skin->GetClusterCount();
			if (numClusters > 0)
			{
				boneWeights.Resize(mesh->GetControlPointsCount());

				for (int i = 0; numClusters > i; ++i)
				{	
					auto cluster = skin->GetCluster(i);
					const char* boneName = cluster->GetLink()->GetName();

					int numIndices = cluster->GetControlPointIndicesCount();
					int* indices = cluster->GetControlPointIndices();
					double* weights = cluster->GetControlPointWeights();

					for (int vi = 0; numIndices > vi; ++vi) {
						boneWeights[indices[vi]].mList.Add({ boneName, weights[vi] });
					}
				}
			}
		}

		return boneWeights;
	}
}