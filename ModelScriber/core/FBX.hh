//===================================================================================
// 
//	Description:
//		This file is helper & wrapper for FBXSDK related stuff.
// 
//===================================================================================
#pragma once

namespace core
{
	using namespace fbxsdk;

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
}