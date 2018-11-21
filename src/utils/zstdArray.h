#pragma once

#include <vector>
#include <utility>
#include <memory>

#include <zstd.h>

#include "basics.h"

class ZSTDArray {
	private:
		size_t compressed_size;
		std::unique_ptr<uint8_t[]> compressed_frames;
		std::vector< std::pair<size_t, uint32_t> > samples;
		std::vector<uint32_t> lines_C;
		std::vector< std::vector<uint32_t> > lines_D;

	public:
		ZSTDArray(std::vector< std::vector<uint32_t> > *initialTimes);

		const size_t getSize();
};