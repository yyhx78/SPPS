#include "Connect.h"

#include <iostream>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <filesystem>

#include <curl/curl.h>

#if defined(__APPLE__) || defined(__linux__)
#include "docker.h"
#endif

#ifdef _WIN32
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#endif

#pragma warning( push, 0 )
#include "vtkFloatArray.h"
#include "vtkPointData.h"
#include "vtkPlane.h"
#pragma warning(pop)

using namespace SPP;

static const std::string MAPPED_FOLDER = "/home/yyhx78/cases";
static const std::string containerName = "yyhx78_sppServer";
static const std::string imageName("yyhx78/spps");
static const std::string imageVersion("1.00");
static std::string m_urlRoot;

Connect& Connect::cn()
{
    static Connect cn;
    return cn;
}

Connect::Connect()
{
}

Connect::~Connect()
{
}

void Connect::urlRoot(const char* url)
{
	m_urlRoot = url;
}

const char* Connect::urlRoot()
{
	return m_urlRoot.c_str();
}

//may not work, has not been fully tested.
bool Connect::IsPortInUse(int aPort)
{
#ifdef _WIN32
	return true;
#else
    int portno     = aPort;
    const char *hostname = "localhost";
 
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
 
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        return false;
    }
 
    server = gethostbyname(hostname);
 
    if (server == NULL) {
        return false;
    }
 
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
 
    serv_addr.sin_port = htons(portno);
    
    bool lInUse;
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        lInUse = false;
    } else {
        lInUse = true;
    }
 
    close(sockfd);
    
    return lInUse;
#endif
}

void Connect::mountedFolder(const char* aFolder)
{
    m_mountedFolder = "";
    if (aFolder)
        m_mountedFolder = aFolder;
}

const char* Connect::mountedFolder()
{
    return m_mountedFolder.c_str();
}

namespace
{
	std::string ssystem(const char* command)
	{
        //build a tmp file name
#if 1
        static std::string lTmpFileName;
        if (lTmpFileName.empty())
        {
            auto lTmpFolder = std::filesystem::temp_directory_path();
            lTmpFileName = lTmpFolder.string() + "/yyhx78_tmp.tmp";
        }
        auto tmpname = lTmpFileName.c_str();
#else
        char tmpname[L_tmpnam];
        std::tmpnam(tmpname);
#endif

        //assemble the command
        std::string cmd = std::string(command) + " >> " + tmpname;
        std::system(cmd.c_str());

        //read the tmp file
		std::ifstream file(tmpname, std::ios::in | std::ios::binary);
		std::string result;
		if (file) {
			while (!file.eof())
			{
				char c = file.get();
				if (c >= 0) //some strange characters may trucate the string
					result.push_back(c);
			}
			file.close();
		}
        
		remove(tmpname);
		
        return result;
	}
}

bool Connect::killService(const std::string &aIdName)
{
#if defined(__APPLE__) || defined(__linux__)
	DOCKER *docker = docker_init("v1.41");
    CURLcode response;
    if (docker)
    {
        std::string cmd;
        cmd = "http://v1.41/containers/" + aIdName + "/kill";
 
        const char* data = "";
        response = docker_post(docker, cmd.c_str(), data);
        if (response == CURLE_OK)
        {
            return true;
        }
    }
#else
	std::string cmdStart = "docker kill " + aIdName;
	auto sAll = ssystem(cmdStart.c_str());
#endif
    
    return true;
}


bool Connect::removeService(const std::string &aIdName)
{
#if defined(__APPLE__) || defined(__linux__)
	DOCKER *docker = docker_init("v1.41");
    CURLcode response;
    if (docker)
    {
        std::string cmd;
        cmd = "http://v1.41/containers/" + aIdName;
 
        response = docker_delete(docker, cmd.c_str());
        if (response == CURLE_OK)
        {
            return true;
        }
    }
#else
	std::string cmd = "docker rm " + aIdName;
	auto sAll = ssystem(cmd.c_str());
#endif
    return false;
}

