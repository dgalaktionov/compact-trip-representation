#include "zstdArray.h"

#ifndef min
#define min(x,y) ((x)<(y)?(x):(y))
#endif

const size_t ZSTD_BUFFER = 512;

ZSTDArray::ZSTDArray(std::vector< std::vector<uint32_t> > *initialTimes) {
	const size_t maxtime = 3600*24*60;
	const size_t tmp_size = 1024*1024;

	const auto tmp = std::unique_ptr<uint8_t[]>(new uint8_t[tmp_size]);
	compressed_frames = std::unique_ptr<uint8_t[]>(new uint8_t[tmp_size]);
	const auto deltas = std::unique_ptr<uint8_t[]>(new uint8_t[tmp_size]);
	sample_pointers = std::vector<size_t>();
	sample_values = std::vector<uint32_t>();
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
		lines_C.push_back(sample_pointers.size());
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

		// we never compress deltas[0] because it's usually either a 0 or an "strange" delta, so we just sample it
		inbuffer.pos = 1;
		
		for (size_t k = 0; k < i; k++) {
			t0 += delta[k];

			if (k%ZSTD_BUFFER == 0) {
				sample_pointers.push_back(outbuffer.pos);
				sample_values.push_back(t0);
				inbuffer.size = inbuffer.pos+min(ZSTD_BUFFER,i-inbuffer.pos);
				ZSTD_initCStream(ctx, 1);
				const auto zsize = ZSTD_compressStream(ctx, &outbuffer, &inbuffer);
				assert(!ZSTD_isError((zsize)));
				ZSTD_endStream(ctx, &outbuffer);
			}
		}
		for (inbuffer.pos = 1; inbuffer.pos < i;) {
			sample_pointers.push_back(outbuffer.pos);
			sample_values.push_back(times[inbuffer.pos-1]);
			inbuffer.size = inbuffer.pos+min(ZSTD_BUFFER,i-inbuffer.pos);
			ZSTD_initCStream(ctx, 1);
			const auto zsize = ZSTD_compressStream(ctx, &outbuffer, &inbuffer);
			assert(!ZSTD_isError((zsize)));
			ZSTD_endStream(ctx, &outbuffer);
		}
	}

	lines_C.push_back(sample_pointers.size());
	compressed_size = outbuffer.pos;
	ZSTD_freeCStream(ctx);

	#ifndef NDEBUG
		std::cerr << std::endl << "Checking ZSTDArray... ";
		check(initialTimes);
		std::cerr << "OK" << std::endl;
	#endif
}

const size_t ZSTDArray::getSize() {
	size_t s = sizeof(ZSTDArray) + compressed_size;
	s += sample_pointers.size()*sizeof(size_t) + sample_values.size()*4;
	s += 5*lines_C.size();
	for (const auto &D : lines_D) s += 4*D.size();
	return s;
}

const void ZSTDArray::check(std::vector< std::vector<uint32_t> > *initialTimes) {
	const auto buffer = std::unique_ptr<uint8_t[]>(new uint8_t[ZSTD_BUFFER]);
	ZSTD_DStream * const ctx = ZSTD_createDStream();
	ZSTD_inBuffer inbuffer = {compressed_frames.get(), compressed_size, 0};
	ZSTD_outBuffer outbuffer = {buffer.get(), ZSTD_BUFFER, 0};

	for (size_t i = 0; i < initialTimes->size(); i++) {
		const auto C = lines_C.at(i);
		const auto D = lines_D.at(i);
		const auto times = initialTimes->at(i);
		auto n = sample_values.at(C);
		size_t k = 0;

		for (size_t j = 1; j < times.size(); j++) {
			if (times[j] == times[j-1])
				continue;

			if (k%ZSTD_BUFFER == 0) {
				std::cout << "FRAME: " << sample_values.at(C+k/ZSTD_BUFFER) << ' ' << times[j-1] << std::endl;
				assert(sample_pointers.at(C+k/ZSTD_BUFFER) == inbuffer.pos);
				assert(sample_values.at(C+k/ZSTD_BUFFER) == times[j-1]);
				outbuffer.pos = 0;
				ZSTD_initDStream(ctx);
				const auto zsize = ZSTD_decompressStream(ctx, &outbuffer, &inbuffer);
				assert(!ZSTD_isError((zsize)));
			}

			//n += D.at(buffer[k % ZSTD_BUFFER]);
			//std::cout << n << ' ' << times[j] << std::endl;
			//assert(k == times[j]);
			k++;
		}
	}

	ZSTD_freeDStream(ctx);
}

const std::pair<size_t, size_t> ZSTDArray::getBounds(uint16_t line_id, uint32_t start_t, uint32_t end_t) {
	const auto C = lines_C.at(line_id);
	const auto D = lines_D.at(line_id);
	const auto buffer = std::unique_ptr<uint8_t[]>(new uint8_t[ZSTD_BUFFER]);
	ZSTD_DStream * const ctx = ZSTD_createDStream();
	ZSTD_inBuffer inbuffer = {compressed_frames.get(), compressed_size, 0};
	ZSTD_outBuffer outbuffer = {buffer.get(), ZSTD_BUFFER, 0};
	const auto it_s = sample_values.cbegin() + C;
	const auto it_e = sample_values.cbegin() + lines_C.at(line_id+1);
	std::pair<size_t, size_t> bounds = std::make_pair(0,0);

	const auto s_lo = std::lower_bound(it_s, it_e, start_t)-1;
	const auto s_hi = std::upper_bound(it_s, it_e, end_t)-1;
	auto k = *s_lo;

	if (s_hi >= it_s) {
		if (s_lo >= it_s) {
			outbuffer.pos = 0;
			inbuffer.pos = sample_pointers.at(s_lo - sample_values.cbegin());
			ZSTD_initDStream(ctx);
			const auto zsize = ZSTD_decompressStream(ctx, &outbuffer, &inbuffer);
			assert(!ZSTD_isError((zsize)));

			size_t i = 0;

			std::cout << it_s - sample_values.cbegin() << ' ' << s_lo - sample_values.cbegin() << ' ' << it_e - sample_values.cbegin() << ' ' << outbuffer.pos << std::endl;
			for(k = *s_lo; k < start_t && i < outbuffer.pos; i++) {
				std::cout << +buffer[i] << ' ' << D.size() << std::endl;
				assert(buffer[i] < D.size());
				k+=D[buffer[i]];
			}

			// should be i-1, but it cancels out with a +1 as deltas[0] is never compressed
			bounds.first = (s_lo-it_s)*ZSTD_BUFFER + i;
		}

		if (k >= start_t) {
			// check if we can reuse the same decompressed frame
			if (s_lo < it_s || s_lo < s_hi) {
				outbuffer.pos = 0;
				inbuffer.pos = sample_pointers.at(s_hi - sample_values.cbegin());
				ZSTD_initDStream(ctx);
				const auto zsize = ZSTD_decompressStream(ctx, &outbuffer, &inbuffer);
				assert(!ZSTD_isError((zsize)));
			}

			size_t i = 0;

			for(k = *s_hi; k <= end_t && i < outbuffer.pos; i++) {
				assert(buffer[i] < D.size());
				k+=D[buffer[i]];
			}

			// -1 here because want to return the last valid index
			bounds.second = (s_hi-it_s)*ZSTD_BUFFER + i-1;
		}
	} else {
		// set to invalid 
		bounds.first = 1;
	}

	ZSTD_freeDStream(ctx);
	return bounds;
}