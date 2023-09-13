#include <chrono>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <vector>

std::vector<std::string>* args;

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<double> rng(0, UINT32_MAX);

const uint64_t MEMDATASIZE = UINT32_MAX / 64;
std::uniform_int_distribution<uint64_t> rngcache(0, MEMDATASIZE);

const uint32_t hardware_concurrency = std::thread::hardware_concurrency();

const uint64_t CountCycles = 768 * 1024;
const uint64_t CountSubCycles = 16;

double adv(double* value)
{
    double sret = rng(gen) + 1.0 * (1.0 / (rng(gen) + 2.0));
    for (uint64_t i = 1; i < CountSubCycles; i++)
    {
        sret += (rng(gen) + 1.0) * (1.0 / (rng(gen) + 2.0)) +
                (*value + i - 2.0 * 1.025283 + 6.237237) / (3.973773 * 1.322265 + i - 0.333333 / 3.627435) /
                    std::sqrt(rng(gen) + 1.0) +
                std::pow(2, rng(gen) + 1.0 * (1.0 / (rng(gen) + 2.0))) * std::cbrt(rng(gen) + 0.0000001) /
                    std::log2(rng(gen));
        value++;
    }
    return sret;
}

//std::vector<double> memdata;
double memdata[MEMDATASIZE];

double advCache(double* value)
{
    double sret = rng(gen) + 1.0 * (1.0 / (rng(gen) + 2.0));
    for (uint64_t i = 1; i < CountSubCycles; i++)
    {
        sret += (rng(gen) + 1.0) * (1.0 / (rng(gen) + 2.0)) +
                (*value + i - 2.0 * 1.025283 + 6.237237) / (3.973773 * 1.322265 + i - 0.333333 / 3.627435) /
                    std::sqrt(rng(gen) + 1.0) +
                std::pow(2, rng(gen) + 1.0 * (1.0 / (rng(gen) + 2.0))) * std::cbrt(rng(gen) + 0.0000001) /
                    std::log2(rng(gen));
        value++;
        memdata[rngcache(gen)] = sret;
    }
    return sret;
}

bool IsStarting = false;

void test(double value)
{
    while (!IsStarting)
    {
        std::this_thread::yield();
    }

    double sret = rng(gen);
    for (uint64_t i = 0; i < CountCycles; i++)
    {
        sret += adv(&value);
    }

#ifdef _DEBUG
    std::cout << " " << std::fixed << sret;
#endif
}

void testCache(double value)
{
    while (!IsStarting)
    {
        std::this_thread::yield();
    }

    double sret = rng(gen);

    for (uint64_t i = 0; i < CountCycles; i++)
    {
        double* cac1 = &memdata[rngcache(gen)];
        sret += advCache(cac1);
        *cac1 = sret;
        sret += *cac1;
    }

#ifdef _DEBUG
    std::cout << " " << std::fixed << sret;
#endif
}

std::chrono::high_resolution_clock::time_point now;
std::chrono::duration<double> delta;

void perfTimeTestSingle()
{
    std::cout << "Start Test SingleThread!" << std::endl;

#ifdef _DEBUG
    std::cout << "Result";
#endif

    double value = rng(gen) + 1.0 * (1.0 / (rng(gen) + 2.0));

    std::thread thread(test, value);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Test Performance
    now = std::chrono::high_resolution_clock::now();
    IsStarting = true;

    thread.join();

    delta = std::chrono::high_resolution_clock::now() - now;
    IsStarting = false;

#ifdef _DEBUG
    std::cout << std::endl;
#endif
    std::cout << "SingleThread " << std::fixed << delta.count() << std::endl;
}