bool Connect::createService(std::string &aPort, std::string &aDataFolder)
{
    bool lStarted = false;
#if defined(__APPLE__) || defined(__linux__)
    DOCKER *docker = docker_init("v1.41");
    if (docker)
    {
        std::string url = "http://v1.41/containers/create?name=" + containerName;

        Json::Value jsonPs;
        jsonPs["Image"] = imageName + ":" + imageVersion;
        jsonPs["HostConfig"]["PortBindings"]["7777/tcp"][0]["HostPort"] = aPort;
        jsonPs["HostConfig"]["Mounts"][0]["Target"] = MAPPED_FOLDER;// + "/damBreak";
        jsonPs["HostConfig"]["Mounts"][0]["Source"] = aDataFolder;
        jsonPs["HostConfig"]["Mounts"][0]["Type"] = "bind";
        jsonPs["HostConfig"]["Mounts"][0]["ReadOnly"] = false;

        std::string sPs = jsonPs.toStyledString();
        const char* dataCreate = sPs.c_str();
        
        CURLcode response = docker_post(docker, url.c_str(), dataCreate);
        if (response == CURLE_OK)
        {
            if (startService())
                lStarted = true;
        }
    }
#else
	std::string cmd = "docker run -idt ";
	cmd += "--name ";
	cmd += containerName;
	cmd += " -v ";
	cmd += aDataFolder; //data folder on host
	cmd += ":" + MAPPED_FOLDER + " -p ";
	cmd += aPort; //port number
	cmd += ":7777 ";
	cmd += imageName + ":" + imageVersion;

	auto sAll = ssystem(cmd.c_str());

	lStarted = true;
#endif
    
    return lStarted;
}

bool Connect::startService()
{
    bool lStarted = false;
#if defined(__APPLE__) || defined(__linux__)
    DOCKER *docker = docker_init("v1.41");
    if (docker)
    {
        std::string cmdStart = "http://v1.41/containers/" + containerName + "/start";
        const char* dataStart = "";
        auto response = docker_post(docker, cmdStart.c_str(), dataStart);
        if (response == CURLE_OK)
        {
			lStarted = true;
        }
    }
#else
	std::string cmdStart = "docker start " + containerName;
	auto sAll = ssystem(cmdStart.c_str());
	lStarted = true;
#endif
    
    return lStarted;
}

bool Connect::findContainerMountSource(const std::string &aId, std::string &aSource)
{
#if defined(__APPLE__) || defined(__linux__)
	std::string cmd = "http://v1.41/containers/" + aId + "/json";
    
    DOCKER *docker = docker_init("v1.41");
    auto response = docker_get(docker, cmd.c_str());
    if (response == CURLE_OK)
    {
        Json::Value jsonData;
        Json::Reader jsonReader;
        if (jsonReader.parse(docker_buffer(docker), jsonData))
        {
            auto mounts = jsonData["Mounts"];
            auto source = mounts[0]["Source"];
            aSource = source.asString();
            return true;
        }
    }
#else
	std::string cmd = "docker inspect " + aId;
	auto sAll = ssystem(cmd.c_str());
	Json::Value jsonData;
	Json::Reader jsonReader;
	//sAll is supposed to be a json
	if (jsonReader.parse(sAll, jsonData))
	{
		auto mounts = jsonData[0]["Mounts"];
		auto source = mounts[0]["Source"];
		aSource = source.asString();
		return true;
	}
#endif
    
    return false;
}

static std::vector<std::string> split(const std::string& s, char seperator)
{
	std::vector<std::string> output;

	std::string::size_type prev_pos = 0, pos = 0;

	while ((pos = s.find(seperator, pos)) != std::string::npos)
	{
		std::string substring(s.substr(prev_pos, pos - prev_pos));

		output.push_back(substring);

		prev_pos = ++pos;
	}

	output.push_back(s.substr(prev_pos, pos - prev_pos)); // Last word

	return output;
}

