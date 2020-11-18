#include <cstring>
#include "wavemap.h"
#include "utilstring.h"

WaveMap::WaveMap() {
}

bool WaveMap::put(
	int key,
	const std::string &filename,
	WaveHeader *header
) {
	bool r = false;
	std::string s = file2string(filename.c_str());
	if (header) {
		if (s.size() >= sizeof(WaveHeader)) {
			memmove(header, s.c_str(), sizeof(WaveHeader));
		}
	}
	if (s.size() >= sizeof(WaveHeader)) {
		mMap[key] = s.substr(sizeof(WaveHeader));
		r = true;
	}
	return r;
}

bool WaveMap::rm(
	int key
) {
	mMap.erase(key);
}

size_t WaveMap::copyData(
	int key,
	size_t ofs,
	size_t sz,
	void *buffer,
	size_t size
) {
	size_t r;
	if (sz > ofs) {
		r = sz - ofs;
	} else {
		r = 0;
	}
	if (r > size) {
		r = size;
	}
	if (r) {
		memmove(buffer, mMap[key].c_str() + ofs, r);
	}
	return r;
}

size_t WaveMap::get(
	int key,
	void *buffer,
	size_t ofs,
	size_t size
) {
	size_t sz = mMap[key].size();
	return copyData(key, ofs, sz, buffer, size);
}

size_t WaveMap::get(
	const std::vector<int> &keys,
	void *buffer,
	size_t ofs,
	size_t size
) {
	size_t r = 0;
	size_t sz = 0;
	size_t o = ofs;
	for (std::vector<int>::const_iterator it(keys.begin()); it != keys.end(); ++it) {
		sz += mMap[*it].size();
		if (o < sz) {
			r += copyData(*it, sz - o, sz, buffer, size);
			o = ofs + r;
			break;
		}
		if (sz >= size) {
			break;
		}
	}
	return r;
}
