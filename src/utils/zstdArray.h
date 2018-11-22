#pragma once

#include <vector>
#include <utility>
#include <memory>
#include <exception>

#include <zstd.h>

#include "basics.h"

const size_t ZSTD_BUFFER = 512;

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
		const void check(std::vector< std::vector<uint32_t> > *initialTimes);
};