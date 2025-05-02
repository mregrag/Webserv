/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   Created: 2025/04/10 16:09:58 by mregrag           #+#    #+#             */
/*   Updated: 2025/04/29 00:16:16 by mregrag          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <cstddef>
#include <string>
#include <map>
#include <fstream>
#include <vector>
#include "../include/LocationConfig.hpp"

class Client;

class HTTPRequest 
{
	public:
		HTTPRequest();
		HTTPRequest(Client* client);
		HTTPRequest(const HTTPRequest& other);
		HTTPRequest& operator=(const HTTPRequest& other);
		~HTTPRequest();
		enum ParseState {
			INIT,
			LINE_METHOD,
			LINE_URI,
			LINE_VERSION,
			LINE_END,
			HEADERS_INIT,
			HEADER_KEY,
			HEADER_VALUE,
			HEADERS_END,
			BODY_INIT,
			BODY_MULTIPART,
			BODY_MULTIPART_DATA,
			BODY_MULTIPART_HEADER,
			BODY_CONTENT_LENGTH,
			BODY_END,
			FINISH        // Replaces BODY_COMPLETE
		};

		enum e_multipart_state {
			PART_HEADER,
			PART_DATA,
			PART_BOUNDARY,
			PART_END
		};
		struct PartInfo {
			std::string name;
			std::string filename;
			bool is_file;

			PartInfo() : is_file(false) {}
		};
		const std::string& getUri() const;
		const std::string& getBody() const;
		const std::string& getMethod() const;
		const std::string& getPath() const;
		const std::string& getHeaderValue(const std::string& key) const;
		int getState() const;
		int getStatusCode() const;
		void setStatusCode(int code);
		void setState(ParseState state);
		int checkCgi();
		bool validateClientMaxBodySize();
		bool validateTransferEncoding();
		bool validateAllowedMethods();
		bool keepAlive() const;
		const LocationConfig* findLocationByPath(const std::string& path) const;
		void findLocation();
		const LocationConfig* getMatchedLocation() const;
		const LocationConfig* getFinalLocation() const;
		void parse(std::string& rawdata);
		void parseRequestHeader();
		void parseRequestLine();
		void parseMethod(std::string& rawdata);
		void parseUri(std::string& rawdata);
		void parseVersion(std::string& rawdata);
		void parseHeadersKey(std::string& rawdata);
		void parseHeadersValue(std::string& rawdata);
		void parseRequestBody(std::string& rawdata);
		bool isMultipartFormData();
		void parseMultipartBody();

		void parseMultipartHeaders();
		void parseMultipartBody(std::string& rawdata);
		void handlePartData(const std::string& data);

		std::string extractBoundary(const std::string& contentType);
		void debugPrintRequest();
		void handleFileUpload(const std::string& filename, const std::string& content);
		

		bool isMultipartUpload() const;

		bool isMultipartInFinalState() const;

		bool processNextPart();
		bool initializeBoundary();

		
		void parsePartHeaders(const std::string& headers, std::string& filename, std::string& name);
		std::string generateUniqueFilename(const std::string& path);
		bool parseContentDisposition(const std::string& headers, PartInfo& part_info);
		std::string ensureTrailingSlash(const std::string& path);

		std::string generateUniquePath(const std::string& path, const std::string& filename);
		void processPartContent(const std::string& headers, size_t content_start, size_t content_end);
		void initBoundary();

		bool fileExists(const std::string& path);



		bool processPartHeader(std::string& rawdata);

		bool processPartBoundary(std::string& rawdata);
		bool processPartData(std::string& rawdata);
	private:

		Client* _client;
		int _statusCode;
		ParseState _state;
		size_t _parsePosition;
		size_t _contentLength;
		std::string _method;
		std::string _uri;
		std::string _path;
		std::string _query;
		std::string _protocol;
		std::string _bodyBuffer;
		std::string _tmpHeaderKey;
		std::string _tmpHeaderValue;
		std::string _request;
		std::map<std::string, std::string> _headers;
		std::string _contentType; // Note: unused, consider removing
		std::string _boundary;    // Note: unused, consider removing
		bool _isChunked;
		std::string _host;        // Note: unused, consider removing
		bool _keepAlive;
		int _chunkSize;
		std::string _multipartBoundary;
		const LocationConfig* _matchedLocation;
		const LocationConfig* _finalLocation;
		 e_multipart_state _multipartState;      // New: Multipart parsing state

		std::string _multipartBuffer;  // Buffer for partial multipart data
		std::string _currentPartName;  // Current part's "name" attribute
		std::string _currentPartFilename; // Current part's "filename" attribute
		int _currentPartFd;           // File descriptor for writing file
		std::string _currentPartHeaders; // Buffer for pa
		// Storage for form fields and files
		std::map<std::string, std::string> _formFields;
		std::vector<std::string> _uploadedFiles;
		size_t _bodysize;

		std::ofstream _uploadFile; // Used to write uploaded file content
		bool _uploadWriting;
		bool _uploadHeadersParsed;
		std::string _uploadBoundary;
		std::string _uploadFilepath;
		void closeCurrentFile(); // New: Helper to close file stream
					 //
};


#endif // HTTPREQUEST_HPP
