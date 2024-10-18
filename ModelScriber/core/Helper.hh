#pragma once

namespace core
{
	void EraseRunTimeData(UFG::qResourceHandle* handle)
	{
		handle->mPrev = handle->mNext = nullptr;
		handle->mData = nullptr;
	}

	void EraseRunTimeData(UFG::qResourceData* data)
	{
		data->mResourceHandles.mNode.mPrev = data->mResourceHandles.mNode.mNext = nullptr;
	}

	void EraseRunTimeData(Illusion::Buffer* buffer)
	{
		EraseRunTimeData(static_cast<UFG::qResourceData*>(buffer));
		UFG::qMemSet(buffer->GetPlat(), 0, sizeof(Illusion::BufferPlat));
	}

	void EraseRunTimeData(Illusion::Mesh* mesh)
	{
		EraseRunTimeData(&mesh->mMaterialHandle);
		EraseRunTimeData(&mesh->mVertexDeclHandle);
		EraseRunTimeData(&mesh->mIndexBufferHandle);
		EraseRunTimeData(&mesh->mVertexBufferHandles[0]);
		EraseRunTimeData(&mesh->mVertexBufferHandles[1]);
		EraseRunTimeData(&mesh->mVertexBufferHandles[2]);
		EraseRunTimeData(&mesh->mVertexBufferHandles[3]);
	}

	void EraseRunTimeData(Illusion::Model* model)
	{
		EraseRunTimeData(static_cast<UFG::qResourceData*>(model));
		EraseRunTimeData(&model->mMaterialTableHandle);
		EraseRunTimeData(&model->mBonePaletteHandle);
		EraseRunTimeData(&model->mMorphTargetsHandle);
		EraseRunTimeData(&model->mLocatorsHandle);

		for (u32 i = 0; model->mNumMeshes > i; ++i) {
			EraseRunTimeData(model->GetMesh(i));
		}
	}

	s8 GetByteN(f32 value) { return static_cast<s8>((value + 1.f) * 0.5f * 255.f); }
	u8 GetUByteN(f32 value) { return static_cast<u8>(value * 255.f); }
}