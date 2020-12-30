/*
Copyright (c) 2013, iRacing.com Motorsport Simulations, LLC.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of iRacing.com Motorsport Simulations nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef IRSDKDISKCLIENT_H
#define IRSDKDISKCLIENT_H

// A C++ wrapper around the irsdk calls that takes care of reading a .ibt file

class irsdkDiskClient
{
public:

	irsdkDiskClient()
		: m_ibtFile(NULL)
		, m_sessionInfoString(NULL)
		, m_varHeaders(NULL)
		, m_varBuf(NULL)
	{
		memset(&m_header, 0, sizeof(m_header));
		memset(&m_diskSubHeader, 0, sizeof(m_diskSubHeader));
	}

	irsdkDiskClient(const char *path)
		: m_ibtFile(NULL)
		, m_sessionInfoString(NULL)
		, m_varHeaders(NULL)
		, m_varBuf(NULL)
	{
		memset(&m_header, 0, sizeof(m_header));
		memset(&m_diskSubHeader, 0, sizeof(m_diskSubHeader));

		openFile(path);
	}

	~irsdkDiskClient() { closeFile(); }

	bool isFileOpen() { return m_ibtFile != NULL; }
	bool openFile(const char *path);
	void closeFile();

	// read next line out of file
	bool getNextData();
	int getDataCount() { return m_diskSubHeader.sessionRecordCount; }

	int getVarIdx(const char *name);

	// what is the base type of the data
	irsdk_VarType getVarType(int idx);
	irsdk_VarType getVarType(const char *name) { return getVarType(getVarIdx(name)); }

	// how many elements in array, or 1 if not an array
	int getVarCount(int idx);
	int getVarCount(const char *name) { return getVarCount(getVarIdx(name)); }

	// idx is the variables index, entry is the array offset, or 0 if not an array element
	// will convert data to requested type
	bool getVarBool(int idx, int entry = 0);
	bool getVarBool(const char *name, int entry = 0) { return getVarBool(getVarIdx(name), entry); }

	int getVarInt(int idx, int entry = 0);
	int getVarInt(const char *name, int entry = 0) { return getVarInt(getVarIdx(name), entry); }
	
	float getVarFloat(int idx, int entry = 0);
	float getVarFloat(const char *name, int entry = 0) { return getVarFloat(getVarIdx(name), entry); }

	double getVarDouble(int idx, int entry = 0);
	double getVarDouble(const char *name, int entry = 0) { return getVarDouble(getVarIdx(name), entry); }

	// 1 success, 0 failure, -n minimum buffer size
	int getSessionStrVal(const char *path, char *val, int valLen);

protected:

	irsdk_header m_header;
	irsdk_diskSubHeader m_diskSubHeader;

	char *m_sessionInfoString;
	irsdk_varHeader *m_varHeaders;
	char *m_varBuf;

	FILE *m_ibtFile;
};

#endif // IRSDKDISKCLIENT_H
