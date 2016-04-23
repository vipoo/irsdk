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

#include <assert.h>
#include "irsdk_defines.h"
#include "yaml_parser.h"
#include "irsdk_diskclient.h"

#pragma warning(disable:4996)


bool irsdkDiskClient::openFile(const char *path)
{
	closeFile();

	m_ibtFile = fopen(path, "rb");
	if(m_ibtFile)
	{
		if(fread(&m_header, 1, sizeof(m_header), m_ibtFile) == sizeof(m_header))
		{
			if(fread(&m_diskSubHeader, 1, sizeof(m_diskSubHeader), m_ibtFile) == sizeof(m_diskSubHeader))
			{
				m_sessionInfoString = new char[m_header.sessionInfoLen];
				if(m_sessionInfoString)
				{
					fseek(m_ibtFile, m_header.sessionInfoOffset, SEEK_SET);
					if(fread(m_sessionInfoString, 1, m_header.sessionInfoLen, m_ibtFile) == m_header.sessionInfoLen)
					{
						m_sessionInfoString[m_header.sessionInfoLen-1] = '\0';

						m_varHeaders = new irsdk_varHeader[m_header.numVars];
						if(m_varHeaders)
						{
							fseek(m_ibtFile, m_header.varHeaderOffset, SEEK_SET);
							int len = m_header.numVars * sizeof(irsdk_varHeader);
							if(fread(m_varHeaders, 1, len, m_ibtFile) == len)
							{
								m_varBuf = new char[m_header.bufLen];
								if(m_varBuf)
								{
									fseek(m_ibtFile, m_header.varBuf[0].bufOffset, SEEK_SET);

									return true;

									//delete [] m_varBuf;
									//m_varBuf = NULL;
								}
							}

							delete [] m_varHeaders;
							m_varHeaders = NULL;
						}
					}

					delete [] m_sessionInfoString;
					m_sessionInfoString = NULL;
				}
			}
		}
		fclose(m_ibtFile);
		m_ibtFile = NULL;
	}

	return false;
}

void irsdkDiskClient::closeFile()
{
	if(m_varBuf)
		delete [] m_varBuf;
	m_varBuf = NULL;

	if(m_varHeaders)
		delete [] m_varHeaders;
	m_varHeaders = NULL;

	if(m_sessionInfoString)
		delete [] m_sessionInfoString;
	m_sessionInfoString = NULL;

	if(m_ibtFile)
		fclose(m_ibtFile);
	m_ibtFile = NULL;
}

bool irsdkDiskClient::getNextData()
{
	if(m_ibtFile)
		return fread(m_varBuf, 1, m_header.bufLen, m_ibtFile) == m_header.bufLen;

	return false;
}

int irsdkDiskClient::getVarIdx(const char *name)
{
	if(m_ibtFile && name)
	{
		for(int idx=0; idx<m_header.numVars; idx++)
		{
			if(0 == strncmp(name, m_varHeaders[idx].name, IRSDK_MAX_STRING))
			{
				return idx;
			}
		}
	}

	return -1;
}

irsdk_VarType irsdkDiskClient::getVarType(int idx)
{
	if(m_ibtFile)
	{
		if(idx >= 0 && idx < m_header.numVars)
		{
			return (irsdk_VarType)m_varHeaders[idx].type;
		}

		//invalid variable index
		assert(false);
	}

	return irsdk_char;
}

int irsdkDiskClient::getVarCount(int idx)
{
	if(m_ibtFile)
	{
		if(idx >= 0 && idx < m_header.numVars)
		{
			return m_varHeaders[idx].count;
		}

		//invalid variable index
		assert(false);
	}

	return 0;
}

bool irsdkDiskClient::getVarBool(int idx, int entry)
{
	if(m_ibtFile)
	{
		if(idx >= 0 && idx < m_header.numVars)
		{
			if(entry >= 0 && entry < m_varHeaders[idx].count)
			{
				const char * data = m_varBuf + m_varHeaders[idx].offset;
				switch(m_varHeaders[idx].type)
				{
				// 1 byte
				case irsdk_char:
				case irsdk_bool:
					return (((const char*)data)[entry]) != 0;
					break;

				// 4 bytes
				case irsdk_int:
				case irsdk_bitField:
					return (((const int*)data)[entry]) != 0;
					break;
					
				// test float/double for greater than 1.0 so that
				// we have a chance of this being usefull
				// technically there is no right conversion...
				case irsdk_float:
					return (((const float*)data)[entry]) >= 1.0f;
					break;

				// 8 bytes
				case irsdk_double:
					return (((const double*)data)[entry]) >= 1.0;
					break;
				}
			}
			else
			{
				// invalid offset
				assert(false);
			}
		}
		else
		{
			//invalid variable index
			assert(false);
		}
	}

	return false;
}

