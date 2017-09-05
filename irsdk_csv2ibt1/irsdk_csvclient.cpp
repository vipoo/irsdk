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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <assert.h>
#include "irsdk_defines.h"
#include "irsdk_csvclient.h"

#pragma warning(disable:4996)

#define MAX_LINE 20000

int timeOffset = -1;
int sessionTimeOffset = -1;
float timeCount = 0;
bool hasSessionTime = false;


bool irsdkCSVClient::openFile(const char *path)
{
	closeFile();

	m_csvFile = fopen(path, "r");
	if(m_csvFile)
	{
		char line[MAX_LINE];
		bool hasHeader = false;
		bool hasUnit = false;

		// grab the header
		while(fgets(line, MAX_LINE, m_csvFile) && strlen(line) > 0)
		{
			line[MAX_LINE-1] = '\0';
			line_type type = getLineType(line, hasHeader, hasUnit);

			if(type == ltHeader)
			{
				int len = (int)strlen(line);
				// guess at how many entrys there are
				int count = 2; // there must be at least one, we hope, and we need room for SessionTime
				for(int i=0; i < len; i++)
				{
					if(line[i] == ',')
					{
						count++;
					}
				}

				// allocate memory
				m_varBuf = new float[count];
				m_varHeaders = new irsdk_varHeader[count];
				m_varScale = new scale[count];


				if(m_varHeaders && m_varBuf && m_varScale)
				{
					memset(m_varBuf, 0, sizeof(float) * count);
					memset(m_varHeaders, 0, sizeof(irsdk_varHeader) * count);
					for(int i=0; i<count; i++)
					{
						m_varScale[i].mul = 1;
						m_varScale[i].add = 0;
					}
					m_varCount = 0;

					int offset = 0;
					char *st = getNextElement(line);
					while(st != NULL && m_varCount < count)
					{
						parceNameAndUnit(st, m_varHeaders[m_varCount], offset);
						if(strcmp(m_varHeaders[m_varCount].name, "SessionTime") == 0)
							hasSessionTime = true;
						else if(strcmp(m_varHeaders[m_varCount].name, "Time") == 0)
							timeOffset = m_varCount;
						m_varCount++;

						st = getNextElement(NULL);
					}

					// make sure thers is a variabl called SessionTime
					if(!hasSessionTime)
					{
						parceNameAndUnit("SessionTime[s]", m_varHeaders[m_varCount], offset);
						sessionTimeOffset = m_varCount;
						m_varCount++;
					}
				}
			}
			else if(type == ltUnit)
			{
					int count = 0;
					char *st = getNextElement(line); 
					while(st != NULL && count < m_varCount)
					{
						st = stripEnds(st);
						strncpy(m_varHeaders[count].unit, st, IRSDK_MAX_STRING);
						m_varHeaders[count].unit[IRSDK_MAX_STRING-1] = '\0';

						count++;
						st = getNextElement(NULL);
					}
			}
			else if(type == ltConvert)
			{
					int count = 0;
					char *st = getNextElement(line); 
					while(st != NULL && count < m_varCount)
					{
						if(st[0])
						{
							char *s;
							s = strchr(st, '*');
							if(s && s[0])
								m_varScale[count].mul = (float)atof((s+1));

							s = strchr(st, '+');
							if(s && s[0])
								m_varScale[count].add = (float)atof((s+1));
						}

						count++;
						st = getNextElement(NULL);
					}
			}
			else if(type == ltData)
			{
				parseDataLine(line);
				return true;
			}
		}
		fclose(m_csvFile);
		m_csvFile = NULL;
	}

	return false;
}

void irsdkCSVClient::closeFile()
{
	if(m_varBuf)
		delete [] m_varBuf;
	m_varBuf = NULL;

	if(m_varHeaders)
		delete [] m_varHeaders;
	m_varHeaders = NULL;

	if(m_varScale)
		delete [] m_varScale;
	m_varScale = NULL;

	if(m_csvFile)
		fclose(m_csvFile);
	m_csvFile = NULL;
}


bool irsdkCSVClient::parseDataLine(char* line)
{
	if(m_varBuf != NULL)
	{
		// clear out old data
		memset(m_varBuf, 0, sizeof(float)*m_varCount);

		if(line)
		{
			int i = 0;
			char *st = getNextElement(line);
			while(st != NULL && i < m_varCount)
			{
				m_varBuf[i] = strToFloat(st);
				m_varBuf[i] = m_varBuf[i] * m_varScale[i].mul;
				m_varBuf[i] = m_varBuf[i] + m_varScale[i].add;
				i++;

				st = getNextElement(NULL);
			}

			// copy or fake up a session time entry
			if(!hasSessionTime)
			{
				if(timeOffset > -1)
					m_varBuf[sessionTimeOffset] = m_varBuf[timeOffset];
				else
				{
					m_varBuf[sessionTimeOffset] = timeCount;
					timeCount += (1.0f / 60.0f);
				}
			}

			return true;
		}
	}

	return false;
}

