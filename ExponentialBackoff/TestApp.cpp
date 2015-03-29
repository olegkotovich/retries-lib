// ConsoleApplication2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "RetryHelper.h"
#include <iostream>

using namespace std;

int ConnectInternal(LPCTSTR server)
{
	//throw "fvdfvdf";
    return 3;
}

bool Connect(LPCTSTR server)
{
    auto fn = [&]() { return ConnectInternal(server); };

    RetryHelper<int>helper(1000, 100000, 180*1000, 1.5, 0.1);

    helper.RetryIfResult(1)
        ->RetryIfAnyException()
		->RetryIfResult(3)
        ->Retry(fn);

    return false;
}

int _tmain(int argc, _TCHAR* argv[])
{
    Connect(L"test");
	return 0;
}

