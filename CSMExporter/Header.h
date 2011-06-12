#ifndef __HEADER_H__
#define __HEADER_H__

#include "max.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"

#include <stdmat.h>		// 处理material的头文件
#include <iskin.h>		// 处理skin Modifier的头文件
#include <modstack.h> 
#include <decomp.h>		// 处理矩阵
#include <cs/bipexp.h>  // 处理Biped

#include <d3dx10.h>			
#pragma comment( lib, "d3dx10" )		// 为了使用D3DXMATRIX

#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

using std::ofstream;
using std::string;
using std::vector;
using std::map;
using std::endl;

#define __STR2__(x) #x
#define __STR1__(x) __STR2__(x)
#define TODO __FILE__ "("__STR1__(__LINE__)") : Warning TODO: "

#define LOG
#define FILTER
//#define DEBUG

typedef D3DXMATRIX MATRIX;

template< typename T>
inline void SAFE_DELETE( T *pT )
{
	if ( NULL != pT )
	{
		delete pT;
		pT = NULL;
	}
}

static const unsigned MAX_STRING_LENGTH = 64;

/*! \brief 截取字符串 */
inline char * TruncateString( char str[] )
{
	if ( sizeof( str ) / sizeof( char ) >= MAX_STRING_LENGTH )
	{
		str[MAX_STRING_LENGTH] = '\0';
	}
	return str;
}

/*! \brief 去掉路径，取文件名 */
inline string TruncatePath( const string &fileName )
{
	size_t token = fileName.find_last_of( "\\" );
	if ( token != string::npos )
	{
		return fileName.substr( token + 1 );
	}
	return fileName;
}

#endif