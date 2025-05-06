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
			BODY_CHUNKED,
			BODY_END,
			FINISH        // Replaces BODY_COMPLETE
		};

		enum e_multipart_state {
			PART_HEADER,
			PART_DATA,
			PART_BOUNDARY,
			PART_END
		};
		enum t_chunk_state
		{
			CHUNK_SIZE,
			CHUNK_DATA,
			CHUNK_END,
			CHUNK_FINISHED
		};

		bool _hasCgi;
		const std::string& getUri() const;
		const std::string& getQuery() const;
		const std::string& getBody() const;
		const std::string& getMethod() const;
		const std::string& getPath() const;
		const std::string& getHeaderValue(const std::string& key) const;
		int getState() const;
		int getStatusCode() const;
		void setStatusCode(int code);
		void setState(ParseState state);
		bool validateClientMaxBodySize();
		bool validateTransferEncoding();
		bool validateAllowedMethods();
		bool validateMultipartFormData();

		bool isCgiRequest();

		bool keepAlive() const;
		const LocationConfig* findLocationByPath(const std::string& path) const;
		void findLocation();
		const LocationConfig* getMatchedLocation() const;
		const LocationConfig* getFinalLocation() const;
		void parse(std::string& rawdata);
		void parseMethod(std::string& rawdata);
		void parseUri(std::string& rawdata);
		void parseVersion(std::string& rawdata);
		void parseHeadersKey(std::string& rawdata);
		void parseHeadersValue(std::string& rawdata);
		void parseRequestBody(std::string& rawdata);

		void parseMultipartHeaders();
		void parseMultipartBody(std::string& rawdata);
		void handlePartData(const std::string& data);

		void debugPrintRequest();

		bool isMultipartInFinalState() const;

		bool processPartHeader(std::string& rawdata);
		bool processPartBoundary(std::string& rawdata);
		bool processPartData(std::string& rawdata);

		void parseChunkBody(std::string& rawdata);
		bool hasCgi(void);
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
		std::string _HeaderKey;
		std::string _HeaderValue;
		std::string _request;
		std::map<std::string, std::string> _headers;
		std::string _contentType; // Note: unused, consider removing
		std::string _boundary;    // Note: unused, consider removing
		bool _isChunked;
		bool _keepAlive;
		size_t			_chunkSize;
		std::string _multipartBoundary;
		const LocationConfig* _matchedLocation;
		const LocationConfig* _location;
		 e_multipart_state _multipartState;      // New: Multipart parsing state

		int _currentPartFd;           // File descriptor for writing file

		std::ofstream _uploadFile; // Used to write uploaded file content
		bool _uploadHeadersParsed;
		std::string _uploadBoundary;
		std::string _uploadFilepath;
		std::string _partialBoundary;
		t_chunk_state	_chunkState;
		std::string		_chunkBuffer;
		std::string		_chunkFilePath;
		bool _chunkFileInitialized;
		void closeCurrentFile(); // New: Helper to close file stream

					 //
};


#endif // HTTPREQUEST_HPP