int irsdkDiskClient::getVarInt(int idx, int entry)
{
	if(m_ibtFile)
	{
		if(idx >= 0 && idx < m_header.numVars)
		{
			if(entry >= 0 && entry < m_varHeaders[idx].count)
			{
				const char * data = m_varBuf + m_varHeaders[idx].offset;
				switch(m_varHeaders[idx].type)
				{
				// 1 byte
				case irsdk_char:
				case irsdk_bool:
					return (int)(((const char*)data)[entry]);
					break;

				// 4 bytes
				case irsdk_int:
				case irsdk_bitField:
					return (int)(((const int*)data)[entry]);
					break;
					
				case irsdk_float:
					return (int)(((const float*)data)[entry]);
					break;

				// 8 bytes
				case irsdk_double:
					return (int)(((const double*)data)[entry]);
					break;
				}
			}
			else
			{
				// invalid offset
				assert(false);
			}
		}
		else
		{
			//invalid variable index
			assert(false);
		}
	}

	return 0;
}

float irsdkDiskClient::getVarFloat(int idx, int entry)
{
	if(m_ibtFile)
	{
		if(idx >= 0 && idx < m_header.numVars)
		{
			if(entry >= 0 && entry < m_varHeaders[idx].count)
			{
				const char * data = m_varBuf + m_varHeaders[idx].offset;
				switch(m_varHeaders[idx].type)
				{
				// 1 byte
				case irsdk_char:
				case irsdk_bool:
					return (float)(((const char*)data)[entry]);
					break;

				// 4 bytes
				case irsdk_int:
				case irsdk_bitField:
					return (float)(((const int*)data)[entry]);
					break;
					
				case irsdk_float:
					return (float)(((const float*)data)[entry]);
					break;

				// 8 bytes
				case irsdk_double:
					return (float)(((const double*)data)[entry]);
					break;
				}
			}
			else
			{
				// invalid offset
				assert(false);
			}
		}
		else
		{
			//invalid variable index
			assert(false);
		}
	}

	return 0.0f;
}

double irsdkDiskClient::getVarDouble(int idx, int entry)
{
	if(m_ibtFile)
	{
		if(idx >= 0 && idx < m_header.numVars)
		{
			if(entry >= 0 && entry < m_varHeaders[idx].count)
			{
				const char * data = m_varBuf + m_varHeaders[idx].offset;
				switch(m_varHeaders[idx].type)
				{
				// 1 byte
				case irsdk_char:
				case irsdk_bool:
					return (double)(((const char*)data)[entry]);
					break;

				// 4 bytes
				case irsdk_int:
				case irsdk_bitField:
					return (double)(((const int*)data)[entry]);
					break;
					
				case irsdk_float:
					return (double)(((const float*)data)[entry]);
					break;

				// 8 bytes
				case irsdk_double:
					return (double)(((const double*)data)[entry]);
					break;
				}
			}
			else
			{
				// invalid offset
				assert(false);
			}
		}
		else
		{
			//invalid variable index
			assert(false);
		}
	}

	return 0.0;
}

//path is in the form of "DriverInfo:Drivers:CarIdx:{%d}UserName:"
int irsdkDiskClient::getSessionStrVal(const char *path, char *val, int valLen)
{
	if(m_ibtFile && path && val && valLen > 0)
	{
		const char *tVal = NULL;
		int tValLen = 0;
		if(parseYaml(m_sessionInfoString, path, &tVal, &tValLen))
		{
			// dont overflow out buffer
			int len = tValLen;
			if(len > valLen)
				len = valLen;

			// copy what we can, even if buffer too small
			memcpy(val, tVal, len);
			val[len] = '\0'; // origional string has no null termination...

			// if buffer was big enough, return success
			if(valLen >= tValLen)
				return 1;
			else // return size of buffer needed
				return -tValLen;
		}
	}

	return 0;
}