bool irsdkCSVClient::getNextData()
{
	if(m_csvFile != NULL)
	{
		char line[MAX_LINE];
		if(fgets(line, MAX_LINE, m_csvFile) && strlen(line) > 0)
		{
			line[MAX_LINE-1] = '\0';
			return parseDataLine(line);
		}
	}

	return false;
}

int irsdkCSVClient::getVarIdx(const char *name)
{
	if(m_csvFile && name)
	{
		for(int idx=0; idx<m_varCount; idx++)
		{
			if(0 == strncmp(name, m_varHeaders[idx].name, IRSDK_MAX_STRING))
			{
				return idx;
			}
		}
	}

	return -1;
}

float irsdkCSVClient::getVarFloat(int idx)
{
	if(m_csvFile)
	{
		if(idx >= 0 && idx < m_varCount)
			return m_varBuf[m_varHeaders[idx].offset];
		else //invalid variable index
			assert(false);
	}

	return 0.0f;
}

void irsdkCSVClient::parceNameAndUnit(const char *str, irsdk_varHeader &head, int &varOffset)
{
	// deal with strings like ' name with space [ unit ] '
	// turn it into 'namewithspace' and 'unit'

	head.name[0] = '\0';
	head.unit[0] = '\0';
	head.type = irsdk_float;
	head.count = 1;
	head.desc[0] = '\0';
	head.offset = varOffset;
	varOffset += 4;

	char *buf = head.name;
	int offset = 0;
	bool isUnit = false;

	if(str)
	{
		while(str[0] != '\0')
		{
			if(str[0] == '[')
			{
				buf = head.unit;
				offset = 0;
				isUnit = true;
			}
			else if(str[0] == ']' || str[0] == ' ' || str[0] == '\t')
			{
				// do nothing
			}
			else if(isUnit || isalnum((unsigned char)str[0]))
			{
				if(offset < (IRSDK_MAX_STRING-1))
				{
					buf[offset] = str[0];
					offset++;
					buf[offset] = '\0';
				}
			}
			// else throw it away

			str++;
		}
	}
}



// misc: scan for delimiters, if less than two found then toss it as a comment
// data: scann for numbers, if only 0-9. and delimiter found then is data, keep a ratio
// scale: scann for * and + characters, if header found then this is scale
// header comes before unit.
irsdkCSVClient::line_type irsdkCSVClient::getLineType(const char *str, bool &hasHeader, bool &hasUnit)
{
	if(str)
	{
		int delimCount = 0;
		int unitCount = 0;
		int cvtCount = 0;
		int numCount = 0;
		int alphaCount = 0;
		int spaceCount = 0;
		int miscCount = 0;

		int len = strlen(str);
		for(int i=0; i<len; i++)
		{
			if(str[i] == ',')
				delimCount++;
			else if(str[i] == '[' || str[i] == ']')
				unitCount++;
			else if(str[i] == '*' || str[i] == '+')
				cvtCount++;
			else if((str[i] >= '0' && str[i] <= '9') || str[i] == '.' || str[i] == '-')
				numCount++;
			else if(isalnum((unsigned char)str[i]))
				alphaCount++;
			else if(str[i] == ' ' || str[i] == '\t')
				spaceCount++;
			else
				miscCount++;
		}

		if(delimCount < 2) // no delimiters, then junk line
			return ltMisc;
		// else it is something

		// check if we have units declared in brackets after our name like name1[unit1],name2[unit2]
		// if we find brackets then we can assume it is a header
		if(!hasHeader && unitCount > delimCount)
		{
			hasUnit = true;
			hasHeader = true;
			return ltHeader;
		}

		// assume first non junk line is header
		// we could try to detect unit conversions first, but for now this works
		if(!hasHeader)
		{
			hasHeader = true;
			return ltHeader;
		}

		// if we find +* and numbers then assume it is a unit conversion line
		if(cvtCount > 0 && numCount > 0)
			return ltConvert;

		// if line is mostly made up of numbers, and there are enough numbers for one in each column
		// then assume this is data.
		if(numCount > delimCount && numCount > (4 * alphaCount))
			return ltData;

		else if(!hasUnit && alphaCount > delimCount)
		{
			hasUnit = true;
			return ltUnit;
		}

		return ltMisc;

	}

	return ltMisc;
}

char* irsdkCSVClient::stripEnds(char *st)
{
	if(st)
	{
		while(st[0] == ' ' || st[0] == '\t')
			st++;
		while(st[0] && (st[strlen(st)-1] == ' ' || st[strlen(st)-1] == '\t'))
			st[strlen(st)-1] = '\0';
	}

	return st;
}

char* irsdkCSVClient::getNextElement(char *baseStr)
{
	static char* str;

	// reset string pointer, if provided
	if(baseStr)
		str = baseStr;

	char *s = str;
	if(str)
	{
		while(str[0] && str[0] != ',')
		{
			str++;
		}

		if(str[0] == ',')
		{
			str[0] = '\0';
			str++;
		}
		else if(str[0] == '\0')
			s = NULL;
	}
	return s;
}

float irsdkCSVClient::strToFloat(const char *str)
{
	if(str)
	{
		while(str[0] && (str[0] == ' ' || str[0] == '\t'))
			str++;
		if(str[0])
			return (float)atof(str);
	}

	return 0.0f;
}