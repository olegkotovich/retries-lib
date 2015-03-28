#include "stdafx.h"
#include <functional>
#include <random>
#include <typeinfo> 

class MyException : public std::exception
{

};

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

        template <class E>
        inline RetryHelper * RetryIfExceptionOfType()
        {
            static_assert(std::is_base_of<std::exception, E>::value, "type parameter of this function must derive from std::exception");

            const std::type_info* exceptionTypeInfo = &typeid(E);

            expectedExceptions.push_back(exceptionTypeInfo);
            return this;
        }

        inline void Retry(std::function<T()> func)
        {
            auto delay = minDelay;
            int spentTime = 0;
            while (true)
            {
                bool isSuccess = ExecuteFunc(func);
                if (isSuccess) break;

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
                func();
            }
            catch (...)
            {
            	auto currentException = std::current_exception();
                const std::type_info& exceptionTypeInfo = typeid(currentException);
                for (auto &savedExceptionTypeInfo : expectedExceptions)
                {
                    if (savedExceptionTypeInfo)
                    {
                    }
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
        std::vector<const std::type_info *> expectedExceptions;
};