bool Connect::findDockerContainer(std::string &aId, std::string &aVersion, std::string &aIP, int &aPort, bool &aState)
{
    bool lFound = false;
#if defined(__APPLE__) || defined(__linux__)
    DOCKER *docker = docker_init("v1.41");
    CURLcode response;
    if (docker)
    {
        response = docker_get(docker, "http://v1.41/containers/json?all=1");
        if (response == CURLE_OK)
        {
            Json::Value jsonData;
            Json::Reader jsonReader;
            if (jsonReader.parse(docker_buffer(docker), jsonData))
            {
                if (jsonData.type() == Json::arrayValue)
                {
                    for (Json::Value::ArrayIndex i = 0; i != jsonData.size(); i++)
                    {
                        std::string ss = jsonData[i].toStyledString();
                        aId = jsonData[i]["Id"].asString();
                        auto lContainerName = jsonData[i]["Names"][0].asString();
                        auto lImageName = jsonData[i]["Image"].asString();
                        aVersion = lImageName.substr(lImageName.find(':') + 1, 4);//assume 4 letters: 1.00, 1.10 ...
                        if (   lContainerName.find(containerName) != std::string::npos
                            && lImageName.find(imageName) != std::string::npos)
                        {
                            if (aVersion != imageVersion)
                            {
                                aVersion.clear(); //mark different version has been found
                            }

                            aState = jsonData[i]["State"].asString().find("running") != std::string::npos;
                            auto port = jsonData[i]["Ports"][0];
                            aPort = port["PublicPort"].asInt();
                            aIP = port["IP"].asString();
                            lFound = true;
                            break;
                        }
                    }
                }
            }
        }
    }

    docker_destroy(docker);
#else
	auto sAll = ssystem("docker ps -a");
	std::istringstream f(sAll);

	std::string sLine;
	while (getline(f, sLine))
	{
		if (   std::string::npos != sLine.find(containerName)
			&& std::string::npos != sLine.find(imageName))
		{//found the container
			std::string lNameWithTag = imageName + ":" + imageVersion;
			if (std::string::npos != sLine.find(lNameWithTag))
			{
				aVersion = imageVersion;
			}
			else
			{
				aVersion.clear();
			}
			//state
			aState = std::string::npos == sLine.find("Exited"); //on Windows, if not "Exited", it is "Running" (?)
			//id
			aId = sLine.substr(0, sLine.find(' '));
			//part number
			auto lPosA = sLine.find("->7777/tcp,");
			if (lPosA != std::string::npos)
			{//before this, are the IP and exported port
				auto left = sLine.substr(0, lPosA);
				auto lPosB = left.rfind(' ');
				auto sIpPort = left.substr(lPosB + 1, lPosA - lPosB);
				auto items = split(sIpPort, ':');
				if (items.size() == 2)
				{
					//items[0] is always 0.0.0.0 which is not localhost (127.0.0.1)
//					aIP = items[0];
					aPort = std::atoi(items[1].c_str());
					lFound = true;
					break;
				}
			}

			//the container exists but invalid, remove it
			removeService(aId);
		}
	}
#endif

    return lFound;
}

namespace
{
	std::size_t callback(
		const char* in,
		std::size_t size,
		std::size_t num,
		std::string* out)
	{
		const std::size_t totalBytes(size * num);
		out->append(in, totalBytes);
		return totalBytes;
	}

	bool executeCmd(const char* url, std::string* httpData)
	{
		if (!url || !httpData)
			return false;

		CURL* curl = curl_easy_init();

		// Set remote URL.
		curl_easy_setopt(curl, CURLOPT_URL, url);

		// Don't bother trying IPv6, which would increase DNS resolution time.
		curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);

		// Don't wait forever, time out after 10 seconds.
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120);//large data may take long time

		// Follow HTTP redirects if necessary.
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

		// Response information.
		int httpCode(0);

		// Hook up data handling function.
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);

		// Hook up data container (will be passed as the last parameter to the
		// callback handling function).  Can be any pointer type, since it will
		// internally be passed as a void pointer.
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, httpData);

		curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");

		// Run our HTTP GET command, capture the HTTP response code, and clean up.
		curl_easy_perform(curl);
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
#if defined(__APPLE__)
        //curl_easy_cleanup(curl);//causes crash on Mac
#else
		curl_easy_cleanup(curl);
#endif
		return (httpCode == 200);
	}
}

bool Connect::getCaseList(Json::Value& jsonData)
{
	const std::string cmd("?cmd=2&openfoam");

	const std::string url(m_urlRoot + cmd);

	std::unique_ptr<std::string> httpData(new std::string());
	if (executeCmd(url.c_str(), httpData.get()))
	{
//		std::cout << "Got successful response from " << url << std::endl;

		// Response looks good - done using Curl now.  Try to parse the results
		// and print them out.
		Json::Reader jsonReader;

		if (jsonReader.parse(*httpData, jsonData))
		{
			std::vector<std::string> lCaseList;
			if (jsonData.type() == Json::arrayValue)
			{
				for (Json::Value::ArrayIndex i = 0; i != jsonData.size(); i++)
				{
//					lCaseList.push_back(jsonData[i].toStyledString());
				}
			}
		}
		else
		{
			std::cout << "Could not parse HTTP data as JSON" << std::endl;
			std::cout << "HTTP data was:\n" << *httpData.get() << std::endl;
			return false;
		}
	}
	else
	{
		std::cout << "Couldn't GET from " << url << " - exiting" << std::endl;
		return false;
	}

	return true;
}

