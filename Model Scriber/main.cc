//=============================================================
// 3rdParty

#include "3rdParty/fast_obj.h"
#include "3rdParty/umHalf.h"

//=============================================================
// Headers

#include <iostream>
#include <vector>
#include <unordered_map>
#include <map>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

//=============================================================
// Defines

#define PROJECT_VERSION		"v1.3.5"
#define PROJECT_NAME		"Model Scriber " PROJECT_VERSION

//=============================================================
// Material Defines

#define MATERIAL_ALPHASTATE			0x12C800F2
#define MATERIAL_RASTERSTATE		0x3BC715E0
#define MATERIAL_SHADER				0x8B5561A1
#define MATERIAL_STATEBLOCK			0x4D04C7F2
#define MATERIAL_TEXTURE			0x8B43FABF

//=============================================================
// Material Default Defines

#define MATERIAL_DEFAULT_DepthBiasSortLayer			0xAF2B2668
#define MATERIAL_DEFAULT_SpecularLook				0x7610933B
#define MATERIAL_DEFAULT_TextureAnim				0xD0B4527C

//=============================================================
// SDK Stuff

#define UFG_MAX(a, b) max(a, b)
#define UFG_PAD_INSERT(x, y) x ## y
#define UFG_PAD_DEFINE(x, y) UFG_PAD_INSERT(x, y)
#define UFG_PAD(size) char UFG_PAD_DEFINE(padding_, __LINE__)[size] = { 0x0 }
#include <SDK/Optional/PermFile/.Includes.hh>
#include <SDK/Optional/StringHash.hh>

//=============================================================
// Helper

namespace Helper
{
	void SetResourceName(UFG::ResourceData_t* p_ResourceData, const char* p_Name, uint32_t p_NameUID = 0, const char* p_Suffix = 0)
	{
		if (p_Suffix) 
		{
			char buffer[128] = { '\0' };
			snprintf(buffer, sizeof(buffer), "%s_%s", p_Name, p_Suffix);
			p_ResourceData->m_NameUID = SDK::StringHashUpper32(buffer);
			snprintf(p_ResourceData->m_DebugName, sizeof(UFG::ResourceData_t::m_DebugName), "0x%X_%s", p_NameUID, p_Suffix);
			return;
		}

		p_ResourceData->m_NameUID = SDK::StringHashUpper32(p_Name);
		strncpy(p_ResourceData->m_DebugName, p_Name, sizeof(UFG::ResourceData_t::m_DebugName));
	}

	void SetUV(uint16_t* p_UVBytes, float* p_UV)
	{
		HalfFloat hUV0 = p_UV[0];
		HalfFloat hUV1 = (1.f - p_UV[1]);

		p_UVBytes[0] = hUV0.bits;
		p_UVBytes[1] = hUV1.bits;
	}

	uint8_t GetPackedFloat(float p_Value)
	{
		return static_cast<uint8_t>((p_Value + 1.f) / 2.f * 255.f);
	}

	void SetPackedFloat3(uint8_t* p_ArrayBytes, float* p_Array)
	{
		p_ArrayBytes[0] = GetPackedFloat(p_Array[0]);
		p_ArrayBytes[1] = GetPackedFloat(p_Array[1]);
		p_ArrayBytes[2] = GetPackedFloat(p_Array[2]);
	}

	void SetPackedFloat4(uint8_t* p_ArrayBytes, float* p_Array)
	{
		p_ArrayBytes[0] = GetPackedFloat(p_Array[0]);
		p_ArrayBytes[1] = GetPackedFloat(p_Array[1]);
		p_ArrayBytes[2] = GetPackedFloat(p_Array[2]);
		p_ArrayBytes[3] = GetPackedFloat(p_Array[3]);
	}
}

//=============================================================
// Classes

class Buffer : public Illusion::Buffer_t
{
public:
	void* m_pData = nullptr;
	uint32_t m_DataSize = 0;

	Buffer()
	{
		// ResourceData
		m_TypeUID	= 0x7A971479;
		m_ChunkUID	= 0x92CDEC8F;
		m_NameUID	= 0x0;
		memset(m_DebugName, 0, sizeof(m_DebugName));

		m_Type		= UINT32_MAX;
	}

