//===================================================================================
// Includes

/* Theory Engine */
#define THEORY_IMPL
#include "theory/theory.hh"

/* FBX SDK */
#include <fbxsdk.h>
#ifdef _DEBUG
    #pragma comment(lib, "libfbxsdk-md.lib")
    #pragma comment(lib, "libxml2-md.lib")
    #pragma comment(lib, "zlib-md.lib")
#else
    #pragma comment(lib, "libfbxsdk-mt.lib")
    #pragma comment(lib, "libxml2-mt.lib")
    #pragma comment(lib, "zlib-mt.lib")
#endif

using namespace fbxsdk;

/* Core */
#include "core/Helper.hh"
#include "core/FBX.hh"
#include "core/MaterialScriber.hh"
#include "core/ModelScriber.hh"
#include "core/VertexStreamDescriptors.hh"

namespace core
{
    ModelScriber* gModelScriber;

    void CreateModel(FbxScene* scene)
    {
        int numMeshes = core::GetNumMeshes(scene);
        qAssertF(numMeshes > 0, "ERROR: FbxScene has no meshes!");

        int num_vertexes = 0;
        int num_cp = 0;

        for (int i = 0; numMeshes > i; ++i)
        {
            auto mesh = core::GetMesh(scene, i);
            qAssert(mesh);
            
            num_vertexes += mesh->GetPolygonVertexCount();
            num_cp += mesh->GetControlPointsCount();
        }

        gModelScriber->CreateIndexBuffer(num_vertexes, num_cp);
        gModelScriber->CreateVertexBuffers(num_cp);
        gModelScriber->CreateModel(numMeshes);
    }

    void ScribeMaterials(UFG::qChunkFileBuilder* chunk_builder, FbxScene* scene)
    {
        u32 numMeshes = gModelScriber->mModel->mNumMeshes;

        for (u32 i = 0; numMeshes > i; ++i)
        {
            auto mesh = GetMesh(scene, i);
            qAssert(mesh);

            core::MaterialScriber mat(gModelScriber->GetMaterialName(mesh->GetName(), i));

            auto material = GetMeshMaterial(mesh);
            if (material)
            {
                struct FbxMaterialMap { const char* mIllusion, *mFbx; };
                FbxMaterialMap fbxMaterialMap[] = {
                    { "texEmissive",    FbxSurfaceMaterial::sEmissive },
                    { "texAmbient",     FbxSurfaceMaterial::sAmbient },
                    { "texDiffuse",     FbxSurfaceMaterial::sDiffuse },
                    { "texSpecular",    FbxSurfaceMaterial::sSpecular },
                    { "texBump",        FbxSurfaceMaterial::sBump },
                    { "texNormal",      FbxSurfaceMaterial::sNormalMap },
                    { "texReflection",  FbxSurfaceMaterial::sReflection }
                };

                for (auto materialMap : fbxMaterialMap)
                {
                    if (auto fileTexture = GetMaterialObject<FbxFileTexture>(material, materialMap.mFbx))
                    {
                        UFG::qString fileName = fileTexture->GetFileName();
                        if (!fileName.IsEmpty()) {
                            mat.AddParam("iTexture", materialMap.mIllusion, "Illusion.Texture", fileName.GetFilenameWithoutExtension().GetStringHashUpper32());
                        }
                    }
                }
            }

            /* Add default params */
            {
                mat.AddParam("iShader", "Illusion.Shader", "HK_SCENERY");
                mat.AddParam("iTexture", "texDiffuse", "Illusion.Texture", "DEFAULT");
                mat.AddParam("iRasterState", "Illusion.RasterState", "Illusion.RasterState.Normal");
                mat.AddParam("iAlphaState", "Illusion.AlphaState");

                /* StateBlocks */
                /* NOTE: StateBlock doesn't have nameuid as their actual name, but their names originally are like: "d513fc25-10a4-4480-84d0-13aeef7d27bd". */
                mat.AddParam("sbDepthBiasSortLayer", "Illusion.StateBlock", 0xAF2B2668); /* StateBlock.Default_DepthBiasSortLayer */
                mat.AddParam("sbSpecularLook", "Illusion.StateBlock", 0x241AB391); /* StateBlock.Default_SpecularLook */
                mat.AddParam("sbTextureAnim", "Illusion.StateBlock", 0xD0B4527C); /* StateBlock.Default_TextureAnim */
            }

            mat.CreateMaterial();
            mat.WriteChunk(chunk_builder);

            gModelScriber->SetMeshMaterial(i, mat.mMaterial);
        }
    }

