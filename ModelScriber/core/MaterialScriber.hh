#pragma once

namespace core
{
	class MaterialScriber
	{
	public:
		UFG::qString mName;

		Illusion::MaterialParam mParams[128];
		u32 mNumParams;

		Illusion::Material* mMaterial;
		u32 mByteSize;

		MaterialScriber(const char* name) : mName(name), mNumParams(0), mMaterial(nullptr), mByteSize(0) {}

		~MaterialScriber()
		{
			if (mMaterial) {
				qFree(mMaterial);
			}
		}

		void AddParam(u32 state_type, u32 state_name, u32 resource_type, u32 resource_name)
		{
			qAssertF(mNumParams < 128, "ERROR: MaterialScriber too many params!");

			/* Check if param already exist. */
			for (u32 i = 0; mNumParams > i; ++i)
			{
				auto& param = mParams[i];
				if (param.mTypeUID == state_type && param.mNameUID == state_name) {
					return;
				}
			}

			/* Add param. */

			auto& param = mParams[mNumParams++];
			{
				param.mTypeUID = state_type;
				param.mNameUID = state_name;
				param.mResourceHandle.mTypeUID = resource_type;
				param.mResourceHandle.mNameUID = resource_name;
			}
		}

		void AddParam(const char* state_type, const char* state_name, const char* resource_type, u32 resource_name)
		{
			AddParam(UFG::qStringHash32(state_type), UFG::qStringHash32(state_name), UFG::qStringHash32(resource_type), resource_name);
		}

		void AddParam(const char* state_type, const char* state_name, const char* resource_type, const char* resource_name)
		{
			AddParam(UFG::qStringHash32(state_type), UFG::qStringHash32(state_name), UFG::qStringHash32(resource_type), UFG::qStringHash32(resource_name));
		}

		void AddParam(const char* state_name, const char* resource_type, u32 resource_name = 0xFFFFFFFF)
		{
			AddParam(state_name, state_name, resource_type, resource_name);
		}

		void AddParam(const char* state_name, const char* resource_type, const char* resource_name)
		{
			AddParam(state_name, state_name, resource_type, resource_name);
		}

		void CreateMaterial()
		{
			auto schema = Illusion::GetSchema();
			mMaterial = Illusion::Factory::NewMaterial(mName, mName.GetStringHashUpper32(), mNumParams, schema);
			mByteSize = static_cast<u32>(schema->mCurrSize);

			/* Copy params */

			for (u32 i = 0; mNumParams > i; ++i) 
			{
				auto param = mMaterial->GetParam(i);
				*param = mParams[i];
				param->mResourceHandle.mPrev = param->mResourceHandle.mNext = nullptr;
			}
		}

		void WriteChunk(UFG::qChunkFileBuilder* chunk_builder)
		{
			chunk_builder->BeginChunk(ChunkUID_Material, mName);
			chunk_builder->Write(mMaterial, mByteSize);
			chunk_builder->EndChunk(ChunkUID_Material);
		}
	};
}