bool Connect::getCaseInfo(const char* aCaseFileName, Json::Value& jsonData)
{
	std::string cmd("?cmd=1&study=");
    cmd += aCaseFileName; //the full path
    const std::string url(m_urlRoot + cmd);
    
	std::unique_ptr<std::string> httpData(new std::string());
	if (executeCmd(url.c_str(), httpData.get()))
	{
//		std::cout << "Got successful response from " << url << std::endl;

		// Response looks good - done using Curl now.  Try to parse the results
		// and print them out.
		Json::Reader jsonReader;

		if (jsonReader.parse(*httpData, jsonData))
		{
			std::string s = jsonData.toStyledString();

			if (jsonData.type() == Json::objectValue)
			{
			}
			else
			{
				std::cout << "Wrong value type" << std::endl;
				return false;
			}
		}
		else
		{
			std::cout << "Could not parse HTTP data as JSON" << std::endl;
			std::cout << "HTTP data was:\n" << *httpData.get() << std::endl;
			return false;
		}
	}
	else
	{
		std::cout << "Couldn't GET from " << url << " - exiting" << std::endl;
		return false;
	}

	return true;
}

/*libcurl receives callback from http header */
static size_t OnHeaderData(void* ptr, size_t size, size_t nmemb, void* stream)
{
	if (strncmp((char*)ptr, "Content-Encoding: gzip", strlen("Content-Encoding: gzip")) == 0) {
		*((int*)stream) = 1;
	}

	return size * nmemb;
}

