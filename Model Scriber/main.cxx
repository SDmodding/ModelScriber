#include <iostream>
#include <Windows.h>
#include "3rdParty/fast_obj.h"
#include "3rdParty/umHalf.h"

// Defines
#define PROJECT_VERSION "v1.0.3"

// SDK Stuff
#define UFG_PAD_INSERT(x, y) x ## y
#define UFG_PAD_DEFINE(x, y) UFG_PAD_INSERT(x, y)
#define UFG_PAD(size) char UFG_PAD_DEFINE(padding_, __LINE__)[size] = { 0x0 }
#include <SDK/Optional/StringHash.hpp>

namespace UFG
{
	struct ResourceData_t
	{
		uint32_t m_TypeUID = 0x0;
		uint32_t m_EntrySize[2] = { 0x0, 0x0 };

		UFG_PAD(0x1C);

		uint32_t m_NameUID = 0x0;

		UFG_PAD(0x14);

		uint32_t m_ChunkUID = 0x0;

		char m_DebugName[36];

		ResourceData_t()
		{
			memset(m_DebugName, 0, sizeof(m_DebugName));
		}

		void SetEntrySize(uint32_t p_Size)
		{
			m_EntrySize[0] = m_EntrySize[1] = (p_Size - 0x10);
		}
	};
}

namespace Illusion
{
	struct Buffer_t : UFG::ResourceData_t
	{
		uint32_t m_Type = 0x0;
		uint32_t m_Size = 0x0;
		uint64_t m_DataOffset = 0xD0;
		uint32_t m_ElementSize = 0x0;
		uint32_t m_NumElements = 0x0;

		uint8_t m_BufferPadding[192] = {
			0xA0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x4D, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC6, 0x42,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x30, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		};

		Buffer_t()
		{
			// ResourceData
			m_TypeUID	= 0x7A971479;
			m_ChunkUID	= 0x92CDEC8F;
		}

		uint32_t GetBytesToReserve()
		{
			uint32_t m_Reserve	= m_Size;
			uint32_t m_Align	= (m_Size % 0x10);
			if (m_Align != 0x0)
				m_Reserve += (0x10 - m_Align);

			return m_Reserve;
		}
	};

	struct Mesh_t
	{
		struct Handle_t
		{
			UFG_PAD(0x18);

			uint32_t m_NameUID = 0x0;

			UFG_PAD(0x4);
		};

		Handle_t m_MaterialHandle;
		Handle_t m_VertexDeclHandle;
		Handle_t m_IndexBufferHandle;
		Handle_t m_VertexBufferHandles[4];
		int m_PrimType = 0;
		int m_IndexStart = 0;
		uint32_t m_NumPrims = 0;
		uint32_t m_Pad = 0;
		const char* m_Description = nullptr;

		Mesh_t()
		{
			m_PrimType = 0x3; // Triangles
			m_VertexDeclHandle.m_NameUID = 0xF2700F96; // VertexDecl.UVN
		}
	};

	struct Material_t : UFG::ResourceData_t
	{
		uint8_t m_Data[488] = {
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC8, 0x01, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x34, 0xC9, 0x19, 0x5C, 0x34, 0xC9, 0x19, 0x5C,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2A, 0x1A, 0x14, 0x84,
			0x00, 0x00, 0x00, 0x00, 0xA1, 0x61, 0x55, 0x8B, 0x00, 0x00, 0x00, 0x00,
			0x8F, 0x74, 0x98, 0xEB, 0x8F, 0x74, 0x98, 0xEB, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
			0xF2, 0x00, 0xC8, 0x12, 0x00, 0x00, 0x00, 0x00, 0x04, 0x06, 0x27, 0xEA,
			0x04, 0x06, 0x27, 0xEA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x3B, 0x93, 0x10, 0x76, 0x00, 0x00, 0x00, 0x00, 0xF2, 0xC7, 0x04, 0x4D,
			0x00, 0x00, 0x00, 0x00, 0xF2, 0xD3, 0xF5, 0xB2, 0xF2, 0xD3, 0xF5, 0xB2,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0x52, 0xB4, 0xD0,
			0x00, 0x00, 0x00, 0x00, 0xF2, 0xC7, 0x04, 0x4D, 0x00, 0x00, 0x00, 0x00,
			0x03, 0xD3, 0x73, 0xF1, 0x03, 0xD3, 0x73, 0xF1, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x68, 0x26, 0x2B, 0xAF, 0x00, 0x00, 0x00, 0x00,
			0xF2, 0xC7, 0x04, 0x4D, 0x00, 0x00, 0x00, 0x00, 0x89, 0x66, 0xE0, 0xDC,
			0x53, 0x74, 0x37, 0xC8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xBF, 0xFA, 0x43, 0x8B,
			0x00, 0x00, 0x00, 0x00, 0x1B, 0xD3, 0xE8, 0x63, 0x53, 0x74, 0x37, 0xC8,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD9, 0x08, 0x83, 0x1D,
			0x00, 0x00, 0x00, 0x00, 0xBF, 0xFA, 0x43, 0x8B, 0x00, 0x00, 0x00, 0x00,
			0xE6, 0x65, 0xC2, 0xC0, 0xE6, 0x65, 0xC2, 0xC0, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
			0xE0, 0x15, 0xC7, 0x3B, 0x00, 0x00, 0x00, 0x00
		};

