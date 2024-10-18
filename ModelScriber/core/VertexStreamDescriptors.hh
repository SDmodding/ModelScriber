//===================================================================================
// 
//	Description:
//		This file contains all VertexStreamDescriptors that will be used by
//		the ModelScriber and is also existing in the game.
// 
//===================================================================================
#pragma once

namespace core
{
	using namespace Illusion;

	class UVNDescriptor : public VertexStreamDescriptor
	{
	public:
		UVNDescriptor() : VertexStreamDescriptor("VertexDecl.UVN", 0xF2700F96)
		{
			AddElement(VERTEX_ELEMENT_POSITION, VERTEX_TYPE_FLOAT3, 0);
			AddElement(VERTEX_ELEMENT_TEXCOORD0, VERTEX_TYPE_HALF2, 1);
			AddElement(VERTEX_ELEMENT_NORMAL, VERTEX_TYPE_BYTE4N, 1);
		}
	};

	class SkinnedDescriptor : public VertexStreamDescriptor
	{
	public:
		SkinnedDescriptor() : VertexStreamDescriptor("VertexDecl.Skinned", 0x276B9567)
		{
			AddElement(VERTEX_ELEMENT_POSITION, VERTEX_TYPE_FLOAT4, 0);
			AddElement(VERTEX_ELEMENT_NORMAL, VERTEX_TYPE_BYTE4N, 0);
			AddElement(VERTEX_ELEMENT_TANGENT, VERTEX_TYPE_BYTE4N, 0);
			AddElement(VERTEX_ELEMENT_BLENDINDEX, VERTEX_TYPE_UBYTE4, 1);
			AddElement(VERTEX_ELEMENT_BLENDWEIGHT, VERTEX_TYPE_UBYTE4N, 1);
			AddElement(VERTEX_ELEMENT_TEXCOORD0, VERTEX_TYPE_HALF2, 2);
		}
	};

	static UVNDescriptor gUVNDescriptor;
	static SkinnedDescriptor gSkinnedDescriptor;
}