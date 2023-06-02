
#include <iostream>
#include <ctime>
#include <signal.h>
#include <atomic>
#include <mutex>
#include <thread>
#include <vector>
#include <chrono>

const int MAX_NUMBER = 1000000;

std::atomic<bool> isInterrupted(false);
std::mutex mtx;
int totalPrimes = 0;
std::vector<int> numbers;

bool isPrime(const int number)
{
    if (number <= 1)
        return false;

    for (int i = 2; i <= sqrt(number); ++i)
        if (number % i == 0)
            return false;

    return true;
}

void generateEvents(const int threadId, const int nEvents)
{
    srand(time(NULL));

    for (int i = 0; i < nEvents && !isInterrupted; ++i)
    {
        int randomNumber = rand() % MAX_NUMBER + 1;

        {
            std::lock_guard<std::mutex> guard(mtx);
            numbers.push_back(randomNumber);

            std::time_t timeNow = time(nullptr);
            char str[26];
            ctime_s(str, sizeof(str), &timeNow);

            std::cout << str <<  " Поток " << threadId << " Событие сгенерировано, число: " << randomNumber << std::endl;
        }
    }
}

void processEvents(const int threadId, const int nEvents)
{
    for (int i = 0; i < nEvents && !isInterrupted; ++i)
    {
        int number{};

        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Симулируем время выполнения.
        
        {
            std::lock_guard<std::mutex> guard(mtx);

            std::cout << "Поток " << threadId << " - Событие обработано: " << i << std::endl;

            if (!numbers.empty())
            {
                number = numbers.back();
                numbers.pop_back();
            }

            if (isPrime(number))
                ++totalPrimes;
        }
    }
}

void signalHandler(const int sig)
{
    std::cout << "Сигнал SIGINT получен, завершаем работу приложения..." << std::endl;
    isInterrupted = true;
}

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "RU");
    signal(SIGINT, signalHandler);

    if (argc < 4)
        std::cerr << "Ошибка: необходимо передать больше аргументов" << std::endl;

    int nGenerators;
    int nEventsPerGenerator;
    int nProcessors;

    nGenerators = atoi(argv[1]);
    nEventsPerGenerator = atoi(argv[2]);
    nProcessors = atoi(argv[3]);

    std::vector<std::thread> generators;
    std::vector<std::thread> processors;

    for (int i = 0; i < nGenerators; ++i)
        generators.emplace_back(generateEvents, i, nEventsPerGenerator);

    for (int i = 0; i < nProcessors; ++i)
        processors.emplace_back(processEvents, i, nEventsPerGenerator * nGenerators);

    for (auto& refThread : generators)
        refThread.join();

    for (auto& refThread : processors)
        refThread.join();

    std::cout << "Общее количество простых чисел: " << totalPrimes << std::endl;

    return 0;
}


