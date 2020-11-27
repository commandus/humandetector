#include <cstring>
#include <sstream>
#include "wavemap.h"
#include "utilstring.h"
#include "number2words.h"

WaveMap::WaveMap()
	: hasSentence(false), sentenceofs(0)
{
}

bool WaveMap::put(
	int32_t key,
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

bool WaveMap::put(
	int32_t key,
	const std::string &prefix,
	const std::string &suffix,
	const std::string &fn,
	WaveHeader *header
) {
	std::stringstream ss;
	ss << prefix << fn << suffix;
	return put(key, ss.str(), header);
}

bool WaveMap::put(
	int32_t key,
	const std::string &prefix,
	const std::string &suffix,
	WaveHeader *header
) {
	std::stringstream ss;
	ss << prefix << key << suffix;
	return put(key, ss.str(), header);
}

bool WaveMap::rm(
	int32_t key
) {
	mMap.erase(key);
}

size_t WaveMap::copyKey(
	int32_t key,
	size_t ofs,
	void *buffer,
	size_t size
) {
	size_t r;
	size_t sz = mMap[key].size();
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

size_t WaveMap::copyNoise(
	void *buffer,
	size_t size
) {
	memset(buffer, 0, size);	///< TODO add comfort noise
}

size_t WaveMap::getKey(
	int32_t key,
	void *buffer,
	size_t ofs,
	size_t size
) {
	return copyKey(key, ofs, buffer, size);
}

size_t WaveMap::get(
	bool &eof,
	const SENTENCE &keys,
	void *buffer,
	size_t &ofs,
	size_t size
) {
	size_t r = 0;
	size_t sz = 0;
	size_t o = ofs;
	for (std::vector<int32_t>::const_iterator it(keys.begin()); it != keys.end(); ++it) {
		sz += mMap[*it].size();
		if (o < sz) {
			r += copyKey(*it, sz - o, buffer, size);
			o = ofs + r;
			break;
		}
		if (sz >= size) {
			eof = true;
			ofs = 0;
			break;
		}
	}
	eof = false;
	return r;
}

size_t WaveMap::get(
	bool &eof,
	void *buffer,
	size_t &ofs,
	size_t size
) {
	if (!hasSentence) {
		if (!nextSentence()) {
			copyNoise(buffer, size);	
			ofs = 0;
			eof = true;
			return 0;
		}
	}
	return get(eof, sentence, buffer, ofs, size);
}

std::string WaveMap::toString() {
	std::stringstream ss;
	for (std::map<int32_t, std::string>::const_iterator it(mMap.begin()); it != mMap.end(); ++it) {
		ss << it->first << ": " << it->second.size() << std::endl;
	}
	return ss.str();	
}


void WaveMap::loadFiles(
	const std::string &prefix,
	const std::string &suffix
) {
	WaveHeader hdr;
	put(0, prefix, suffix, &hdr);
	put(-1, prefix, suffix, "minus", &hdr);
	put(-2, prefix, suffix, "dot", &hdr);
	put(-3, prefix, suffix, "silence", &hdr);
	// powers
	for (int i = 1000; i <= 1000000000; i *= 10) {
		put(i, prefix, suffix, &hdr);
		put(i + 1, prefix, suffix, &hdr);	// -a
		put(i + 2, prefix, suffix, &hdr);	// -ov
	}
	// digits
    for (int i = 1; i < 10; i++) {
		put(i, prefix, suffix, &hdr);				// m
		put(i * 10, prefix, suffix, &hdr);
		put(i * 100, prefix, suffix, &hdr);
    }
	// digits female
    for (int i = 1; i <= 2; i++) {
		put((i * 100) + 88, prefix, suffix, &hdr);	// f
    }
}

void WaveMap::next(
	void *buffer,
	size_t size
) {
	uint8_t *b = (uint8_t *) buffer;
	if (!hasSentence) {
		if (!nextSentence()) {
			copyNoise(buffer, size);
			return;
		}
	}
}

void WaveMap::say(
	int64_t value
) {
	SENTENCE codes;
	if (value < 0) {
		codes.push_back(-1);
		value = -value;
	}
	number2codeRU(codes, value);
	addSentence(codes);
}

static int pow10(int n)
{
    static int pow10[10] = {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000};
	if (n < 10)
		return pow10[n]; 
	else
		return 0;	
}

void WaveMap::say(
	float value,
	int precision
) {
	SENTENCE codes;
	if (value < 0) {
		codes.push_back(-1);
		value = -value;
	}
	number2codeRU(codes, (int32_t) value);
	if (precision > 0) {
		int v = (value - (int64_t) value) * pow10(precision);
		if (v > 0) {
			codes.push_back(-2);	// dot
			number2codeRU(codes, v);
		}
	}
	addSentence(codes);

}

/**
 * Set hasSentence and sentence
 */
bool WaveMap::nextSentence()
{
	mutexsentences.lock();
	SENTENCES::const_iterator it = sentences.cbegin();
	hasSentence = it != sentences.end();
	if (hasSentence) {
		sentence = *it;
		sentences.pop_front();
	}
	mutexsentences.unlock();
	if (hasSentence) {
		sentenceofs = 0;
	}
	return true;
}

void WaveMap::addSentence(const SENTENCE &value)
{
	mutexsentences.lock();
	sentences.push_back(value);
	mutexsentences.unlock();
}
