// 3rdParty
#include "3rdParty/fast_obj.h"
#include "3rdParty/umHalf.h"

// Headers
#include <iostream>
#include <vector>
#include <unordered_map>
#include <map>
#include <Windows.h>

// Defines
#define PROJECT_VERSION		"v1.2.0"
#define PROJECT_NAME		"Model Scriber " PROJECT_VERSION

// Material Defines
#define MATERIAL_ALPHASTATE			0x12C800F2
#define MATERIAL_RASTERSTATE		0x3BC715E0
#define MATERIAL_SHADER				0x8B5561A1
#define MATERIAL_STATEBLOCK			0x4D04C7F2
#define MATERIAL_TEXTURE			0x8B43FABF

// Material Default Defines
#define MATERIAL_DEFAULT_DepthBiasSortLayer			0xAF2B2668
#define MATERIAL_DEFAULT_SpecularLook				0x7610933B
#define MATERIAL_DEFAULT_TextureAnim				0xD0B4527C

// SDK Stuff
#define UFG_PAD_INSERT(x, y) x ## y
#define UFG_PAD_DEFINE(x, y) UFG_PAD_INSERT(x, y)
#define UFG_PAD(size) char UFG_PAD_DEFINE(padding_, __LINE__)[size] = { 0x0 }
#include <SDK/Optional/PermFile/.Includes.hpp>
#include <SDK/Optional/StringHash.hpp>