	~Buffer()
	{
		if (m_pData) {
			free(m_pData);
		}
	}

	void SetElements(uint32_t p_NumElements, uint32_t p_ElementSize)
	{
		m_NumElements	= p_NumElements;
		m_ElementSize	= p_ElementSize;
		m_Size			= (m_NumElements * m_ElementSize);
	}

	void Initialize()
	{
		m_DataSize = GetBytesToReserve();
		m_pData = malloc(static_cast<size_t>(m_DataSize));
		memset(m_pData, 0, static_cast<size_t>(m_DataSize));

		// ResourceData
		SetEntrySize(sizeof(Illusion::Buffer_t) + m_DataSize);
	}

	void WriteToFile(FILE* p_File)
	{
		fwrite(this, sizeof(Illusion::Buffer_t), 1, p_File);
		fwrite(m_pData, sizeof(uint8_t), static_cast<size_t>(m_DataSize), p_File);
	}
};

class Material : public Illusion::Material_t
{
public:
	std::vector<Illusion::MaterialParam_t> m_Params;
	Illusion::MaterialUser_t m_MaterialUser;

	Material()
	{
		// ResourceData
		m_TypeUID	= 0xF5F8516F;
		m_ChunkUID	= 0xB4C26312;
		m_NameUID	= 0x0;
		memset(m_DebugName, 0, sizeof(m_DebugName));
	}

	void SetNumParams(uint32_t p_NumParams)
	{
		uint64_t uMaterialTableSize = (static_cast<uint64_t>(p_NumParams) * sizeof(Illusion::MaterialParam_t));
		SetEntrySize(static_cast<uint32_t>(sizeof(Material_t) + uMaterialTableSize + sizeof(Illusion::MaterialUser_t)));

		m_NumParams = p_NumParams;
		m_MaterialUserOffset = (uMaterialTableSize + 0x8);
	}

	void AddParam(uint32_t p_StateTypeUID, uint32_t p_StateNameUID, uint32_t p_TypeUID, uint32_t p_NameUID)
	{
		Illusion::MaterialParam_t mParam;
		memset(&mParam, 0, sizeof(Illusion::MaterialParam_t));
		{
			mParam.m_StateParam.m_TypeUID = p_StateTypeUID;
			mParam.m_StateParam.m_NameUID = p_StateNameUID;
			mParam.m_TypeUID = p_TypeUID;
			mParam.m_NameUID = p_NameUID;
		}

		m_Params.emplace_back(mParam);
		SetNumParams(static_cast<uint32_t>(m_Params.size()));
	}

	void AddParam(uint32_t p_StateUID, uint32_t p_TypeUID, uint32_t p_NameUID)
	{
		AddParam(p_StateUID, p_StateUID, p_TypeUID, p_NameUID);
	}

	void WriteToFile(FILE* p_File)
	{
		fwrite(this, sizeof(Illusion::Material_t), 1, p_File);
		fwrite(m_Params.data(), sizeof(Illusion::MaterialParam_t), m_Params.size(), p_File);
		fwrite(&m_MaterialUser, sizeof(Illusion::MaterialUser_t), 1, p_File);
	}
};

class ModelData : public Illusion::ModelData_t
{
public:
	ModelData()
	{
		// ResourceData
		m_TypeUID	= 0x6DF963B3;
		m_ChunkUID	= 0xA2ADCD77;
		m_NameUID	= 0x0;
		memset(m_DebugName, 0, sizeof(m_DebugName));

		// Handles
		m_MaterialTableHandle.m_NameUID = 0x0;
		m_BonePaletteHandle.m_NameUID	= 0x0;

		// Offsets
		m_MeshOffset		= 0xA8;
		m_ModelUserOffset	= 0x88;
	}

	void SetNumMeshes(uint32_t p_NumMeshes)
	{
		m_NumMeshes = p_NumMeshes;
		SetEntrySize(sizeof(Illusion::ModelData_t) + sizeof(Illusion::ModelUser_t) + (0x110 * static_cast<int64_t>(m_NumMeshes)) + 0x38);
	}

