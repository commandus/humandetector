#include <cstring>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "wavemap.h"
#include "utilstring.h"
#include "number2words.h"

#define DEBUG 1

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

void WaveMap::rm(
	int32_t key
) {
	mMap.erase(key);
}

size_t WaveMap::copyKey(
	int32_t key,
	size_t keyofs,
	void *buffer,
	size_t ofs,
	size_t size
) {
	size_t r = 0;
	int sz = mMap[key].size() - keyofs;
	if (sz <= 0)
		return r;

	size_t endpos = ofs + sz;
	if (endpos <= size) {
		r = sz;
	} else {
		if (ofs < size) {
			r = size - ofs;
		}
	}
	if (r + ofs > size) {
		r = size;
	}

#ifdef DEBUG
	std::cerr << "    copy key: " << key << ", keyofs: " << keyofs << ", ofs: " << ofs << ", r: " << r << ", bytes: " << r + ofs << std::endl;
#endif
	if (r) {
		memmove(((char *) buffer) + ofs, mMap[key].c_str() + keyofs, r);
		// keyofs += r;
	}
	return r;
}

void WaveMap::copyNoise(
	void *buffer,
	size_t ofs,
	size_t size
) {
	if (ofs < size)
		memset(((char *) buffer) + ofs, 0, size - ofs);	///< TODO add comfort noise
}

bool WaveMap::isLast(
	const SENTENCE &keys,
	size_t &keysofs,
	void *buffer,
	size_t &ofs,
	size_t size
) {

}

size_t WaveMap::get(
	const SENTENCE &keys,
	size_t &keysofs,
	void *buffer,
	size_t &ofs,
	size_t size
) {
#ifdef DEBUG
	std::cerr << "  get" << std::endl << ", size: " << keys.size;
	for (std::vector<int32_t>::const_iterator it(keys.data.begin()); it != keys.data.end(); ++it)
		std::cerr << *it << " ";
	std::cerr << std::endl;
#endif

	size_t keyssz = 0;
	size_t sizeCopiedAll = 0;
	for (std::vector<int32_t>::const_iterator it(keys.data.begin()); it != keys.data.end(); ++it) {
		if (ofs >= size) {
#ifdef DEBUG
	std::cerr << "  break: ofs >= size " <<  ofs << ", " << size << std::endl;
#endif
			break;
		}
		size_t ksz = mMap[*it].size();
		int keyofs = keysofs - keyssz;
		if (keyofs < 0)
			continue;
#ifdef DEBUG
	std::cerr << "  sizeCopiedAll " <<  sizeCopiedAll  << std::endl;
#endif
		if (sizeCopiedAll >= size) {
#ifdef DEBUG
	std::cerr << "  break: sizeCopiedAll >= size " <<  sizeCopiedAll  << ", " << size << std::endl;
#endif
			break;
		}			
		// if (keysofs >= keyssz)continue;
#ifdef DEBUG
	std::cerr << "  key size: " << mMap[*it].size() << ", ofs: " << ofs << ", keysofs: " << keysofs<< ", keyofs: " << keyofs<< ", keyssz: " << keyssz << std::endl;
#endif
		size_t sizeCopied = copyKey(*it, keyofs, buffer, ofs, size);
		ofs += sizeCopied; 
		sizeCopiedAll += sizeCopied;
		keyssz += ksz;
	}
#ifdef DEBUG
	std::cerr << "  sizeCopiedAll " <<  sizeCopiedAll << std::endl;
#endif

	if (keysofs + sizeCopiedAll < keys.size) {
		hasSentence = true;
		keysofs += sizeCopiedAll;
	} else {
#ifdef DEBUG
	std::cerr << "  keysofs + sizeCopiedAll >= keys.size " 
		<< " keysofs: " << keysofs
		<< " sizeCopiedAll: " << sizeCopiedAll
		<< " keys.size: " << keys.size
		<< std::endl;
#endif
		hasSentence = false;
		keysofs = 0;
	}
	return sizeCopiedAll;
}

size_t WaveMap::get(
	size_t &keysofs,
	void *buffer,
	size_t size
) {
	size_t ofs = 0;
	while (true) {
		if (!hasSentence) {
#ifdef DEBUG
			std::cerr << "no sentence, sentences count: " << sentences.size() << ", trying get next sentence" << std::endl;
#endif
			if (!nextSentence()) {
#ifdef DEBUG
				std::cerr << "no sentence, sentences count: " << sentences.size() << ", noise" << std::endl;
#endif
				copyNoise(buffer, ofs, size);	
				keysofs = 0;
				return size;
			}
		}
#ifdef DEBUG
		std::cerr << "has sentence, sentences count: " << sentences.size() << ", size: " << size << ", keysofs: " << keysofs << std::endl;
#endif
		size_t sz = get(sentence, keysofs, buffer, ofs, size);
		if ((sz == 0) || (ofs >= size))
			break;
	}
	return ofs;
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
	put(0, prefix, suffix, &header);
	put(-1, prefix, suffix, "minus", &header);
	put(-2, prefix, suffix, "dot", &header);
	put(-3, prefix, suffix, "silence", &header);
	// powers
	for (int i = 1000; i <= 1000000000; i *= 10) {
		put(i, prefix, suffix, &header);
		put(i + 1, prefix, suffix, &header);	// -a
		put(i + 2, prefix, suffix, &header);	// -ov
	}
	// digits
    for (int i = 1; i < 10; i++) {
		put(i, prefix, suffix, &header);				// m
		put(i * 10, prefix, suffix, &header);
		put(i * 100, prefix, suffix, &header);
    }
	// digits female
    for (int i = 1; i <= 2; i++) {
		put((i * 100) + 88, prefix, suffix, &header);	// f
    }
}

void WaveMap::next(
	void *buffer,
	size_t size
) {
	uint8_t *b = (uint8_t *) buffer;
	if (!hasSentence) {
		if (!nextSentence()) {
			copyNoise(buffer, 0, size);
			return;
		}
	}
}

void WaveMap::say(
	int64_t value
) {
	SENTENCE codes;
	if (value < 0) {
		codes.data.push_back(-1);
		value = -value;
	}
	number2codeRU(codes.data, value);
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
	double value,
	int precision
) {
	SENTENCE codes;
	if (value < 0) {
		codes.data.push_back(-1);
		value = -value;
	}
	number2codeRU(codes.data, (int32_t) value);
	if (codes.data.size())
		codes.data.pop_back();	// remove last delimiter
	if (precision > 0) {
		int v = (value - (int64_t) value) * pow10(precision);
		if (v > 0) {
			codes.data.push_back(-2);	// dot
			number2codeRU(codes.data, v);
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
	return hasSentence;
}

void WaveMap::calcSizes(SENTENCE &value)
{
	size_t sz = 0;
	for (std::vector<int32_t>::const_iterator it(value.data.begin()); it != value.data.end(); ++it) {
		sz += mMap[*it].size();
	}
	value.size = sz;
}

void WaveMap::addSentence(SENTENCE &value)
{
	mutexsentences.lock();
	calcSizes(value);
	sentences.push_back(value);
	mutexsentences.unlock();
}

const SENTENCE* WaveMap::getSentence() const {
	return &sentence;
}

const SENTENCES* WaveMap::getSentences() const {
	return &sentences;
}

bool WaveMap::hasQueuedSentence() const {
	return hasSentence;
}
