#include "FileHelpers.h"

#include <filesystem>
namespace fs = std::experimental::filesystem;

void relativitify(std::string & path) {
	static std::string workingDir = fs::current_path().string();
	std::string workingDirWorking = workingDir;

	int start = 0;
	int end = 0;
	for (int ix = 0; ix <= workingDir.size(); ix++) {
		if (workingDir[ix] == path[ix])
			end = ix + 1;
		else
			break;
	}
	path.erase(path.begin() + start, path.begin() + end + 1);
	workingDirWorking.erase(workingDirWorking.begin() + start, workingDirWorking.begin() + end);

	if (workingDirWorking.size() > 0) {
		throw("To be implemented");
	}
}

void Write(std::fstream & stream, void * data, size_t size) {
	stream.write(reinterpret_cast<char*>(data), size);
}

void Read(std::fstream & stream, void * data, size_t size) {
	stream.read(reinterpret_cast<char*>(data), size);
}
