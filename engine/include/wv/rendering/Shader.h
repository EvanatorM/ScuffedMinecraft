#pragma once

#include <wv/pch.h>

namespace WillowVox
{
	class Shader
	{
	public:
		// Should only be called by Renderer
		Shader(void* device, void* pipelineLayout, void* pipeline);
		~Shader();

		void* GetPipeline() const;
		void* GetPipelineLayout() const;

	private:
		struct Impl;
		std::unique_ptr<Impl> _impl;
	};
}