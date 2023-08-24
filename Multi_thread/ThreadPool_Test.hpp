/*

	쓰레드 풀

	오브젝트 풀과 유사하다. 여러 개의 쓰레드들이 대기하고 있다가 태스크가 들어오게 된다면, 대기하던 쓰레드중 하나가 이를 받아서 실행한다.

    // 렌더링과 충돌처리에 사용 가능 -> 코어를 최대 개수로 활용

    https://modoocode.com/285

*/

#include "Defines.h"

namespace ThreadPool 
{
    class ThreadPool 
    {
        public:
            ThreadPool(size_t num_threads);
            ~ThreadPool();

            // job 을 추가한다.
            // EnqueJob()함수가 임의의 형태의 함수를 받고 그 함수의 리턴값을 보관하는 feture를 리턴하도록 한다. feture의 리턴 값이 <>안에 들어간다.
            template <class F, class... Args> // 가변 길이 템플릿
            future<typename result_of<F(Args...)>::type> EnqueueJob(F&& f, Args&&... args); // 함수 F의 리턴값을 알기위해 result_of를 사용한다.
            // typename std::result_of<F(Args...)>::type  ==  f 의 리턴값

            // 위의 정의는 아래와 같다.
            // template <class F, class... Args>
            // std::future</* f 의 리턴 타입*/> EnqueueJob(F f, Args... args);

        private:
            // Worker 쓰레드
            void WorkerThread();

        private:
            // 총 Worker 쓰레드의 개수.
            size_t num_threads_;

            // Worker 쓰레드를 보관하는 벡터.
            vector<thread> worker_threads_;

            // 할일들을 보관하는 job 큐. -> 커맨드 리스트 (실행해야할 함수를 보관한다, function는 함수를 객체로 보관하기 위해 사용)
            // 모든 작업 쓰레드들에서 접근이 가능하다. 따라서 아래 장치로 보호
            queue<function<void()>> jobs_;

            // 위의 job 큐를 위한 cv 와 m.
            condition_variable cv_job_q_; // (생산자 역할은 쓰레드풀을 사용하는 사람들 = 작업을 추가하는 사람들 / 소비자들은 쓰레드)
            mutex m_job_q_;

            // 모든 쓰레드를 종료하기 위함
            // 워커 쓰레드들은 기본적으로 jobs를 처리하는 동안 무한루프를 돌고 있는데 stop_All이 설정된다면 무한루프를 빠져나가게 된다.
            bool stop_all;
    };

    ThreadPool::ThreadPool(size_t num_threads)
        : num_threads_(num_threads), stop_all(false) 
    {
        // 스레드를 생성 및 시작한다.

        worker_threads_.reserve(num_threads_);

        for (size_t i = 0; i < num_threads_; ++i) 
        {
            worker_threads_.emplace_back([this]() { this->WorkerThread(); });
        }
    }

    void ThreadPool::WorkerThread() 
    {
        // jobs에 작업이 추가될 때까지 대기하고 있다가 작업이 추가되면 받아서 처리한다.
        while (true) 
        {
            unique_lock<mutex> lock(m_job_q_);

            // 작업이 없거나 스탑올이 설정될 경우 대기상태로 들어간다.
            cv_job_q_.wait(lock, [this]() { return !this->jobs_.empty() || stop_all; });
            
            // 스레드 종료 조건 (unique_lock은 스택 변수로서 소멸자가 호출되면 스레드가 종료된다)
            if (stop_all && this->jobs_.empty())  
                return;
            
            // 맨 앞의 job 을 뺀다.
            function<void()> job = move(jobs_.front()); // 복사생성자를 호출할 수 없기 때문에 move 사용
            jobs_.pop();
            lock.unlock();

            // 해당 job 을 수행한다 :)
            job();
        }
    }


    template <class F, class... Args>
    future<typename result_of<F(Args...)>::type> ThreadPool::EnqueueJob(F&& f, Args&&... args) 
    {
        if (stop_all) 
        {
            throw runtime_error("ThreadPool 사용 중지됨");
        }

        using return_type = typename result_of<F(Args...)>::type; // 편의상 따로 정의
        auto job = make_shared<packaged_task<return_type()>>(bind(forward<F>(f), forward<Args>(args)...)); // forward : 레퍼런스 전달을 위해 사용
        // 비동기적으로 실행되는 함수의 리턴값을 받아내기 위해 packaged_task 사용
        // packaged_task의 생성자는 함수 만을 받기 때문에 인자들을 bind한다.

        future<return_type> job_result_future = job->get_future(); // 잡의 실행결과를 보관한다.
        {
            lock_guard<mutex> lock(m_job_q_); // 뮤텍스 락 
            jobs_.push([job]() { (*job)(); }); // jobs에 job를 추가한다.
        }

        cv_job_q_.notify_one();

        // 이후 잡이 실행되면 잡(함수 f)의 리턴값이 job_result_future에 들어가게 되고 이는 풀 사용자가 접근할 수 있게된다.

        return job_result_future; 
    }

    ThreadPool::~ThreadPool() 
    {
        // 스탑을 설정한 뒤 모든 쓰레드를 깨운다.
        stop_all = true;
        cv_job_q_.notify_all();

        for (auto& t : worker_threads_) 
            t.join();
    }

}  // namespace ThreadPool





int work(int t, int id) 
{
    /*printf("%d start \n", id);
    this_thread::sleep_for(chrono::seconds(t));
    printf("%d end after %ds\n", id, t);*/

    printf("%d start \n", id);
    this_thread::sleep_for(chrono::seconds(t));
    printf("%d end\n", id);

    return t + id;
}

void ThreadPool_Test() 
{
    ThreadPool::ThreadPool pool(3); // 3개의 쓰레드 생성

    vector<future<int>> futures; // 쓰레드에 부여하는 태스크 함수가 반환값이 존재하는 경우를 대비해 반환값을 받을 수 있도록 구현한다.

    for (int i = 0; i < 10; i++)  // 3개에 쓰레드에 대해 총 10개의 커맨드(실행을 원하는 함수) 부여
    {
        futures.emplace_back(pool.EnqueueJob(work, i % 3 + 1, i));
    }

    for (auto& f : futures) 
    {
        printf("result : %d \n", f.get());
    }
}


//ThreadPool_Test