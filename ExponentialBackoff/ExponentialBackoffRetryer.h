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
        }

        inline ExponentialBackoffRetryer* RetryIfResult(T value)
        {
            _expectedResults.push_back(value);
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
			T result;
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
        std::vector<T> _expectedResults;
		bool _retryIfAnyException;

		inline int CalculateNextDelay(int currentDelay)
		{
			auto delay = min(currentDelay*_multiplier , _maxDelay);

			std::random_device rd;
			std::mt19937 prng(rd());
            std::normal_distribution<double> distribution(delay * _jitter);
			auto randomPart = distribution(prng);

			delay += randomPart;
			//printf("%i\n", (int)delay);
			return (int)delay;
		}

		inline bool ExecuteFunc(std::function<T()> func, T &returnValue)
        {
            try
            {
                returnValue = func();
				
				auto isExpectedResult = std::any_of(_expectedResults.begin(), _expectedResults.end(), [&](T item){return item == returnValue;});

				if(isExpectedResult && _spentTime <= _maxRetryTime)
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