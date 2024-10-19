#pragma once

namespace core
{
	class ModelScriber
	{
	public:
		UFG::qString mName;
		Illusion::VertexStreamDescriptor* mStreamDescriptor;

		Illusion::BonePalette* mBonePalette;
		u32 mBonePaletteSize;

		Illusion::Buffer* mIndexBuffer;
		Illusion::Buffer* mVertexBuffers[4]; /* VertexStreamDescriptor can only hold 4 streams. */
		Illusion::Model* mModel;


		ModelScriber(const char* name, const char* vertex_decl)
		{
			mName = name;

			/* Find streamDescriptor */

			auto streamDescriptors = Illusion::VertexStreamDescriptor::GetStreamDescriptors();
			for (auto streamDescriptor = streamDescriptors->begin(); streamDescriptor != streamDescriptors->end(); streamDescriptor = streamDescriptor->next())
			{
				if (UFG::qStringFindInsensitive(streamDescriptor->mName, vertex_decl))
				{
					mStreamDescriptor = streamDescriptor;
					break;
				}
			}
			qAssertF(mStreamDescriptor, "ERROR: Failed to find VertexStreamDescriptor.");

			/* Setup bonePalette */

			mBonePalette = nullptr;
			mBonePaletteSize = 0;

			/* Setup buffers*/

			mIndexBuffer = nullptr;
			mVertexBuffers[0] = nullptr;
			mVertexBuffers[1] = nullptr;
			mVertexBuffers[2] = nullptr;
			mVertexBuffers[3] = nullptr;

			/* Setup model */

			mModel = nullptr;
		}

		~ModelScriber()
		{
			if (mIndexBuffer) {
				UFG::qFree(mIndexBuffer);
			}

			for (auto vertexBuffer : mVertexBuffers)
			{
				if (vertexBuffer) {
					UFG::qFree(vertexBuffer);
				}
			}

			UFG::qFree(mModel);
		}

		//-------------------------------------------------------------
		// Name Helpers
		//-------------------------------------------------------------

		UFG::qString GetMaterialName(const char* name, u32 index)
		{
			return { "%s_MAT|%u|%s", mName.mData, index, name, mName.mData };
			//return { "%s_MAT|%u|%s_MAT.%s.ufg", mName.mData, index, name, mName.mData };
		}

		UFG::qString GetBonePaletteName()
		{
			return { "%s.BonePalette", mName.mData };
		}

		UFG::qString GetIndexBufferName()
		{
			return { "%s.IndexBuffer", mName.mData };
		}

		UFG::qString GetVertexBufferName(u32 stream_num, u32 index = 0)
		{
			return { "%s.VertexBuffer.%s.%u.%u", mName.mData, mStreamDescriptor->mName, stream_num, index };
		}

		//-------------------------------------------------------------
		// Helpers
		//-------------------------------------------------------------

		void EraseRunTimeDatas()
		{
			EraseRunTimeData(mIndexBuffer);

			for (auto vertexBuffer : mVertexBuffers)
			{
				if (vertexBuffer) {
					EraseRunTimeData(vertexBuffer);
				}
			}

			EraseRunTimeData(mModel);
		}

		Illusion::VertexStreamElement* GetStreamElement(Illusion::VertexStreamElementUsage usage)
		{
			for (int i = 0; mStreamDescriptor->GetTotalElements() > i; ++i)
			{
				auto stream_element = mStreamDescriptor->GetElement(i);
				if (stream_element && stream_element->mUsage == usage) {
					return stream_element;
				}
			}

			return nullptr;
		}

		/**
		* Get the pointer to vertex buffer data.
		* \param stream_element - VertexStreamElement pointer.
		* \param vertex_index
		* \return Pointer to data if success.
		*/
		void* GetVertexBufferData(Illusion::VertexStreamElement* stream_element, int vertex_index)
		{
			auto stream_num = stream_element->mStream;
			if (auto vertexBuffer = mVertexBuffers[stream_num]) {
				return vertexBuffer->mData.Get(stream_element->mOffset + (mStreamDescriptor->GetStreamSize(stream_num) * vertex_index));
			}

			return nullptr;
		}