    void ScribeModel(UFG::qChunkFileBuilder* chunk_builder, FbxScene* scene)
    {
        int indexStart = 0;
        int vertexIndexOffset = 0;

        u32 numMeshes = gModelScriber->mModel->mNumMeshes;
        for (u32 i = 0; numMeshes > i; ++i)
        {
            auto mesh = GetMesh(scene, i);
            qAssert(mesh);

            auto cpBoneWeights = core::GetCPBoneWeights(mesh);

            fbxsdk::FbxStringList uvSetNames;
            mesh->GetUVSetNames(uvSetNames);

            auto uvSetName = uvSetNames[0];

            int num_polygon = mesh->GetPolygonCount();
            for (int i = 0; num_polygon > i; ++i)
            {
                for (int j = 0; 3 > j; ++j)
                {
                    /* Fetch data from FBX */

                    int vertexIndex = mesh->GetPolygonVertex(i, j);
                    auto pos = mesh->GetControlPointAt(vertexIndex);

                    fbxsdk::FbxVector2 uv; bool uvUnmapped;
                    mesh->GetPolygonVertexUV(i, j, uvSetName, uv, uvUnmapped);

                    fbxsdk::FbxVector4 normal;
                    mesh->GetPolygonVertexNormal(i, j, normal);

                    /* Pass data to ModelScribe */

                    int insertIndex = (vertexIndex + vertexIndexOffset);
                    gModelScriber->WriteIndex(i + indexStart, j, insertIndex);

                    gModelScriber->WritePosition(insertIndex, { static_cast<float>(pos[0]), static_cast<float>(pos[1]), static_cast<float>(pos[2]), static_cast<float>(pos[3]) });
                    gModelScriber->WriteNormal(insertIndex, { static_cast<float>(normal[0]), static_cast<float>(normal[1]), static_cast<float>(normal[2]), static_cast<float>(normal[3]) });
                    gModelScriber->WriteTangent(insertIndex, { -1.f, -1.f, -1.f, 1.f });
                    gModelScriber->WriteTexCoord(insertIndex, { static_cast<float>(uv[0]), static_cast<float>(uv[1]) });

                    /* Blend Index & Weight */

                    u8 blend_indexes[4] = { 0, 0, 0, 0 };
                    float blend_weights[4] = { 0.f, 0.f, 0.f, 0.f };
                    
                    if (cpBoneWeights.Size() > vertexIndex)
                    {
                        auto boneWeights = &cpBoneWeights[vertexIndex].mList;
                        int blend_count = qMin(boneWeights->Size(), 4);
                        for (int i = 0; blend_count > i; ++i)
                        {
                            auto boneWeight = boneWeights->GetAt(i);
                            blend_indexes[i] = gModelScriber->GetBoneIndex(boneWeight.mName);
                            blend_weights[i] = static_cast<float>(boneWeight.mWeight);
                        }
                    }

                    gModelScriber->WriteBlendIndex(insertIndex, blend_indexes);
                    gModelScriber->WriteBlendWeight(insertIndex, blend_weights);
                }
            }

            gModelScriber->SetMeshPrims(i, indexStart * 3, num_polygon);
            indexStart += num_polygon;
            vertexIndexOffset += mesh->GetControlPointsCount();
        }

        gModelScriber->EraseRunTimeDatas();
        gModelScriber->WriteChunks(chunk_builder);
    }
}

//===================================================================================
// Entrypoint

int main(int argc, char** argv)
{
    /* Initialize Theory Engine */
    UFG::qInit();
    Illusion::gEngine.Init();

    /* Initialize core stuff */

    SetConsoleTitleA("ModelScriberPC64");

    core::InitializeFBX();
    auto fbxScene = core::ImportScene("Skinned2.fbx");
    //auto fbxScene = core::ImportScene("PoliceBaton001_A.fbx");
    if (!fbxScene) {
        return 1;
    }

    core::gModelScriber = new core::ModelScriber("ALEX_SKIN_BODY", "VertexDecl.Skinned");
    //core::gModelScriber = new core::ModelScriber("PoliceBaton001_A", "VertexDecl.UVN");

    core::gModelScriber->SetBonePalette(reinterpret_cast<UFG::qChunk*>(UFG::qReadEntireFile("Alex.BonePalette.perm.bin")));

    core::CreateModel(fbxScene);

    UFG::qChunkFileBuilder chunk_builder;
    chunk_builder.CreateBuilder("PC64", "Alex.perm.bin");

    core::ScribeMaterials(&chunk_builder, fbxScene);
    core::ScribeModel(&chunk_builder, fbxScene);

    chunk_builder.CloseBuilder(0, true);

    /* Deinitialize Theory Engine */
    UFG::qClose();

    return 0;
}