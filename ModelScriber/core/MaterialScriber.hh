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

		MaterialScriber(const char* name) : mName(name), mNumParams(0), mMaterial(nullptr) {}

		void AddParam(const char* state_type, const char* state_name, const char* resource_type, const char* resource_name)
		{
			qAssertF(mNumParams < 128, "ERROR: MaterialScriber too many params!");

			auto& param = mParams[mNumParams++];
			{
				param.mTypeUID = UFG::qStringHash32(state_type);
				param.mNameUID = UFG::qStringHash32(state_name);
				param.mResourceHandle.mTypeUID = UFG::qStringHash32(resource_type);
				param.mResourceHandle.mNameUID = (resource_name ? UFG::qStringHash32(resource_name) : 0xFFFFFFFF);
			}
		}

		void AddParam(const char* state_name, const char* resource_type, const char* resource_name)
		{
			AddParam(state_name, state_name, resource_type, resource_name);
		}

		void CreateMaterial()
		{
			mMaterial = Illusion::Factory::NewMaterial(mName, mName.GetStringHashUpper32(), mNumParams);

			/* Copy params */

			for (u32 i = 0; mNumParams > i; ++i) 
			{
				auto param = mMaterial->GetParam(i);
				*param = mParams[i];
				param->mResourceHandle.mPrev = param->mResourceHandle.mNext = nullptr;
			}
		}
	};
}