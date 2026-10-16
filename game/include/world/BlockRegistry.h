#pragma once

#include <world/WorldDefines.h>
#include <world/Block.h>
#include <world/BlockDisplay.h>
#include <wv/pch.h>

namespace ScuffedMinecraft
{
	class BlockRegistry
	{
	public:
		BlockRegistry()
		{
			_blocks.push_back({ "Invalid" }); // 0 = Invalid Block Position
			_blocks.push_back({ "Air" }); // 1 = Empty (Air)
			_blockDisplays.push_back({ 0, 0, 0, 0 });
			_blockDisplays.push_back({ 0, 0, 0, 0 });
		}

		static BlockRegistry* GetInstance()
		{
			static BlockRegistry instance;
			return &instance;
		}

		BlockId RegisterBlock(Block&& block, BlockDisplay&& blockDisplay)
		{
			BlockId id = _blocks.size();
			_blocks.push_back(std::move(block));
			_blockDisplays.push_back(std::move(blockDisplay));

			return id;
		}

		BlockId GetBlockId(const std::string& name)
		{
			auto it = _blockNameToId.find(name);
			if (it == _blockNameToId.end())
				return 0;
			
			return it->second;
		}

		Block* GetBlock(BlockId blockId)
		{
			if (blockId == 0) return nullptr;
			return &_blocks[blockId];
		}
		Block* GetBlock(const std::string& name)
		{
			BlockId id = GetBlockId(name);
			return GetBlock(id);
		}

		BlockDisplay* GetBlockDisplay(BlockId blockId)
		{
			if (blockId == 0) return nullptr;
			return &_blockDisplays[blockId];
		}
		BlockDisplay* GetBlockDisplay(const std::string& name)
		{
			BlockId id = GetBlockId(name);
			return GetBlockDisplay(id);
		}

	private:
		std::vector<Block> _blocks;
		std::unordered_map<std::string, BlockId> _blockNameToId;
		std::vector<BlockDisplay> _blockDisplays;
	};
}