		uint16_t m_VisibilityFlags = 0x1F;
		uint16_t mShadowFlags = 0x0;
		uint8_t m_MaterialPadding[12] = {
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		};

		Material_t()
		{
			// ResourceData
			m_TypeUID	= 0xF5F8516F;
			m_ChunkUID	= 0xB4C26312;
			SetEntrySize(sizeof(Material_t));
		}

		void SetTextureNameUID(uint32_t p_NameUID)
		{
			*reinterpret_cast<uint32_t*>(&m_Data[0x168]) = p_NameUID;
		}
	};

	struct Model_t : UFG::ResourceData_t
	{
		float m_AABBMin[3];
		uint32_t m_NumPrims;
		float m_AABBMax[3];

		uint8_t m_MeshPadding[252] = {
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA8, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x41, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x30, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		};

		Mesh_t Mesh;

		UFG_PAD(0x18);

		Model_t()
		{
			// ResourceData
			m_TypeUID	= 0x6DF963B3;
			m_ChunkUID	= 0xA2ADCD77;
			SetEntrySize(sizeof(Model_t));
		}

		void SetNumPrims(uint32_t p_NumPrims)
		{
			m_NumPrims			= p_NumPrims;
			Mesh.m_NumPrims		= p_NumPrims;
		}

		void CheckVertice(float* p_Vertice)
		{
			for (int i = 0; 3 > i; ++i)
			{
				if (p_Vertice[i] < m_AABBMin[i])
					m_AABBMin[i] = p_Vertice[i];
				else if (p_Vertice[i] > m_AABBMax[i])
					m_AABBMax[i] = p_Vertice[i];
			}
		}
	};
}

class CModel
{
public:
	fastObjMesh* m_Mesh = nullptr;
	uint32_t m_NameUID = 0x0;

	struct Buffer_t : Illusion::Buffer_t
	{
		void* m_DataPtr			= nullptr;
		uint32_t m_DataSize		= 0;

		void Initialize()
		{
			m_DataSize	= GetBytesToReserve();
			m_DataPtr	= malloc(m_DataSize);
			memset(m_DataPtr, 0, m_DataSize);

			// ResourceData
			SetEntrySize(sizeof(Illusion::Buffer_t) + m_DataSize);
		}
	};

	// Perm.Bin
	Illusion::Material_t m_Material;
	Buffer_t m_IndexBuffer;
	Buffer_t m_VertexBuffer;
	Buffer_t m_UVBuffer;
	Illusion::Model_t m_ModelData;

	CModel() 
	{ 
	}

	~CModel()
	{
		if (m_Mesh)
			fast_obj_destroy(m_Mesh);

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
		m_Mesh = fast_obj_read(p_FilePath);
	}

	bool IsUVMappingValid()
	{
		if (m_Mesh->texcoord_count > 1)
		{
			for (uint32_t i = 0; m_Mesh->index_count > i; ++i)
			{
				fastObjIndex* m_Index = &m_Mesh->indices[i];
				if (m_Index->p != m_Index->t) // Check if texture index isnt different to face index, otherwise we fucked...
					return false;
			}
		}

		return true;
	}