		void RecalculateAABB(const UFG::qVector4& position)
		{
			if (mModel->mAABBMin[0] > position.x) {
				mModel->mAABBMin[0] = position.x;
			}
			if (mModel->mAABBMax[0] < position.x) {
				mModel->mAABBMax[0] = position.x;
			}

			if (mModel->mAABBMin[1] > position.y) {
				mModel->mAABBMin[1] = position.y;
			}
			if (mModel->mAABBMax[1] < position.y) {
				mModel->mAABBMax[1] = position.y;
			}

			if (mModel->mAABBMin[2] > position.z) {
				mModel->mAABBMin[2] = position.z;
			}
			if (mModel->mAABBMax[2] < position.z) {
				mModel->mAABBMax[2] = position.z;
			}
		}

		void SetMeshPrims(u32 mesh_index, int index_start, u32 num_prims)
		{
			qAssertF(mesh_index < mModel->mNumMeshes, "ERROR: Invalid mesh index while setting prims.");

			auto mesh = mModel->GetMesh(mesh_index);
			mesh->mIndexStart = index_start;
			mesh->mNumPrims = num_prims;

			/* Recalculate total prims */

			mModel->mNumPrims = 0;

			for (u32 i = 0; mModel->mNumMeshes > i; ++i) {
				mModel->mNumPrims += mModel->GetMesh(mesh_index)->mNumPrims;
			}
		}

		void SetMeshMaterial(u32 mesh_index, Illusion::Material* material)
		{
			qAssertF(mesh_index < mModel->mNumMeshes, "ERROR: Invalid mesh index while setting material.");

			mModel->GetMesh(mesh_index)->mMaterialHandle.mNameUID = material->mNode.mUID;
		}

		void WriteChunks(UFG::qChunkFileBuilder* chunk_builder)
		{
			auto memory_pool = UFG::GetMainMemoryPool();

			if (mBonePalette)
			{
				chunk_builder->BeginChunk(ChunkUID_BonePalette, GetBonePaletteName());
				chunk_builder->Write(mBonePalette, mBonePaletteSize);
				chunk_builder->Align(16);
				chunk_builder->EndChunk(ChunkUID_BonePalette);
			}

			chunk_builder->BeginChunk(ChunkUID_Buffer, GetIndexBufferName());
			chunk_builder->Write(mIndexBuffer, static_cast<u32>(memory_pool->Size(mIndexBuffer)));
			chunk_builder->EndChunk(ChunkUID_Buffer);

			for (auto vertex_buffer : mVertexBuffers)
			{
				if (!vertex_buffer) {
					continue;
				}

				chunk_builder->BeginChunk(ChunkUID_Buffer, GetIndexBufferName());
				chunk_builder->Write(vertex_buffer, static_cast<u32>(memory_pool->Size(vertex_buffer)));
				chunk_builder->EndChunk(ChunkUID_Buffer);
			}

			chunk_builder->BeginChunk(ChunkUID_Model, mName);
			chunk_builder->Write(mModel, static_cast<u32>(memory_pool->Size(mModel)));
			chunk_builder->EndChunk(ChunkUID_Model);
		}

		//-------------------------------------------------------------
		// Bone Helpers
		//-------------------------------------------------------------
		
		/* TODO: Change this when we have internal qChunk "Loader". */
		void SetBonePalette(UFG::qChunk* bone_palette_chunk)
		{
			if (!bone_palette_chunk || bone_palette_chunk->mUID != ChunkUID_BonePalette) {
				return;
			}

			mBonePalette = static_cast<Illusion::BonePalette*>(bone_palette_chunk->GetData());
			mBonePaletteSize = bone_palette_chunk->mDataSize;

			auto name = GetBonePaletteName();
			mBonePalette->mNode.SetUID(name.GetStringHash32());
			mBonePalette->SetDebugName(name);
		}

		u8 GetBoneIndex(const char* bone_name)
		{
			if (mBonePalette)
			{
				u32 boneUID = UFG::qStringHashUpper32(bone_name);
				u32* boneUIDTable = mBonePalette->mBoneUIDTable.Get();

				for (u32 i = 0; mBonePalette->mNumBones > i; ++i)
				{
					if (boneUIDTable[i] == boneUID) {
						return i;
					}
				}
			}

			return 0;
		}

		//-------------------------------------------------------------
		// Create Functions
		//-------------------------------------------------------------