static bool loadPiece(char *&p, int lStartPtId, int iPortion, 
	vtkSmartPointer<vtkPoints> &lPts, 
	vtkSmartPointer<vtkFloatArray>& lTextureCoords,
	vtkSmartPointer<vtkFloatArray>& lNormals,
	vtkSmartPointer<vtkCellArray>& lTriCells,
	vtkSmartPointer<vtkCellArray>& lMeshEdgeCells,
	vtkSmartPointer<vtkCellArray>& lFeatureEdgeCells
	)
{
	//vertex coordinate
	auto pi = (int*)p;
	int nFs = pi[0]; p += sizeof(int);
	if (nFs > 0)
	{
		if (!lPts)
		{
			lPts = vtkSmartPointer<vtkPoints>::New();
		}

		float* pf = (float*)p;
		for (auto i = 0; i < nFs; i += 3)
		{
			lPts->InsertNextPoint(pf[i], pf[i + 1], pf[i + 2]);
		}
		p += nFs * sizeof(float);
	}
	else if (iPortion == 0)
		return false; //vertices are required

	//another integer (number of texture coordinates)
	pi = (int*)p;
	nFs = pi[0];  p += sizeof(int);
	if (nFs > 0)
	{
		if (!lTextureCoords)
		{
			lTextureCoords = vtkSmartPointer<vtkFloatArray>::New();
		}
		float* pf = (float*)p;
		for (auto i = 0; i < nFs; ++i)
		{
			lTextureCoords->InsertNextTuple(pf + i);
		}
		p += nFs * sizeof(float);
	}
	//normals
	pi = (int*)p; p += sizeof(int);
	nFs = pi[0];
	if (nFs > 0)
	{
		if (!lNormals)
		{
			lNormals = vtkSmartPointer<vtkFloatArray>::New();
			lNormals->SetNumberOfComponents(3);
		}
		float* pf = (float*)p;
		for (auto i = 0; i < nFs; i += 3)
		{
			lNormals->InsertNextTuple(pf + i);
		}
		p += nFs * sizeof(float);
	}
	//mesh edges
	pi = (int*)p;
	int nIs = pi[0]; p += sizeof(int);
	if (nIs > 0)
	{
		//the edge index is in unsigned short
		auto pui = (unsigned short*)p;

		if (!lMeshEdgeCells)
		{
			lMeshEdgeCells = vtkSmartPointer<vtkCellArray>::New();
		}
		vtkIdType ids[2];
		for (auto i = 0; i < nIs; i += 2)
		{
			ids[0] = lStartPtId + pui[i];
			ids[1] = lStartPtId + pui[i + 1];
			lMeshEdgeCells->InsertNextCell(2, ids);
		}

		if ((nIs % 2) != 0)
			nIs++; //number of bytes must be even

		p += nIs * sizeof(unsigned short);
	}

	//feature edges
	pi = (int*)p;
	nIs = pi[0]; p += sizeof(int);
	if (nIs > 0)
	{
		//the edge index is in unsigned short
		auto pui = (unsigned short*)p;

		if (!lFeatureEdgeCells)
		{
			lFeatureEdgeCells = vtkSmartPointer<vtkCellArray>::New();
		}

		vtkIdType ids[2];
		for (auto i = 0; i < nIs; i += 2)
		{
			ids[0] = lStartPtId + pui[i];
			ids[1] = lStartPtId + pui[i + 1];
			lFeatureEdgeCells->InsertNextCell(2, ids);
		}

		if ((nIs % 2) != 0)
			nIs++; //number of bytes must be even

		p += nIs * sizeof(unsigned short);
	}

	//triangles
	pi = (int*)p;
	nIs = pi[0]; p += sizeof(int);
	if (nIs > 0)
	{
		//the edge index is in unsigned short
		auto pui = (unsigned short*)p;

		if (!lTriCells)
			lTriCells = vtkSmartPointer<vtkCellArray>::New();

		vtkIdType ids[3];
		for (auto i = 0; i < nIs; i += 3)
		{
			ids[0] = lStartPtId + pui[i];
			ids[1] = lStartPtId + pui[i + 1];
			ids[2] = lStartPtId + pui[i + 2];
			lTriCells->InsertNextCell(3, ids);
		}

		if ((nIs % 2) != 0)
			nIs++; //number of bytes must be even

		p += nIs * sizeof(unsigned short);
	}

	return true;
}
bool Connect::getPart(const char* aCaseFileName, int iPart, int iPortion,
	vtkSmartPointer<vtkPolyData>& aMeshTrisPD, 
	vtkSmartPointer<vtkPolyData>& aMeshEdgePD, 
	vtkSmartPointer<vtkPolyData>& aFeatureEdgePD)
{
	std::string cmd;
	if (iPortion == 0)
		cmd = "?cmd=3&study=" + std::string(aCaseFileName) + "&iPart=" + std::to_string(iPart);
	else
		cmd = "?cmd=8&study=" + std::string(aCaseFileName) + "&iPart=" + std::to_string(iPart) + "&iPortion=" + std::to_string(iPortion);

	const std::string url(m_urlRoot + cmd);

	std::unique_ptr<std::string> httpData(new std::string());
	if (executeCmd(url.c_str(), httpData.get()))
	{
		//they share the same points

		char* p = (char*)httpData.get()->data();

		int* pi = (int*)p;
		int lPartId = pi[0]; p += sizeof(int);
		{//triangles
			vtkSmartPointer<vtkPoints> lPts;
			vtkSmartPointer<vtkFloatArray> lTextureCoords;
			vtkSmartPointer<vtkFloatArray> lNormals;
			vtkSmartPointer<vtkCellArray> lTriCells;
			vtkSmartPointer<vtkCellArray> lMeshEdgeCells, lFeatureEdgeCells;

			pi = (int*)p;
			int nPieces = pi[0]; p += sizeof(int);
			int lStartPtId = 0;
			for (auto iPiece = 0; iPiece < nPieces; ++iPiece)
			{
				if (lPts)
					lStartPtId = lPts->GetNumberOfPoints();

				loadPiece(p, lStartPtId, iPortion,
					lPts, lTextureCoords, lNormals,
					lTriCells, lMeshEdgeCells, lFeatureEdgeCells);
			}

			if (lPts || iPortion != 0)
			{
				if (lTextureCoords || lNormals || lTriCells)
				{
					aMeshTrisPD = vtkSmartPointer<vtkPolyData>::New();
					aMeshTrisPD->SetPoints(lPts);

					if (lTextureCoords)
					{
						aMeshTrisPD->GetPointData()->SetTCoords(lTextureCoords);
					}

					if (lNormals)
					{
						aMeshTrisPD->GetPointData()->SetNormals(lNormals);
					}

					if (lTriCells)
					{
						aMeshTrisPD->SetPolys(lTriCells);

						//part information
						{
							int* pi = (int*)p;
							int lTotoalNumberOfPoints = pi[0]; p += sizeof(int);
							int lTotoalNumberOfTriangles = pi[0]; p += sizeof(int);

							float* pf = (float*)p;
							float bs[6];
							for (int i = 0; i < 6; ++i)
								bs[i] = pf[i];
							p += (6 * sizeof(float));
						}
					}
				}
			}
		}

		{//mesh edges
			vtkSmartPointer<vtkPoints> lPts;
			vtkSmartPointer<vtkFloatArray> lTextureCoords;
			vtkSmartPointer<vtkFloatArray> lNormals;
			vtkSmartPointer<vtkCellArray> lTriCells;
			vtkSmartPointer<vtkCellArray> lMeshEdgeCells, lFeatureEdgeCells;

			pi = (int*)p;
			int nPieces = pi[0]; p += sizeof(int);
			int lStartPtId = 0;
			for (auto iPiece = 0; iPiece < nPieces; ++iPiece)
			{
				if (lPts)
					lStartPtId = lPts->GetNumberOfPoints();

				loadPiece(p, lStartPtId, iPortion,
					lPts, lTextureCoords, lNormals,
					lTriCells, lMeshEdgeCells, lFeatureEdgeCells);
			}

			if (lPts || iPortion != 0)
			{
				if (lMeshEdgeCells)
				{
					aMeshEdgePD = vtkSmartPointer<vtkPolyData>::New();
					aMeshEdgePD->SetPoints(lPts);
					aMeshEdgePD->SetLines(lMeshEdgeCells);
				}
			}
		}

		{//feature edges
			vtkSmartPointer<vtkPoints> lPts;
			vtkSmartPointer<vtkFloatArray> lTextureCoords;
			vtkSmartPointer<vtkFloatArray> lNormals;
			vtkSmartPointer<vtkCellArray> lTriCells;
			vtkSmartPointer<vtkCellArray> lMeshEdgeCells, lFeatureEdgeCells;

			pi = (int*)p;
			int nPieces = pi[0]; p += sizeof(int);
			int lStartPtId = 0;
			for (auto iPiece = 0; iPiece < nPieces; ++iPiece)
			{
				if (lPts)
					lStartPtId = lPts->GetNumberOfPoints();

				loadPiece(p, lStartPtId, iPortion,
					lPts, lTextureCoords, lNormals,
					lTriCells, lMeshEdgeCells, lFeatureEdgeCells);
			}

			if (lPts || iPortion != 0)
			{
				if (lFeatureEdgeCells)
				{
					aFeatureEdgePD = vtkSmartPointer<vtkPolyData>::New();
					aFeatureEdgePD->SetPoints(lPts);
					aFeatureEdgePD->SetLines(lFeatureEdgeCells);
				}
			}
		}
	}
	else
	{
		std::cout << "Couldn't GET from " << url << " - exiting" << std::endl;
		return false;
	}

	return true;
}

