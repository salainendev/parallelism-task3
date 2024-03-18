#include <iostream>
#include <boost/program_options.hpp>
#include <thread>
#include <vector>
#include <chrono>
namespace opt = boost::program_options;


// функция для перемножения строк на вектор
void product_row_on_vector(const std::unique_ptr<double[]>& matrix, const std::unique_ptr<double[]>& vector, std::unique_ptr<double[]>& result,const int lb,const int ub,const int N )
{
    for (size_t row = lb; row < ub; row++)
    {
        for (size_t j = 0; j < N; j++)
        {
            result[row] += matrix[row*N+j] * vector[j];
        }
        
    }
    
}


// инициализация 
void initialize_vectors(std::unique_ptr<double[]>& matrix,const std::unique_ptr<double[]>& vector,std::unique_ptr<double[]>& result,const int lb,const int ub,const int N)
{
    for (size_t row = lb; row <= ub ; row++)
    {
        
        vector[row] = (double)row+1;
        result[row] = 0.0;
        for (size_t j = 0; j < N; j++)
        {
            matrix[row*N+j] = (row==j) ? j:1;
        }
    }
    //std::cout<<"дело сделано\n";
}


int main(int argc, char const *argv[])
{
    //парсим аргументы
    opt::options_description desc("опции");
    desc.add_options()
        ("treadCount",opt::value<int>(),"сколько требуется потоков")
        ("cellsCount",opt::value<int>(),"размер матрицы")
        ("help","помощь")
    ;

    opt::variables_map vm;

    opt::store(opt::parse_command_line(argc, argv, desc), vm);

    opt::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    int thrCnt = vm["treadCount"].as<int>();
    int N = vm["cellsCount"].as<int>();
    // и это всё было только ради того чтобы спарсить аргументы.......


    std::vector<std::thread> threads;
    int rows_per_thread =  N / thrCnt;

    // объявление векторов и матрицы
    std::unique_ptr<double[]> matrix(new double[N*N]); // матрица множитель
    std::unique_ptr<double[]> vector(new double[N]); // вектор множитель
    std::unique_ptr<double[]> result(new double[N]); // вектор произведение


    // проводим инициализацию векторов и матриц
    for (size_t threadid = 0; threadid < thrCnt; threadid++)
    {
        int lower_bound = threadid * rows_per_thread;
        int upper_bound = (threadid == thrCnt - 1) ? (N - 1) : (lower_bound + rows_per_thread - 1);

        threads.emplace_back(initialize_vectors,std::ref(matrix),std::ref(vector),std::ref(result),lower_bound,upper_bound,N);

    }

    // ожидание завершения инициализации
    for (auto& thr : threads)
    {
        thr.join();
    }
    
    threads.clear();
    // уничтожаем потоки

    const auto start{std::chrono::steady_clock::now()};
    for (size_t threadid = 0; threadid < thrCnt; threadid++)
    {
        int lower_bound = threadid * rows_per_thread;
        int upper_bound = (threadid == thrCnt - 1) ? (N - 1) : (lower_bound + rows_per_thread - 1);

        threads.emplace_back(product_row_on_vector,std::ref(matrix),std::ref(vector),std::ref(result),lower_bound,upper_bound,N);

    }
    
    for (auto& thr : threads)
    {
        thr.join();
    }

    threads.clear();
    const auto end{std::chrono::steady_clock::now()};
    const std::chrono::duration<double> elapsed_seconds{end - start};
    std::cout << "дело сделано полностбю время выполнения без учёта инициализации - "<<elapsed_seconds.count()<<std::endl;

    return 0;
}


