#include <wv/rendering/Shader.h>

#include <vulkan/vulkan.hpp>

namespace WillowVox
{
	struct Shader::Impl
	{
		vk::Device device;
		vk::PipelineLayout pipelineLayout;
		vk::Pipeline pipeline;
	};

	Shader::Shader(void* device, void* pipelineLayout, void* pipeline)
	{
		_impl = std::make_unique<Impl>();
		_impl->device = *static_cast<vk::Device*>(device);
		_impl->pipelineLayout = *static_cast<vk::PipelineLayout*>(pipelineLayout);
		_impl->pipeline = *static_cast<vk::Pipeline*>(pipeline);
	}

	Shader::~Shader()
	{
		_impl->device.destroyPipeline(_impl->pipeline);
		_impl->device.destroyPipelineLayout(_impl->pipelineLayout);
	}

	void* Shader::GetPipeline() const
	{
		return &_impl->pipeline;
	}

	void* Shader::GetPipelineLayout() const
	{
		return &_impl->pipelineLayout;
	}
}