		/**
		* Creates index buffer.
		* \param num_vertexes - Number of indices.
		* \param num_cp - Number of control points.
		*/
		void CreateIndexBuffer(u32 num_vertexes, u32 num_cp)
		{
			qAssertF(num_vertexes > 0, "ERROR: Invalid number of vertexes.");
			qAssertF(num_cp > 0, "ERROR: Invalid number of control points.");

			auto name = GetIndexBufferName();
			u32 byte_size = (num_cp >= UINT16_MAX ? sizeof(u32) : sizeof(u16));

			mIndexBuffer = Illusion::Factory::NewBuffer(name, name.GetStringHash32(), (byte_size * num_vertexes), 0, name);
			mIndexBuffer->mBufferType		= Illusion::Buffer::TYPE_INDEX;
			mIndexBuffer->mRunTimeCreated	= false;
			mIndexBuffer->mElementByteSize	= byte_size;
			mIndexBuffer->mNumElements		= num_vertexes;
			mIndexBuffer->mMemoryPool		= 0;
		}

		/**
		* Creates vertex buffers.
		* \param num_cp - Number of control points (vertices, uv, normals, ...)
		*/
		void CreateVertexBuffers(u32 num_cp)
		{
			qAssertF(num_cp > 0, "ERROR: Invalid number of control points.");
			
			for (u32 stream_num = 0; 4 > stream_num; ++stream_num)
			{
				/* Skip creating vertex buffer which stream size is 0. */
				u32 stream_size = static_cast<u32>(mStreamDescriptor->GetStreamSize(stream_num));
				if (stream_size <= 0) {
					continue;
				}

				auto name = GetVertexBufferName(stream_num);

				auto vertexBuffer = Illusion::Factory::NewBuffer(name, name.GetStringHash32(), (stream_size * num_cp), 0, name);
				vertexBuffer->mBufferType = Illusion::Buffer::TYPE_VERTEX;
				vertexBuffer->mRunTimeCreated = false;
				vertexBuffer->mElementByteSize = stream_size;
				vertexBuffer->mNumElements = num_cp;
				vertexBuffer->mMemoryPool = 0;

				mVertexBuffers[stream_num] = vertexBuffer;
			}
		}

		void CreateModel(u32 num_meshes)
		{
			mModel = Illusion::Factory::NewModel(mName, mName.GetStringHashUpper32(), num_meshes);

			if (mBonePalette) {
				mModel->mBonePaletteHandle.mNameUID = mBonePalette->mNode.mUID;
			}

			/* Setup meshes */

			for (u32 i = 0; num_meshes > i; ++i)
			{
				auto mesh = mModel->GetMesh(i);
				mesh->mMaterialHandle.mNameUID = GetMaterialName(mName, i).GetStringHashUpper32();
				mesh->mVertexDeclHandle.mNameUID = mStreamDescriptor->mNameUID;
				mesh->mIndexBufferHandle.mNameUID = mIndexBuffer->mNode.mUID;

				for (u32 stream_num = 0; 4 > stream_num; ++stream_num)
				{
					auto vertexBuffer = mVertexBuffers[stream_num];
					mesh->mVertexBufferHandles[stream_num].mNameUID = (vertexBuffer ? vertexBuffer->mNode.mUID : 0);
				}

				mesh->mPrimType = 3; /* Triangles */
			}
		}

		//-------------------------------------------------------------
		// Write Functions
		//-------------------------------------------------------------

		void WriteIndex(int poly_index, int position, int index)
		{
			qAssert(mIndexBuffer);

			void* data = mIndexBuffer->mData.Get(poly_index * 3 * mIndexBuffer->mElementByteSize);

			switch (mIndexBuffer->mElementByteSize)
			{
			default: qAssertF(false, "ERROR: Unknown IndexBuffer size!"); break;
			case sizeof(u16): reinterpret_cast<u16*>(data)[position] = index; break;
			case sizeof(u32): reinterpret_cast<u32*>(data)[position] = index; break;
			}
		}

		void WritePosition(int index, const UFG::qVector4& position)
		{
			RecalculateAABB(position);

			auto stream_element = GetStreamElement(Illusion::VERTEX_ELEMENT_POSITION);
			if (!stream_element) {
				return;
			}

			void* data = GetVertexBufferData(stream_element, index);
			if (!data) {
				return;
			}

			switch (stream_element->mType)
			{
			default: qAssertF(false, "ERROR: Unknown Vertex Position type!"); break;
			case Illusion::VERTEX_TYPE_FLOAT3:
			{
				UFG::qVector3 f3 = { position.x, position.y, position.z};
				UFG::qMemCopy(data, &f3, sizeof(f3));
			}
			break;
			case Illusion::VERTEX_TYPE_FLOAT4:
				UFG::qMemCopy(data, &position, sizeof(position)); break;
			}
		}

