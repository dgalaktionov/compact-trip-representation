#pragma once

#include <vector>
#include <memory>
#include <utility>
#include <iostream>
#include <exception>
#include <algorithm>

#include <zstd.h>
#include <assert.h>

class ZSTDArray {
	private:
		size_t compressed_size;
		std::unique_ptr<uint8_t[]> compressed_frames;
		std::vector<size_t> sample_pointers;
		std::vector<uint32_t> sample_values;
		std::vector<uint32_t> lines_C;
		std::vector< std::vector<uint32_t> > lines_D;

	public:
		ZSTDArray(std::vector< std::vector<uint32_t> > *initialTimes);

		const size_t getSize();
		const void check(std::vector< std::vector<uint32_t> > *initialTimes);
		const std::pair<size_t, size_t> getBounds(uint16_t line_id, uint32_t start_t, uint32_t end_t);
};