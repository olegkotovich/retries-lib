#include "stdafx.h"
#include <functional>
#include <random>
#include <typeinfo> 
#include <algorithm>

/**
* A ExponentialBackoffRetryer, which executes a call, and retries it using exponential-backoff algorithm until it succeeds, or
* maximum retry time is reached or exceeded. 
*/
class ExponentialBackoffRetryer
{
public:
	/**
	* @param minDelay - the minimum delay to sleep
	* @param maxDelay - the maximum delay to sleep
	* @param maxRetryTime - the maximum time to execute retries. As this time is reached or exceeded
	the last result will be returned or last exception will be thrown
	* @param multiplier - the growth of the wait time between attempts might be controlled by modifying of multiplier.
	* @param jitter - specifies random part of the wait time growth
	*/
	inline ExponentialBackoffRetryer(int minDelay, int maxDelay, int maxRetryTime, double multiplier, double jitter)
	{
		_minDelay = minDelay;
		_maxDelay = maxDelay;
		_maxRetryTime = maxRetryTime;
		_multiplier = multiplier;
		_jitter = jitter;
		Reset();
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
		};

		RetryInternal([&](){return fn();});
		return result;
	}

	inline void Retry(std::function<void()> func)
	{
		RetryInternal([&](){return ExecuteFunc(func);});
	}

	inline void Reset()
	{
		int _spentTime = 0;
		bool _retryIfAnyException = false;
		int _attemptsNumber = 0;
	}

	inline int ElapsedMilliseconds()
	{
		return _elapsedMilliseconds;
	}

	inline int AttemptsCount()
	{
		return _attemptsCount;
	}

private:
	int _minDelay;
	int _maxDelay;
	double _maxRetryTime;
	double _multiplier;
	double _jitter;

	int _elapsedMilliseconds;
	bool _retryIfAnyException;
	int _attemptsCount;

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
		_attemptsCount = 0;
		_elapsedMilliseconds = 0;
		auto delay = _minDelay;

		while (true)
		{
			bool shouldContinue = func();

			_attemptsCount++;

			if (!shouldContinue) break;

			Sleep(delay);
			_elapsedMilliseconds += delay;

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
			if (!_retryIfAnyException || _elapsedMilliseconds >= _maxRetryTime)
			{
				throw;
			}
			return true;
		}
	}
};