	void CreateBuffers()	
	{
		// IndexBuffer
		{
			m_IndexBuffer.m_Type			= 0x1;
			m_IndexBuffer.m_Size			= (m_Mesh->face_count * (sizeof(uint16_t) * 3));
			m_IndexBuffer.m_ElementSize		= sizeof(uint16_t);
			m_IndexBuffer.m_NumElements		= (m_Mesh->face_count * sizeof(uint16_t));
			m_IndexBuffer.Initialize();

			uint16_t* m_IndexBufferData = reinterpret_cast<uint16_t*>(m_IndexBuffer.m_DataPtr);

			for (uint32_t i = 0; m_Mesh->index_count > i; ++i)
			{
				fastObjIndex* m_Index = &m_Mesh->indices[i];
				m_IndexBufferData[i] = (m_Index->p - 1);
			}
		}

		// VertexBuffer
		{
			m_VertexBuffer.m_Type			= 0x0;
			m_VertexBuffer.m_Size			= ((m_Mesh->position_count - 1) * sizeof(float) * 3);
			m_VertexBuffer.m_ElementSize	= (sizeof(float) * 3);
			m_VertexBuffer.m_NumElements	= (m_Mesh->position_count - 1);
			m_VertexBuffer.Initialize();

			memcpy(m_VertexBuffer.m_DataPtr, &m_Mesh->positions[3], m_VertexBuffer.m_DataSize);
		}

		// UVBuffer
		{
			m_UVBuffer.m_Type				= 0x0;
			m_UVBuffer.m_Size				= ((m_Mesh->texcoord_count - 1) * (sizeof(uint16_t) * 4));
			m_UVBuffer.m_ElementSize		= (sizeof(uint16_t) * 4);
			m_UVBuffer.m_NumElements		= (m_Mesh->texcoord_count - 1);
			m_UVBuffer.Initialize();

			struct UVBufferIndex_t
			{
				uint16_t m_UV[2];
				uint8_t m_Unknown[4];
			};
			UVBufferIndex_t* m_UVBufferData = reinterpret_cast<UVBufferIndex_t*>(m_UVBuffer.m_DataPtr);

			for (uint32_t i = 1; m_Mesh->texcoord_count > i; ++i)
			{
				float* m_UV		= &m_Mesh->texcoords[i * 2];
				HalfFloat m_UV0 = m_UV[0];
				HalfFloat m_UV1 = (1.f - m_UV[1]);

				UVBufferIndex_t* m_UVBufferIndex = &m_UVBufferData[i - 1];
				{
					m_UVBufferIndex->m_UV[0] = m_UV0.bits;
					m_UVBufferIndex->m_UV[1] = m_UV1.bits;

					uint8_t m_Padding[] = { 0x7F, 0xFF, 0x7F, 0xFF };
					memcpy(m_UVBufferIndex->m_Unknown, m_Padding, sizeof(m_Padding));
				}
			}
		}
	}

	bool AreBuffersValid()
	{
		if (m_IndexBuffer.m_NumElements == 0 || m_VertexBuffer.m_NumElements == 0)
			return false;

		return true;
	}

	void OutputFile(const char* p_FilePath)
	{
		FILE* m_File = fopen(p_FilePath, "wb");
		if (!m_File)
			return;

		// Material
		fwrite(&m_Material, sizeof(Illusion::Material_t), 1, m_File);

		// IndexBuffer
		fwrite(&m_IndexBuffer, sizeof(Illusion::Buffer_t), 1, m_File);
		fwrite(m_IndexBuffer.m_DataPtr, sizeof(uint8_t), m_IndexBuffer.m_DataSize, m_File);

		// VertexBuffer
		fwrite(&m_VertexBuffer, sizeof(Illusion::Buffer_t), 1, m_File);
		fwrite(m_VertexBuffer.m_DataPtr, sizeof(uint8_t), m_VertexBuffer.m_DataSize, m_File);

		// UVBuffer
		fwrite(&m_UVBuffer, sizeof(Illusion::Buffer_t), 1, m_File);
		fwrite(m_UVBuffer.m_DataPtr, sizeof(uint8_t), m_UVBuffer.m_DataSize, m_File);

		// ModelData
		{
			// AABB
			{
				float* m_VertexBufferData = reinterpret_cast<float*>(m_VertexBuffer.m_DataPtr);
				for (uint32_t i = 0; m_VertexBuffer.m_NumElements > i; ++i)
					m_ModelData.CheckVertice(&m_VertexBufferData[i * 3]);
			}

			m_ModelData.Mesh.m_MaterialHandle.m_NameUID			= m_Material.m_NameUID;
			m_ModelData.Mesh.m_IndexBufferHandle.m_NameUID		= m_IndexBuffer.m_NameUID;
			m_ModelData.Mesh.m_VertexBufferHandles[0].m_NameUID = m_VertexBuffer.m_NameUID;
			m_ModelData.Mesh.m_VertexBufferHandles[1].m_NameUID = m_UVBuffer.m_NameUID;
			m_ModelData.SetNumPrims(m_Mesh->face_count);

			fwrite(&m_ModelData, sizeof(Illusion::Model_t), 1, m_File);
		}

		fclose(m_File);
	}
};

