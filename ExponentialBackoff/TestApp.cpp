// ConsoleApplication2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "RetryHelper.h"
#include <iostream>

using namespace std;

bool ConnectInternal(LPCTSTR server)
{
    return false;
}

bool Connect(LPCTSTR server)
{
    auto fn = [&]() { return ConnectInternal(server); };

    RetryHelper<bool>helper(1000, 100000, 180*1000, 1.5, 0.1);

    helper.RetryIfResult(true)
        ->RetryIfExceptionOfType<MyException>()
        ->RetryIfExceptionOfType<std::bad_alloc>()
        ->Retry(fn);

    return false;
}

int _tmain(int argc, _TCHAR* argv[])
{
    Connect(L"test");
	return 0;
}

