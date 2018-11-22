#include "zstdArray.h"

#include <assert.h>

ZSTDArray::ZSTDArray(std::vector< std::vector<uint32_t> > *initialTimes) {
	const size_t maxtime = 3600*24*60;
	const size_t tmp_size = 1024*1024;

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
			inbuffer.size = inbuffer.pos+min(ZSTD_BUFFER,i-inbuffer.pos);
			ZSTD_initCStream(ctx, 1);
			const auto zsize = ZSTD_compressStream(ctx, &outbuffer, &inbuffer);
			assert(!ZSTD_isError((zsize)));
			ZSTD_endStream(ctx, &outbuffer);
		}
	}

	compressed_size = outbuffer.pos;
	ZSTD_freeCStream(ctx);

	#ifndef NDEBUG
		std::cerr << std::endl << "Checking ZSTDArray... ";
		std::cerr << "OK" << std::endl;
	#endif
}

const size_t ZSTDArray::getSize() {
	size_t s = sizeof(ZSTDArray) + compressed_size;
	s += samples.size() * (sizeof(size_t) + 4);
	s += 5*lines_C.size();
	for (const auto &D : lines_D) s += 4*D.size();
	return s;
}

const void ZSTDArray::check(std::vector< std::vector<uint32_t> > *initialTimes) {
	for (size_t i = 0; i < initialTimes->size(); i++) {
		const auto C = lines_C.at(i);
		const auto D = lines_D.at(i);
		const auto times = initialTimes->at(i);
		const auto buffer = std::unique_ptr<uint8_t[]>(new uint8_t[ZSTD_BUFFER]);
		ZSTD_DStream * const ctx = ZSTD_createDStream();
		ZSTD_inBuffer inbuffer = {compressed_frames.get(), compressed_size, 0};
		ZSTD_outBuffer outbuffer = {buffer.get(), ZSTD_BUFFER, 0};
		auto k = samples.at(C).second;

		for (size_t j = 1; j < times.size(); j++) {
			if (j%ZSTD_BUFFER == 1) {
				assert(samples.at(C+j/ZSTD_BUFFER).second == times[j-1]);
				outbuffer.pos = 0;
				ZSTD_initDStream(ctx);
				const auto zsize = ZSTD_decompressStream(ctx, &outbuffer, &inbuffer);
				assert(!ZSTD_isError((zsize)));
			}

			k += D.at(buffer[(j-1) % ZSTD_BUFFER]);
			assert(k == times[j]);
		}

		ZSTD_freeDStream(ctx);
	}
}