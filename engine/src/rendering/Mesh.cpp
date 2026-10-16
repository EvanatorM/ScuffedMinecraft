#include <wv/rendering/Mesh.h>

#include <wv/rendering/Renderer.h>
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

namespace WillowVox
{
	struct Mesh::Impl
	{
		Renderer* renderer;
		vk::Buffer vertexBuffer;
		VmaAllocation vertexBufferAlloc;
		vk::Buffer indexBuffer;
		VmaAllocation indexBufferAlloc;
	};

	Mesh::Mesh(Renderer* renderer, void* vertexBuffer, void* vertexBufferAlloc, void* indexBuffer, void* indexBufferAlloc, uint32_t indexCount)
	{
		_impl = std::make_unique<Impl>();
		
		_impl->renderer = renderer;
		if (indexCount == 0)
		{
			_impl->vertexBuffer = nullptr;
			_impl->indexBuffer = nullptr;
			_indexCount = 0;
			return;
		}

		_impl->vertexBuffer = *static_cast<vk::Buffer*>(vertexBuffer);
		_impl->vertexBufferAlloc = *static_cast<VmaAllocation*>(vertexBufferAlloc);
		_impl->indexBuffer = *static_cast<vk::Buffer*>(indexBuffer);
		_impl->indexBufferAlloc = *static_cast<VmaAllocation*>(indexBufferAlloc);

		_indexCount = indexCount;
	}

	Mesh::~Mesh()
	{
		if (_impl->vertexBuffer)
			_impl->renderer->DestroyMesh(this);
	}

	void* Mesh::GetVertexBuffer() const
	{
		return &_impl->vertexBuffer;
	}

	void* Mesh::GetVertexBufferAlloc() const
	{
		return &_impl->vertexBufferAlloc;
	}

	void* Mesh::GetIndexBuffer() const
	{
		return &_impl->indexBuffer;
	}

	void* Mesh::GetIndexBufferAlloc() const
	{
		return &_impl->indexBufferAlloc;
	}

	uint32_t Mesh::GetIndexCount() const
	{
		return _indexCount;
	}
}