	void CalculateAABB(uint8_t* p_VertexBuffer, uint32_t p_NumElements, uint32_t p_ElementSize)
	{
		for (uint32_t i = 0; p_NumElements > i; ++i)
		{
			float* pVertex = reinterpret_cast<float*>(&p_VertexBuffer[i * p_ElementSize]);
			for (int v = 0; 3 > v; ++v)
			{
				if (pVertex[v] < m_AABBMin[v]) {
					m_AABBMin[v] = pVertex[v];
				}
				else if (pVertex[v] > m_AABBMax[v]) {
					m_AABBMax[v] = pVertex[v];
				}
			}
		}
	}
};

class ModelScriber
{
public:
	fastObjMesh* m_ObjMesh = nullptr;

	enum eVertexDeclType : int
	{
		eVertexDeclType_Unknown = -1,
		eVertexDeclType_UVN,
		eVertexDeclType_Skinned,
	};
	eVertexDeclType m_VertexDeclType = eVertexDeclType_Unknown;

	// Perm.Bin
	Material m_Material;
	Buffer m_IndexBuffer;
	Buffer m_VertexBuffers[4];
	ModelData m_ModelData;
	Illusion::ModelUser_t m_ModelUser;
	Illusion::Mesh_t m_Mesh;

	ModelScriber()
	{ 
		// ModelUser
		m_ModelUser.m_SoftBodyDataOffset	= (sizeof(Illusion::ModelUser_t) + 0x10 + (0x110 * 0x1)); // 0x110 (Mesh Align Size) * 0x1 (Mesh Count)
		m_ModelUser.m_HasSoftBodyInfo		= 0;
	}

	~ModelScriber()
	{
		if (m_ObjMesh) {
			fast_obj_destroy(m_ObjMesh);
		}
	}

	void SetName(const char* p_Name)
	{
		uint32_t uNameUID = SDK::StringHashUpper32(p_Name);

		// Material
		Helper::SetResourceName(&m_Material, p_Name, uNameUID, "Material");

		// IndexBuffer
		Helper::SetResourceName(&m_IndexBuffer, p_Name, uNameUID, "IndexBuffer");

		// VertexBuffers
		{
			switch (m_VertexDeclType)
			{
				case eVertexDeclType_UVN:
				{
					Helper::SetResourceName(&m_VertexBuffers[0], p_Name, uNameUID, "VertexBuffer");
					Helper::SetResourceName(&m_VertexBuffers[1], p_Name, uNameUID, "UVNBuffer");
				}
				break;
				case eVertexDeclType_Skinned:
				{
					Helper::SetResourceName(&m_VertexBuffers[0], p_Name, uNameUID, "VertexBuffer");
					Helper::SetResourceName(&m_VertexBuffers[1], p_Name, uNameUID, "BlendBuffer");
					Helper::SetResourceName(&m_VertexBuffers[2], p_Name, uNameUID, "UVBuffer");
				}
				break;
			}
		}

		// ModelData
		Helper::SetResourceName(&m_ModelData, p_Name);
	}
	
	void LoadMesh(const char* p_FilePath)
	{
		m_ObjMesh = fast_obj_read(p_FilePath);
	}

	bool IsUVMappingValid()
	{
		for (uint32_t i = 0; m_ObjMesh->index_count > i; ++i)
		{
			auto pIndex = &m_ObjMesh->indices[i];
			if (pIndex->p != pIndex->t) { // Check if texture index isnt different to face index, otherwise we fucked...
				return false;
			}
		}

		return true;
	}

	bool AreNormalsValid()
	{
		return (m_ObjMesh->normal_count == m_ObjMesh->texcoord_count);
	}

	bool IsFaceCountValid()
	{
		return ((m_ObjMesh->face_count * 3) == m_ObjMesh->index_count);
	}

