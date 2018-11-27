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
		ZSTD_DStream * d_stream = ZSTD_createDStream();

		const void decompressFrame(ZSTD_outBuffer* output, ZSTD_inBuffer* input);

	public:
		ZSTDArray(const std::vector< std::vector<uint32_t> > *initialTimes);
		~ZSTDArray();

		const size_t getSize();
		const void check(const std::vector< std::vector<uint32_t> > *initialTimes);
		const std::pair<size_t, size_t> getBounds(const uint16_t line_id, uint32_t start_t, uint32_t end_t);
		const uint32_t access(const uint16_t line_id, const size_t i);
};