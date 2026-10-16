#pragma once

#include <wv/pch.h>

namespace WillowVox
{
	class Renderer;

	class Mesh
	{
	public:
		// Should only be called by Renderer
		Mesh(Renderer* renderer, void* vertexBuffer, void* vertexBufferAlloc, void* indexBuffer, void* indexBufferAlloc, uint32_t indexCount);
		~Mesh();

		void* GetVertexBuffer() const;
		void* GetVertexBufferAlloc() const;
		void* GetIndexBuffer() const;
		void* GetIndexBufferAlloc() const;
		uint32_t GetIndexCount() const;

	private:
		struct Impl;
		std::unique_ptr<Impl> _impl;

		uint32_t _indexCount;
	};
}