void perfTimeTestMulti()
{
    std::cout << "Start Test MultiThread!" << std::endl;

    std::vector<std::thread> threads;

#ifdef _DEBUG
    std::cout << "Result";
#endif

    double value = (rng(gen) + 1.0) * (1.0 / (rng(gen) + 2.0));
    for (uint64_t i = 0; i < hardware_concurrency; i++)
    {
        threads.emplace_back(std::thread(test, value));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Test Performance
    now = std::chrono::high_resolution_clock::now();
    IsStarting = true;

    for (std::thread& th : threads)
    {
        th.join();
    }

    IsStarting = false;
    delta = std::chrono::high_resolution_clock::now() - now;

#ifdef _DEBUG
    std::cout << std::endl;
#endif
    std::cout << "MultiThread " << std::fixed << delta.count() << std::endl;
}

void perfTimeTestSingleCache()
{
    std::cout << "Start Test SingleThread Cache!" << std::endl;

#ifdef _DEBUG
    std::cout << "Result";
#endif

    double value = (rng(gen) + 1.0) * (1.0 / (rng(gen) + 2.0));

    std::thread thread(testCache, value);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Test Performance
    now = std::chrono::high_resolution_clock::now();
    IsStarting = true;

    thread.join();

    IsStarting = false;
    delta = std::chrono::high_resolution_clock::now() - now;

#ifdef _DEBUG
    std::cout << std::endl;
#endif
    std::cout << "SingleThread Cache " << std::fixed << delta.count() << std::endl;
}

void perfTimeTestMultiCache()
{
    std::cout << "Start Test MultiThread Cache!" << std::endl;

    std::vector<std::thread> threads;

#ifdef _DEBUG
    std::cout << "Result";
#endif

    double value = (rng(gen) + 1.0) * (1.0 / (rng(gen) + 2.0));

    for (uint64_t i = 0; i < hardware_concurrency; i++)
    {
        threads.emplace_back(std::thread(testCache, value));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Test Performance
    now = std::chrono::high_resolution_clock::now();
    IsStarting = true;

    for (std::thread& th : threads)
    {
        th.join();
    }

    IsStarting = false;
    delta = std::chrono::high_resolution_clock::now() - now;

#ifdef _DEBUG
    std::cout << std::endl;
#endif
    std::cout << "MultiThread Cache " << std::fixed << delta.count() << std::endl;
}

class TestThread
{
    std::thread* thread;

public:
    bool IsRunning;
    bool IsWork = false;

    TestThread()
    {
        IsRunning = true;
        thread = new std::thread(prefTestMultiThread, this);
    }

    ~TestThread()
    {
        IsRunning = false;
        thread->join();
        delete thread;
    }

    static void prefTestMultiThread(TestThread* thiz)
    {
        while (thiz->IsRunning)
        {
            double value = (rng(gen) + 1.0) * (1.0 / (rng(gen) + 2.0));

            if (thiz->IsWork)
            {
                test(value);
                thiz->IsWork = false;
            }
        }
    }
};

void perfTestSingle()
{
    std::cout << "Start Test SingleThread Score!" << std::endl;

#ifdef _DEBUG
    std::cout << "Result";
#endif

    TestThread tes;

    now = std::chrono::high_resolution_clock::now();
    IsStarting = true;

    // Variable on the first pass
    bool started = false;

    // Variable on last pass and exit
    bool ended = false;

    // Count of test runs
    uint64_t count = 0;
    while (true)
    {
        if (std::chrono::high_resolution_clock::now() >= now + std::chrono::minutes(1))
        {
            ended = true;
        }

        if (!tes.IsWork)
        {
            if (!ended)
                tes.IsWork = true;
            if (started)
                count++;
        }
        started = true;

        if (ended)
            break;
    }
    IsStarting = false;

#ifdef _DEBUG
    std::cout << std::endl;
#endif
    std::cout << "SingleThread Score " << count << std::endl;
}

void perfTestMulti()
{
    std::cout << "Start Test MultiThread Score!" << std::endl;

    std::vector<TestThread*> threads;

#ifdef _DEBUG
    std::cout << "Result";
#endif

    for (uint64_t i = 0; i < hardware_concurrency; i++)
    {
        TestThread* t = new TestThread;
        threads.push_back(t);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    now = std::chrono::high_resolution_clock::now();
    IsStarting = true;

    // Variable on the first pass
    bool started = false;

    // Variable on last pass and exit
    bool ended = false;

    // Count of test runs
    uint64_t count = 0;
    while (true)
    {
        if (std::chrono::high_resolution_clock::now() >= now + std::chrono::minutes(1))
        {
            ended = true;
        }

        for (TestThread* tes : threads)
        {
            if (!tes->IsWork)
            {
                if (!ended)
                    tes->IsWork = true;
                if (started)
                    count++;
            }
        }
        started = true;

        if (ended)
            break;
    }

    IsStarting = false;

    for (TestThread* tes : threads)
    {
        delete tes;
    }

#ifdef _DEBUG
    std::cout << std::endl;
#endif
    std::cout << "MultiThread Score " << count << std::endl;
}

void DisplayHelp()
{
    std::cout << "Usage: PerfCPPTest [OPTION]" << std::endl;
    std::cout << "  -h, --help\tdisplay this help and exit" << std::endl;
    std::cout << "  -s        \ttest singlethread time" << std::endl;
    std::cout << "  -m        \ttest multithread time" << std::endl;
    std::cout << "  -c        \ttest singlethread cache time" << std::endl;
    std::cout << "  -v        \ttest multithread cache time" << std::endl;
    std::cout << "  -o        \ttest singlethread score" << std::endl;
    std::cout << "  -p        \ttest multithread score" << std::endl;
}

int main(int argc, char* argv[])
{
    const uint8_t max_precision = 7;
    std::cout.precision(max_precision);

    args = new std::vector<std::string>(argv, argv + argc);
    std::string argstostr;
    for (uint64_t i = 0; i < argc; i++)
    {
        argstostr = argstostr + "argv[" + std::to_string(i) + "]: " + argv[i] + " ";
    }

    const std::string hellostr = "Welcome to PerfCPPTest argc=" + std::to_string(argc) + " argv=" + argstostr;

    std::cout << hellostr << std::endl;

    // fill mem data
    for (uint64_t i = 0; i < MEMDATASIZE; i++)
    {
        //memdata.push_back(rng(gen));
        memdata[rngcache(gen)];
    }

    bool IsArgumented = false;
    for (std::string arg : *args)
    {
        if (arg.find("-h") != std::string::npos || arg.find("--help") != std::string::npos)
        {
            DisplayHelp();
            IsArgumented = true;
        }
        else if (arg.find("-s") != std::string::npos)
        {
            perfTimeTestSingle();
            IsArgumented = true;
        }
        else if (arg.find("-m") != std::string::npos)
        {
            perfTimeTestMulti();
            IsArgumented = true;
        }
        else if (arg.find("-c") != std::string::npos)
        {
            perfTimeTestSingleCache();
            IsArgumented = true;
        }
        else if (arg.find("-v") != std::string::npos)
        {
            perfTimeTestMultiCache();
            IsArgumented = true;
        }
        else if (arg.find("-o") != std::string::npos)
        {
            perfTestSingle();
            IsArgumented = true;
        }
        else if (arg.find("-p") != std::string::npos)
        {
            perfTestMulti();
            IsArgumented = true;
        }
        else if (arg == args->at(0))
        {
            continue;
        }
        else
        {
            std::cout << "Unknown argument: " << arg << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    if (IsArgumented)
    {
        return 0;
    }

    // Test
    perfTimeTestSingle();

    std::this_thread::sleep_for(std::chrono::seconds(1));

    perfTimeTestMulti();

    std::this_thread::sleep_for(std::chrono::seconds(1));

    perfTimeTestSingleCache();

    std::this_thread::sleep_for(std::chrono::seconds(1));

    perfTimeTestMultiCache();

    std::this_thread::sleep_for(std::chrono::seconds(1));

    perfTestSingle();

    std::this_thread::sleep_for(std::chrono::seconds(1));

    perfTestMulti();
}