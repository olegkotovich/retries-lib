#include "stdafx.h"
#include <functional>
#include <random>
#include <typeinfo> 
#include <algorithm>

template <typename T>
class RetryHelper
{
    public:
        inline RetryHelper(int minDelay, int maxDelay, int maxRetryTime, double factor, double jitter)
        {
            this->minDelay = minDelay;
            this->maxDelay = maxDelay;
            this->maxRetryTime = maxRetryTime;
            this->factor = factor;
            this->jitter = jitter;
        }

        inline RetryHelper * RetryIfResult(T value)
        {
            expectedResults.push_back(value);
            return this;
        }

        inline RetryHelper* RetryIfAnyException()
        {
			this->retryIfAnyException = true;
            return this;
        }

        inline void Retry(std::function<T()> func)
        {
            auto delay = minDelay;
            int spentTime = 0;
            while (true)
            {
                bool shouldContinue = ExecuteFunc(func);

				if (!shouldContinue) break;

                Sleep(delay);
                spentTime += delay;

                delay = min(delay*factor, maxDelay);
                std::default_random_engine generator;
                std::normal_distribution<double> distribution(delay*jitter);
                delay += distribution(generator);

                printf("%i\n", delay);
            }
        }

        inline bool ExecuteFunc(std::function<T()> func)
        {
            try
            {
                auto result = func();
				if (std::any_of(expectedResults.begin(), expectedResults.end(), [&](T item){return item == result; }))
				{
					return true;
				}
				return false;
            }
            catch (...)
            {
				if (!retryIfAnyException)
				{
					throw;
				}
            }
           return true;
        }
        
    private:
        int minDelay;
        int maxDelay;
        int maxRetryTime;
        double factor;
        double jitter;

        std::vector<T> expectedResults;
		bool retryIfAnyException;
};