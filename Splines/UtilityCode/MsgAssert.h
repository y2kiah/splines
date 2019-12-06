//	----==== MSGASSERT.H ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		1
//	Date:			3/04
//	Description:	A macro to make the standard assert macro more meaningful
//----------------------------------------------------------------------------------------


#include <cassert>

#define msgAssert(a,b)	assert(a && b)