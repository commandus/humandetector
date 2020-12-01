#include <string>
#include <map>
#include <vector>
#include <deque>
#include <mutex>

/**
 * @see https://gist.github.com/Jon-Schneider/8b7c53d27a7a13346a643dac9c19d34f
 */
typedef struct WaveHeader {
    // RIFF Header
    char riffHeader[4]; 	// Contains "RIFF"
    int wavSize; 			// Size of the wav portion of the file, which follows the first 8 bytes. File size - 8
    char waveHeader[4]; 	// Contains "WAVE"
    
    // Format Header
    char fmtHeader[4]; 		// Contains "fmt " (includes trailing space)
    int fmtChunkSize; 		// Should be 16 for PCM
    short audioFormat; 		// Should be 1 for PCM. 3 for IEEE Float
    short numChannels;
    int sampleRate;
    int byteRate; 			// Number of bytes per second. sample_rate * num_channels * Bytes Per Sample
    short sampleAlignment; 	// num_channels * Bytes Per Sample
    short bitDepth; 		// Number of bits per sample
    
    // Data
    char dataHeader[4];		// Contains "data"
    int dataBytes;			// Number of bytes in data. Number of samples * num_channels * sample byte size
    // uint8_t bytes[]; 	// Remainder of wave file is bytes
} WaveHeader;


class SENTENCE {
	public:
		std::vector<int32_t> data;
		size_t size;
};

typedef std::deque<SENTENCE> SENTENCES;

void number2keys(
	SENTENCE &keys,
	const uint32_t number
);


class WaveMap {
private:
	std::map<int32_t, std::string> mMap; ///< wav file buffers (as string)
	std::mutex mutexsentences;
	SENTENCES sentences;
	bool hasSentence;
	SENTENCE sentence;	// sentence to play
	size_t sentenceofs;

	size_t copyKey(
		int32_t key,
		size_t keyofs,
		void *buffer,
		size_t ofs,
		size_t size
	);
	void copyNoise(
		void *buffer,
		size_t ofs,
		size_t size
	);

	bool isLast(
		const SENTENCE &keys,
		size_t &keysofs,
		void *buffer,
		size_t &ofs,
		size_t size
	);
	void calcSizes(
		SENTENCE &value
	);
protected:
	bool nextSentence();
	void addSentence(
		SENTENCE &value
	);
public:
	WaveHeader header;
	WaveMap();
	bool put(
		int32_t key,
		const std::string &filename,
		WaveHeader *header
	);
	bool put(
		int32_t key,
		const std::string &prefix,
		const std::string &suffix,
		const std::string &fn,
		WaveHeader *header
	);
	bool put(
		int32_t key,
		const std::string &prefix,
		const std::string &suffix,
		WaveHeader *header
	);
	void rm(
		int32_t key
	);
	size_t get(
		const SENTENCE &keys,
		size_t &keysofs,
		void *buffer,
		size_t &ofs,
		size_t size
	);
	size_t get(
		size_t &keysofs,
		void *buffer,
		size_t size
	);
	std::string toString();
	void loadFiles(
		const std::string &prefix,
		const std::string &suffix
	);
	void next(
		void *buffer,
		size_t size
	);

	void say(
		int64_t value
	);

	void say(
		double value,
		int precision
	);

	// debug
	const SENTENCE* getSentence() const;
	const SENTENCES* getSentences() const;
	bool hasQueuedSentence() const;
};
