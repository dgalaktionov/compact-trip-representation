#include "zstdArray.h"

#include <assert.h>

ZSTDArray::ZSTDArray(std::vector< std::vector<uint32_t> > *initialTimes) {
	const size_t maxtime = 3600*24*60;
	const size_t tmp_size = 1024*1024;

	const size_t buffer_size = 512;
	const auto tmp = std::unique_ptr<uint8_t[]>(new uint8_t[tmp_size]);
	compressed_frames = std::unique_ptr<uint8_t[]>(new uint8_t[tmp_size]);
	const auto deltas = std::unique_ptr<uint8_t[]>(new uint8_t[tmp_size]);
	ZSTD_CStream * const ctx = ZSTD_createCStream();
	ZSTD_inBuffer inbuffer = {deltas.get(), 0, 0};
	ZSTD_outBuffer outbuffer = {compressed_frames.get(), tmp_size, 0};
	size_t initialSize = 0;

	for (const auto &times : *initialTimes) {
		size_t i = 0;
		uint8_t j = 0;
		auto t0 = times[0];
		std::fill(tmp.get(), tmp.get()+tmp_size, 0);
		std::fill(deltas.get(), deltas.get()+tmp_size, 0);
		assert(times.size() < tmp_size);

		for (const auto &t : times) {
			assert(t >= t0);
			assert(t-t0 < tmp_size);

			if (t-t0 != 0 && tmp[t-t0] == 0) {
				tmp[t-t0] = ++j;
				assert(j < 255);
			}

			deltas[i++] = tmp[t-t0];
			t0 = t;
		}

		for (size_t k = 1; k < i; k += buffer_size) {
			inbuffer.pos = k;
			inbuffer.size = k+min(buffer_size,i-k);
			ZSTD_initCStream(ctx, 1);
			const auto zsize = ZSTD_compressStream(ctx, &outbuffer, &inbuffer);
			assert(!ZSTD_isError((zsize)));
			ZSTD_endStream(ctx, &outbuffer);
			initialSize += sizeof(size_t) + 4;
		}
		
		initialSize += j*4;
	}

	initialSize += outbuffer.pos;
	std::cout << std::endl << "initials bitmap size: " << initialSize << std::endl;
	ZSTD_freeCStream(ctx);
}