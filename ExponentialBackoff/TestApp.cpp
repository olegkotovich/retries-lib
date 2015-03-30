// ConsoleApplication2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ExponentialBackoffRetryer.h"
#include <iostream>
#include <memory>

using namespace std;

int ConnectInternal(LPCTSTR server)
{
	//throw "fvdfvdf";
    return 3;
}

bool Connect(LPCTSTR server)
{
    auto fn = [&]() { return ConnectInternal(server); };

	std::unique_ptr<ExponentialBackoffRetryer<int>> retrier(new ExponentialBackoffRetryer<int>(500, 60*1000, 300*1000, 1.2, 0.01));

    int result = retrier
           ->RetryIfAnyException()
		   ->RetryIfResult([](int result)->bool{return result == 3;})
           ->Retry(fn);

    return false;
}

int _tmain(int argc, _TCHAR* argv[])
{
    Connect(L"test");
	return 0;
}