		void WriteNormal(int index, const UFG::qVector4& normal)
		{
			auto stream_element = GetStreamElement(Illusion::VERTEX_ELEMENT_NORMAL);
			if (!stream_element) {
				return;
			}

			void* data = GetVertexBufferData(stream_element, index);
			if (!data) {
				return;
			}

			switch (stream_element->mType)
			{
			default: qAssertF(false, "ERROR: Unknown Vertex Normal type!"); break;
			case Illusion::VERTEX_TYPE_BYTE4N:
			{
				s8 normal4N[4] = { GetByteN(normal.x), GetByteN(normal.y), GetByteN(normal.z), GetByteN(normal.w) };
				UFG::qMemCopy(data, normal4N, sizeof(normal4N));
			}
			break;
			}
		}

		void WriteTangent(int index, const UFG::qVector4& tangent)
		{
			auto stream_element = GetStreamElement(Illusion::VERTEX_ELEMENT_TANGENT);
			if (!stream_element) {
				return;
			}

			void* data = GetVertexBufferData(stream_element, index);
			if (!data) {
				return;
			}

			switch (stream_element->mType)
			{
			default: qAssertF(false, "ERROR: Unknown Vertex Tangent type!"); break;
			case Illusion::VERTEX_TYPE_BYTE4N:
			{
				s8 tangent4N[4] = { GetByteN(tangent.x), GetByteN(tangent.y), GetByteN(tangent.z), GetByteN(tangent.w) };
				UFG::qMemCopy(data, tangent4N, sizeof(tangent4N));
			}
			break;
			}
		}

		void WriteTexCoord(int index, const UFG::qVector2& tex_coord, u8 coord_index = 0)
		{
			qAssertF(coord_index <= 7, "ERROR: Coord index is beyond limit");

			auto stream_element = GetStreamElement(static_cast<Illusion::VertexStreamElementUsage>(Illusion::VERTEX_ELEMENT_TEXCOORD0 + coord_index));
			if (!stream_element) {
				return;
			}

			void* data = GetVertexBufferData(stream_element, index);
			if (!data) {
				return;
			}

			float coordX = tex_coord.x;
			float coordY = tex_coord.y;

			/* Normalize Tex coords */

			while (coordX > 1.f) {
				coordX -= 1.f;
			}

			while (coordX < 0.f) {
				coordX += 1.f;
			}

			while (coordY > 1.f) {
				coordY -= 1.f;
			}

			while (coordY < 0.f) {
				coordY += 1.f;
			}

			/* Flip Y */

			coordY = (1.f - coordY);

			/* Write */

			switch (stream_element->mType)
			{
			default: qAssertF(false, "ERROR: Unknown Vertex Texcoord type!"); break;
			case Illusion::VERTEX_TYPE_HALF2:
			{
				UFG::qHalfFloat half_coords[2] = { coordX, coordY };
				UFG::qMemCopy(data, half_coords, sizeof(half_coords));
			}
			break;
			}
		}

		void WriteBlendIndex(int index, u8* blend_indexes)
		{
			auto stream_element = GetStreamElement(Illusion::VERTEX_ELEMENT_BLENDINDEX);
			if (!stream_element) {
				return;
			}

			void* data = GetVertexBufferData(stream_element, index);
			if (!data) {
				return;
			}

			switch (stream_element->mType)
			{
			default: qAssertF(false, "ERROR: Unknown Vertex Blend Index type!"); break;
			case Illusion::VERTEX_TYPE_UBYTE4:
				UFG::qMemCopy(data, blend_indexes, 4); break;
			}
		}

		void WriteBlendWeight(int index, float* blend_weights)
		{
			auto stream_element = GetStreamElement(Illusion::VERTEX_ELEMENT_BLENDWEIGHT);
			if (!stream_element) {
				return;
			}

			void* data = GetVertexBufferData(stream_element, index);
			if (!data) {
				return;
			}

			switch (stream_element->mType)
			{
			default: qAssertF(false, "ERROR: Unknown Vertex Blend Weight type!"); break;
			case Illusion::VERTEX_TYPE_UBYTE4N:
			{
				u8* blendWeights = reinterpret_cast<u8*>(data);
				blendWeights[0] = GetUByteN(blend_weights[0]);
				blendWeights[1] = GetUByteN(blend_weights[1]);
				blendWeights[2] = GetUByteN(blend_weights[2]);
				blendWeights[3] = GetUByteN(blend_weights[3]);
				UFG::qMemCopy(data, blendWeights, 4);
			}
			break;
			}
		}
	};
}