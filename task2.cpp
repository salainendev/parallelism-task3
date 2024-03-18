#include <iostream>
#include <queue>
#include <mutex>
#include <future>
#include <thread>
#include <chrono>
#include <cmath>
#include <functional>
#include <unordered_map>
#include <fstream>
#include <random>
#include <iomanip>

// задача
template<typename T>
T f_pow(T x, T y)
{
    return std::pow(x, y);
}

template<typename T>
T f_sin(T x)
{
    return std::sin(x);
}

template<typename T>
T f_sqrt(T x)
{
    return std::sqrt(x);
}

// пример сервера обрабатывающего задачи из очереди
template<typename T>
class Server{
public:
    Server(){}
    void start(){
        working = true;
        potok = std::thread(&Server::work,this);
    }

    void stop(){
         working = false;
         potok.join();
    }
    size_t add_task(std::packaged_task<T()> task){
                // добавляем задачу в словарь
                std::lock_guard<std::mutex> lock(mtx);
                tasks.insert({task_idx,std::move(task)});
                
                //std::cout<<"help";
                return task_idx++;
    }
    T request_result(size_t idx){
        // берем задачу из словаря решаем её и кидаем резы в results
        
        std::lock_guard<std::mutex> lock(mtx);
        auto it = results.find(idx);
        if (it!= results.end()){
            return it->second;
        }
        else{
            return -1000;
        }

    }

private:
    void work()
    {
        // пока не получили сигнал стоп
        while (working)
        {
            // если очередь не пуста, то достаем задачу и решаем
            if(!tasks.empty())
            {

                //std::cout<<"что то есть";
                auto it = tasks.begin();
                size_t idx = it->first;
                std::packaged_task<T()> task = std::move(it->second);
                tasks.erase(it);
                std::future<double> preres = task.get_future();
                task();
                T res = preres.get();
                results.insert(std::make_pair(idx,res));
                
            }
        }
        std::cout << "Server stop!\n";
    }

    std::mutex mtx;
    std::thread potok;
    bool working = 0;
    // Очередь задач
    size_t task_idx = 0;
    std::unordered_map<size_t,std::packaged_task<T()>> tasks;
    std::unordered_map<size_t,T> results;
};

// Пример потока добавляющего задачи
// Обратите внимание на метод emplace(), для конструирования на месте
std::mutex client_mutex;
void client(Server<double>& server,int N,int func_type,const std::string& filename)
{
    // для рандомной генерации аргументов
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(1, 10);
    std::uniform_real_distribution<double> dist_real(0.0, 10.0);
    
    // открываем файл

    std::ofstream file(filename);
        if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл" << filename << std::endl;
        return;
    }


    for (size_t i = 0; i < N; i++)
    {
        if (func_type==0)
        {
        
            int arg1 = dist(gen);
            int arg2 = dist(gen);
            std::packaged_task<double()> task(std::bind(f_pow<int>,arg1,arg2));

            client_mutex.lock();
            size_t idx = server.add_task(std::move(task));
            client_mutex.unlock();

            while (server.request_result(idx)==-1000){
                //если результат не обработан сервером то спим на 50 мс
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }

            file << "pow "<<arg1<<" "<<arg2<<" = "<< std::fixed <<server.request_result(idx)<<std::endl;
            continue;     
        }
        if (func_type==1)
        {
            int arg1 = dist_real(gen);
            std::packaged_task<double()> task(std::bind(f_sin<double>,arg1));
            client_mutex.lock();
            size_t idx = server.add_task(std::move(task));
            client_mutex.unlock();
            while (server.request_result(idx)==-1000){
                //если результат не обработан сервером то спим на 50 мс
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
            file << "sin "<<arg1<<" = "<< std::fixed << server.request_result(idx)<<std::endl;
            continue;   
        }
        if (func_type==2)
        {
            int arg1 = dist_real(gen);
            std::packaged_task<double()> task(std::bind(f_sqrt<double>,arg1));
            client_mutex.lock();
            size_t idx = server.add_task(std::move(task));
            client_mutex.unlock();
            while (server.request_result(idx)==-1000){
                //если результат не обработан сервером то спим на 50 мс
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
            file << "sqrt "<<arg1<<" = "<<  std::fixed << server.request_result(idx)<<std::endl;
            continue;   
        }
    }
    file.close();
    //формируем задачу
    

    // отправляем задачу на сервер, получаем и записываем обратно айди, ждём пока результат по нашему айди придёт, повторяем N раз

}

int main()
{
    std::cout << "Start\n";
    Server<double> servak;
    servak.start();
    std::thread client_pow(client,std::ref(servak),10,0,"pow.txt");
    std::thread client_sin(client,std::ref(servak),10,1,"sin.txt");
    std::thread client_sqrt(client,std::ref(servak),10,2,"sqrt.txt");

    client_pow.join();
    client_sin.join();
    client_sqrt.join();


    servak.stop();

    std::cout << "End\n";
}