	void CreateBuffers()	
	{
		// IndexBuffer
		{
			m_IndexBuffer.m_Type			= 1;
			m_IndexBuffer.m_Size			= (m_ObjMesh->face_count * (sizeof(uint16_t) * 3));
			m_IndexBuffer.m_ElementSize		= sizeof(uint16_t);
			m_IndexBuffer.m_NumElements		= (m_ObjMesh->face_count * sizeof(uint16_t));
			m_IndexBuffer.Initialize();

			uint16_t* pIndexBuffer = reinterpret_cast<uint16_t*>(m_IndexBuffer.m_pData);
			for (uint32_t i = 0; m_ObjMesh->index_count > i; ++i) {
				pIndexBuffer[i] = (m_ObjMesh->indices[i].p - 1);
			}
		}

		switch (m_VertexDeclType)
		{
			case eVertexDeclType_UVN:
			{
				// VertexBuffer
				{
					auto pVertexBuffer = &m_VertexBuffers[0];
					pVertexBuffer->m_Type = 0;
					pVertexBuffer->SetElements(m_ObjMesh->position_count - 1, (sizeof(float) * 3));
					pVertexBuffer->Initialize();

					memcpy(pVertexBuffer->m_pData, &m_ObjMesh->positions[3], static_cast<size_t>(pVertexBuffer->m_DataSize));
				}

				// UVNBuffer
				{
					struct UVNIndex_t
					{
						uint16_t m_UV[2];
						uint8_t m_Normal[4];
					};

					auto pUVNBuffer = &m_VertexBuffers[1];
					pUVNBuffer->m_Type = 0;
					pUVNBuffer->SetElements(m_ObjMesh->texcoord_count - 1, sizeof(UVNIndex_t));
					pUVNBuffer->Initialize();

					for (uint32_t i = 1; pUVNBuffer->m_NumElements >= i; ++i)
					{
						auto pUVNIndex = &reinterpret_cast<UVNIndex_t*>(pUVNBuffer->m_pData)[i - 1];
						Helper::SetUV(pUVNIndex->m_UV, &m_ObjMesh->texcoords[i * 2]);

						if (m_ObjMesh->normal_count > i) {
							Helper::SetPackedFloat3(pUVNIndex->m_Normal, &m_ObjMesh->normals[i * 3]);
						}
						else {
							memset(pUVNIndex->m_Normal, 0, 3);
						}

						pUVNIndex->m_Normal[3] = 0xFF;
					}
				}
			}
			break;
			case eVertexDeclType_Skinned:
			{
				// VertexBuffer
				{
					struct Skinned_t
					{
						float m_Vertices[4];
						uint8_t m_Normal[4];
						uint8_t m_Tangent[4];
					};

					auto pVertexBuffer = &m_VertexBuffers[0];
					pVertexBuffer->m_Type = 0;
					pVertexBuffer->SetElements(m_ObjMesh->position_count - 1, sizeof(Skinned_t));
					pVertexBuffer->Initialize();

					for (uint32_t i = 1; pVertexBuffer->m_NumElements >= i; ++i)
					{
						auto pSkinned = &reinterpret_cast<Skinned_t*>(pVertexBuffer->m_pData)[i - 1];
				
						// Vertices
						{
							memcpy(pSkinned->m_Vertices, &m_ObjMesh->positions[i * 3], (sizeof(float) * 3));
							pSkinned->m_Vertices[3] = 0.f;
						}

						// Normal
						{
							if (m_ObjMesh->normal_count > i) {
								Helper::SetPackedFloat3(pSkinned->m_Normal, &m_ObjMesh->normals[i * 3]);
							}
							else {
								memset(pSkinned->m_Normal, 0, 3);
							}
						}
						pSkinned->m_Normal[3] = 0xFF;

						// Tangent (Is currently unknown, probably need to be calculated somehow...)
						memset(pSkinned->m_Tangent, 0, 3);
						pSkinned->m_Tangent[3] = 0xFF;
					}
				}

				// BlendBuffer
				{
					struct Blend_t
					{
						uint8_t m_Index[4];
						uint8_t m_Weight[4];
					};

					auto pBlendBuffer = &m_VertexBuffers[1];
					pBlendBuffer->m_Type = 0;
					pBlendBuffer->SetElements(m_ObjMesh->position_count - 1, sizeof(Blend_t));
					pBlendBuffer->Initialize();

					for (uint32_t i = 0; pBlendBuffer->m_NumElements > i; ++i)
					{
						auto pBlend = &reinterpret_cast<Blend_t*>(pBlendBuffer->m_pData)[i];
						
						// We just fill this up with static blend data since we can't do bone/blending stuff in OBJ
						memset(pBlend->m_Index, 0, 4);

						pBlend->m_Weight[0] = 0xFF;
						pBlend->m_Weight[1] = pBlend->m_Weight[2] = pBlend->m_Weight[3] = 0x0;
					}
				}

				// UVBuffer
				{
					struct UVIndex_t
					{
						uint16_t m_UV[2];
					};

					auto pUVBuffer = &m_VertexBuffers[2];
					pUVBuffer->m_Type = 0;
					pUVBuffer->SetElements(m_ObjMesh->texcoord_count - 1, sizeof(UVIndex_t));
					pUVBuffer->Initialize();

					for (uint32_t i = 1; pUVBuffer->m_NumElements >= i; ++i)
					{
						auto pUVIndex = &reinterpret_cast<UVIndex_t*>(pUVBuffer->m_pData)[i - 1];
						Helper::SetUV(pUVIndex->m_UV, &m_ObjMesh->texcoords[i * 2]);
					}
				}
			}
			break;
		}
	}
		
