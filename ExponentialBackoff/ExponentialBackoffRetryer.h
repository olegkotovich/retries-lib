#include "stdafx.h"
#include <functional>
#include <random>
#include <typeinfo> 
#include <algorithm>

class ExponentialBackoffRetryer
{
public:
	inline ExponentialBackoffRetryer(int minDelay, int maxDelay, int maxRetryTime, double multiplier, double jitter)
	{
		_minDelay = minDelay;
		_maxDelay = maxDelay;
		_maxRetryTime = maxRetryTime;
		_multiplier = multiplier;
		_jitter = jitter;
		_spentTime = 0;
		_retryIfAnyException = false;
	}

	inline ExponentialBackoffRetryer* RetryIfAnyException()
	{
		this->_retryIfAnyException = true;
		return this;
	}

	template <typename T> 
	inline T WaitFor(std::function<bool(T result)> resultChecker, std::function<T()> func)
	{
		T result;

		auto fn = [&]()
		{
			bool shouldContinue = ExecuteFunc(func, resultChecker, result);
			return shouldContinue;
		}

		RetryInternal([&](){return fn();});
		return result;
	}

	inline void Retry(std::function<void()> func)
	{
		RetryInternal([&](){return ExecuteFunc(func);});
	}

private:
	int _minDelay;
	int _maxDelay;
	double _maxRetryTime;
	double _multiplier;
	double _jitter;

	int _spentTime;
	bool _retryIfAnyException;

	inline int CalculateNextDelay(int currentDelay)
	{
		auto delay = min(currentDelay * _multiplier, _maxDelay);

		std::random_device rd;
		std::mt19937 prng(rd());
		std::normal_distribution<double> distribution(delay * _jitter);
		auto randomPart = distribution(prng);

		delay += randomPart;
		printf("%i\n", (int)delay);
		return (int)delay;
	}

	inline void RetryInternal(std::function<bool()> func)
	{
		auto delay = _minDelay;

		while (true)
		{
			bool shouldContinue = func();

			if (!shouldContinue) break;

			Sleep(delay);
			_spentTime += delay;

			delay = CalculateNextDelay(delay);
		}
	}

	//Return bool that determines whether should we continue waiting.
	template <typename T> 
	inline bool ExecuteFunc(std::function<T()> func, std::function<bool(T result)> resultChecker, T &result)
	{
		auto fn = [&]() 
		{
			result = func();
			bool isNeededResult = resultChecker(result);
			return isNeededResult ? false : true;
		};

		return HandleExceptions(fn);
	}

	//Return bool that determines whether should we continue waiting.
	inline bool ExecuteFunc(std::function<void()> func)
	{
		auto fn = [&]() 
		{
			func();
			return false;
		};
		return HandleExceptions(fn);
	}

	inline bool HandleExceptions(std::function<bool()> func)
	{
		try
		{
			return func();
		}
		catch (...)
		{
			if (!_retryIfAnyException || _spentTime >= _maxRetryTime)
			{
				throw;
			}
			return true;
		}
	}
};