bool Connect::getResult(const char* aCaseFileName, int iRlt, int iIndp,
	float aScalarRange[],
	std::vector<vtkSmartPointer<vtkPolyData> >& aPdArrays, //return streamlines
	std::vector<vtkSmartPointer<vtkFloatArray> >& aRltArrays,
	std::vector<vtkSmartPointer<vtkFloatArray> >& aDispArrays)
{
	std::string cmd = "?cmd=4&study=" + std::string(aCaseFileName) + "&iRlt=" + std::to_string(iRlt) + "&iIndp=" + std::to_string(iIndp);

	const std::string url(m_urlRoot + cmd);

	std::unique_ptr<std::string> httpData(new std::string());
	if (executeCmd(url.c_str(), httpData.get()))
	{
		char* p = (char*)httpData.get()->data();
		//the first 3 numbers are integers
		int *pi = (int*)p;
		int nParts = pi[0]; p += sizeof(int);

		float* pf = (float*)p;
		aScalarRange[0] = pf[0];
		aScalarRange[1] = pf[1];
		p += 2 * sizeof(float);

		aRltArrays.resize(nParts);
		aDispArrays.resize(nParts);
		for (auto iPart = 0; iPart < nParts; ++iPart)
		{
			//element status: 
			// 0: mesh has not been modified, only result in the return
			// 1: mesh has been modified and included in the return 
			// 2: streamline               
			pi = (int*)p;
			int iElementStatusChanged = pi[0]; p += sizeof(int);

			if (iElementStatusChanged == 2)
			{//streamlines
				vtkSmartPointer<vtkPoints> lPts;
				vtkSmartPointer<vtkFloatArray> lTextureCoords;
				vtkSmartPointer<vtkFloatArray> lNormals;
				vtkSmartPointer<vtkCellArray> lMeshEdgeCells, lFeatureEdgeCells, lTriCells;

				pi = (int*)p;
				int nPieces = pi[0]; p += sizeof(int);
				int lStartPtId = 0;
				for (auto iPiece = 0; iPiece < nPieces; ++iPiece)
				{
					if (lPts)
						lStartPtId = lPts->GetNumberOfPoints();

					loadPiece(p, lStartPtId, 0,
						lPts, lTextureCoords, lNormals,
						lTriCells, lMeshEdgeCells, lFeatureEdgeCells);

					if (lPts && lMeshEdgeCells)
					{
						auto lPD = vtkSmartPointer<vtkPolyData>::New();
						lPD->SetPoints(lPts);
						lPD->SetLines(lMeshEdgeCells);
						lPD->GetPointData()->SetScalars(lTextureCoords);
						aPdArrays.push_back(lPD);
					}
				}
			}
			else
			{
				vtkSmartPointer<vtkFloatArray> lTextureCoords;
				vtkSmartPointer<vtkFloatArray> lDisplacements;

				pi = (int*)p;
				int nPieces = pi[0]; p += sizeof(int);
				for (auto iPiece = 0; iPiece < nPieces; ++iPiece)
				{
					//texture coordinates
					pi = (int*)p;
					int nFs = pi[0]; p += sizeof(int);
					if (nFs > 0)
					{
						if (!lTextureCoords)
						{
							lTextureCoords = vtkSmartPointer<vtkFloatArray>::New();
						}
						float* pf = (float*)p;
						for (auto i = 0; i < nFs; ++i)
						{
							lTextureCoords->InsertNextTuple(pf + i);
						}
						p += nFs * sizeof(float);
					}

					//displacements, include if exist
					pi = (int*)p; p += sizeof(int);
					nFs = pi[0];
					if (nFs > 0)
					{
						if (!lDisplacements)
						{
							lDisplacements = vtkSmartPointer<vtkFloatArray>::New();
							lDisplacements->SetNumberOfComponents(3);
						}
						float* pf = (float*)p;
						for (auto i = 0; i < nFs; i += 3)
						{
							lDisplacements->InsertNextTuple(pf + i);
						}
						p += nFs * sizeof(float);
					}
				}

				aRltArrays[iPart] = lTextureCoords;
				aDispArrays[iPart] = lDisplacements;
			}
		}
	}
	else
	{
		std::cout << "Couldn't GET from " << url << " - exiting" << std::endl;
		return false;
	}

	return true;
}

