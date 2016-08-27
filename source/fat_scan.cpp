
#include <iostream>
#include <fstream>
#include <cassert>

#include <sys/time.h>
typedef double TimeT;
TimeT systemTime () {
	struct timeval t;
	gettimeofday (&t, (struct timezone*)0);
	return ((TimeT)t.tv_sec) + ((TimeT)t.tv_usec / 1000000.0);
}

#define packed __attribute__((__packed__))

const uint8_t ATTR_READ_ONLY = 0x01;
const uint8_t ATTR_HIDDEN = 0x02;
const uint8_t ATTR_SYSTEM = 0x04;
const uint8_t ATTR_VOLUME_ID = 0x08;
const uint8_t ATTR_DIRECTORY = 0x10;
const uint8_t ATTR_ARCHIVE = 0x20;
const uint8_t ATTR_LONG_NAME = ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID;

const unsigned long BufferLength = 1024 * 4;
typedef unsigned char byte_t;

struct Buffer {
	char *data;
	unsigned long length;
	
	Buffer(unsigned long _length) : length(_length) {
		data = new char[length];
	}
	
	~Buffer () {
		if (data != NULL)
			delete[] data;
	}
};

struct FatHeader {
	byte_t jmpBoot[3];
	byte_t fmtName[8];
	uint16_t bytesPerSector;
	uint8_t sectorsPerCluster;
	uint16_t reservedSectorCount;
	uint8_t numFATs;
	uint16_t rootEntriesCount; // FAT12/16 only
	uint16_t totalSectors16; // if non-zero read totalSectors32
	uint8_t mediaType;
	uint16_t FATSize16;
	uint16_t sectorsPerTrack;
	uint16_t numHeads;
	uint32_t hiddenSectors;
	uint32_t totalSectors32;
} packed;

struct Fat32ExtendedHeader {
	uint32_t FATSize32;
	uint16_t flags;
	uint16_t fsVersion;
	uint32_t rootCluster;
	uint16_t fsInfo;
	uint16_t backupBootSector;
	byte_t reserved1[12];
	uint8_t driveNumber;
	byte_t reserved2;
	byte_t bootSignature;
	uint32_t volumeID;
	byte_t volumeLabel[11];
	byte_t fileSystemType[8];
	byte_t padding;
} packed;

bool isValidNameCharacter (char c) {
	if (c < 0x20) return false;
	
	if (c == 0x22) return false;
	if (c >= 0x2A && c <= 0x2F) return false;
	if (c >= 0x3A && c <= 0x3F) return false;
	if (c >= 0x5B && c <= 0x5D) return false;
	if (c == 0x7C) return false;
	
	return true;
}

struct Fat32Dir {
	byte_t name[11];
	uint8_t attrs;
	uint8_t reserved1;
	uint8_t createTimeTenth;
	uint16_t createTime;
	uint16_t createDate;
	uint16_t lastAccessDate;
	uint16_t firstClusterHI;
	uint16_t writeTime;
	uint16_t writeDate;
	uint16_t firstClusterLO;
	uint32_t fileSize;
	
	bool isValidName () {
		for (unsigned i = 0; i < 11; ++i) {
			if (!isValidNameCharacter(name[i]))
				return false;
		}
		
		return true;
	}
	
	bool isValidAttrs () {
		return reserved1 == '\0' && (attrs >> 6) == 0;
	}
	
	bool isDirectory() {
		return attrs & ATTR_DIRECTORY;
	}
	
	bool isFreeDir () {
		return name[0] == 0xE5;
	}
	
	bool isDirEnd () {
		return name[0] == 0x00;
	}
} packed;

int main (int argc, char ** argv) {
	using namespace std;
	Buffer buffer(BufferLength);
	
	std::ifstream dev;
	const char * device = NULL;
	
	if (argc >= 2) {
		device = argv[1];
	} else {
		cout << "Usage: " << argv[0] << " {device} [start-sector]" << std::endl;
		return 1;
	}
	
	// Set a big input buffer
	dev.rdbuf()->pubsetbuf(buffer.data, buffer.length);
	dev.open(device, std::ios::binary | std::ios::in);
	
	FatHeader hdr;
	Fat32ExtendedHeader extHdr;
	
	dev.read((char*)&hdr, sizeof(hdr));
	assert(dev.good());
	dev.read((char*)&extHdr, sizeof(extHdr));
	assert(dev.good());
	
	extHdr.padding = '\0';
	
	uint32_t firstDataSector = hdr.reservedSectorCount + (hdr.numFATs * extHdr.FATSize32);
	
	cout << "File System Type: " << extHdr.fileSystemType << endl;
	cout << "Bytes per sector: " << hdr.bytesPerSector << endl;
	cout << "First data sector: " << firstDataSector << endl;
	cout << "Root sector is at " << firstDataSector * hdr.bytesPerSector << " bytes " << endl;
	
	uint32_t currentDataSector = firstDataSector;
	// Force start scanning from beginning of disk
	//currentDataSector = 0;
	
	if (argc == 3) {
		currentDataSector = atoi(argv[2]);
	}
	
	size_t lastOffset = 0;
	TimeT lastTime = systemTime();
	
	dev.seekg(currentDataSector * hdr.bytesPerSector, ios::beg);
	while (!dev.bad()) {
		//dev.seekg(currentDataSector * hdr.bytesPerSector, ios::beg);
		Fat32Dir dir;
		
		std::size_t offset = dev.tellg();
		if (offset - lastOffset > (1024 * 1024 * 50)) {
			TimeT now = systemTime();
			TimeT diff = now - lastTime;
			
			size_t count = offset - lastOffset;
			TimeT perSecond = ((TimeT)count / diff) / (1024.0 * 1024.0);
			
			
			cout << "Scanning sector " << currentDataSector 
				<< " at offset " << (int)((float)offset / (1024.0 * 1024.0)) << "mb" 
				<< " : " << perSecond << "mb/s"<< std::endl;
			
			lastTime = now;
			lastOffset = offset;
		}
		
		do {
			dev.read((char*)&dir, sizeof(dir));
			
			if (dir.isValidName() && dir.isValidAttrs()) {
				if (dir.isDirectory()) {
					dir.attrs = '\0';
					cout << "Dir: " << dir.name << " at " << dev.tellg() << std::endl;
				} else {
					dir.attrs = '\0';
					cout << "File: " << dir.name << " at " << dev.tellg() << std::endl;
				}
			} else {
				cout << "Junk: " << dir.name << " at " << dev.tellg() << std::endl;
			}
		} while (!dir.isDirEnd());
		
		//currentDataSector += 1;
	};
}