// Helper
namespace Helper
{
	void SetUV(uint16_t* p_UVBytes, float* p_UV)
	{
		HalfFloat m_UV0 = p_UV[0];
		HalfFloat m_UV1 = (1.f - p_UV[1]);

		p_UVBytes[0] = m_UV0.bits;
		p_UVBytes[1] = m_UV1.bits;
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

// Classes
class CBuffer : public Illusion::Buffer_t
{
public:
	void* m_DataPtr = nullptr;
	uint32_t m_DataSize = 0;

	CBuffer()
	{
		// ResourceData
		m_TypeUID	= 0x7A971479;
		m_ChunkUID	= 0x92CDEC8F;
	}

	void InitializeSize(uint32_t p_NumElements, uint32_t p_ElementSize)
	{
		m_NumElements	= p_NumElements;
		m_ElementSize	= p_ElementSize;
		m_Size			= (m_NumElements * m_ElementSize);
	}

	void Initialize()
	{
		m_DataSize = GetBytesToReserve();
		m_DataPtr = malloc(m_DataSize);
		memset(m_DataPtr, 0, m_DataSize);

		// ResourceData
		SetEntrySize(sizeof(Illusion::Buffer_t) + m_DataSize);
	}

	void WriteToFile(FILE* p_File)
	{
		fwrite(this, sizeof(Illusion::Buffer_t), 1, p_File);
		fwrite(m_DataPtr, sizeof(uint8_t), m_DataSize, p_File);
	}
};

class CMaterial : public Illusion::Material_t
{
public:
	std::vector<Illusion::MaterialParam_t> m_Params;
	Illusion::MaterialUser_t m_MaterialUser;

	CMaterial()
	{
		// ResourceData
		m_TypeUID	= 0xF5F8516F;
		m_ChunkUID	= 0xB4C26312;
	}

	void SetNumParams(uint32_t p_NumParams)
	{
		uint64_t m_MaterialTableSize = (static_cast<uint64_t>(p_NumParams) * sizeof(Illusion::MaterialParam_t));
		SetEntrySize(static_cast<uint32_t>(sizeof(Material_t) + m_MaterialTableSize + sizeof(Illusion::MaterialUser_t)));

		m_NumParams = p_NumParams;
		m_MaterialUserOffset = (m_MaterialTableSize + 0x8);
	}

	void AddParam(uint32_t p_StateTypeUID, uint32_t p_StateNameUID, uint32_t p_TypeUID, uint32_t p_NameUID)
	{
		Illusion::MaterialParam_t m_Param;
		memset(&m_Param, 0, sizeof(Illusion::MaterialParam_t));
		{
			m_Param.m_StateParam.m_TypeUID = p_StateTypeUID;
			m_Param.m_StateParam.m_NameUID = p_StateNameUID;
			m_Param.m_TypeUID = p_TypeUID;
			m_Param.m_NameUID = p_NameUID;
		}
		m_Params.emplace_back(m_Param);
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

class CModelData : public Illusion::ModelData_t
{
public:
	CModelData()
	{
		// ResourceData
		m_TypeUID	= 0x6DF963B3;
		m_ChunkUID	= 0xA2ADCD77;

		// Handles
		m_MaterialTableHandle.m_NameUID = 0x0;
		m_BonePaletteHandle.m_NameUID	= 0x0;

		// Offsets
		m_MeshOffset		= 0xA8;
		m_ModelUserOffset	= 0x88;
	}

	void SetNumMeshes(uint32_t p_NumMeshes)
	{
		m_NumMeshes = 1;
		SetEntrySize(sizeof(Illusion::ModelData_t) + sizeof(Illusion::ModelUser_t) + (0x110 * static_cast<int64_t>(m_NumMeshes)) + 0x38);
	}

	void CalculateAABB(float* p_VertexBuffer, uint32_t p_NumElements)
	{
		for (uint32_t i = 0; p_NumElements > i; ++i)
		{
			float* m_Vertex = &p_VertexBuffer[i * 3];
			for (int v = 0; 3 > v; ++v)
			{
				if (m_Vertex[v] < m_AABBMin[v])
					m_AABBMin[v] = m_Vertex[v];
				else if (m_Vertex[v] > m_AABBMax[v])
					m_AABBMax[v] = m_Vertex[v];
			}
		}
	}
};

class CModel
{
public:
	fastObjMesh* m_ObjMesh = nullptr;
	uint32_t m_NameUID = 0x0;

	// Perm.Bin
	CMaterial m_Material;
	CBuffer m_IndexBuffer;
	CBuffer m_VertexBuffer;
	CBuffer m_UVBuffer;
	CModelData m_ModelData;
	Illusion::ModelUser_t m_ModelUser;
	Illusion::Mesh_t m_Mesh;

	CModel() 
	{ 
		// ModelUser
		m_ModelUser.m_SoftBodyDataOffset	= (sizeof(Illusion::ModelUser_t) + 0x10 + (0x110 * 0x1)); // 0x110 (Mesh Align Size) * 0x1 (Mesh Count)
		m_ModelUser.m_HasSoftBodyInfo		= 0;
	}

	~CModel()
	{
		if (m_ObjMesh)
			fast_obj_destroy(m_ObjMesh);

		if (m_IndexBuffer.m_DataPtr)
			free(m_IndexBuffer.m_DataPtr);

		if (m_VertexBuffer.m_DataPtr)
			free(m_VertexBuffer.m_DataPtr);

		if (m_UVBuffer.m_DataPtr)
			free(m_UVBuffer.m_DataPtr);
	}

	void SetName(const char* p_Name)
	{
		m_NameUID = SDK::StringHash32(p_Name);

		char m_Buffer[128];

		// Material
		{
			snprintf(m_Buffer, sizeof(m_Buffer), "%s.Material", p_Name);
			m_Material.m_NameUID = SDK::StringHash32(m_Buffer);

			memset(m_Material.m_DebugName, 0, sizeof(Illusion::Buffer_t::m_DebugName));
			snprintf(m_Material.m_DebugName, sizeof(Illusion::Buffer_t::m_DebugName), "0x%X_Material", m_NameUID);
		}

		// IndexBuffer
		{
			snprintf(m_Buffer, sizeof(m_Buffer), "%s.IndexBuffer", p_Name);
			m_IndexBuffer.m_NameUID = SDK::StringHash32(m_Buffer);

			memset(m_IndexBuffer.m_DebugName, 0, sizeof(Illusion::Buffer_t::m_DebugName));
			snprintf(m_IndexBuffer.m_DebugName, sizeof(Illusion::Buffer_t::m_DebugName), "0x%X_IndexBuffer", m_NameUID);
		}

		// VertexBuffer
		{
			snprintf(m_Buffer, sizeof(m_Buffer), "%s.VertexBuffer", p_Name);
			m_VertexBuffer.m_NameUID = SDK::StringHash32(m_Buffer);

			memset(m_VertexBuffer.m_DebugName, 0, sizeof(Illusion::Buffer_t::m_DebugName));
			snprintf(m_VertexBuffer.m_DebugName, sizeof(Illusion::Buffer_t::m_DebugName), "0x%X_VertexBuffer", m_NameUID);
		}

		// VertexBuffer
		{
			snprintf(m_Buffer, sizeof(m_Buffer), "%s.UVBuffer", p_Name);
			m_UVBuffer.m_NameUID = SDK::StringHash32(m_Buffer);

			memset(m_UVBuffer.m_DebugName, 0, sizeof(Illusion::Buffer_t::m_DebugName));
			snprintf(m_UVBuffer.m_DebugName, sizeof(Illusion::Buffer_t::m_DebugName), "0x%X_UVBuffer", m_NameUID);
		}

		// ModelData
		{
			snprintf(m_Buffer, sizeof(m_Buffer), "%s.ModelData", p_Name);
			m_ModelData.m_NameUID = SDK::StringHash32(m_Buffer);

			memset(m_ModelData.m_DebugName, 0, sizeof(Illusion::Buffer_t::m_DebugName));
			snprintf(m_ModelData.m_DebugName, sizeof(Illusion::Buffer_t::m_DebugName), "0x%X_ModelData", m_NameUID);
		}
	}
	
	void LoadMesh(const char* p_FilePath)
	{
		m_ObjMesh = fast_obj_read(p_FilePath);
	}

	bool IsUVMappingValid()
	{
		for (uint32_t i = 0; m_ObjMesh->index_count > i; ++i)
		{
			fastObjIndex* m_Index = &m_ObjMesh->indices[i];
			if (m_Index->p != m_Index->t) // Check if texture index isnt different to face index, otherwise we fucked...
				return false;
		}

		return true;
	}

	bool AreNormalsValid()
	{
		return (m_ObjMesh->normal_count == m_ObjMesh->texcoord_count);
	}

	void CreateBuffers()	
	{
		// IndexBuffer
		{
			m_IndexBuffer.m_Type			= 0x1;
			m_IndexBuffer.m_Size			= (m_ObjMesh->face_count * (sizeof(uint16_t) * 3));
			m_IndexBuffer.m_ElementSize		= sizeof(uint16_t);
			m_IndexBuffer.m_NumElements		= (m_ObjMesh->face_count * sizeof(uint16_t));
			m_IndexBuffer.Initialize();

			uint16_t* m_IndexBufferData = reinterpret_cast<uint16_t*>(m_IndexBuffer.m_DataPtr);

			for (uint32_t i = 0; m_ObjMesh->index_count > i; ++i)
			{
				fastObjIndex* m_Index = &m_ObjMesh->indices[i];
				m_IndexBufferData[i] = (m_Index->p - 1);
			}
		}

		switch (m_Mesh.m_VertexDeclHandle.m_NameUID)
		{
			case 0x276B9567: // Skinned (WIP)
			{
				// VertexBuffer
				{
					struct Skinned_t
					{
						float m_Vertices[4];
						uint8_t m_Normal[4];
						uint8_t m_Tangent[4];
					};

					m_VertexBuffer.m_Type = 0x0;
					m_VertexBuffer.InitializeSize((m_ObjMesh->position_count - 1), sizeof(Skinned_t));
					m_VertexBuffer.Initialize();

					for (uint32_t i = 1; m_VertexBuffer.m_NumElements >= i; ++i)
					{
						Skinned_t* m_Skinned = &reinterpret_cast<Skinned_t*>(m_VertexBuffer.m_DataPtr)[i - 1];
				
						// Vertices
						{
							memcpy(m_Skinned->m_Vertices, &m_ObjMesh->positions[i * 3], (sizeof(float) * 3));
							m_Skinned->m_Vertices[3] = 0.f;
						}

						// Normal
						{
							if (m_ObjMesh->normal_count > i)
								Helper::SetPackedFloat3(m_Skinned->m_Normal, &m_ObjMesh->normals[i * 3]);
							else
								m_Skinned->m_Normal[0] = m_Skinned->m_Normal[1] = m_Skinned->m_Normal[2] = 0x0;
						}
						m_Skinned->m_Normal[3] = 0xFF;

						// Tangent (Is currently unknown, probably need to be calculated somehow...)
						m_Skinned->m_Tangent[0] = m_Skinned->m_Tangent[1] = m_Skinned->m_Tangent[2] = 0x0;
						m_Skinned->m_Tangent[3] = 0xFF;
					}
				}

				// UVBuffer
				{
					struct UVIndex_t
					{
						uint16_t m_UV[2];
					};

					m_UVBuffer.m_Type = 0x0;
					m_UVBuffer.InitializeSize((m_ObjMesh->texcoord_count - 1), sizeof(UVIndex_t));
					m_UVBuffer.Initialize();

					for (uint32_t i = 1; m_ObjMesh->texcoord_count > i; ++i)
					{
						UVIndex_t* m_UVIndex = &reinterpret_cast<UVIndex_t*>(m_UVBuffer.m_DataPtr)[i - 1];
						Helper::SetUV(m_UVIndex->m_UV, &m_ObjMesh->texcoords[i * 2]);
					}
				}
			}
			break;
			case 0xF2700F96: // UVN
			{
				// VertexBuffer
				{
					m_VertexBuffer.m_Type = 0x0;
					m_VertexBuffer.InitializeSize((m_ObjMesh->position_count - 1), (sizeof(float) * 3));
					m_VertexBuffer.Initialize();

					memcpy(m_VertexBuffer.m_DataPtr, &m_ObjMesh->positions[3], m_VertexBuffer.m_DataSize);
				}

				// UVBuffer
				{
					struct UVNIndex_t
					{
						uint16_t m_UV[2];
						uint8_t m_Normal[4];
					};

					m_UVBuffer.m_Type = 0x0;
					m_UVBuffer.InitializeSize((m_ObjMesh->texcoord_count - 1), sizeof(UVNIndex_t));
					m_UVBuffer.Initialize();

					for (uint32_t i = 1; m_ObjMesh->texcoord_count > i; ++i)
					{
						UVNIndex_t* m_UVNIndex = &reinterpret_cast<UVNIndex_t*>(m_UVBuffer.m_DataPtr)[i - 1];
						Helper::SetUV(m_UVNIndex->m_UV, &m_ObjMesh->texcoords[i * 2]);

						if (m_ObjMesh->normal_count > i)
							Helper::SetPackedFloat3(m_UVNIndex->m_Normal, &m_ObjMesh->normals[i * 3]);
						else
							m_UVNIndex->m_Normal[0] = m_UVNIndex->m_Normal[1] = m_UVNIndex->m_Normal[2] = 0x0;

						m_UVNIndex->m_Normal[3] = 0xFF;
					}
				}
			}
			break;
		}
	}
		
	bool AreBuffersValid()
	{
		if (m_IndexBuffer.m_NumElements == 0 || m_VertexBuffer.m_NumElements == 0)
			return false;

		return true;
	}

	void InitializeMesh(uint32_t p_VertexDeclUID)
	{
		// Handles
		m_Mesh.m_VertexDeclHandle.m_NameUID			= p_VertexDeclUID;
		m_Mesh.m_MaterialHandle.m_NameUID			= m_Material.m_NameUID;
		m_Mesh.m_IndexBufferHandle.m_NameUID		= m_IndexBuffer.m_NameUID;
		m_Mesh.m_VertexBufferHandles[0].m_NameUID	= m_VertexBuffer.m_NameUID;
		m_Mesh.m_VertexBufferHandles[1].m_NameUID	= m_UVBuffer.m_NameUID;

		// ...
		m_Mesh.m_PrimType = 0x3; // Triangles
		m_Mesh.m_NumPrims = m_ObjMesh->face_count;
	}

	void OutputFile(const char* p_FilePath)
	{
		FILE* m_File = fopen(p_FilePath, "wb");
		if (!m_File)
			return;

		// Material
		m_Material.WriteToFile(m_File);

		// IndexBuffer
		m_IndexBuffer.WriteToFile(m_File);

		// VertexBuffer
		m_VertexBuffer.WriteToFile(m_File);

		// UVBuffer
		m_UVBuffer.WriteToFile(m_File);

		// ModelData
		{
			m_ModelData.CalculateAABB(reinterpret_cast<float*>(m_VertexBuffer.m_DataPtr), m_VertexBuffer.m_NumElements);
			m_ModelData.m_NumPrims = m_ObjMesh->face_count;
			fwrite(&m_ModelData, sizeof(Illusion::Model_t), 1, m_File);
		}

		// Padding
		{
			uint8_t m_Padding[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
			fwrite(m_Padding, 1, sizeof(m_Padding), m_File);
		}

		// Some Unknown Data
		{
			uint8_t m_Bytes[] = {
				0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
			};
			fwrite(m_Bytes, 1, sizeof(m_Bytes), m_File);
		}

		// MaterialUser
		fwrite(&m_ModelUser, sizeof(Illusion::ModelUser_t), 1, m_File);

		// Mesh Offset
		{
			struct MeshOffset_t
			{
				int64_t m_Value;
				UFG_PAD(0x8);

				MeshOffset_t() { }
				MeshOffset_t(int64_t p_Value) { m_Value = p_Value; }
			};
			MeshOffset_t m_MeshOffset(0x10);
			fwrite(&m_MeshOffset, sizeof(MeshOffset_t), 1, m_File);
		}

		// Mesh
		{
			fwrite(&m_Mesh, sizeof(Illusion::Mesh_t), 1, m_File);

			uint8_t Align[] = { 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
			};
			fwrite(Align, 1, sizeof(Align), m_File);
		}

		fclose(m_File);
	}
};

HANDLE g_ConsoleOutput = 0;
CONSOLE_SCREEN_BUFFER_INFO g_ConsoleBufferInfo;
void InitConsole()
{
	g_ConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(g_ConsoleOutput, &g_ConsoleBufferInfo);
	SetConsoleTitleA(PROJECT_NAME);
}
void SetConColor(WORD p_Attribute) { SetConsoleTextAttribute(g_ConsoleOutput, p_Attribute); }
void ResetConColor() { SetConColor(g_ConsoleBufferInfo.wAttributes); }
void PrintError() { SetConColor(12); printf("[ ERROR ] "); ResetConColor(); }
void PrintWarning() { SetConColor(14); printf("[ WARNING ] "); ResetConColor(); }

int g_Argc = 0;
char** g_Argv = nullptr;
void InitArgParam(int p_Argc, char** p_Argv)
{
	g_Argc = p_Argc;
	g_Argv = p_Argv;
}

std::string GetArgParam(const char* p_Arg)
{
	for (int i = 0; g_Argc > i; ++i)
	{
		if (_stricmp(p_Arg, g_Argv[i]) != 0)
			continue;

		int m_ParamIndex = (i + 1);
		if (m_ParamIndex >= g_Argc)
			break;

		return g_Argv[m_ParamIndex];
	}

	return "";
}

int GetArgParamInt(const char* p_Arg)
{
	std::string m_Param = GetArgParam(p_Arg);
	if (m_Param.empty())
		return 0;

	return atoi(&m_Param[0]);
}

bool IsArgSet(const char* p_Arg)
{
	for (int i = 0; g_Argc > i; ++i)
	{
		if (_stricmp(p_Arg, g_Argv[i]) == 0)
			return true;
	}

	return false;
}

void ShowArgOptions()
{
	std::pair<const char*, const char*> m_Args[] =
	{
		{ "-obj", "Object file to scribe." },
		{ "-dir", "Output directory, if not set it uses object folder." },
		{ "-name", "Object internal name (optional)." },
		{ "-texdiffuse", "Texture diffuse name." },
		{ "-texnormal", "Texture normal map name." },
		{ "-texspecular", "Texture specular map name." },
		{ "-rasterstate", "0 - None\n1 - Normal\n2 - Disable Write\n3 - Invert Disable Write\n4 - Disable Depth Test\n5 - Double Sided\n6 - Double Sided Alpha\n7 - Invert Culling" },
		{ "-skinned", "Use skinned export. (WIP)" },
	};

	SetConColor(14); printf("Launch Options:\n");
	for (auto& m_Pair : m_Args)
	{
		SetConColor(13); printf("\t%s", m_Pair.first);
		SetConColor(11); 
		
		std::string m_Description = m_Pair.second;
		size_t m_DescNewline = m_Description.find('\n');
		while (m_DescNewline != std::string::npos)
		{
			++m_DescNewline;
			m_Description.insert(m_DescNewline, "\t\t\t");
			m_DescNewline = m_Description.find('\n', m_DescNewline);
		}

		printf("\t%s\n", &m_Description[0]);
	}
}

int main(int p_Argc, char** p_Argv)
{
#ifdef _DEBUG
	int m_DebugKey = getchar();
#endif

	InitConsole();
	InitArgParam(p_Argc, p_Argv);
	atexit(ResetConColor);

	SetConColor(240);
	printf(PROJECT_NAME "\n");
	ResetConColor();

	std::string m_ObjectFile		= GetArgParam("-obj");
	std::string m_OutputDirectory	= GetArgParam("-dir");
	std::string m_ObjectName		= GetArgParam("-name");
	std::string m_TextureDiffuse	= GetArgParam("-texdiffuse");
	std::string m_TextureNormal		= GetArgParam("-texnormal");
	std::string m_TextureSpecular	= GetArgParam("-texspecular");
	int m_RasterState				= GetArgParamInt("-rasterstate");

	if (m_ObjectFile.empty())
	{
		PrintError(); printf("No object file set!\n");
		ShowArgOptions();
		return 1;
	}

	if (m_ObjectFile.find(".obj") == std::string::npos)
	{
		PrintError(); printf("Object file must have extension .obj!\n");
		return 1;
	}

	SetConColor(224); printf("Object File: %s\n", &m_ObjectFile[0]); ResetConColor(); printf("\n");

	if (m_ObjectName.empty())
	{
		size_t m_LastDelimer = m_ObjectFile.find_last_of("/\\");
		if (m_LastDelimer == std::string::npos)
			m_LastDelimer = 0;
		else
			++m_LastDelimer;

		m_ObjectName = m_ObjectFile.substr(m_LastDelimer);
		m_ObjectName = m_ObjectName.substr(0, m_ObjectName.find_last_of('.'));
	}

	if (m_TextureDiffuse.empty())
	{
		PrintWarning(); printf("No texture diffuse name specified using default!\n");
		m_TextureDiffuse = "DEFAULT";
	}

	CModel m_Model;
	m_Model.LoadMesh(&m_ObjectFile[0]);

	if (!m_Model.m_ObjMesh)
	{
		PrintError(); printf("Failed to load object file!\n");
		return 1;
	}

	if (1 >= m_Model.m_ObjMesh->texcoord_count)
	{
		PrintError(); printf("Object has no uv mapping!\n");
		return 1;
	}

	if (!m_Model.IsUVMappingValid())
	{
		PrintWarning(); printf("Object file contains invalid uv mapping, texture indexes doesn't match face indexes!\n");
	}

	if (!m_Model.AreNormalsValid())
	{
		PrintWarning(); printf("Object file contains invalid normals mapping, size doesn't match face count!\n");
	}

	m_Model.SetName(&m_ObjectName[0]);
	m_Model.m_ModelData.SetNumMeshes(1);

	uint32_t m_VertexDeclUID = SDK::StringHash32("VertexDecl.UVN");
	if (IsArgSet("-skinned"))
		m_VertexDeclUID = SDK::StringHash32("VertexDecl.Skinned");

	m_Model.InitializeMesh(m_VertexDeclUID);

	m_Model.CreateBuffers();
	if (!m_Model.AreBuffersValid())
	{
		PrintError(); printf("Failed to create buffer, make sure vertices/faces are correctly exported!\n");
		return 1;
	}

	struct PrintHashTable_t
	{
		const char* m_Name = nullptr;
		std::map<std::string, uint32_t> m_Map;

		PrintHashTable_t() {}
		PrintHashTable_t(const char* p_Name) { m_Name = p_Name; };

		void Add(const char* p_Name, uint32_t p_Hash) { m_Map[p_Name] = p_Hash; }
	};
	std::vector<PrintHashTable_t> m_PrintTables;

	// Material
	PrintHashTable_t m_MaterialTable("Material");
	{
		m_Model.m_Material.AddParam(SDK::StringHash32("iAlphaState"), MATERIAL_ALPHASTATE, UINT32_MAX);

		// RasterState
		{
			uint32_t m_Hash = UINT32_MAX;
			switch (m_RasterState)
			{
				case 1: m_Hash = SDK::StringHash32("Illusion.RasterState.Normal"); break;
				case 2: m_Hash = SDK::StringHash32("Illusion.RasterState.DisableWrite"); break;
				case 3: m_Hash = SDK::StringHash32("Illusion.RasterState.InvertDisableWrite"); break;
				case 4: m_Hash = SDK::StringHash32("Illusion.RasterState.DisableDepthTest"); break;
				case 5: m_Hash = SDK::StringHash32("Illusion.RasterState.DoubleSided"); break;
				case 6: m_Hash = SDK::StringHash32("Illusion.RasterState.DoubleSidedAlpha"); break;
				case 7: m_Hash = SDK::StringHash32("Illusion.RasterState.InvertCulling"); break;
			}
			m_Model.m_Material.AddParam(SDK::StringHash32("iRasterState"), MATERIAL_RASTERSTATE, m_Hash);
			if (m_Hash != UINT32_MAX)
				m_MaterialTable.Add("RasterState", m_Hash);
		}

		m_Model.m_Material.AddParam(SDK::StringHash32("iShader"), MATERIAL_SHADER, SDK::StringHash32("HK_SCENERY"));
		m_Model.m_Material.AddParam(SDK::StringHash32("sbDepthBiasSortLayer"), MATERIAL_STATEBLOCK, MATERIAL_DEFAULT_DepthBiasSortLayer);
		m_Model.m_Material.AddParam(SDK::StringHash32("sbSpecularLook"), MATERIAL_STATEBLOCK, MATERIAL_DEFAULT_SpecularLook);
		m_Model.m_Material.AddParam(SDK::StringHash32("sbTextureAnim"), MATERIAL_STATEBLOCK, MATERIAL_DEFAULT_TextureAnim);

		// Diffuse
		{
			uint32_t m_Hash = SDK::StringHashUpper32(&m_TextureDiffuse[0]);
			m_Model.m_Material.AddParam(SDK::StringHash32("iTexture"), SDK::StringHash32("texDiffuse"), MATERIAL_TEXTURE, m_Hash);
			m_MaterialTable.Add("Tex.Diffuse", m_Hash);
		}

		// Normal
		if (!m_TextureNormal.empty())
		{
			uint32_t m_Hash = SDK::StringHashUpper32(&m_TextureNormal[0]);
			m_Model.m_Material.AddParam(SDK::StringHash32("iTexture"), SDK::StringHash32("texNormal"), MATERIAL_TEXTURE, m_Hash);
			m_MaterialTable.Add("Tex.Normal", m_Hash);
		}

		// Specular
		if (!m_TextureSpecular.empty())
		{
			uint32_t m_Hash = SDK::StringHashUpper32(&m_TextureSpecular[0]);
			m_Model.m_Material.AddParam(SDK::StringHash32("iTexture"), SDK::StringHash32("texSpecular"), MATERIAL_TEXTURE, m_Hash);
			m_MaterialTable.Add("Tex.Specular", m_Hash);
		}

		m_Model.m_Material.AddParam(SDK::StringHash32("iTexture"), SDK::StringHash32("texNoise"), MATERIAL_TEXTURE, SDK::StringHash32("LITWINDOWNOISE"));
	}
	m_PrintTables.emplace_back(m_MaterialTable);

	// Perm Hashes
	PrintHashTable_t m_PermHashesTable("Perm Hashes");
	{
		m_PermHashesTable.Add("Material", m_Model.m_Material.m_NameUID);
		m_PermHashesTable.Add("IndexBuffer", m_Model.m_IndexBuffer.m_NameUID);
		m_PermHashesTable.Add("VertexBuffer", m_Model.m_VertexBuffer.m_NameUID);
		m_PermHashesTable.Add("UVBuffer", m_Model.m_UVBuffer.m_NameUID);
		m_PermHashesTable.Add("ModelData", m_Model.m_ModelData.m_NameUID);
	}
	m_PrintTables.emplace_back(m_PermHashesTable);

	for (auto& m_PrintTable : m_PrintTables)
	{
		SetConColor(14); printf("[ %s ]:\n", m_PrintTable.m_Name);

		for (auto& m_Pair : m_PrintTable.m_Map)
		{
			SetConColor(13); printf("\t%s: ", &m_Pair.first[0]);
			SetConColor(11); printf("\t0x%X\n", m_Pair.second);
		}

		printf("\n");
		ResetConColor();
	}

	// Output
	{
		std::string m_OutputFileNameGeneric;
		if (m_OutputDirectory.empty())
			m_OutputFileNameGeneric = m_ObjectFile.substr(0, m_ObjectFile.find_last_of('.'));
		else
			m_OutputFileNameGeneric = m_OutputDirectory + "\\" + m_ObjectName;

		std::string m_PermBinFileName = m_OutputFileNameGeneric + ".perm.bin";
		std::string m_TempBinFileName = m_OutputFileNameGeneric + ".temp.bin";
		{
			m_Model.OutputFile(&m_PermBinFileName[0]);

			FILE* m_TempFile = fopen(&m_TempBinFileName[0], "wb");
			if (m_TempFile)
				fclose(m_TempFile);
		}

		SetConColor(14); printf("[ Output ]:\n"); SetConColor(13);
		printf("\t%s\n", &m_PermBinFileName[0]);
		printf("\t%s\n\n", &m_TempBinFileName[0]);
	}
}