bool Connect::getSlicesOnParts(const char* aCaseFileName, vtkPlane* aPlane, int aRltIdx,
	std::vector<vtkSmartPointer<vtkPolyData> >& aSlicePDs,
	std::vector<vtkSmartPointer<vtkPolyData> >& aSliceMeshEdgePDs)
{
	if (!aPlane)
		return false;

	double* lOrigin = aPlane->GetOrigin();
	double lNormal[3];
	aPlane->GetNormal(lNormal);
	vtkMath::Normalize(lNormal);
	double d = vtkMath::Dot(lOrigin, lNormal);

	std::string comma(",");
	std::string lPlaneStr(
		std::to_string(lNormal[0]) + comma +
		std::to_string(lNormal[1]) + comma +
		std::to_string(lNormal[2]) + comma + std::to_string(d)
	);

	std::string cmd = "?cmd=5&study=" + std::string(aCaseFileName) + "&plane=" + lPlaneStr + "&iRlt=" + std::to_string(aRltIdx);

	const std::string url(m_urlRoot + cmd);

	std::unique_ptr<std::string> httpData(new std::string());
	if (executeCmd(url.c_str(), httpData.get()))
	{
		char* p = (char*)httpData.get()->data();
		//the first 3 numbers are integers
		int* pi = (int*)p;
		int nCuts = pi[0]; p += sizeof(int);

		for (auto iCut = 0; iCut < nCuts; ++iCut)
		{
			//they share the same points
			vtkSmartPointer<vtkPoints> lPts;
			vtkSmartPointer<vtkFloatArray> lTextureCoords;
			vtkSmartPointer<vtkFloatArray> lNormals;
			vtkSmartPointer<vtkCellArray> lMeshEdgeCells, lFeatureEdgeCells, lTriCells;

			pi = (int*)p;
			int nPieces = pi[0]; p += sizeof(int);
			int lStartPtId = 0;
			for (auto iPiece = 0; iPiece < nPieces; ++iPiece)
			{
				if (lPts)
					lStartPtId = lPts->GetNumberOfPoints();

				loadPiece(p, lStartPtId, 0,
					lPts, lTextureCoords, lNormals,
					lTriCells, lMeshEdgeCells, lFeatureEdgeCells);

				if (lPts && lTriCells)
				{
					auto lPD = vtkSmartPointer<vtkPolyData>::New();
					lPD->SetPoints(lPts);
					lPD->SetPolys(lTriCells);
					lPD->GetPointData()->SetScalars(lTextureCoords);
					aSlicePDs.push_back(lPD);
				}
				if (lPts && lMeshEdgeCells)
				{
					auto lPD = vtkSmartPointer<vtkPolyData>::New();
					lPD->SetPoints(lPts);
					lPD->SetPolys(lMeshEdgeCells);
					aSliceMeshEdgePDs.push_back(lPD);
				}
			}

		}
	}
	else
	{
		std::cout << "Couldn't GET from " << url << " - exiting" << std::endl;
		return false;
	}

	return true;
}

