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
#include "core/Console.hh"
#include "core/Helper.hh"
#include "core/FBX.hh"
#include "core/MaterialScriber.hh"
#include "core/ModelScriber.hh"
#include "core/VertexStreamDescriptors.hh"

namespace core
{
    ModelScriber* gModelScriber;
    UFG::qList<UFG::qString> gTextureScriberResources;
    UFG::qFile* gTextureScriberConfig = nullptr;

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

    void AddTextureScriberResource(UFG::qString& resource_file, const char* resource_name)
    {
        if (gTextureScriberResources.IsEmpty()) {
            UFG::qWriteString(gTextureScriberConfig, "<MediaPack>\n");
        }
        else
        {
            /* Check resource already exist. */
            for (auto resource = gTextureScriberResources.begin(); resource != gTextureScriberResources.end(); resource = resource->next())
            {
                if (resource->GetStringHash32() == resource_file.GetStringHash32()) {
                    return;
                }
            }
        }

        gTextureScriberResources.Insert(new UFG::qString(resource_file));

        UFG::qFPrintf(gTextureScriberConfig, "\t<Resource OutputName=\"%s\">%s</Resource>\n", resource_name, resource_file.GetFilenameWithoutExtension().mData);
    }

    void ScribeMaterials(UFG::qChunkFileBuilder* chunk_builder, FbxScene* scene)
    {
        u32 numMeshes = gModelScriber->mModel->mNumMeshes;

        for (u32 i = 0; numMeshes > i; ++i)
        {
            auto mesh = GetMesh(scene, i);
            qAssert(mesh);

            auto material = GetMeshMaterial(mesh);
            bool material_dup = false;
            UFG::qString material_name = (material ? gModelScriber->GetMaterialName(material->GetName()) : gModelScriber->GetMaterialName(mesh->GetName(), i));

            /* Try find duplicate material. */
            if (material && i)
            {
                for (u32 m = 0; i > m; ++m)
                {
                    auto meshMaterial = gModelScriber->GetMeshMaterial(m);
                    if (meshMaterial && meshMaterial->mNode.mUID == material_name.GetStringHashUpper32())
                    {
                        gModelScriber->SetMeshMaterial(i, meshMaterial);
                        material_dup = true;
                        break;
                    }
                }
            }

            if (material_dup) {
                continue;
            }

            core::MaterialScriber mat(material_name);

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
                        if (!fileName.IsEmpty())
                        {
                            auto resource_name = fileName.GetFilenameWithoutExtension();
                            const char* suffix = nullptr;

                            if (UFG::qStringCompareInsensitive(materialMap.mFbx, FbxSurfaceMaterial::sNormalMap) == 0) {
                                suffix = "_N";
                            }
                            else if (UFG::qStringCompareInsensitive(materialMap.mIllusion, FbxSurfaceMaterial::sSpecular) == 0) {
                                suffix = "_S";
                            }

                            /* Normal & Specular texture needs to have suffix so TextureScriber set correct type on 'Illusion::Texture'! */
                            if (suffix && !resource_name.EndsWith(suffix)) {
                                resource_name += suffix;
                            }

                            if (gTextureScriberConfig) {
                                AddTextureScriberResource(fileName, resource_name);
                            }

                            mat.AddParam("iTexture", materialMap.mIllusion, "Illusion.Texture", resource_name.GetStringHashUpper32());
                        }
                    }
                }
            }

            /* Add default params */
            {
                struct DefaultTextureMap { const char* mState, *mName; };
                DefaultTextureMap defaultTextureMap[] = {
                    { "texDiffuse", "DEFAULTGREY" },
                    //{ "texDiffuseBlend", "COLOURCUBEBLEND" } /* Unknown */
                };

                mat.AddParam("iShader", "Illusion.Shader", "HK_SCENERY");

                for (auto textureMap : defaultTextureMap) {
                    mat.AddParam("iTexture", textureMap.mState, "Illusion.Texture", textureMap.mName);
                }

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

            FbxLayerElementTangent* tangents = mesh->GetElementTangent();
            if (tangents) {
                tangents->SetMappingMode(FbxLayerElement::eByControlPoint);
            }

            FbxLayerElementVertexColor* colors = mesh->GetElementVertexColor();
            if (colors) {
                colors->SetMappingMode(FbxLayerElement::eByControlPoint);
            }

            auto cpBoneWeights = core::GetCPBoneWeights(mesh);

            FbxStringList uvSetNames;
            mesh->GetUVSetNames(uvSetNames);

            auto uvSetName = uvSetNames[0];

            int num_polygon = mesh->GetPolygonCount();
            int poly_count = 0;
            for (int i = 0; num_polygon > i; ++i)
            {
                for (int j = 0; 3 > j; ++j)
                {
                    /* Fetch data from FBX */

                    int vertexIndex = mesh->GetPolygonVertex(i, j);
                    auto pos = mesh->GetControlPointAt(vertexIndex);

                    FbxVector2 uv; bool uvUnmapped;
                    mesh->GetPolygonVertexUV(i, j, uvSetName, uv, uvUnmapped);

                    FbxVector4 normal;
                    mesh->GetPolygonVertexNormal(i, j, normal);

                    FbxVector4 tangent = core::GetTangent(tangents, vertexIndex, poly_count);
                    FbxColor color = core::GetVertexColor(colors, vertexIndex, poly_count);

                    /* Pass data to ModelScriber */

                    int writeIndex = (vertexIndex + vertexIndexOffset);
                    gModelScriber->WriteIndex(i + indexStart, j, writeIndex);

                    /* Position */
                    gModelScriber->WritePosition(writeIndex, {
                        static_cast<float>(pos[0]), static_cast<float>(pos[1]), static_cast<float>(pos[2]), static_cast<float>(pos[3]) 
                    });

                    /* Normal */
                    gModelScriber->WriteNormal(writeIndex, {
                        static_cast<float>(normal[0]), static_cast<float>(normal[1]), static_cast<float>(normal[2]), static_cast<float>(normal[3]) 
                    });

                    /* Tangent */
                    gModelScriber->WriteTangent(writeIndex, {
                        static_cast<float>(tangent[0]), static_cast<float>(tangent[1]), static_cast<float>(tangent[2]), static_cast<float>(tangent[3]) 
                    });

                    /* TexCoord */
                    gModelScriber->WriteTexCoord(writeIndex, {
                        static_cast<float>(uv[0]), static_cast<float>(uv[1]) 
                    });

                    /* Color */
                    gModelScriber->WriteColor(writeIndex, {
                        static_cast<float>(color[0]), static_cast<float>(color[1]), static_cast<float>(color[2]), static_cast<float>(color[3])
                    });     

                    /* Blend Index & Weight */

                    u8 blend_indexes[4] = { 0, 0, 0, 0 };
                    float blend_weights[4] = { 1.f, 0.f, 0.f, 0.f };
                    
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

                    gModelScriber->WriteBlendIndex(writeIndex, blend_indexes);
                    gModelScriber->WriteBlendWeight(writeIndex, blend_weights);
                }

                ++poly_count;
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

    /* Initialize console & arguments */

    Con::Initialize(argc, argv);

    auto fbx_file = Con::GetArg("-fbx");
    auto out_file = Con::GetArg("-outfile");
    auto vertex_decl = Con::GetArg("-vertexdecl");
    auto bone_palette = Con::GetArg("-bonepalette");
    auto model_name = Con::GetArg("-name");

    bool texturescriber = Con::HasArg("-texturescriber"); /* Create '.xml' config file fore TextureScriber. */

    /* Check arguments/setup if needed */

    if (fbx_file.IsEmpty() || !UFG::qFileExists(fbx_file))
    {
        UFG::qPrintf("ERROR: Argument '-fbx' is empty or file doesn't exist!\n");
        return 1;
    }

    if (out_file.IsEmpty()) 
    {
        out_file = fbx_file;
        out_file = out_file.ReplaceExtension(".perm.bin");
    }

    if (vertex_decl.IsEmpty())
    {
        UFG::qPrintf(
            "ERROR: Argument '-vertexdecl' is empty!\n"
            "Supported vertex declarations are:\n"
        );

        auto streamDescriptors = Illusion::VertexStreamDescriptor::GetStreamDescriptors();
        for (auto streamDescriptor = streamDescriptors->begin(); streamDescriptor != streamDescriptors->end(); streamDescriptor = streamDescriptor->next()) {
            UFG::qPrintf("\t%s\n", streamDescriptor->mName);
        }

        return 1;
    }

    if (model_name.IsEmpty()) {
        model_name = fbx_file.GetFilenameWithoutExtension();
    }

    /* Initialize core stuff */

    core::InitializeFBX();

    auto fbxScene = core::ImportScene(fbx_file);
    if (!fbxScene) {
        return 1;
    }

    /* Texture Scriber (Config) */

    if (texturescriber)
    {
        auto config_file = out_file.GetFilenameWithoutExtension();
        config_file += ".xml";

        core::gTextureScriberConfig = UFG::qOpen(config_file, UFG::QACCESS_WRITE, true);
    }

    /* Model Scriber */

    core::gModelScriber = new core::ModelScriber(model_name, vertex_decl);

    if (!bone_palette.IsEmpty()) {
        core::gModelScriber->SetBonePalette(reinterpret_cast<UFG::qChunk*>(UFG::qReadEntireFile(bone_palette)));
    }

    core::CreateModel(fbxScene);

    UFG::qChunkFileBuilder chunk_builder;
    chunk_builder.CreateBuilder("PC64", out_file);

    core::ScribeMaterials(&chunk_builder, fbxScene);
    core::ScribeModel(&chunk_builder, fbxScene);

    chunk_builder.CloseBuilder(0, true);

    if (auto config_file = core::gTextureScriberConfig)
    {
        if (UFG::qGetFileSize(config_file)) {
            UFG::qWriteString(config_file, "</MediaPack>\n");
        }
        UFG::qClose(config_file);
    }

    /* Deinitialize Theory Engine */
    UFG::qClose();

    return 0;
}