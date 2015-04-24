// ConsoleApplication2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ExponentialBackoffRetryer.h"
#include <iostream>
#include <memory>

using namespace std;

void ConnectWithoutReturnValue(LPCTSTR server)
{
	throw "fvdfvdf";
    //return 3;
}

bool ConnectWithRetVal(LPCTSTR server)
{
    throw "asdasda";
	return true;
}

bool Connect(LPCTSTR server)
{
    auto fn = [&]() { return ConnectWithRetVal(server); };

	ExponentialBackoffRetryer retrier(500, 60*1000, 300*1000, 1.2, 0.01);

	auto r = retrier
		.RetryIfAnyException()
		.WaitFor<bool>([](bool result){return !result; }, fn);

	/*retrier
		->RetryIfAnyException()
		->Retry([&](){ConnectWithoutReturnValue(server);});*/

    return false;
}

int _tmain(int argc, _TCHAR* argv[])
{
    Connect(L"test");
	return 0;
}

