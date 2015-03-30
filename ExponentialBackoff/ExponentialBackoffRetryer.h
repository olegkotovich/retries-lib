#include "stdafx.h"
#include <functional>
#include <random>
#include <typeinfo> 
#include <algorithm>

template <typename T>
class ExponentialBackoffRetryer
{
    public:
        inline ExponentialBackoffRetryer(int minDelay, int maxDelay, int maxRetryTime, double multiplier , double jitter)
        {
            _minDelay = minDelay;
            _maxDelay = maxDelay;
            _maxRetryTime = maxRetryTime;
            _multiplier  = multiplier;
            _jitter = jitter;
			_spentTime = 0;
			_retryIfAnyException = false;
			_checkResultFunc = nullptr;
        }

        inline ExponentialBackoffRetryer* RetryIfResult(std::function<bool(T result)> func)
        {
            _checkResultFunc = func;
            return this;
        }

        inline ExponentialBackoffRetryer* RetryIfAnyException()
        {
			this->_retryIfAnyException = true;
            return this;
        }

        inline T Retry(std::function<T()> func)
        {
            auto delay = _minDelay;
			T result = T();
            while (true)
            {
                bool shouldContinue = ExecuteFunc(func, result);

				if (!shouldContinue) break;

                Sleep(delay);
                _spentTime += delay;

				delay = CalculateNextDelay(delay);
            }
			return result;
        }
        
    private:
        int _minDelay;
        int _maxDelay;
        double _maxRetryTime;
        double _multiplier ;
        double _jitter;

		int _spentTime;
        std::function<bool(T result)> _checkResultFunc;
		bool _retryIfAnyException;

		inline int CalculateNextDelay(int currentDelay)
		{
			auto delay = min(currentDelay*_multiplier , _maxDelay);

			std::random_device rd;
			std::mt19937 prng(rd());
            std::normal_distribution<double> distribution(delay * _jitter);
			auto randomPart = distribution(prng);

			delay += randomPart;
			printf("%i\n", (int)delay);
			return (int)delay;
		}

		inline bool ExecuteFunc(std::function<T()> func, T &returnValue)
        {
            try
            {
                returnValue = func();
				
				auto shouldRetry = _checkResultFunc(returnValue);

				if(shouldRetry && _spentTime <= _maxRetryTime)
				{
					return true;
				}

				if(_spentTime >= _maxRetryTime) 
				{
					return false;
				}

				return false;
            }
            catch (...)
            {
				if (!_retryIfAnyException || _spentTime >= _maxRetryTime)
				{
					throw;
				}
            }
           return true;
        }
};