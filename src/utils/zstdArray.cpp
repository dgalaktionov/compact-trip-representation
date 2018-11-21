#include "zstdArray.h"

#include <assert.h>

ZSTDArray::ZSTDArray(std::vector< std::vector<uint32_t> > *initialTimes) {
	const size_t maxtime = 3600*24*60;
	const size_t tmp_size = 1024*1024;

	const size_t buffer_size = 512;
	const auto tmp = std::unique_ptr<uint8_t[]>(new uint8_t[tmp_size]);
	compressed_frames = std::unique_ptr<uint8_t[]>(new uint8_t[tmp_size]);
	const auto deltas = std::unique_ptr<uint8_t[]>(new uint8_t[tmp_size]);
	samples = std::vector< std::pair<size_t, uint32_t> >();
	lines_C = std::vector<uint32_t>();
	lines_D = std::vector< std::vector<uint32_t> >();
	ZSTD_CStream * const ctx = ZSTD_createCStream();
	ZSTD_inBuffer inbuffer = {deltas.get(), 0, 0};
	ZSTD_outBuffer outbuffer = {compressed_frames.get(), tmp_size, 0};

	for (const auto &times : *initialTimes) {
		size_t i = 0;
		uint8_t j = 0;
		auto t0 = times[0];
		std::fill(tmp.get(), tmp.get()+tmp_size, 0);
		std::fill(deltas.get(), deltas.get()+tmp_size, 0);
		assert(times.size() < tmp_size);
		lines_C.push_back(samples.size());
		auto D = std::vector<uint32_t>();

		for (const auto &t : times) {
			if (i > 0 && t == t0)
				continue;

			assert(i == 0 || t > t0);
			assert(t-t0 < tmp_size);

			if (tmp[t-t0] == 0) {
				tmp[t-t0] = ++j;
				assert(j < 255);
				D.push_back(t-t0);
			}

			deltas[i++] = tmp[t-t0];
			t0 = t;
		}

		assert (i > 1);
		lines_D.push_back(D);

		for (inbuffer.pos = 1; inbuffer.pos < i;) {
			samples.push_back(std::make_pair(outbuffer.pos, times[inbuffer.pos-1]));
			inbuffer.size = inbuffer.pos+min(buffer_size,i-inbuffer.pos);
			ZSTD_initCStream(ctx, 1);
			const auto zsize = ZSTD_compressStream(ctx, &outbuffer, &inbuffer);
			assert(!ZSTD_isError((zsize)));
			ZSTD_endStream(ctx, &outbuffer);
		}
	}

	compressed_size = outbuffer.pos;
	ZSTD_freeCStream(ctx);
}

const size_t ZSTDArray::getSize() {
	size_t s = sizeof(ZSTDArray) + compressed_size;
	s += samples.size() * (sizeof(size_t) + 4);
	s += 5*lines_C.size();
	for (const auto &D : lines_D) s += 4*D.size();
	return s;
}