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
		std::vector<uint32_t> samples;
		std::vector<size_t> frame_pointers;
		std::vector<uint32_t> lines_C;

	public:
		ZSTDArray(std::vector< std::vector<uint32_t> > *initialTimes);
};