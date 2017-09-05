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

class irsdkCSVClient
{
public:

	irsdkCSVClient()
		: m_csvFile(NULL)
		, m_varHeaders(NULL)
		, m_varScale(NULL)
		, m_varBuf(NULL)
		, m_varCount(0)
	{
	}

	irsdkCSVClient(const char *path)
		: m_csvFile(NULL)
		, m_varHeaders(NULL)
		, m_varScale(NULL)
		, m_varBuf(NULL)
		, m_varCount(0)
	{
		openFile(path);
	}

	~irsdkCSVClient() { closeFile(); }

	bool isFileOpen() { return m_csvFile != NULL; }
	bool openFile(const char *path);
	void closeFile();

	// read next line out of file
	bool getNextData();

	int getVarIdx(const char *name);

	float getVarFloat(int idx);
	float getVarFloat(const char *name) { return getVarFloat(getVarIdx(name)); }

	int getVarCount() { return m_varCount; }
	const irsdk_varHeader* getVarHeaders() { return m_varHeaders; }
	const float* getVarArray() { return m_varBuf; }

protected:
	enum line_type
	{
		ltMisc = 0,
		ltHeader,
		ltUnit,
		ltConvert,
		ltData
	};

	bool parseDataLine(char* line);
	void parceNameAndUnit(const char *str, irsdk_varHeader &head, int &varOffset);
	irsdkCSVClient::line_type getLineType(const char *str, bool &hasHeader, bool &hasUnit);
	char* stripEnds(char *st);
	char* getNextElement(char *baseStr);
	float strToFloat(const char *str);


	// o = i * mul + add;
	struct scale
	{
		float mul;
		float add;
	};

	int m_varCount;
	irsdk_varHeader *m_varHeaders;
	scale *m_varScale;
	float *m_varBuf;

	FILE *m_csvFile;
};

#endif // IRSDKDISKCLIENT_H