	bool AreBuffersValid()
	{
		if (m_IndexBuffer.m_NumElements == 0) {
			return false;
		}

		for (auto& vertexBuffer : m_VertexBuffers)
		{
			if (vertexBuffer.m_Type == UINT32_MAX) {
				continue;
			}

			if (!vertexBuffer.m_pData || vertexBuffer.m_NumElements == 0) {
				return false;
			}
		}

		return true;
	}

	void InitializeMesh()
	{
		// VertexDecl
		switch (m_VertexDeclType)
		{
			case eVertexDeclType_Unknown:
				m_Mesh.m_VertexDeclHandle.m_NameUID = UINT32_MAX; break;
			case eVertexDeclType_UVN:
				m_Mesh.m_VertexDeclHandle.m_NameUID = SDK::StringHash32("VertexDecl.UVN"); break;
			case eVertexDeclType_Skinned:
				m_Mesh.m_VertexDeclHandle.m_NameUID = SDK::StringHash32("VertexDecl.Skinned"); break;
		}

		// Handles
		m_Mesh.m_MaterialHandle.m_NameUID			= m_Material.m_NameUID;
		m_Mesh.m_IndexBufferHandle.m_NameUID		= m_IndexBuffer.m_NameUID;

		for (int i = 0; 4 > i; ++i) {
			m_Mesh.m_VertexBufferHandles[i].m_NameUID = m_VertexBuffers[i].m_NameUID;
		}

		// ...
		m_Mesh.m_PrimType = 0x3; // Triangles
		m_Mesh.m_NumPrims = m_ObjMesh->face_count;
	}

