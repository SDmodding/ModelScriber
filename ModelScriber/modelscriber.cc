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

/* Core */
#include "core/Helper.hh"
#include "core/FBX.hh"
#include "core/MaterialScriber.hh"
#include "core/ModelScriber.hh"
#include "core/VertexStreamDescriptors.hh"

//===================================================================================
// Entrypoint

int main(int argc, char** argv)
{
    /* Initialize Theory Engine */
    UFG::qInit();
    Illusion::gEngine.Init();

    /* Initialize core stuff */

    SetConsoleTitleA("ModelScriberPC64");

    core::ModelScriber model("PoliceBaton001_A", "VertexDecl.UVN");

    core::InitializeFBX();
    auto fbxScene = core::ImportScene("cube.fbx");
    if (!fbxScene) {
        return 1;
    }

    fbxsdk::FbxMesh* fbxMesh = nullptr;
    int num_meshes = 0;

    for (int i = 0; fbxScene->GetNodeCount() > i; ++i)
    {
        auto fbxNode = fbxScene->GetNode(i);
        if (!fbxNode->GetMesh()) {
            continue;
        }

        ++num_meshes;
        fbxMesh = fbxNode->GetMesh();
      
    }

    /*auto num_mat = fbxMesh->GetNode()->GetMaterialCount();
    auto mat = fbxMesh->GetNode()->GetMaterial(0);*/

    int num_polygon = fbxMesh->GetPolygonCount();
    int num_cp = fbxMesh->GetControlPointsCount();
    int num_vertexes = fbxMesh->GetPolygonVertexCount();


    fbxsdk::FbxStringList uvSetNames;
    fbxMesh->GetUVSetNames(uvSetNames);

    auto uvSetName = uvSetNames[0];

    model.CreateIndexBuffer(num_vertexes, num_cp);
    model.CreateVertexBuffers(num_cp);
    model.CreateModel(1);
    model.SetMeshPrims(0, 0, num_polygon);

    for (int i = 0; num_polygon > i; ++i)
    {
        for (int j = 0; 3 > j; ++j)
        {
            int vertexIndex = fbxMesh->GetPolygonVertex(i, j);
            auto position = fbxMesh->GetControlPointAt(vertexIndex);

            fbxsdk::FbxVector2 uv;
            bool bUnmapped;
            fbxMesh->GetPolygonVertexUV(i, j, uvSetName, uv, bUnmapped);

            fbxsdk::FbxVector4 normal;
            fbxMesh->GetPolygonVertexNormal(i, j, normal);

            model.WritePosition(vertexIndex, { static_cast<float>(position[0]), static_cast<float>(position[1]), static_cast<float>(position[2]) });
            model.WriteNormal(vertexIndex, { static_cast<float>(normal[0]), static_cast<float>(normal[1]), static_cast<float>(normal[2]), static_cast<float>(normal[3]) });
            model.WriteTexCoord(vertexIndex, { static_cast<float>(uv[0]), static_cast<float>(uv[1]) });

            model.WriteIndex(i, j, vertexIndex);
        }
    }

    core::MaterialScriber material(model.GetMaterialName(model.mName, 0));

    material.AddParam("iShader", "Illusion.Shader", "HK_SCENERY");
    material.AddParam("iTexture", "texDiffuse", "Illusion.Texture", "DEFAULT");
    material.AddParam("iRasterState", "Illusion.RasterState", "Illusion.RasterState.Normal");
    material.AddParam("iAlphaState", "Illusion.AlphaState", 0);

    material.CreateMaterial();

    model.EraseRunTimeDatas();

    UFG::qChunkFileBuilder chunk_builder;
    chunk_builder.CreateBuilder("PC64", "test.perm.bin");

    chunk_builder.BeginChunk(ChunkUID_Material, material.mName);
    chunk_builder.Write(material.mMaterial, UFG::GetMainMemoryPool()->Size(material.mMaterial));
    chunk_builder.EndChunk(ChunkUID_Material);

    chunk_builder.BeginChunk(ChunkUID_Buffer, model.GetIndexBufferName());
    chunk_builder.Write(model.mIndexBuffer, UFG::GetMainMemoryPool()->Size(model.mIndexBuffer));
    chunk_builder.EndChunk(ChunkUID_Buffer);

    for (auto vertex_buffer : model.mVertexBuffers)
    {
        if (!vertex_buffer) {
            continue;
        }

        chunk_builder.BeginChunk(ChunkUID_Buffer, model.GetIndexBufferName());
        chunk_builder.Write(vertex_buffer, UFG::GetMainMemoryPool()->Size(vertex_buffer));
        chunk_builder.EndChunk(ChunkUID_Buffer);
    }

    chunk_builder.BeginChunk(ChunkUID_Model, model.mName);
    chunk_builder.Write(model.mModel, UFG::GetMainMemoryPool()->Size(model.mModel));
    chunk_builder.EndChunk(ChunkUID_Model);

    chunk_builder.CloseBuilder(0, true);

    /* Deinitialize Theory Engine */
    UFG::qClose();

	return 0;
}