bool Connect::getIsoSurfaces(const char* aCaseFileName, int aNbIsoSs, int aRltIdx,
                             std::vector<float>& aIsoClipValues,
	std::vector<vtkSmartPointer<vtkPolyData> >& aIsoPDs)
{
	if (aNbIsoSs < 1 || aRltIdx < 0 || !aCaseFileName)
		return false;

	std::string cmd = "?cmd=6&study=" + std::string(aCaseFileName) + "&nIso=" + std::to_string(aNbIsoSs) + "&iRlt=" + std::to_string(aRltIdx);
	const std::string url(m_urlRoot + cmd);

	std::unique_ptr<std::string> httpData(new std::string());
	if (executeCmd(url.c_str(), httpData.get()))
	{
		char* p = (char*)httpData.get()->data();

		//clip values
		int* pi = (int*)p;
		int nClipValues = pi[0]; p += sizeof(int);
        aIsoClipValues.resize(nClipValues);
		float* pf = (float*)p;
		for (int i = 0; i < nClipValues; ++i)
		{
			aIsoClipValues[i] = pf[i];
		}
		p += (nClipValues * sizeof(float));
		
		//clip PDs
		pi = (int*)p;
		int nIsoPDs = pi[0]; p += sizeof(int);
		for (auto iIsoPD = 0; iIsoPD < nIsoPDs; ++iIsoPD)
		{
			//they share the same points
			vtkSmartPointer<vtkPoints> lPts;
			vtkSmartPointer<vtkFloatArray> lTextureCoords;
			vtkSmartPointer<vtkFloatArray> lNormals;
			vtkSmartPointer<vtkCellArray> lMeshEdgeCells, lFeatureEdgeCells, lTriCells;

			pi = (int*)p;
			int nPieces = pi[0]; p += sizeof(int);
			int lStartPtId = 0;
			for (auto iPiece = 0; iPiece < nPieces; ++iPiece)
			{
				if (lPts)
					lStartPtId = lPts->GetNumberOfPoints();

				loadPiece(p, lStartPtId, 0,
					lPts, lTextureCoords, lNormals,
					lTriCells, lMeshEdgeCells, lFeatureEdgeCells);

				if (lPts && (lTriCells || lMeshEdgeCells))
				{
					auto lPD = vtkSmartPointer<vtkPolyData>::New();
					lPD->SetPoints(lPts);
					lPD->SetPolys(lTriCells);
					lPD->SetLines(lMeshEdgeCells); //iso-lines (if the mesh is 2d)
					lPD->GetPointData()->SetScalars(lTextureCoords);
					aIsoPDs.push_back(lPD);
				}
			}

		}
	}
	else
	{
		std::cout << "Couldn't GET from " << url << " - exiting" << std::endl;
		return false;
	}

	return true;
}

bool Connect::doExport()
{
	//12: export
	std::string cmd = "?cmd=12";
	const std::string url(m_urlRoot + cmd);

	std::unique_ptr<std::string> httpData(new std::string());
	if (executeCmd(url.c_str(), httpData.get()))
		return true;

	return false;
}