	void OutputFile(const char* p_FilePath, const char* p_Mode)
	{
		FILE* pFile = fopen(p_FilePath, p_Mode);
		if (!pFile) {
			return;
		}

		// Material
		m_Material.WriteToFile(pFile);

		// IndexBuffer
		m_IndexBuffer.WriteToFile(pFile);

		// VertexBuffers
		for (auto& vertexBuffer : m_VertexBuffers)
		{
			if (!vertexBuffer.m_pData) {
				continue;
			}

			vertexBuffer.WriteToFile(pFile);
		}

		// ModelData
		{
			m_ModelData.CalculateAABB(reinterpret_cast<uint8_t*>(m_VertexBuffers[0].m_pData), m_VertexBuffers[0].m_NumElements, m_VertexBuffers[0].m_ElementSize);
			m_ModelData.m_NumPrims = m_ObjMesh->face_count;
			fwrite(&m_ModelData, sizeof(Illusion::Model_t), 1, pFile);
		}

		// Padding
		{
			uint8_t bPadding[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
			fwrite(bPadding, 1, sizeof(bPadding), pFile);
		}

		// Some Unknown Data
		{
			uint8_t bBytes[] = {
				0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
			};
			fwrite(bBytes, 1, sizeof(bBytes), pFile);
		}

		// MaterialUser
		fwrite(&m_ModelUser, sizeof(Illusion::ModelUser_t), 1, pFile);

		// Mesh Offset
		{
			struct MeshOffset_t
			{
				int64_t m_Value;
				UFG_PAD(0x8);

				MeshOffset_t() { }
				MeshOffset_t(int64_t p_Value) : m_Value(p_Value) { }
			};
			MeshOffset_t meshOffset(0x10);
			fwrite(&meshOffset, sizeof(MeshOffset_t), 1, pFile);
		}

		// Mesh
		{
			fwrite(&m_Mesh, sizeof(Illusion::Mesh_t), 1, pFile);

			uint8_t bAlign[] = { 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
			};
			fwrite(bAlign, 1, sizeof(bAlign), pFile);
		}

		fclose(pFile);
	}
};

#include "Console.hh"

void ShowArgOptions()
{
	std::pair<const char*, const char*> arrArgs[] =
	{
		{ "-obj", "Obj file to scribe." },
		{ "-dir", "Output directory, if not set it uses object folder." },
		{ "-name", "Obj internal name (optional)." },
		{ "-texdiffuse", "Texture diffuse name." },
		{ "-texnormal", "Texture normal map name." },
		{ "-texspecular", "Texture specular map name." },
		{ "-rasterstate", "0 - None\n1 - Normal\n2 - Disable Write\n3 - Invert Disable Write\n4 - Disable Depth Test\n5 - Double Sided\n6 - Double Sided Alpha\n7 - Invert Culling" },
		{ "-skinned", "Use skinned export. (WIP)" },
		{ "-append", "Appends model to pre-existing perm.bin file." },
	};

	Con::Print(14, "Launch Options:\n");
	for (auto& argPair : arrArgs)
	{
		Con::Print(13, "\t%s", argPair.first);
		Con::SetColor(11); 
		
		std::string sDescription = argPair.second;
		size_t sDescNewline = sDescription.find('\n');
		while (sDescNewline != std::string::npos)
		{
			++sDescNewline;
			sDescription.insert(sDescNewline, "\t\t\t");
			sDescNewline = sDescription.find('\n', sDescNewline);
		}

		printf("\t%s\n", &sDescription[0]);
	}
}

int main(int p_Argc, char** p_Argv)
{
	Con::Initialize(PROJECT_NAME, p_Argc, p_Argv);
	Con::Print(240, PROJECT_NAME "\n");

	//--------------------------------------------------------------------------
	// Arguments

	std::string sObjectFile			= Con::GetArgParam("-obj");
	std::string sOutputDirectory	= Con::GetArgParam("-dir");
	std::string sObjectName			= Con::GetArgParam("-name");
	std::string sTextureDiffuse		= Con::GetArgParam("-texdiffuse");
	std::string sTextureNormal		= Con::GetArgParam("-texnormal");
	std::string sTextureSpecular	= Con::GetArgParam("-texspecular");
	int iRasterState				= Con::GetArgParamInt("-rasterstate");
	bool bSkinned					= Con::IsArgSet("-skinned");
	bool bAppend					= Con::IsArgSet("-append");

	if (Con::IsArgSet("-debug")) 
	{
		Con::Print(240, "Waiting for debugger...\n");
		while (!IsDebuggerPresent()) {
			Sleep(1);
		}
	}

	//--------------------------------------------------------------------------

	if (sObjectFile.empty())
	{
		Con::PrintError("No obj file found!\n");
		ShowArgOptions();
		return 1;
	}

	if (sObjectFile.find(".obj") == std::string::npos)
	{
		Con::PrintError("Obj file must have extension .obj!\n");
		return 1;
	}

	Con::Print(224, "Obj File: %s\n\n", &sObjectFile[0]);

	if (sObjectName.empty())
	{
		size_t sLastDelimer = sObjectFile.find_last_of("/\\");
		if (sLastDelimer == std::string::npos) {
			sLastDelimer = 0;
		}
		else {
			++sLastDelimer;
		}

		sObjectName = sObjectFile.substr(sLastDelimer);
		sObjectName = sObjectName.substr(0, sObjectName.find_last_of('.'));
	}

	if (sTextureDiffuse.empty())
	{
		Con::PrintWarning("No texture diffuse name specified using default!\n");
		sTextureDiffuse = "DEFAULT";
	}

	ModelScriber modelScribe;
	modelScribe.LoadMesh(&sObjectFile[0]);

	if (!modelScribe.m_ObjMesh)
	{
		Con::PrintError("Failed to load obj file!\n");
		return 1;
	}

	if (1 >= modelScribe.m_ObjMesh->texcoord_count)
	{
		Con::PrintError("Obj has no uv mapping!\n");
		return 1;
	}

	if (!modelScribe.IsFaceCountValid())
	{
		Con::PrintError("Obj face count is invalid, make sure vertices are triangles!\n");
		return 1;
	}

	if (!modelScribe.IsUVMappingValid()) {
		Con::PrintWarning("Obj file contains invalid uv mapping, texture indexes doesn't match face indexes!\n");
	}

	if (!modelScribe.AreNormalsValid()) {
		Con::PrintWarning("Obj file contains invalid normals mapping, size doesn't match face count!\n");
	}

	// Vertex Decl Type
	{
		modelScribe.m_VertexDeclType = ModelScriber::eVertexDeclType_UVN;
		if (bSkinned) {
			modelScribe.m_VertexDeclType = ModelScriber::eVertexDeclType_Skinned;
		}
	}

	modelScribe.SetName(&sObjectName[0]);
	modelScribe.m_ModelData.SetNumMeshes(1);

	modelScribe.InitializeMesh();
	modelScribe.CreateBuffers();

	if (!modelScribe.AreBuffersValid())
	{
		Con::PrintError("Failed to create buffer, make sure vertices/faces are correctly exported!\n");
		return 1;
	}

	struct PrintHashTable_t
	{
		const char* m_Name = nullptr;
		std::map<std::string, uint32_t> m_Map;

		PrintHashTable_t() {}
		PrintHashTable_t(const char* p_Name) : m_Name(p_Name) { };

		void Add(const char* p_Name, uint32_t p_Hash) 
		{ 
			m_Map[p_Name] = p_Hash; 
		}
	};
	std::vector<PrintHashTable_t> vecPrintTables;

	// Material
	PrintHashTable_t materialTable("Material");
	{
		modelScribe.m_Material.AddParam(SDK::StringHash32("iAlphaState"), MATERIAL_ALPHASTATE, UINT32_MAX);

		// RasterState
		{
			uint32_t uHash = UINT32_MAX;
			switch (iRasterState)
			{
				case 1: uHash = SDK::StringHash32("Illusion.RasterState.Normal"); break;
				case 2: uHash = SDK::StringHash32("Illusion.RasterState.DisableWrite"); break;
				case 3: uHash = SDK::StringHash32("Illusion.RasterState.InvertDisableWrite"); break;
				case 4: uHash = SDK::StringHash32("Illusion.RasterState.DisableDepthTest"); break;
				case 5: uHash = SDK::StringHash32("Illusion.RasterState.DoubleSided"); break;
				case 6: uHash = SDK::StringHash32("Illusion.RasterState.DoubleSidedAlpha"); break;
				case 7: uHash = SDK::StringHash32("Illusion.RasterState.InvertCulling"); break;
			}
			modelScribe.m_Material.AddParam(SDK::StringHash32("iRasterState"), MATERIAL_RASTERSTATE, uHash);

			if (uHash != UINT32_MAX) {
				materialTable.Add("RasterState", uHash);
			}
		}

		modelScribe.m_Material.AddParam(SDK::StringHash32("iShader"), MATERIAL_SHADER, SDK::StringHash32("HK_SCENERY"));
		modelScribe.m_Material.AddParam(SDK::StringHash32("sbDepthBiasSortLayer"), MATERIAL_STATEBLOCK, MATERIAL_DEFAULT_DepthBiasSortLayer);
		modelScribe.m_Material.AddParam(SDK::StringHash32("sbSpecularLook"), MATERIAL_STATEBLOCK, MATERIAL_DEFAULT_SpecularLook);
		modelScribe.m_Material.AddParam(SDK::StringHash32("sbTextureAnim"), MATERIAL_STATEBLOCK, MATERIAL_DEFAULT_TextureAnim);

		// Diffuse
		{
			uint32_t uHash = SDK::StringHashUpper32(&sTextureDiffuse[0]);
			modelScribe.m_Material.AddParam(SDK::StringHash32("iTexture"), SDK::StringHash32("texDiffuse"), MATERIAL_TEXTURE, uHash);

			materialTable.Add("Tex.Diffuse", uHash);
		}

		// Normal
		if (!sTextureNormal.empty())
		{
			uint32_t uHash = SDK::StringHashUpper32(&sTextureNormal[0]);
			modelScribe.m_Material.AddParam(SDK::StringHash32("iTexture"), SDK::StringHash32("texNormal"), MATERIAL_TEXTURE, uHash);

			materialTable.Add("Tex.Normal", uHash);
		}

		// Specular
		if (!sTextureSpecular.empty())
		{
			uint32_t uHash = SDK::StringHashUpper32(&sTextureSpecular[0]);
			modelScribe.m_Material.AddParam(SDK::StringHash32("iTexture"), SDK::StringHash32("texSpecular"), MATERIAL_TEXTURE, uHash);

			materialTable.Add("Tex.Specular", uHash);
		}

		modelScribe.m_Material.AddParam(SDK::StringHash32("iTexture"), SDK::StringHash32("texNoise"), MATERIAL_TEXTURE, SDK::StringHash32("LITWINDOWNOISE"));
	}
	vecPrintTables.emplace_back(materialTable);

	// Perm Hashes
	PrintHashTable_t permHashesTable("Perm Hashes");
	{
		permHashesTable.Add("Material", modelScribe.m_Material.m_NameUID);
		permHashesTable.Add("IndexBuffer", modelScribe.m_IndexBuffer.m_NameUID);

		for (int i = 0; 4 > i; ++i)
		{
			if (!modelScribe.m_VertexBuffers[i].m_pData) {
				continue;
			}

			const char* szName = nullptr;
			switch (i)
			{
				case 0: szName = "VertexBuffer[0]"; break;
				case 1: szName = "VertexBuffer[1]"; break;
				case 2: szName = "VertexBuffer[2]"; break;
				case 3: szName = "VertexBuffer[3]"; break;
			}

			permHashesTable.Add(szName, modelScribe.m_VertexBuffers[i].m_NameUID);
		}

		permHashesTable.Add("ModelData", modelScribe.m_ModelData.m_NameUID);
	}
	vecPrintTables.emplace_back(permHashesTable);

	for (auto& m_PrintTable : vecPrintTables)
	{
		Con::Print(14, "[ %s ]:\n", m_PrintTable.m_Name);

		for (auto& m_Pair : m_PrintTable.m_Map)
		{
			Con::Print(13, "\t%s: ", &m_Pair.first[0]);
			Con::Print(11, "\t0x%X\n", m_Pair.second);
		}

		printf("\n");
	}

	// Output
	{
		std::string sOutputFileNameNoExt;
		if (sOutputDirectory.empty()) {
			sOutputFileNameNoExt = sObjectFile.substr(0, sObjectFile.find_last_of('.'));
		}
		else {
			sOutputFileNameNoExt = sOutputDirectory + "\\" + sObjectName;
		}

		std::string sPermBinFileName = sOutputFileNameNoExt + ".perm.bin";
		std::string sTempBinFileName = sOutputFileNameNoExt + ".temp.bin";
		{
			const char* pFileMode = (bAppend ? "ab" : "wb");

			modelScribe.OutputFile(&sPermBinFileName[0], pFileMode);

			FILE* pTempFile = fopen(&sTempBinFileName[0], pFileMode);
			if (pTempFile) {
				fclose(pTempFile);
			}
		}

		Con::Print(14, "[ Output ]:\n"); 
		Con::SetColor(13);
		printf("\t%s\n", &sPermBinFileName[0]);
		printf("\t%s\n\n", &sTempBinFileName[0]);
	}
}