int g_Argc		= 0;
char** g_Argv	= nullptr;

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

bool HasArgSet(const char* p_Arg)
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
		{ "-o", "Object file to scribe." },
		{ "-d", "Output directory, if not set it uses object folder." },
		{ "-n", "Object internal name (optional)." },
		{ "-t", "Texture name for model." },
		{ "-ignoreuv", "Ignores uvmapping check." }
	};

	printf("Options:\n");
	for (auto& m_Pair : m_Args)
		printf("\t%s\t%s\n", m_Pair.first, m_Pair.second);
}

int main(int p_Argc, char** p_Argv)
{
#ifdef _DEBUG
	int m_DebugKey = getchar();
#endif

	SetConsoleTitleA(("Model Scribe " PROJECT_VERSION));
	InitArgParam(p_Argc, p_Argv);

	std::string m_ObjectFile		= GetArgParam("-o");
	std::string m_OutputDirectory	= GetArgParam("-d");
	std::string m_ObjectName		= GetArgParam("-n");
	std::string m_TextureName		= GetArgParam("-t");

	if (m_ObjectFile.empty())
	{
		printf("No object file set!\n");
		ShowArgOptions();
		return 1;
	}

	if (m_ObjectFile.find(".obj") == std::string::npos)
	{
		printf("Object file must have extension .obj!\n");
		return 1;
	}

	printf("Object File: %s\n", &m_ObjectFile[0]);

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

	if (m_TextureName.empty())
	{
		printf("No texture name set!\n");
		ShowArgOptions();
		return 1;
	}

	std::transform(m_TextureName.begin(), m_TextureName.end(), m_TextureName.begin(), ::toupper);
	uint32_t m_TextureHash = SDK::StringHash32(&m_TextureName[0]);
	printf("Texture Name: %s (0x%X)\n", &m_TextureName[0], m_TextureHash);

	CModel m_Model;
	m_Model.LoadMesh(&m_ObjectFile[0]);

	if (!m_Model.m_Mesh)
	{
		printf("Failed to load object file!\n");
		return 1;
	}

	if (!m_Model.IsUVMappingValid())
	{
		if (HasArgSet("-ignoreuv"))
			printf("Object file contains invalid uv mapping, but '-ignoreuv' was specified!\n");
		else
		{
			printf("Object file contains invalid uv mapping, texture indexes doesn't match face indexes!\n");
			return 1;
		}
	}

	m_Model.CreateBuffers();

	if (!m_Model.AreBuffersValid())
	{
		printf("Failed to create buffer, make sure vertices/faces are correctly exported!\n");
		return 1;
	}

	m_Model.SetName(&m_ObjectName[0]);

	m_Model.m_Material.SetTextureNameUID(m_TextureHash);

	printf("Hashes:\n");
	{
		printf("\tMaterial:\t0x%X\n", m_Model.m_Material.m_NameUID);
		printf("\tIndexBuffer:\t0x%X\n", m_Model.m_IndexBuffer.m_NameUID);
		printf("\tVertexBuffer:\t0x%X\n", m_Model.m_VertexBuffer.m_NameUID);
		printf("\tUVBuffer:\t0x%X\n", m_Model.m_UVBuffer.m_NameUID);
		printf("\tModelData:\t0x%X\n", m_Model.m_ModelData.m_NameUID);
	}

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

	printf("Output files:\n\t%s\n\t%s\n", &m_PermBinFileName[0], &m_TempBinFileName[0]);
}